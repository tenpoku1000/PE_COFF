
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_WASM_STACK_POP_U_DECODE(symbol_table, format_string, result, result_param) \
{ \
    uint32_t param_size = 0; \
\
    (result_param) = tp_decode_ui32leb128( \
        &((symbol_table)->member_wasm_code_body_buffer[(symbol_table)->member_wasm_code_body_pos]), \
        &param_size \
    ); \
\
    if (((symbol_table)->member_wasm_code_body_pos + param_size) >= (symbol_table)->member_wasm_code_body_size){ \
\
        uint64_t param1 = (uint64_t)((symbol_table)->member_wasm_code_body_pos); \
        uint64_t param2 = (uint64_t)param_size; \
\
        TP_PUT_LOG_MSG( \
            (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
            TP_MSG_FMT( \
                format_string \
                ": symbol_table->member_wasm_code_body_pos + param_size: %1 >= " \
                "symbol_table->member_wasm_code_body_size: %2" \
            ), \
            TP_LOG_PARAM_UINT64_VALUE(param1 + param2), \
            TP_LOG_PARAM_UINT64_VALUE((symbol_table)->member_wasm_code_body_size) \
        ); \
\
        memset(&result, 0, sizeof(result)); \
        return result; \
    } \
\
    (symbol_table)->member_wasm_code_body_pos += param_size; \
}   

#define TP_WASM_STACK_POP_DECODE(symbol_table, format_string, result, result_param) \
{ \
    uint32_t param_size = 0; \
\
    (result_param) = tp_decode_si32leb128( \
        &((symbol_table)->member_wasm_code_body_buffer[(symbol_table)->member_wasm_code_body_pos]), \
        &param_size \
    ); \
\
    if (((symbol_table)->member_wasm_code_body_pos + param_size) >= (symbol_table)->member_wasm_code_body_size){ \
\
        uint64_t param1 = (uint64_t)((symbol_table)->member_wasm_code_body_pos); \
        uint64_t param2 = (uint64_t)param_size; \
\
        TP_PUT_LOG_MSG( \
            (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
            TP_MSG_FMT( \
                format_string \
                ": symbol_table->member_wasm_code_body_pos + param_size: %1 >= " \
                "symbol_table->member_wasm_code_body_size: %2" \
            ), \
            TP_LOG_PARAM_UINT64_VALUE(param1 + param2), \
            TP_LOG_PARAM_UINT64_VALUE((symbol_table)->member_wasm_code_body_size) \
        ); \
\
        memset(&result, 0, sizeof(result)); \
        return result; \
    } \
\
    (symbol_table)->member_wasm_code_body_pos += param_size; \
}   

static uint32_t convert_section_code_content2x64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION* code_section, uint32_t return_type, uint8_t* x64_code_buffer
);
static bool wasm_stack_and_use_register_init(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* wasm_code_body_buffer, uint32_t wasm_code_body_size
);
static bool wasm_stack_and_wasm_code_is_empty(TP_SYMBOL_TABLE* symbol_table);
static bool wasm_stack_is_empty(TP_SYMBOL_TABLE* symbol_table);
static TP_WASM_STACK_ELEMENT wasm_stack_pop(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_POP_MODE pop_mode);

static bool allocate_variable_common(
    TP_SYMBOL_TABLE* symbol_table, TP_X64_ALLOCATE_MODE allocate_mode,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_WASM_STACK_ELEMENT* wasm_stack_element
);
static bool get_free_register(
    TP_SYMBOL_TABLE* symbol_table, int32_t arg_index,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    TP_X86_32_REGISTER* x86_32_register, TP_X64_32_REGISTER* x64_32_register, bool* is_zero_free_register
);
static bool spilling_variable(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_X86_32_REGISTER* x86_32_register, TP_X64_32_REGISTER* x64_32_register,
    TP_WASM_STACK_ELEMENT* wasm_stack_element
);
static bool get_free_register_in_wasm_stack(
    TP_SYMBOL_TABLE* symbol_table, int32_t arg_index,
    TP_X86_32_REGISTER* free_x86_32_register, TP_X64_32_REGISTER* free_x64_32_register,
    bool* is_zero_free_register_in_wasm_stack
);
static bool set_nv_register(
    TP_SYMBOL_TABLE* symbol_table,
    TP_X86_32_REGISTER x86_32_register, TP_X64_32_REGISTER x64_32_register
);

typedef int (*x64_jit_func)(void);

bool tp_make_x64_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol, int32_t* return_value,
    bool is_pe_coff, char* fname, char* ext)
{
    uint8_t* x64_code_buffer = NULL;
    uint32_t x64_code_buffer_size1 = 0;
    uint32_t x64_code_buffer_size2 = 0;

    uint8_t* coff_buffer = NULL;
    rsize_t coff_file_size = 0;
    rsize_t code_offset = 0;

    TP_WASM_MODULE_SECTION* code_section = NULL;
    uint32_t return_type = 0;

    if ( ! tp_get_wasm_export_code_section(
        symbol_table, entry_point_symbol, &code_section, &return_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    memset(
        symbol_table->member_use_nv_register,
        TP_X64_NV64_REGISTER_NULL, sizeof(symbol_table->member_use_nv_register)
    );

    x64_code_buffer_size1 = convert_section_code_content2x64(
        symbol_table, code_section, return_type, NULL
    );

    if (0 == x64_code_buffer_size1){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == x64_code_buffer_size1")
        );

        goto fail;
    }

    for (rsize_t i = 0; TP_X64_NV64_REGISTER_NUM > i; ++i){

        if (TP_X64_NV64_REGISTER_NULL != symbol_table->member_use_nv_register[i]){

            ++x64_code_buffer_size1;
        }
    }

    uint8_t* tmp_x64_code_buffer = (uint8_t*)VirtualAlloc(
        NULL, x64_code_buffer_size1,
        MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
    );

    if (NULL == tmp_x64_code_buffer){

        TP_GET_LAST_ERROR(symbol_table);

        goto fail;
    }

    x64_code_buffer = tmp_x64_code_buffer;

    // Temporary variables.
    {
        int32_t v = symbol_table->member_register_bytes +
            symbol_table->member_padding_register_bytes +
            symbol_table->member_temporary_variable_size;

        symbol_table->member_padding_temporary_variable_bytes = ((-v) & TP_PADDING_MASK);
    }

    x64_code_buffer_size2 = convert_section_code_content2x64(
        symbol_table, code_section, return_type, x64_code_buffer
    );

    if (0 == x64_code_buffer_size2){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == x64_code_buffer_size2")
        );

        goto fail;
    }

    if ((false ==  symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files && symbol_table->member_is_output_x64_file)){

        if ( ! tp_write_file(
            symbol_table, symbol_table->member_x64_file_path, x64_code_buffer, x64_code_buffer_size1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        if ( ! tp_disasm_x64(
            symbol_table, symbol_table->member_x64_text_file_path, x64_code_buffer,
            x64_code_buffer_size1, NULL, NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }
    }

    if (x64_code_buffer_size1 != x64_code_buffer_size2){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: x64_code_buffer_size1 != x64_code_buffer_size2")
        );

        goto fail;
    }

    if (symbol_table->member_wasm_relocation){

        if ( ! tp_get_coff_size_and_calc_offset(
            symbol_table, is_pe_coff, x64_code_buffer_size1, &coff_file_size, &code_offset)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        uint8_t* tmp_coff_buffer = (uint8_t*)VirtualAlloc(
            NULL, coff_file_size,
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
        );

        if (NULL == tmp_coff_buffer){

            TP_GET_LAST_ERROR(symbol_table);

            goto fail;
        }

        coff_buffer = tmp_coff_buffer;

        if (is_pe_coff){

            memset(coff_buffer, 0, coff_file_size);
        }

        if ( ! tp_make_COFF(
            symbol_table, fname, ext,
            x64_code_buffer, x64_code_buffer_size1, coff_buffer, coff_file_size, code_offset)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        memset(x64_code_buffer, 0, x64_code_buffer_size1);

        if ( ! VirtualFree(x64_code_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);

            x64_code_buffer = NULL;

            goto fail;
        }

        x64_code_buffer = coff_buffer + code_offset;
    }

    if (false == is_pe_coff){

        uint8_t* entry_point = NULL;

        if (symbol_table->member_wasm_relocation){

            if ( ! tp_get_entry_point_and_calc_code_relocations(
                symbol_table, coff_buffer, coff_file_size, code_offset, entry_point_symbol, &entry_point)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
        }else{

            entry_point = x64_code_buffer;
        }

        if (NULL == entry_point){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        DWORD old_protect = 0;

        if ( ! VirtualProtect(x64_code_buffer, x64_code_buffer_size1, PAGE_EXECUTE_READ, &old_protect)){

            TP_GET_LAST_ERROR(symbol_table);

            goto fail;
        }

        x64_jit_func func = (x64_jit_func)entry_point;

        int value = func();

        if ( ! symbol_table->member_is_no_output_messages){

            printf("x64_jit_func() = %d\n", value);
        }

        if (return_value){

            *return_value = value;
        }

        errno_t err = _set_errno(0);
    }

    if (symbol_table->member_wasm_relocation){

        memset(coff_buffer, 0, coff_file_size);

        if ( ! VirtualFree(coff_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);

            return false;
        }

        coff_buffer = NULL;
        coff_file_size = 0;
    }else{

        if ( ! VirtualFree(x64_code_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);

            return false;
        }

        x64_code_buffer = NULL;
        x64_code_buffer_size1 = 0;
        x64_code_buffer_size2 = 0;
    }

    return true;

fail:

    if (coff_buffer){

        memset(coff_buffer, 0, coff_file_size);

        if ( ! VirtualFree(coff_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);
        }

        coff_buffer = NULL;
        coff_file_size = 0;
    }

    if (x64_code_buffer){

        memset(x64_code_buffer, 0, x64_code_buffer_size1);

        if ( ! VirtualFree(x64_code_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);
        }

        x64_code_buffer = NULL;
        x64_code_buffer_size1 = 0;
        x64_code_buffer_size2 = 0;
    }

    return false;
}

static uint32_t convert_section_code_content2x64(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_MODULE_SECTION* code_section, uint32_t return_type, uint8_t* x64_code_buffer)
{
    uint32_t x64_code_size = 0;

    uint8_t* payload = code_section->member_name_len_name_payload_data;
    uint32_t offset = 0;

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, code_section, payload, offset, TP_WASM_SECTION_TYPE_CODE);

    uint32_t payload_len = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, payload_len);

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, code_section, payload, offset, TP_WASM_MODULE_SECTION_CODE_COUNT_EXPR);

    uint32_t body_size = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, body_size);

    if (0 == body_size){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == body_size")
        );

        goto fail;
    }

    uint32_t wasm_code_body_size = body_size;

    uint32_t local_count = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, local_count);

    wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, local_count);

    uint32_t local_variable_size = 0;

    uint32_t arg_num = symbol_table->member_local_types_num;
    uint32_t arg_offset = (arg_num ? symbol_table->member_local_types[arg_num - 1].member_offset : 0);
    uint32_t arg_types_index = arg_num;

    for (uint32_t i = 0; local_count > i; ++i){

        uint32_t var_count = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, var_count);

        uint32_t var_type = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, var_type);

        symbol_table->member_local_types_num += var_count;

        TP_WASM_LOCAL_TYPE* tmp_arg_types = (TP_WASM_LOCAL_TYPE*)TP_REALLOC(
            symbol_table,
            symbol_table->member_local_types, sizeof(TP_WASM_LOCAL_TYPE)* symbol_table->member_local_types_num
        );

        if (NULL == tmp_arg_types){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        memset(tmp_arg_types + arg_num, 0, sizeof(TP_WASM_LOCAL_TYPE)* var_count);

        symbol_table->member_local_types = tmp_arg_types;

        for (uint32_t j = 0; var_count > j; ++j){

            if (arg_types_index >= symbol_table->member_local_types_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                goto fail;
            }

            symbol_table->member_local_types[arg_types_index].member_type = var_type;
            symbol_table->member_local_types[arg_types_index].member_offset += arg_offset;

            ++arg_types_index;

            switch (var_type){
            case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32:
                arg_offset += sizeof(int32_t);
                local_variable_size += sizeof(int32_t);
                break;
            case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64:
                arg_offset += sizeof(int64_t);
                local_variable_size += sizeof(int64_t);
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }
        }

        if (INT32_MAX < local_variable_size){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: INT32_MAX(%1) < temp_local_variable_size(%2)"),
                TP_LOG_PARAM_INT32_VALUE(INT32_MAX),
                TP_LOG_PARAM_UINT64_VALUE(local_variable_size)
            );

            goto fail;
        }

        wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, var_count);
        wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, var_type);
    }

    uint8_t* wasm_code_body_buffer = payload + offset;

    if ( ! wasm_stack_and_use_register_init(symbol_table, wasm_code_body_buffer, wasm_code_body_size)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    uint32_t tmp_x64_code_size = tp_encode_allocate_stack(
        symbol_table, x64_code_buffer, x64_code_size, local_variable_size
    );

    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    do{
        bool status = false;

        TP_WASM_STACK_ELEMENT label = { 0 };
        TP_WASM_STACK_ELEMENT op1 = { 0 };
        TP_WASM_STACK_ELEMENT op2 = { 0 };

        TP_WASM_STACK_ELEMENT opcode = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_DEFAULT);

        if (opcode.member_wasm_relocation){

            TP_WASM_RELOCATION_TYPE wasm_relocation_type = opcode.member_wasm_relocation->member_type;

            if ((TP_WASM_RELOCATION_TYPE_PROLOGUE == wasm_relocation_type) ||
                (TP_WASM_RELOCATION_TYPE_EPILOGUE == wasm_relocation_type) ||
                (TP_WASM_RELOCATION_TYPE_SKIP == wasm_relocation_type)){

                continue;
            }

            if (TP_WASM_RELOCATION_TYPE_BASE == wasm_relocation_type){

                tmp_x64_code_size = tp_encode_relocation_type_base(
                    symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                continue;
            }
        }

        switch (opcode.member_wasm_opcode){
        // Control flow operators
        case TP_WASM_OPCODE_LOOP: // sig: block_type
            if ( ! tp_encode_loop_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_sig)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
            break;
        case TP_WASM_OPCODE_END:
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);

            if (wasm_stack_and_wasm_code_is_empty(symbol_table)){

                TP_WASM_STACK_ELEMENT dst = {
                    .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
                    .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                    .member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX
                };

                // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
                uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
                    symbol_table, x64_code_buffer, x64_code_size,
                    TP_X64_MOV, &dst, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

                tmp_x64_code_size = tp_encode_return_code(
                    symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

                return x64_code_size;
            }
            break;
        case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32         
            if ( ! tp_wasm_stack_push(symbol_table, &opcode)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            label = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_LABEL);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_br_if_code(
                symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation,
                opcode.member_relative_depth, &label, &op1, NULL
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_RETURN:
            tmp_x64_code_size = tp_encode_return_code(
                symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            if ((symbol_table->member_wasm_code_body_pos + 1) >= symbol_table->member_wasm_code_body_size){

                return x64_code_size;
            }
            break;

        // Call operators
        case TP_WASM_OPCODE_CALL_INDIRECT: // type_index: varuint32, reserved: varuint1
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_call_indirect_code(
                symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation,
                opcode.member_type_index, opcode.member_reserved, &op1
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;

        // Parametric operators
        case TP_WASM_OPCODE_DROP:
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            break;

        // Variable access
        case TP_WASM_OPCODE_GET_LOCAL: // local_index: varuint32
            tmp_x64_code_size = tp_encode_get_local_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_local_index, &status
            );
            TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_SET_LOCAL: // local_index: varuint32
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_set_local_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_local_index, &op1
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_TEE_LOCAL: // local_index: varuint32
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_tee_local_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_local_index, &op1, &status
            );
            TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_GET_GLOBAL: // global_index: varuint32
            tmp_x64_code_size = tp_encode_get_global_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_global_index
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_SET_GLOBAL: // global_index: varuint32
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_set_global_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_global_index, &op1
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;

        // Memory-related operators
        case TP_WASM_OPCODE_I32_LOAD: // flags: varuint32, offset: varuint32
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_i32_load_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I32_STORE: // flags: varuint32, offset: varuint32
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_i32_store_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I64_LOAD: // flags: varuint32, offset: varuint32
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_i64_load_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I64_STORE: // flags: varuint32, offset: varuint32
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_i64_store_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;

        // Constants
        case TP_WASM_OPCODE_I32_CONST: // value: varint32
            tmp_x64_code_size = tp_encode_i32_const_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_i32, &status
            );
            TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I64_CONST: // value: varint64
            tmp_x64_code_size = tp_encode_i64_const_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, opcode.member_i64, &status
            );
            TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);
            break;

        // Comparison operators
        case TP_WASM_OPCODE_I64_NE: // op1 != op2
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_i64_ne_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;

        // Numeric operators(i32)
        // Numeric operators(i64)
        case TP_WASM_OPCODE_I32_ADD:
//          break;
        case TP_WASM_OPCODE_I64_ADD:
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_add_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I32_SUB:
//          break;
        case TP_WASM_OPCODE_I64_SUB:
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_sub_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I32_MUL:
//          break;
        case TP_WASM_OPCODE_I64_MUL:
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_mul_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I32_DIV_S:
//          break;
        case TP_WASM_OPCODE_I64_DIV_S:
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_div_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        case TP_WASM_OPCODE_I32_XOR:
//          break;
        case TP_WASM_OPCODE_I64_XOR:
            op2 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            op1 = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            tmp_x64_code_size = tp_encode_xor_code(
                symbol_table, x64_code_buffer, x64_code_size,
                opcode.member_wasm_relocation, &op1, &op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;

        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
    }while (true);

fail:

    return 0;
}

static bool wasm_stack_and_use_register_init(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* wasm_code_body_buffer, uint32_t wasm_code_body_size)
{
    // wasm_stack_init

    if ( ! tp_wasm_init_relocation(symbol_table)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (symbol_table->member_stack){

        TP_FREE(symbol_table, &(symbol_table->member_stack), symbol_table->member_stack_size);

        symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
        symbol_table->member_stack_size = 0;
    }

    symbol_table->member_stack = (TP_WASM_STACK_ELEMENT*)TP_CALLOC(
        symbol_table, symbol_table->member_stack_size_allocate_unit, sizeof(TP_WASM_STACK_ELEMENT)
    );

    if (NULL == symbol_table->member_stack){

        TP_PRINT_CRT_ERROR(symbol_table);

        symbol_table->member_wasm_code_body_buffer = NULL;
        symbol_table->member_wasm_code_body_size = 0;
        symbol_table->member_wasm_code_body_pos = 0;

        symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
        symbol_table->member_stack_size = 0;

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: NULL == symbol_table->member_stack")
        );

        return false;
    }

    symbol_table->member_wasm_code_body_buffer = wasm_code_body_buffer;
    symbol_table->member_wasm_code_body_size = wasm_code_body_size;
    symbol_table->member_wasm_code_body_pos = 0;

    symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
    symbol_table->member_stack_size =
        symbol_table->member_stack_size_allocate_unit * sizeof(TP_WASM_STACK_ELEMENT);

    // use_register_init

    symbol_table->member_local_variable_size = 0;
    symbol_table->member_local_variable_size_max = TP_WASM_LOCAL_VARIABLE_MAX_DEFAULT;
    symbol_table->member_padding_local_variable_bytes = 0;

//  symbol_table->member_temporary_variable_size = 0;
    symbol_table->member_temporary_variable_size_max = TP_WASM_TEMPORARY_VARIABLE_MAX_DEFAULT;
    symbol_table->member_padding_temporary_variable_bytes = 0;

    memset(symbol_table->member_use_X86_32_register, 0, sizeof(symbol_table->member_use_X86_32_register));
    memset(symbol_table->member_use_X64_32_register, 0, sizeof(symbol_table->member_use_X64_32_register));
    memset(symbol_table->member_use_v_register, TP_X64_V64_REGISTER_NULL, sizeof(symbol_table->member_use_v_register));

    symbol_table->member_register_bytes = 0;
    symbol_table->member_padding_register_bytes = 0;

    symbol_table->member_stack_imm32 = 0;

    symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EAX].member_x64_item_kind =
        TP_X64_ITEM_KIND_X86_32_REGISTER;

    uint32_t local_types_num = symbol_table->member_local_types_num;

    for (uint32_t i = 0; local_types_num > i; ++i){

        if (4 <= i){

            break;
        }

        switch (symbol_table->member_local_types[i].member_type){
        case TP_WASM_VAR_TYPE_PSEUDO_I32:
//          break;
        case TP_WASM_VAR_TYPE_PSEUDO_I64:{

            TP_WASM_STACK_ELEMENT* use_x86_32_register = NULL;
            TP_WASM_STACK_ELEMENT* use_x64_32_register = NULL;

            switch (i){
            case 0:
                use_x86_32_register = &(symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_ECX]);
                break;
            case 1:
                use_x86_32_register = &(symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX]);
                break;
            case 2:
                use_x64_32_register = &(symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R8D]);
                break;
            case 3:
                use_x64_32_register = &(symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R9D]);
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            switch (i){
            case 0:
//              break;
            case 1:
                if (use_x86_32_register){

                    use_x86_32_register->member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                }
                break;
            case 2:
//              break;
            case 3:
                if (use_x64_32_register){

                    use_x64_32_register->member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }
        default:
            break;
        }
    }

    return true;
}

static bool wasm_stack_and_wasm_code_is_empty(TP_SYMBOL_TABLE* symbol_table)
{
    if (TP_WASM_STACK_EMPTY == symbol_table->member_stack_pos){

        if (symbol_table->member_wasm_code_body_pos == symbol_table->member_wasm_code_body_size){

            return true;
        }
    }

    return false;
}

static bool wasm_stack_is_empty(TP_SYMBOL_TABLE* symbol_table)
{
    if (TP_WASM_STACK_EMPTY == symbol_table->member_stack_pos){

        return true;
    }

    return false;
}

bool tp_wasm_stack_push(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* value)
{
    if (symbol_table->member_stack_pos ==
        ((symbol_table->member_stack_size / sizeof(TP_WASM_STACK_ELEMENT)) - 1)){

        int32_t wasm_stack_size_allocate_unit =
            symbol_table->member_stack_size_allocate_unit * sizeof(TP_WASM_STACK_ELEMENT);

        int32_t wasm_stack_size = symbol_table->member_stack_size + wasm_stack_size_allocate_unit;

        if (symbol_table->member_stack_size > wasm_stack_size){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: symbol_table->member_stack_size(%1) > wasm_stack_size(%2)"),
                TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_stack_size),
                TP_LOG_PARAM_UINT64_VALUE(wasm_stack_size)
            );

            goto fail;
        }

        TP_WASM_STACK_ELEMENT* wasm_stack = (TP_WASM_STACK_ELEMENT*)TP_REALLOC(
            symbol_table, symbol_table->member_stack, wasm_stack_size
        );

        if (NULL == wasm_stack){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        memset(
            ((uint8_t*)wasm_stack) + symbol_table->member_stack_size, 0,
            wasm_stack_size_allocate_unit
        );

        symbol_table->member_stack = wasm_stack;
        symbol_table->member_stack_size = wasm_stack_size;
    }

    ++(symbol_table->member_stack_pos);

    symbol_table->member_stack[symbol_table->member_stack_pos] = *value;

    return true;

fail:

    if (symbol_table->member_stack){

        TP_FREE(symbol_table, &(symbol_table->member_stack), symbol_table->member_stack_size);
    }

    symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
    symbol_table->member_stack_size = 0;

    return false;
}

static TP_WASM_STACK_ELEMENT wasm_stack_pop(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_POP_MODE pop_mode)
{
    TP_WASM_STACK_ELEMENT result = { 0 };

    if (wasm_stack_and_wasm_code_is_empty(symbol_table)){

        return result;
    }

    if (TP_WASM_STACK_EMPTY < symbol_table->member_stack_pos){

        if (TP_WASM_STACK_POP_MODE_PARAM == pop_mode){

            result = symbol_table->member_stack[symbol_table->member_stack_pos];

            --(symbol_table->member_stack_pos);

            return result;
        }else if (TP_WASM_STACK_POP_MODE_LABEL == pop_mode){

            TP_WASM_STACK_ELEMENT control_op = wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM);
            uint32_t relative_depth = 0;

            switch (control_op.member_wasm_opcode){
            case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32
                relative_depth = control_op.member_relative_depth;
                ++relative_depth;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            int32_t relative_depth_count = 0;

            for (int32_t i = symbol_table->member_stack_pos; 0 <= i; --i){

                if (TP_WASM_OPCODE_LOOP_LABEL != symbol_table->member_stack[i].member_wasm_opcode){

                    continue;
                }

                ++relative_depth_count;

                if (relative_depth_count != relative_depth){

                    continue;
                }

                result = symbol_table->member_stack[i];

                return result;
            }

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }
    }

    result.member_wasm_opcode = symbol_table->member_wasm_code_body_buffer[symbol_table->member_wasm_code_body_pos];

    TP_WASM_RELOCATION* wasm_relocation = NULL;

    if ( ! tp_wasm_get_relocation(symbol_table, symbol_table->member_wasm_code_body_pos, &wasm_relocation)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    result.member_wasm_relocation = wasm_relocation;

    ++(symbol_table->member_wasm_code_body_pos);

    switch (result.member_wasm_opcode){
    // Control flow operators
    case TP_WASM_OPCODE_LOOP: // sig: block_type
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_LOOP", result, result.member_sig);
        break;
    case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_BR_IF", result, result.member_relative_depth);
        break;

    // Call operators
    case TP_WASM_OPCODE_CALL_INDIRECT: // type_index: varuint32, reserved: varuint1
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_CALL_INDIRECT", result, result.member_type_index);
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_CALL_INDIRECT", result, result.member_reserved);
        break;

    // Variable access
    case TP_WASM_OPCODE_GET_LOCAL:
//      break;
    case TP_WASM_OPCODE_SET_LOCAL:
//      break;
    case TP_WASM_OPCODE_TEE_LOCAL:
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_GET/SET/TEE_LOCAL", result, result.member_local_index);
        break;
    case TP_WASM_OPCODE_GET_GLOBAL: // global_index: varuint32
//      break;
    case TP_WASM_OPCODE_SET_GLOBAL: // global_index: varuint32
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_GET/SET_GLOBAL", result, result.member_global_index);
        break;

    // Memory-related operators
    case TP_WASM_OPCODE_I32_LOAD: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I32_STORE: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I64_LOAD: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I64_STORE: // flags: varuint32, offset: varuint32
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_I32/I64_LOAD/STORE", result, result.member_flags);
        TP_WASM_STACK_POP_U_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_I32/I64_LOAD/STORE", result, result.member_mem_offset);
        break;

    // Constants
    case TP_WASM_OPCODE_I32_CONST: // value: varint32
        TP_WASM_STACK_POP_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_I32_CONST", result, result.member_i32);
        break;
    case TP_WASM_OPCODE_I64_CONST: // value: varint64
        TP_WASM_STACK_POP_DECODE(symbol_table, "ERROR: TP_WASM_OPCODE_I64_CONST", result, result.member_i64);
        break;
    default:
        break;
    }

    return result;

fail:

    memset(&result, 0, sizeof(result));

    return result;
}

bool tp_get_local_variable_offset(
    TP_SYMBOL_TABLE* symbol_table, uint32_t local_index, int32_t* local_variable_offset)
{
    if (local_index >= symbol_table->member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    int32_t offset = (int32_t)(symbol_table->member_local_types[local_index].member_offset);

    if (symbol_table->member_local_variable_size_max < offset){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: symbol_table->member_local_variable_size_max(%1) < offset(%2)"),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_local_variable_size_max),
            TP_LOG_PARAM_INT32_VALUE(offset)
        );

        return false;
    }

    *local_variable_offset = offset;

    return true;
}

bool tp_allocate_temporary_variable(
    TP_SYMBOL_TABLE* symbol_table, TP_X64_ALLOCATE_MODE allocate_mode,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_WASM_STACK_ELEMENT* wasm_stack_element)
{
    int32_t offset =
          symbol_table->member_local_variable_size
        + symbol_table->member_padding_local_variable_bytes
        + symbol_table->member_temporary_variable_size;

    int32_t max_size = 
        symbol_table->member_temporary_variable_size_max + symbol_table->member_local_variable_size_max;

    int32_t size = 0;

    if (TP_WASM_OPCODE_I64_VALUE == wasm_stack_element->member_wasm_opcode){

        size = (int32_t)sizeof(int64_t);
    }else{

        size = (int32_t)sizeof(int32_t);
    }
    
    if (max_size < (offset + size)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: max_size: %1 < (offset + size): %2"),
            TP_LOG_PARAM_INT32_VALUE(max_size),
            TP_LOG_PARAM_INT32_VALUE(offset + size)
        );

        return false;
    }

    wasm_stack_element->member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_TEMP;

    wasm_stack_element->member_offset = offset;

    wasm_stack_element->member_is_stack_base = false;

    symbol_table->member_temporary_variable_size += size;
    
    if ( ! allocate_variable_common(symbol_table, allocate_mode,
        x64_code_buffer, x64_code_offset, x64_code_size, arg_index, wasm_stack_element)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool allocate_variable_common(
    TP_SYMBOL_TABLE* symbol_table, TP_X64_ALLOCATE_MODE allocate_mode,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_WASM_STACK_ELEMENT* wasm_stack_element)
{
    switch (allocate_mode){
    case TP_X64_ALLOCATE_DEFAULT:
        break;
    case TP_X64_ALLOCATE_MEMORY:
        if (TP_WASM_ARG_INDEX_VOID != arg_index){
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        wasm_stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        return true;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    bool is_zero_free_register = true;

    TP_X86_32_REGISTER x86_32_register = TP_X86_32_REGISTER_NULL;
    TP_X64_32_REGISTER x64_32_register = TP_X64_32_REGISTER_NULL;

    if ( ! get_free_register(
        symbol_table, arg_index, x64_code_buffer, x64_code_offset, x64_code_size,
        &x86_32_register, &x64_32_register, &is_zero_free_register)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_zero_free_register){

        if ( ! spilling_variable(
            symbol_table, x64_code_buffer, x64_code_offset, x64_code_size,
            arg_index, &x86_32_register, &x64_32_register, wasm_stack_element)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if (TP_X86_32_REGISTER_NULL != x86_32_register){

        wasm_stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
        wasm_stack_element->member_x64_item.member_x86_32_register = x86_32_register;

        symbol_table->member_use_X86_32_register[x86_32_register] = *wasm_stack_element;

    }else if (TP_X64_32_REGISTER_NULL != x64_32_register){

        wasm_stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
        wasm_stack_element->member_x64_item.member_x64_32_register = x64_32_register;

        symbol_table->member_use_X64_32_register[x64_32_register] = *wasm_stack_element;
    }else{

        return true;
    }
    
    if ( ! set_nv_register(symbol_table, x86_32_register, x64_32_register)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool get_free_register(
    TP_SYMBOL_TABLE* symbol_table, int32_t arg_index,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    TP_X86_32_REGISTER* x86_32_register, TP_X64_32_REGISTER* x64_32_register, bool* is_zero_free_register)
{
    switch (arg_index){
    case TP_WASM_ARG_INDEX_VOID:
        break;
    case TP_WASM_ARG_INDEX_EAX:
        *is_zero_free_register = false;
        *x86_32_register = TP_X86_32_REGISTER_EAX;
        return true;
    case 0:{
        TP_WASM_STACK_ELEMENT* use_x86_32_register =
            &(symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_ECX]);

        if (TP_X64_ITEM_KIND_X86_32_REGISTER == use_x86_32_register->member_x64_item_kind){

            // PUSH – Push Operand onto the Stack
            uint32_t push_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + *x64_code_size, TP_X64_64_REGISTER_RCX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, *x64_code_size, push_code_size);
            symbol_table->member_use_v_register[TP_X64_V64_REGISTER_RCX_INDEX] = TP_X64_V64_REGISTER_RCX;
        }
        *is_zero_free_register = false;
        *x86_32_register = TP_X86_32_REGISTER_ECX;
        return true;
    }
    case 1:{
        TP_WASM_STACK_ELEMENT* use_x86_32_register =
            &(symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX]);

        if (TP_X64_ITEM_KIND_X86_32_REGISTER == use_x86_32_register->member_x64_item_kind){

            // PUSH – Push Operand onto the Stack
            uint32_t push_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + *x64_code_size, TP_X64_64_REGISTER_RDX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, *x64_code_size, push_code_size);
            symbol_table->member_use_v_register[TP_X64_V64_REGISTER_RDX_INDEX] = TP_X64_V64_REGISTER_RDX;
        }
        *is_zero_free_register = false;
        *x86_32_register = TP_X86_32_REGISTER_EDX;
        return true;
    }
    case 2:{
        TP_WASM_STACK_ELEMENT* use_x64_32_register = &(symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R8D]);

        if (TP_X64_ITEM_KIND_X64_32_REGISTER == use_x64_32_register->member_x64_item_kind){

            // PUSH – Push Operand onto the Stack
            uint32_t push_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + *x64_code_size, TP_X64_64_REGISTER_R8
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, *x64_code_size, push_code_size);
            symbol_table->member_use_v_register[TP_X64_V64_REGISTER_R8_INDEX] = TP_X64_V64_REGISTER_R8;
        }
        *is_zero_free_register = false;
        *x64_32_register = TP_X64_32_REGISTER_R8D;
        return true;
    }
    case 3:{
        TP_WASM_STACK_ELEMENT* use_x64_32_register = &(symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R9D]);

        if (TP_X64_ITEM_KIND_X64_32_REGISTER == use_x64_32_register->member_x64_item_kind){

            // PUSH – Push Operand onto the Stack
            uint32_t push_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + *x64_code_size, TP_X64_64_REGISTER_R9
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, *x64_code_size, push_code_size);
            symbol_table->member_use_v_register[TP_X64_V64_REGISTER_R9_INDEX] = TP_X64_V64_REGISTER_R9;
        }
        *is_zero_free_register = false;
        *x64_32_register = TP_X64_32_REGISTER_R9D;
        return true;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    uint32_t types_num =
        ((4 < symbol_table->member_local_types_num) ? 4 : symbol_table->member_local_types_num);

    for (uint32_t i = 0; TP_X86_32_REGISTER_NUM > i; ++i){

        TP_WASM_STACK_ELEMENT* use_x86_32_register = &(symbol_table->member_use_X86_32_register[i]);

        if (TP_X64_ITEM_KIND_X86_32_REGISTER != use_x86_32_register->member_x64_item_kind){

            if ((TP_X86_32_REGISTER_ESP == i) || (TP_X86_32_REGISTER_EBP == i)){

                continue;
            }

            *is_zero_free_register = false;

            *x86_32_register = (TP_X86_32_REGISTER)i;

            break;
        }
    }

    if (*is_zero_free_register){

        for (uint32_t i = 0; TP_X64_32_REGISTER_NUM > i; ++i){

            TP_WASM_STACK_ELEMENT* use_x64_32_register = &(symbol_table->member_use_X64_32_register[i]);

            if (TP_X64_ITEM_KIND_X64_32_REGISTER != use_x64_32_register->member_x64_item_kind){

                *is_zero_free_register = false;

                *x64_32_register = (TP_X64_32_REGISTER)i;

                break;
            }
        }
    }

    return true;
}

static bool spilling_variable(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_X86_32_REGISTER* x86_32_register, TP_X64_32_REGISTER* x64_32_register,
    TP_WASM_STACK_ELEMENT* wasm_stack_element)
{
    TP_X86_32_REGISTER spill_x86_32_register = TP_X86_32_REGISTER_NULL;
    TP_X64_32_REGISTER spill_x64_32_register = TP_X64_32_REGISTER_NULL;

    if ( ! wasm_stack_is_empty(symbol_table)){

        bool is_zero_free_register_in_wasm_stack = true;

        if ( ! get_free_register_in_wasm_stack(
            symbol_table, arg_index, &spill_x86_32_register, &spill_x64_32_register,
            &is_zero_free_register_in_wasm_stack)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_zero_free_register_in_wasm_stack){

            wasm_stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;

            return true;
        }
    }

    if ((TP_X86_32_REGISTER_NULL == spill_x86_32_register) &&
        (TP_X64_32_REGISTER_NULL == spill_x64_32_register)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING(
                "ERROR: (TP_X86_32_REGISTER_NULL == spill_x86_32_register) && "
                "(TP_X64_32_REGISTER_NULL == spill_x64_32_register)"
            )
        );

        return false;
    }

    TP_WASM_STACK_ELEMENT dst = { 0 };
    TP_WASM_STACK_ELEMENT* src = NULL;

    if (TP_X86_32_REGISTER_NULL != spill_x86_32_register){

        dst = symbol_table->member_use_X86_32_register[spill_x86_32_register];
        src = &(symbol_table->member_use_X86_32_register[spill_x86_32_register]);
    }else if (TP_X64_32_REGISTER_NULL != spill_x64_32_register){

        dst = symbol_table->member_use_X64_32_register[spill_x64_32_register];
        src = &(symbol_table->member_use_X64_32_register[spill_x64_32_register]);
    }else{

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT(
                "ERROR: (false == (TP_X86_32_REGISTER_NULL != spill_x86_32_register: %1)) &&"
                "(false == (TP_X64_32_REGISTER_NULL != spill_x64_32_register: %2))"
            ),
            TP_LOG_PARAM_INT32_VALUE(spill_x86_32_register),
            TP_LOG_PARAM_INT32_VALUE(spill_x64_32_register)
        );

        return false;
    }

    dst.member_wasm_opcode = wasm_stack_element->member_wasm_opcode;
    dst.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;

    src->member_wasm_opcode = wasm_stack_element->member_wasm_opcode;

    // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
    uint32_t mov_code_size = tp_encode_x64_2_operand(
        symbol_table, x64_code_buffer, x64_code_offset,
        TP_X64_MOV, &dst, src
    );

    TP_X64_CHECK_CODE_SIZE(symbol_table, *x64_code_size, mov_code_size);

    src->member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;

    return true;
}

static bool get_free_register_in_wasm_stack(
    TP_SYMBOL_TABLE* symbol_table, int32_t arg_index,
    TP_X86_32_REGISTER* free_x86_32_register, TP_X64_32_REGISTER* free_x64_32_register, bool* is_zero_free_register_in_wasm_stack)
{
    bool x86_32_register[TP_X86_32_REGISTER_NUM] = { false };
    bool x64_32_register[TP_X64_32_REGISTER_NUM] = { false };

    for (size_t i = 0; symbol_table->member_stack_size > i; ++i){

        TP_WASM_STACK_ELEMENT* wasm_stack_element = &(symbol_table->member_stack[i]);

        switch (wasm_stack_element->member_x64_item_kind){
        case TP_X64_ITEM_KIND_X86_32_REGISTER:{

            TP_X86_32_REGISTER temp_x86_32_register = wasm_stack_element->member_x64_item.member_x86_32_register;

            if (TP_X86_32_REGISTER_NULL <= temp_x86_32_register){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_NULL <= temp_x86_32_register(%1)"),
                    TP_LOG_PARAM_INT32_VALUE(temp_x86_32_register)
                );

                return false;
            }

            x86_32_register[temp_x86_32_register] = true;

            break;
        }
        case TP_X64_ITEM_KIND_X64_32_REGISTER:{

            TP_X64_32_REGISTER temp_x64_32_register = wasm_stack_element->member_x64_item.member_x64_32_register;

            if (TP_X64_32_REGISTER_NULL <= temp_x64_32_register){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: TP_X64_32_REGISTER_NULL <= temp_x64_32_register(%1)"),
                    TP_LOG_PARAM_INT32_VALUE(temp_x64_32_register)
                );

                return false;
            }

            x64_32_register[temp_x64_32_register] = true;

            break;
        }
        default:
            break;
        }
    }

    bool is_zero_free_x86_32_register = true;

    const int32_t bad_index = -1;
    int32_t free_x86_32_index = bad_index;

    for (int32_t i = 0; TP_X86_32_REGISTER_NUM > i; ++i){

        if (false == x86_32_register[i]){

            is_zero_free_x86_32_register = false;

            free_x86_32_index = i;

            break;
        }
    }

    bool is_zero_free_x64_32_register = true;
    int32_t free_x64_32_index = bad_index;

    for (int32_t i = 0; TP_X64_32_REGISTER_NUM > i; ++i){

        if (false == x64_32_register[i]){

            is_zero_free_x64_32_register = false;

            free_x64_32_index = i;

            break;
        }
    }

    if (is_zero_free_x86_32_register && is_zero_free_x64_32_register){

        *is_zero_free_register_in_wasm_stack = true;

        return true;
    }

    if ((false == is_zero_free_x86_32_register) && (bad_index != free_x86_32_index)){

        *free_x86_32_register = free_x86_32_index;

        return true;
    }

    if ((false == is_zero_free_x64_32_register) && (bad_index != free_x64_32_index)){

        *free_x64_32_register = free_x64_32_index;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: at %1"), TP_LOG_PARAM_STRING(__func__)
    );

    return false;
}

static bool set_nv_register(
    TP_SYMBOL_TABLE* symbol_table,
    TP_X86_32_REGISTER x86_32_register, TP_X64_32_REGISTER x64_32_register)
{
    switch (x86_32_register){
    case TP_X86_32_REGISTER_EBX:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_RBX_INDEX] = TP_X64_NV64_REGISTER_RBX;
        break;
    case TP_X86_32_REGISTER_ESI:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_RSI_INDEX] = TP_X64_NV64_REGISTER_RSI;
        break;
    case TP_X86_32_REGISTER_EDI:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_RDI_INDEX] = TP_X64_NV64_REGISTER_RDI;
        break;
    default:
        break;
    }

    switch (x64_32_register){
    case TP_X64_32_REGISTER_R12D:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_R12_INDEX] = TP_X64_NV64_REGISTER_R12;
        break;
    case TP_X64_32_REGISTER_R13D:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_R13_INDEX] = TP_X64_NV64_REGISTER_R13;
        break;
    case TP_X64_32_REGISTER_R14D:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_R14_INDEX] = TP_X64_NV64_REGISTER_R14;
        break;
    case TP_X64_32_REGISTER_R15D:
        symbol_table->member_use_nv_register[TP_X64_NV64_REGISTER_R15_INDEX] = TP_X64_NV64_REGISTER_R15;
        break;
    default:
        break;
    }

    return true;
}

bool tp_free_register(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* stack_element)
{
    uint32_t types_num =
        ((4 < symbol_table->member_local_types_num) ? 4 : symbol_table->member_local_types_num);

    switch (stack_element->member_x64_item_kind){
    case TP_X64_ITEM_KIND_X86_32_REGISTER:{

x86_32:
        ;
        TP_X86_32_REGISTER x86_32_register = stack_element->member_x64_item.member_x86_32_register;

        if (TP_X86_32_REGISTER_NULL <= x86_32_register){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_NULL <= x86_32_register(%1)"),
                TP_LOG_PARAM_INT32_VALUE(x86_32_register)
            );

            return false;
        }

        if ((TP_X86_32_REGISTER_EAX == x86_32_register) ||
            (TP_X86_32_REGISTER_ESP == x86_32_register) ||
            (TP_X86_32_REGISTER_EBP == x86_32_register)){

            return true;
        }

        for (uint32_t i = 0; types_num > i; ++i){

            switch (symbol_table->member_local_types[i].member_type){
            case TP_WASM_VAR_TYPE_PSEUDO_I32:
    //          break;
            case TP_WASM_VAR_TYPE_PSEUDO_I64:
                switch (i){
                case 0:
                    if (TP_X86_32_REGISTER_ECX == x86_32_register){

                        return true;
                    }
                    break;
                case 1:
                    if (TP_X86_32_REGISTER_EDX == x86_32_register){

                        return true;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        symbol_table->member_use_X86_32_register[x86_32_register].member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        symbol_table->member_use_X86_32_register[x86_32_register].member_x64_item.member_x86_32_register
            = TP_X86_32_REGISTER_NULL;

        stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        stack_element->member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_NULL;
        break;
    }
    case TP_X64_ITEM_KIND_X64_32_REGISTER:{

x64_32:
        ;
        TP_X64_32_REGISTER x64_32_register = stack_element->member_x64_item.member_x64_32_register;

        if (TP_X86_32_REGISTER_NULL <= x64_32_register){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_NULL <= x64_32_register(%1)"),
                TP_LOG_PARAM_INT32_VALUE(x64_32_register)
            );

            return false;
        }

        for (uint32_t i = 0; types_num > i; ++i){

            switch (symbol_table->member_local_types[i].member_type){
            case TP_WASM_VAR_TYPE_PSEUDO_I32:
//              break;
            case TP_WASM_VAR_TYPE_PSEUDO_I64:
                switch (i){
                case 2:
                    if (TP_X64_32_REGISTER_R8D == x64_32_register){

                        return true;
                    }
                    break;
                case 3:
                    if (TP_X64_32_REGISTER_R9D == x64_32_register){

                        return true;
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        symbol_table->member_use_X64_32_register[x64_32_register].member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        symbol_table->member_use_X86_32_register[x64_32_register].member_x64_item.member_x64_32_register
            = TP_X64_32_REGISTER_NULL;

        stack_element->member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        stack_element->member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL;
        break;
    }
    case TP_X64_ITEM_KIND_MEMORY:

        if (TP_X64_ITEM_MEMORY_KIND_GLOBAL == stack_element->member_x64_memory_kind){

            stack_element->member_offset = stack_element->member_offset_backup;
            stack_element->member_offset_backup = 0;
            stack_element->member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_TEMP;

            switch (stack_element->member_x64_memory_item_kind){
            case TP_X64_ITEM_KIND_X86_32_REGISTER:
                goto x86_32;
            case TP_X64_ITEM_KIND_X64_32_REGISTER:
                goto x64_32;
            default:
                break;
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

