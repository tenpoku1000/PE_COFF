
// Copyright (C) 2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_SECTION_NUM 8

#define TP_HEAP_BASE 65536
#define TP_DATA_BASE 1024

#define TP_WASM_DATA_ALIGN_8(size) (uint32_t)((size) + (rsize_t)(-((int64_t)(size)) & (8 - 1)))
#define TP_WASM_DATA_ALIGN_16(size) (uint32_t)((size) + (rsize_t)(-((int64_t)(size)) & (16 - 1)))

static bool wasm_append_linear_memory(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind,
    uint8_t* data, rsize_t size, TP_SECTION_ALIGN section_align, int32_t* offset
);
static bool wasm_calc_coff_offset(
    TP_SYMBOL_TABLE* symbol_table,
    TP_SECTION_KIND section_kind, int32_t wasm_offset, int32_t* coff_offset
);

bool tp_make_wasm_coff(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section, bool is_origin_wasm, bool is_64)
{
    TP_WASM_MODULE_SECTION** tmp_section =
        (TP_WASM_MODULE_SECTION**)TP_CALLOC(symbol_table, TP_SECTION_NUM, sizeof(TP_WASM_MODULE_SECTION*));

    if (NULL == tmp_section){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    symbol_table->member_wasm_data_offset_1 = 0;
    symbol_table->member_wasm_data_size_1 = 0;
    TP_FREE(
        symbol_table,
        &(symbol_table->member_wasm_data_data_1), symbol_table->member_wasm_data_size_1
    );

    symbol_table->member_wasm_data_offset_2 = 0;
    symbol_table->member_wasm_data_size_2 = 0;
    TP_FREE(
        symbol_table,
        &(symbol_table->member_wasm_data_data_2), symbol_table->member_wasm_data_size_2
    );

    if (is_origin_wasm){

        if (is_64){

            tmp_section[6] = tp_make_section_code_origin_wasm_UEFI64(symbol_table);
        }else{

            tmp_section[6] = tp_make_section_code_origin_wasm_UEFI32(symbol_table);
        }

        uint32_t data_end = symbol_table->member_wasm_data_data_2
            ? (symbol_table->member_wasm_data_offset_2 + symbol_table->member_wasm_data_size_2)
            : (symbol_table->member_wasm_data_offset_1 + symbol_table->member_wasm_data_size_1);

        uint32_t heap_base = data_end + TP_HEAP_BASE;
        uint32_t heap_current = heap_base;

        tmp_section[0] = tp_make_section_type_UEFI(symbol_table, is_64);
        tmp_section[1] = tp_make_section_function_UEFI(symbol_table);
        tmp_section[2] = tp_make_section_table_UEFI(symbol_table);
        tmp_section[3] = tp_make_section_memory_UEFI(symbol_table);
        tmp_section[4] = tp_make_section_global_UEFI(symbol_table, heap_current, heap_base, data_end);
        tmp_section[5] = tp_make_section_export_UEFI(symbol_table);

        tmp_section[7] = tp_make_section_data_UEFI(
            symbol_table,
            symbol_table->member_wasm_data_offset_1,
            symbol_table->member_wasm_data_size_1,
            symbol_table->member_wasm_data_data_1,
            symbol_table->member_wasm_data_offset_2,
            symbol_table->member_wasm_data_size_2,
            symbol_table->member_wasm_data_data_2
        );
    }else{

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    *section = tmp_section;

    if (tmp_section[0] && tmp_section[1] && tmp_section[2] && tmp_section[3]
        && tmp_section[4] && tmp_section[5] && tmp_section[6]){

        if (tmp_section[7]){

            module->member_section_num = TP_SECTION_NUM;
        }else{

            module->member_section_num = TP_SECTION_NUM - 1;
        }

        return true;
    }

    return false;
}

bool tp_wasm_add_global_variable_i32(
    TP_SYMBOL_TABLE* symbol_table, uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
    uint32_t init_value, uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index)
{
    if (symbol_table->member_is_add_data_section){

        if ( ! tp_add_coff_section(symbol_table, TP_SECTION_KIND_DATA, TP_SECTION_ALIGN_8_BYTE)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_append_coff_symbol(symbol_table, TP_SECTION_KIND_DATA, NULL, 0, 0, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        symbol_table->member_is_add_data_section = false;
    }

    if ( ! wasm_append_linear_memory(
        symbol_table,
        TP_SECTION_KIND_DATA, (uint8_t*)&init_value, sizeof(init_value), TP_SECTION_ALIGN_8_BYTE, wasm_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! wasm_calc_coff_offset(symbol_table, TP_SECTION_KIND_DATA, *wasm_offset, coff_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_append_coff_symbol(
        symbol_table, TP_SECTION_KIND_DATA, symbol, symbol_length, *coff_offset, symbol_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_append_coff_data_section(
        symbol_table, (uint8_t*)&init_value, sizeof(init_value), TP_SECTION_ALIGN_8_BYTE)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_wasm_add_global_variable_i64(
    TP_SYMBOL_TABLE* symbol_table, uint32_t var_id, uint8_t* symbol, rsize_t symbol_length,
    uint64_t init_value, uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index)
{
    if (symbol_table->member_is_add_data_section){

        if ( ! tp_add_coff_section(symbol_table, TP_SECTION_KIND_DATA, TP_SECTION_ALIGN_8_BYTE)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_append_coff_symbol(symbol_table, TP_SECTION_KIND_DATA, NULL, 0, 0, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        symbol_table->member_is_add_data_section = false;
    }

    if ( ! wasm_append_linear_memory(
        symbol_table,
        TP_SECTION_KIND_DATA, (uint8_t*)&init_value, sizeof(init_value), TP_SECTION_ALIGN_8_BYTE, wasm_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! wasm_calc_coff_offset(symbol_table, TP_SECTION_KIND_DATA, *wasm_offset, coff_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_append_coff_symbol(
        symbol_table, TP_SECTION_KIND_DATA, symbol, symbol_length, *coff_offset, symbol_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    } 

    if ( ! tp_append_coff_data_section(
        symbol_table, (uint8_t*)&init_value, sizeof(init_value), TP_SECTION_ALIGN_8_BYTE)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_wasm_add_local_variable_i32(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, uint32_t* wasm_offset)
{
    symbol_table->member_wasm_local_pos += (uint32_t)(sizeof(int32_t) * 2);

    *wasm_offset = symbol_table->member_wasm_local_pos;

    return true;
}

bool tp_wasm_add_local_variable_i64(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, uint32_t* wasm_offset)
{
    symbol_table->member_wasm_local_pos += (uint32_t)(sizeof(int64_t));

    *wasm_offset = symbol_table->member_wasm_local_pos;

    return true;
}

bool tp_wasm_add_string_literal(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t* string_literal_index, uint8_t* symbol, rsize_t symbol_length,
    uint16_t* string, rsize_t string_length,
    uint32_t* wasm_offset, uint32_t* coff_offset, uint32_t* symbol_index)
{
    if (symbol_table->member_is_add_rdata_section){

        if ( ! tp_add_coff_section(symbol_table, TP_SECTION_KIND_RDATA, TP_SECTION_ALIGN_16_BYTE)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_append_coff_symbol(symbol_table, TP_SECTION_KIND_RDATA, NULL, 0, 0, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        symbol_table->member_is_add_rdata_section = false;
    }

    string_length *= sizeof(uint16_t);

    if ( ! wasm_append_linear_memory(
        symbol_table,
        TP_SECTION_KIND_RDATA, (uint8_t*)string, string_length, TP_SECTION_ALIGN_16_BYTE, wasm_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! wasm_calc_coff_offset(symbol_table, TP_SECTION_KIND_RDATA, *wasm_offset, coff_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_append_coff_symbol(
        symbol_table, TP_SECTION_KIND_RDATA, symbol, symbol_length, (uint32_t)*coff_offset, symbol_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_append_coff_rdata_section(
        symbol_table, (uint8_t*)string, (string_length + 2), TP_SECTION_ALIGN_16_BYTE)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_wasm_add_function(TP_SYMBOL_TABLE* symbol_table, uint8_t* symbol, rsize_t symbol_length)
{
    if (symbol_table->member_is_add_text_section){

        if ( ! tp_add_coff_section(symbol_table, TP_SECTION_KIND_TEXT, TP_SECTION_ALIGN_16_BYTE)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_append_coff_symbol(symbol_table, TP_SECTION_KIND_TEXT, NULL, 0, 0, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        symbol_table->member_is_add_text_section = false;
    }

    if ( ! tp_append_coff_symbol(symbol_table, TP_SECTION_KIND_TEXT, symbol, symbol_length, 0, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_wasm_init_relocation(TP_SYMBOL_TABLE* symbol_table)
{
    symbol_table->member_wasm_relocation_cur = 0;

    return true;
}

bool tp_wasm_add_relocation(TP_SYMBOL_TABLE* symbol_table, TP_WASM_RELOCATION* wasm_relocation)
{
    if (symbol_table->member_wasm_relocation_pos ==
        (symbol_table->member_wasm_relocation_size / sizeof(TP_WASM_RELOCATION))){

        rsize_t tp_wasm_relocation_allocate_unit = 
            symbol_table->member_wasm_relocation_allocate_unit * sizeof(TP_WASM_RELOCATION);

        rsize_t tp_wasm_relocation_size =  symbol_table->member_wasm_relocation_size + tp_wasm_relocation_allocate_unit;

        TP_WASM_RELOCATION* tp_wasm_relocation = (TP_WASM_RELOCATION*)TP_REALLOC(
            symbol_table, symbol_table->member_wasm_relocation, tp_wasm_relocation_size
        );

        if (NULL == tp_wasm_relocation){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((uint8_t*)tp_wasm_relocation) + symbol_table->member_wasm_relocation_size, 0,
            tp_wasm_relocation_allocate_unit
        );

        symbol_table->member_wasm_relocation = tp_wasm_relocation;
        symbol_table->member_wasm_relocation_size = tp_wasm_relocation_size;
    }

    symbol_table->member_wasm_relocation[symbol_table->member_wasm_relocation_pos] = *wasm_relocation;

    ++(symbol_table->member_wasm_relocation_pos);

    return true;
}

bool tp_wasm_get_relocation(
    TP_SYMBOL_TABLE* symbol_table, rsize_t begin_index, TP_WASM_RELOCATION** wasm_relocation)
{
    if (NULL == wasm_relocation){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (symbol_table->member_wasm_relocation){

        if (symbol_table->member_wasm_relocation_cur > symbol_table->member_wasm_relocation_pos){

            return true;
        }

        TP_WASM_RELOCATION* tmp_wasm_relocation =
            &(symbol_table->member_wasm_relocation[symbol_table->member_wasm_relocation_cur]);

        if (tmp_wasm_relocation->member_begin_index == begin_index){

            *wasm_relocation = tmp_wasm_relocation;
            ++(symbol_table->member_wasm_relocation_cur);

            return true;
        }
    }

    return true;
}

static bool wasm_append_linear_memory(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind,
    uint8_t* data, rsize_t size, TP_SECTION_ALIGN section_align, int32_t* offset)
{
    switch (section_kind){
    case TP_SECTION_KIND_DATA:{

        if (NULL == symbol_table->member_wasm_data_data_1){

            if (NULL == symbol_table->member_wasm_data_data_2){

                symbol_table->member_wasm_data_offset_1 = TP_DATA_BASE;
            }else{

                symbol_table->member_wasm_data_offset_1 =
                    TP_DATA_BASE + symbol_table->member_wasm_data_size_2;
            }

            symbol_table->member_wasm_data_base_1 = 
                symbol_table->member_wasm_data_offset_1;
        }

        uint32_t offset_data_1 = symbol_table->member_wasm_data_offset_1;
        uint32_t data_size_1 = symbol_table->member_wasm_data_size_1;

        uint32_t size_1 = 0;
        uint32_t padding_1 = 0;

        switch (section_align){
        case TP_SECTION_ALIGN_8_BYTE:
            padding_1 = TP_WASM_DATA_ALIGN_8(size);
            size_1 = (uint32_t)(size) + padding_1;
            symbol_table->member_wasm_data_offset_1 += size_1;
            symbol_table->member_wasm_data_size_1 += size_1;
            break;
        case TP_SECTION_ALIGN_16_BYTE:
//          break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        uint8_t* tmp_data_1 = (uint8_t*)TP_REALLOC(
            symbol_table, symbol_table->member_wasm_data_data_1,
            sizeof(uint8_t)* symbol_table->member_wasm_data_size_1
        );

        if (NULL == tmp_data_1){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_wasm_data_data_1 = tmp_data_1;

        memcpy(symbol_table->member_wasm_data_data_1 + data_size_1, data, size);

        if (padding_1){

            memset(symbol_table->member_wasm_data_data_1 + data_size_1 + size, 0, padding_1);
        }

        *offset = (int32_t)offset_data_1;

        break;
    }
    case TP_SECTION_KIND_RDATA:{

        if (NULL == symbol_table->member_wasm_data_data_2){

            if (NULL == symbol_table->member_wasm_data_data_1){

                symbol_table->member_wasm_data_offset_2 = TP_DATA_BASE;
            }else{

                symbol_table->member_wasm_data_offset_2 =
                    TP_DATA_BASE + symbol_table->member_wasm_data_size_1;
                symbol_table->member_wasm_data_offset_2 +=
                    TP_WASM_DATA_ALIGN_16(symbol_table->member_wasm_data_offset_2);
            }

            symbol_table->member_wasm_data_base_2 = 
                symbol_table->member_wasm_data_offset_2;
        }

        uint32_t offset_data_2 = symbol_table->member_wasm_data_offset_2;
        uint32_t data_size_2 = symbol_table->member_wasm_data_size_2;

        uint32_t size_2 = 0;
        uint32_t padding_2 = 0;

        switch (section_align){
        case TP_SECTION_ALIGN_16_BYTE:
            padding_2 = TP_WASM_DATA_ALIGN_16(size);
            size_2 = (uint32_t)(size) + padding_2;
            symbol_table->member_wasm_data_offset_2 += size_2;
            symbol_table->member_wasm_data_size_2 += size_2;
            break;
        case TP_SECTION_ALIGN_8_BYTE:
//          break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        uint8_t* tmp_data_2 = (uint8_t*)TP_REALLOC(
            symbol_table, symbol_table->member_wasm_data_data_2,
            sizeof(uint8_t) * symbol_table->member_wasm_data_size_2
        );

        if (NULL == tmp_data_2){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        symbol_table->member_wasm_data_data_2 = tmp_data_2;

        memcpy(symbol_table->member_wasm_data_data_2 + data_size_2, data, size);

        if (padding_2){

            memset(symbol_table->member_wasm_data_data_2 + data_size_2 + size, 0, padding_2);
        }

        *offset = (int32_t)offset_data_2;

        break;
    }
    case TP_SECTION_KIND_TEXT:
//      break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool wasm_calc_coff_offset(
    TP_SYMBOL_TABLE* symbol_table,
    TP_SECTION_KIND section_kind, int32_t wasm_offset, int32_t* coff_offset)
{
    uint32_t tmp_wasm_offset = (uint32_t)wasm_offset;
 
    switch (section_kind){
    case TP_SECTION_KIND_DATA:
        if (tmp_wasm_offset >= symbol_table->member_wasm_data_base_1){

            *coff_offset = (int32_t)(tmp_wasm_offset - symbol_table->member_wasm_data_base_1);
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    case TP_SECTION_KIND_RDATA:
        if (tmp_wasm_offset >= symbol_table->member_wasm_data_base_2){

            *coff_offset = (int32_t)(tmp_wasm_offset - symbol_table->member_wasm_data_base_2);
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    case TP_SECTION_KIND_TEXT:
//      break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

