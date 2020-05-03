
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool wasm_gen(TP_SYMBOL_TABLE* symbol_table, bool is_origin_wasm, bool is_pe_coff, bool is_64);

// WebAssembly headers.
static bool make_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section
);
static TP_WASM_MODULE_SECTION** allocate_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module
);
static void free_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section
);
static bool get_wasm_module_code_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t* code_section_index, uint32_t* return_type
);
static bool get_wasm_module_export_section_item_index(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint8_t* name, uint8_t kind, uint32_t* item_index
);
static bool get_wasm_module_function_section_type(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t item_index, uint32_t* type
);
static bool get_wasm_module_type_section_return_type(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t type, uint32_t* return_type
);
static bool get_wasm_module_code_section_index(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t* code_section_index
);

bool tp_make_wasm(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, bool is_origin_wasm, bool is_pe_coff, bool is_64)
{
    if ( ! wasm_gen(symbol_table, is_origin_wasm, is_pe_coff, is_64)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (symbol_table->member_is_output_wasm_text_file){

        if ( ! tp_disasm_wasm(symbol_table, entry_point_symbol)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool wasm_gen(TP_SYMBOL_TABLE* symbol_table, bool is_origin_wasm, bool is_pe_coff, bool is_64)
{
    TP_WASM_MODULE* module = &(symbol_table->member_wasm_module);

    TP_WASM_MODULE_SECTION** section = NULL;

    bool is_success = false;

    if (is_pe_coff){

        if ( ! tp_make_wasm_coff(symbol_table, module, &section, is_origin_wasm, is_64)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }

        is_success = true;
    }else{

        if ( ! tp_make_wasm_expr(symbol_table, module, &section, is_origin_wasm)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }

        is_success = true;
    }

    if (is_success){

        module->member_section = section;

        for (size_t i = 0; module->member_section_num > i; ++i){

            module->member_content_size += section[i]->member_section_size;
        }

        module->member_content_size +=
            (sizeof(module->member_module_content->member_magic_number) +
            sizeof(module->member_module_content->member_version));

        {
            TP_WASM_MODULE_CONTENT* tmp = (TP_WASM_MODULE_CONTENT*)calloc(
                sizeof(TP_WASM_MODULE_CONTENT) + module->member_content_size, sizeof(uint8_t)
            );

            if (NULL == tmp){

                TP_PRINT_CRT_ERROR(symbol_table);

                goto error_proc;
            }

            module->member_module_content = (TP_WASM_MODULE_CONTENT*)tmp;

            memcpy(&(module->member_module_content->member_magic_number), TP_WASM_MODULE_MAGIC_NUMBER, sizeof(uint32_t));
            module->member_module_content->member_version = TP_WASM_MODULE_VERSION;

            {
                uint32_t prev_content_size = 0;

                for (size_t i = 0; module->member_section_num > i; ++i){

                    memcpy(
                        module->member_module_content->member_payload + prev_content_size,
                        section[i]->member_name_len_name_payload_data, section[i]->member_section_size
                    );

                    prev_content_size += section[i]->member_section_size;
                }
            }
        }

        if ((false ==  symbol_table->member_is_no_output_files) ||
            (symbol_table->member_is_no_output_files && symbol_table->member_is_output_wasm_file)){

            if ( ! tp_write_file(
                symbol_table, symbol_table->member_wasm_file_path,
                module->member_module_content, module->member_content_size)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto error_proc;
            }
        }

        return true;
    }

error_proc:

    if (section){

        for (size_t i = 0; module->member_section_num > i; ++i){

            if (section[i]){

                TP_FREE(symbol_table, &(section[i]->member_name_len_name_payload_data), section[i]->member_section_size);

                TP_FREE(symbol_table, &(section[i]), sizeof(TP_WASM_MODULE_SECTION));
            }
        }

        TP_FREE2(symbol_table, &section, module->member_section_num * sizeof(TP_WASM_MODULE_SECTION*));
    }

    module->member_section = NULL;

    module->member_section_num = 0;

    if (module->member_module_content){

        TP_FREE(symbol_table, &(module->member_module_content), sizeof(TP_WASM_MODULE_CONTENT) + module->member_content_size);
    }

    return false;
}

bool tp_get_wasm_export_code_section(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, TP_WASM_MODULE_SECTION** code_section, uint32_t* return_type)
{
    TP_WASM_MODULE* module = &(symbol_table->member_wasm_module);

    TP_WASM_MODULE_CONTENT* module_content = module->member_module_content;

    uint8_t magic_number[sizeof(TP_WASM_MODULE_MAGIC_NUMBER)];
    memset(magic_number, 0, sizeof(magic_number));

    memcpy(magic_number, &(module_content->member_magic_number), sizeof(uint32_t));
    magic_number[sizeof(TP_WASM_MODULE_MAGIC_NUMBER) - 1] = '\0';

    if ((0 != strcmp(TP_WASM_MODULE_MAGIC_NUMBER, magic_number)) ||
        (TP_WASM_MODULE_VERSION != module_content->member_version)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT(
                "ERROR: Bad value. module_content->member_magic_number(%1), "
                "module_content->member_version(%2)."
            ),
            TP_LOG_PARAM_STRING(magic_number),
            TP_LOG_PARAM_UINT64_VALUE(module_content->member_version)
        );

        return false;
    }

    if (NULL == module->member_section){

        if ( ! make_wasm_module_section(symbol_table, module, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        TP_WASM_MODULE_SECTION** section = NULL;

        if ( ! make_wasm_module_section(symbol_table, module, &section)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        module->member_section = section;
    }

    uint32_t code_section_index = 0;

    if ( ! get_wasm_module_code_section(
        symbol_table, entry_point_symbol, 
        module->member_section, module->member_section_num, &code_section_index, return_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *code_section = module->member_section[code_section_index];

    return true;
}

static bool make_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section)
{
    if (section){

        TP_WASM_MODULE_SECTION** tmp_section = allocate_wasm_module_section(symbol_table, module);

        if (NULL == tmp_section){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        *section = tmp_section;

    }else if (0 != module->member_section_num){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: 0 != module->member_section_num(%1)"),
            TP_LOG_PARAM_UINT64_VALUE(module->member_section_num)
        );

        return false;
    }

    TP_WASM_MODULE_CONTENT* module_content = module->member_module_content;

    uint8_t* module_content_payload = module_content->member_payload;

    uint32_t current_size = (sizeof(module_content->member_magic_number) +
            sizeof(module_content->member_version));

    uint32_t section_num = 0;

    do{
        uint32_t size_id = 0;

        uint32_t id = tp_decode_ui32leb128(module_content_payload, &size_id);

        if (TP_WASM_SECTION_TYPE_CUSTOM == id){

            // NOTE: Not implemented.

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: TP_WASM_SECTION_TYPE_CUSTOM is not implemented.")
            );

            return false;
        }

        uint32_t size_payload_len = 0;

        uint32_t payload_len = tp_decode_ui32leb128(module_content_payload + size_id, &size_payload_len);

        uint32_t section_size = size_id + size_payload_len + payload_len;

        if (section){

            (*section)[section_num]->member_section_size = section_size;
            (*section)[section_num]->member_id = id;
            (*section)[section_num]->member_payload_len = payload_len;

            // NOTE: Not implemented.
            // name_len: 0 == member_id
            // name: 0 == member_id

            uint8_t* tmp_payload = (uint8_t*)calloc(payload_len, sizeof(uint8_t));

            if (NULL == tmp_payload){

                TP_PRINT_CRT_ERROR(symbol_table);

                free_wasm_module_section(symbol_table, module, section);

                return false;
            }

            (*section)[section_num]->member_name_len_name_payload_data = tmp_payload;

            memcpy((*section)[section_num]->member_name_len_name_payload_data,
                module_content_payload + size_id + size_payload_len, payload_len
            );
        }

        current_size += section_size;

        if (module->member_content_size < current_size){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: module->member_content_size(%1) < current_size(%2)"),
                TP_LOG_PARAM_UINT64_VALUE(module->member_content_size),
                TP_LOG_PARAM_UINT64_VALUE(current_size)
            );

            return false;
        }

        module_content_payload -= section_size;

        ++section_num;

    }while (module->member_content_size != current_size);

    module->member_section_num = section_num;

    return true;
}

static TP_WASM_MODULE_SECTION** allocate_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module)
{
    TP_WASM_MODULE_SECTION** tmp_section =
        (TP_WASM_MODULE_SECTION**)calloc(module->member_section_num, sizeof(TP_WASM_MODULE_SECTION*));

    if (NULL == tmp_section){

        module->member_section_num = 0;

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    bool status = true;

    for (uint32_t i = 0; module->member_section_num > i; ++i){

        tmp_section[i] = (TP_WASM_MODULE_SECTION*)calloc(1, sizeof(TP_WASM_MODULE_SECTION));

        if (NULL == tmp_section[i]){

            status = false;

            TP_PRINT_CRT_ERROR(symbol_table);
        }
    }

    if ( ! status){

        free_wasm_module_section(symbol_table, module, &tmp_section);

        return false;
    }

    return  tmp_section;
}

static void free_wasm_module_section(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE* module, TP_WASM_MODULE_SECTION*** section)
{
    if (NULL == section){

        return;
    }

    for (uint32_t i = 0; module->member_section_num > i; ++i){

        if (section[i]){

            TP_FREE(symbol_table, &((*section)[i]->member_name_len_name_payload_data), (*section)[i]->member_section_size);

            TP_FREE(symbol_table, &(*section)[i], sizeof(TP_WASM_MODULE_SECTION));
        }
    }

    TP_FREE2(symbol_table, section, module->member_section_num * sizeof(TP_WASM_MODULE_SECTION*));

    module->member_section_num = 0;
}

static bool get_wasm_module_code_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t* code_section_index, uint32_t* return_type)
{
    uint32_t item_index = 0;

    if ( ! get_wasm_module_export_section_item_index(
        symbol_table,
        section, section_num, entry_point_symbol, TP_WASM_SECTION_KIND_FUNCTION, &item_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint32_t type = 0;

    if ( ! get_wasm_module_function_section_type(symbol_table, section, section_num, item_index, &type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! get_wasm_module_type_section_return_type(symbol_table, section, section_num, type, return_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! get_wasm_module_code_section_index(symbol_table, section, section_num, code_section_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool get_wasm_module_export_section_item_index(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint8_t* name, uint8_t kind, uint32_t* item_index)
{
    TP_WASM_MODULE_SECTION* export_section = NULL;

    for (uint32_t i = 0; section_num > i; ++i){

        if (TP_WASM_SECTION_TYPE_EXPORT == section[i]->member_id){

            export_section = section[i];

            break;
        }
    }

    if (NULL == export_section){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: NULL == export_section")
        );

        return false;
    }

    uint8_t* payload = export_section->member_name_len_name_payload_data;
    uint32_t offset = 0;

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, export_section, payload, offset, TP_WASM_SECTION_TYPE_EXPORT);

    uint32_t payload_len = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, export_section, payload, offset, payload_len);

    uint32_t count = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, export_section, payload, offset, count);

    if (0 == count){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: 0 == count")
        );

        return false;
    }

    for (uint32_t i = 0; count > i; ++i){

        uint32_t name_length = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, export_section, payload, offset, name_length);

        bool is_match = false;
        TP_WASM_CHECK_STRING(symbol_table, export_section, payload, offset, name, name_length, is_match);

        uint32_t export_kind = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, export_section, payload, offset, export_kind);

        uint32_t export_item_index = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, export_section, payload, offset, export_item_index);

        if (is_match && (kind == export_kind)){

            *item_index = export_item_index;

            return true;
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"),
        TP_LOG_PARAM_STRING("ERROR: WASM module export section item not found.")
    );

    return false;
}

static bool get_wasm_module_function_section_type(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t item_index, uint32_t* type)
{
    TP_WASM_MODULE_SECTION* function_section = NULL;

    for (uint32_t i = 0; section_num > i; ++i){

        if (TP_WASM_SECTION_TYPE_FUNCTION == section[i]->member_id){

            function_section = section[i];

            break;
        }
    }

    if (NULL == function_section){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: WASM module type section function item not found.")
        );

        return false;
    }

    uint8_t* payload = function_section->member_name_len_name_payload_data;
    uint32_t offset = 0;

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, function_section, payload, offset, TP_WASM_SECTION_TYPE_FUNCTION);

    uint32_t payload_len = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, function_section, payload, offset, payload_len);

    TP_DECODE_UI32LEB128_CHECK_VALUE(
        symbol_table, function_section, payload, offset, TP_WASM_MODULE_SECTION_FUNCTION_COUNT_EXPR
    );
    TP_DECODE_UI32LEB128_CHECK_VALUE(
        symbol_table, function_section, payload, offset, TP_WASM_MODULE_SECTION_FUNCTION_TYPES_EXPR
    );

    *type = 0;

    return true;
}

static bool get_wasm_module_type_section_return_type(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t type, uint32_t* return_type)
{
    TP_WASM_MODULE_SECTION* type_section = NULL;

    for (uint32_t i = 0; section_num > i; ++i){

        if (TP_WASM_SECTION_TYPE_TYPE == section[i]->member_id){

            type_section = section[i];

            break;
        }
    }

    if (NULL == type_section){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: WASM module type section type item not found.")
        );

        return false;
    }

    uint8_t* payload = type_section->member_name_len_name_payload_data;
    uint32_t offset = 0;

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, type_section, payload, offset, TP_WASM_SECTION_TYPE_TYPE);

    uint32_t payload_len = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, payload_len);

    uint32_t type_count = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, type_count);

    for (uint32_t i = 0; type_count > i; ++i){
        
        if (type == i){

            TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, type_section, payload, offset, TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC);

            uint32_t param_count = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, param_count);

            if (symbol_table->member_local_types){

                TP_FREE(
                    symbol_table,
                    &(symbol_table->member_local_types), sizeof(TP_WASM_LOCAL_TYPE) * symbol_table->member_local_types_num
                );
                symbol_table->member_local_types_num = 0;
            }

            if (param_count){

                if (4 < param_count){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                symbol_table->member_local_types = (TP_WASM_LOCAL_TYPE*)calloc(param_count, sizeof(TP_WASM_LOCAL_TYPE));

                if (NULL == symbol_table->member_local_types){

                    TP_PRINT_CRT_ERROR(symbol_table);

                    return false;
                }

                symbol_table->member_local_types_num = param_count;
            }

            for (uint32_t j = 0, arg_offset = 0; param_count > j; ++j){

                uint32_t param_types = 0;
                TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, param_types);

                symbol_table->member_local_types[j].member_type = (param_types | TP_WASM_VAR_TYPE_PSEUDO_PREFIX);
                symbol_table->member_local_types[j].member_offset += arg_offset;

                switch (param_types){
                case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32:
//                  break;
                case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64:
                    arg_offset += sizeof(int64_t);
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
            }

            TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, type_section, payload, offset, TP_WASM_MODULE_SECTION_TYPE_RETURN_COUNT);

            uint32_t ret_type = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, ret_type);

            switch (ret_type){
            case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32:
//              break;
            case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64:
                *return_type = ret_type;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            return true;
        }else{

            TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, type_section, payload, offset, TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC);

            uint32_t param_count = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, param_count);

            for (uint32_t j = 0; param_count > j; ++j){

                uint32_t param_types = 0;
                TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, param_types);
            }

            TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, type_section, payload, offset, TP_WASM_MODULE_SECTION_TYPE_RETURN_COUNT);

            uint32_t ret_type = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, type_section, payload, offset, ret_type);
        }
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
}

static bool get_wasm_module_code_section_index(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION** section, uint32_t section_num, uint32_t* code_section_index)
{
    for (uint32_t i = 0; section_num > i; ++i){

        if (TP_WASM_SECTION_TYPE_CODE == section[i]->member_id){

            *code_section_index = i;

            return true;
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"),
        TP_LOG_PARAM_STRING("ERROR: WASM module code section item not found.")
    );

    return false;
}

