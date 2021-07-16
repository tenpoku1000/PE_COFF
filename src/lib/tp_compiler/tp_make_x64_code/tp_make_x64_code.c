
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_WASM_STACK_POP_U_DECODE(symbol_table, format_string, result, result_param, offset) \
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
\
    if (offset){ \
\
        (*(offset)) += param_size; \
    } \
}

#define TP_WASM_STACK_POP_DECODE(symbol_table, format_string, result, result_param, offset) \
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
\
    if (offset){ \
\
        (*(offset)) += param_size; \
    } \
}

static uint32_t convert_section_code_content2x64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION* code_section, uint32_t return_type, uint8_t* x64_code_buffer
);
static uint32_t calc_padding_and_size(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    uint8_t* x64_code_buffer, uint32_t x64_code_size, TP_X64_FUNC_LOCAL* x64_func_local
);
static uint32_t convert_section_code_content2x64_body_local_var(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    TP_WASM_MODULE_SECTION* code_section, uint8_t* x64_code_buffer, uint32_t x64_code_size,
    uint8_t* payload, uint32_t* offset
);
static bool get_wasm_type_index_function_index_max(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    rsize_t* wasm_type_index, rsize_t* wasm_function_index_max
);
static bool wasm_stack_and_use_register_init(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* wasm_code_body_buffer, uint32_t wasm_code_body_size,
    uint8_t* x64_code_buffer, rsize_t code_index
);
static bool preset_user_register_of_function_args(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, rsize_t code_index
);
static bool wasm_stack_and_wasm_code_is_empty(TP_SYMBOL_TABLE* symbol_table);
static bool wasm_stack_is_empty(TP_SYMBOL_TABLE* symbol_table);
static bool wasm_pop_function_args(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t* x64_code_offset,
    bool is_function_index, rsize_t function_or_type_index
);
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
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* entry_point_symbol, int32_t* return_value)
{
    bool status = false;
    bool is_pe_coff = symbol_table->member_is_output_pe_coff_file;

    symbol_table->member_func_local_types_current = 0;

    uint8_t* x64_code_buffer = NULL;
    uint32_t x64_code_buffer_size1 = 0;
    uint32_t x64_code_buffer_size2 = 0;

    uint8_t* pe_buffer = NULL;
    rsize_t pe_buffer_size = 0;

    TP_WASM_MODULE_SECTION* code_section = NULL;
    uint32_t return_type = 0;

    if ( ! tp_get_wasm_export_code_section(
        symbol_table, entry_point_symbol, &code_section, &return_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

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

    uint8_t* tmp_x64_code_buffer = (uint8_t*)VirtualAlloc(
        NULL, x64_code_buffer_size1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
    );

    if (NULL == tmp_x64_code_buffer){

        TP_GET_LAST_ERROR(symbol_table);

        goto fail;
    }

    x64_code_buffer = tmp_x64_code_buffer;

    memset(x64_code_buffer, TP_X64_OPCODE_NOP, x64_code_buffer_size1);

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

    if ((NULL == symbol_table->member_wasm_relocation) || (false == is_pe_coff)){

        if ((false == symbol_table->member_is_no_output_files) ||
            (symbol_table->member_is_no_output_files &&
            symbol_table->member_is_output_x64_file)){

            if ( ! tp_write_file(
                symbol_table, symbol_table->member_x64_file_path,
                x64_code_buffer, x64_code_buffer_size1)){

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
    }

    if (x64_code_buffer_size1 != x64_code_buffer_size2){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: x64_code_buffer_size1(%1) != x64_code_buffer_size2(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(x64_code_buffer_size1),
            TP_LOG_PARAM_UINT64_VALUE(x64_code_buffer_size2)
        );

        goto fail;
    }

    if (symbol_table->member_wasm_relocation){

        if ( ! tp_make_COFF(
            symbol_table, x64_code_buffer, x64_code_buffer_size1, is_pe_coff)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        memset(x64_code_buffer, 0, x64_code_buffer_size1);

        if ( ! VirtualFree(x64_code_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);

            x64_code_buffer = NULL;

            goto fail;
        }

        x64_code_buffer = NULL;

        if (is_pe_coff && (false == symbol_table->member_is_test_mode)){

            TP_FREE(symbol_table, &(symbol_table->member_pe_coff_buffer), symbol_table->member_pe_coff_size);

            if ( ! tp_make_PE_file(
                symbol_table, entry_point_symbol, &is_pe_coff)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            if ( ! tp_make_PE_file(symbol_table, NULL, NULL)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
        }else{

            if ( ! tp_make_PE_file(symbol_table, entry_point_symbol, NULL)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            // NOTE: Call VirtualAlloc() in tp_load_dll().
            if ( ! tp_load_dll(symbol_table, &pe_buffer, &pe_buffer_size)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            x64_code_buffer = pe_buffer;
        }
    }

    if ((false == is_pe_coff) || (symbol_table->member_is_test_mode)){

        switch (return_type){
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32:
            break;
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64:
//          break;
        case TP_WASM_RETURN_VOID:
//          break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        uint8_t* code_page = NULL;
        uint8_t* entry_point = NULL;

        if (symbol_table->member_wasm_relocation){

            TP_PE_HEADER64_WRITE* pe_header64_write = (TP_PE_HEADER64_WRITE*)x64_code_buffer;

            if (NULL == pe_header64_write){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                goto fail;
            }

            code_page = x64_code_buffer + pe_header64_write->OptionalHeader.BaseOfCode;
            entry_point = x64_code_buffer + pe_header64_write->OptionalHeader.AddressOfEntryPoint;
        }else{

            code_page = x64_code_buffer;
            entry_point = x64_code_buffer;
        }

        if (NULL == entry_point){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        DWORD old_protect = 0;

        if ( ! VirtualProtect(code_page, x64_code_buffer_size1, PAGE_EXECUTE_READ, &old_protect)){

            TP_GET_LAST_ERROR(symbol_table);

            goto fail;
        }

        x64_jit_func func = (x64_jit_func)entry_point;

        int value = func();

        TP_PUT_LOG_PRINT(
            symbol_table, TP_MSG_FMT("x64_jit_func() = %1"),
            TP_LOG_PARAM_INT32_VALUE(value)
        );

        if (return_value){

            *return_value = value;
        }

        errno_t err = _set_errno(0);

        if ( ! VirtualProtect(code_page, x64_code_buffer_size1, PAGE_READWRITE, &old_protect)){

            TP_GET_LAST_ERROR(symbol_table);

            goto fail;
        }
    }

    status = true;

fail:
    ;

    if (pe_buffer){

        memset(pe_buffer, 0, pe_buffer_size);

        if ( ! VirtualFree(pe_buffer, 0, MEM_RELEASE)){

            TP_GET_LAST_ERROR(symbol_table);
        }

        pe_buffer = NULL;
        pe_buffer_size = 0;
    }else{

        if (x64_code_buffer){

            memset(x64_code_buffer, 0, x64_code_buffer_size1);

            if ( ! VirtualFree(x64_code_buffer, 0, MEM_RELEASE)){

                TP_GET_LAST_ERROR(symbol_table);
            }

            x64_code_buffer = NULL;
            x64_code_buffer_size1 = 0;
            x64_code_buffer_size2 = 0;
        }
    }

    return status;
}

static uint32_t convert_section_code_content2x64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_WASM_MODULE_SECTION* code_section, uint32_t return_type, uint8_t* x64_code_buffer)
{
    uint32_t x64_code_size = 0;

    uint8_t* payload = code_section->member_name_len_name_payload_data;
    uint32_t offset = 0;

    TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, code_section, payload, offset, TP_WASM_SECTION_TYPE_CODE);

    uint32_t payload_len = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, payload_len);

    uint32_t code_count = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, code_count);

    if (0 == code_count){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == code_count")
        );

        goto fail;
    }

    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    if (symbol_table->member_wasm_function_count < code_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    symbol_table->member_func_local_types_current = 0;
    symbol_table->member_x64_func_index = 0;

    symbol_table->member_wasm_relocation_cur = 0;

    for (uint32_t i = 0; code_count > i; ++i, ++(symbol_table->member_x64_func_index)){

        uint32_t x64_code_offset = x64_code_size;

        symbol_table->member_x64_call_num = 0;

        // Stack depth of control flow operators.
        int64_t wasm_stack_depth_of_control_flow = 1;

        char* string_x64_code_buffer =
            (x64_code_buffer ? "NULL != x64_code_buffer" : "NULL == x64_code_buffer");

        uint64_t v = (uint64_t)i;

        TP_PUT_LOG_MSG(
            (symbol_table), TP_LOG_TYPE_HIDE,
            TP_MSG_FMT("START x64 code gen.(%1 function: %2): (%3), (%4/%5)"),
            TP_LOG_PARAM_STRING(__func__), TP_LOG_PARAM_UINT64_VALUE(__LINE__),
            TP_LOG_PARAM_STRING(string_x64_code_buffer),
            TP_LOG_PARAM_UINT64_VALUE(v + 1),
            TP_LOG_PARAM_UINT64_VALUE(code_count)
        );

        // Local variables.
        uint32_t tmp_x64_code_size = convert_section_code_content2x64_body_local_var(
            symbol_table, i, code_section, x64_code_buffer, x64_code_size, payload, &offset
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[i]);
        x64_func_local->member_temporary_variable_size = 0;

        // Loop of x64 code gen.
        do{
            bool status = false;

            TP_WASM_STACK_ELEMENT label = { 0 };
            TP_WASM_STACK_ELEMENT op1 = { 0 };
            TP_WASM_STACK_ELEMENT op2 = { 0 };

            TP_WASM_STACK_ELEMENT opcode = tp_wasm_stack_pop(
                symbol_table, TP_WASM_STACK_POP_MODE_DEFAULT, &offset
            );

            if (opcode.member_wasm_relocation){

                TP_WASM_RELOCATION_TYPE wasm_relocation_type = opcode.member_wasm_relocation->member_type;

                if ((TP_WASM_RELOCATION_TYPE_PROLOGUE == wasm_relocation_type) ||
                    (TP_WASM_RELOCATION_TYPE_EPILOGUE == wasm_relocation_type) ||
                    (TP_WASM_RELOCATION_TYPE_SKIP == wasm_relocation_type)){

                    continue;
                }

                if ((TP_WASM_RELOCATION_TYPE_BASE == wasm_relocation_type) ||
                    (TP_WASM_RELOCATION_TYPE_BASE_MOV_LESS == wasm_relocation_type)){

                    tmp_x64_code_size = tp_encode_relocation_type_base(
                        symbol_table, x64_code_buffer, x64_code_size,
                        opcode.member_wasm_relocation, &status
                    );
                    TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);
                    continue;
                }
            }

            if (0 >= wasm_stack_depth_of_control_flow){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: accessing stack depth <= 0(wasm_opcode: %1)"),
                    TP_LOG_PARAM_UINT64_VALUE(opcode.member_wasm_opcode)
                );
                goto fail;
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
                ++wasm_stack_depth_of_control_flow;
                break;
            case TP_WASM_OPCODE_END:
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);

                if (wasm_stack_and_wasm_code_is_empty(symbol_table)){

                    TP_WASM_STACK_ELEMENT dst = {
                        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
                        .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                        .member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX
                    };

                    // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
                    tmp_x64_code_size = tp_encode_x64_2_operand(
                        symbol_table, x64_code_buffer, x64_code_size,
                        TP_X64_MOV, &dst, &op1
                    );
                    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

                    tmp_x64_code_size = tp_encode_return_code(
                        symbol_table, x64_code_buffer, x64_code_size, opcode.member_wasm_relocation
                    );
                    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

                    uint32_t tmp_x64_code_size = calc_padding_and_size(
                        symbol_table, i, x64_code_buffer, x64_code_size, x64_func_local
                    );
                    TP_X64_CHECK_CODE_ASSIGN(symbol_table, x64_code_size, tmp_x64_code_size);

                    if ( ! tp_set_function_offset_to_symbol(symbol_table, i, x64_code_size, x64_code_offset)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);
                        goto fail;
                    }

                    goto next;
                }else{

                   if (0 == wasm_stack_depth_of_control_flow){

                        TP_PUT_LOG_MSG(
                            symbol_table, TP_LOG_TYPE_DISP_FORCE,
                            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: TP_WASM_OPCODE_END(accessing stack depth = 0)")
                        );
                        goto fail;
                    }
                    --wasm_stack_depth_of_control_flow;
                }
                break;
            case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32
                if (opcode.member_relative_depth >= wasm_stack_depth_of_control_flow){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("ERROR: TP_WASM_OPCODE_BR_IF(accessing stack depth <= %1)"),
                        TP_LOG_PARAM_UINT64_VALUE(wasm_stack_depth_of_control_flow)
                    );
                    goto fail;
                }
                wasm_stack_depth_of_control_flow -= opcode.member_relative_depth;

                if ( ! tp_wasm_stack_push(symbol_table, &opcode)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }

                label = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_LABEL, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
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

                if ((symbol_table->member_wasm_code_body_pos + 1) >= symbol_table->member_wasm_code_body_size_current){

                    ++(symbol_table->member_wasm_relocation_cur);
                    ++offset;

                    uint32_t tmp_x64_code_size = calc_padding_and_size(
                        symbol_table, i, x64_code_buffer, x64_code_size, x64_func_local
                    );
                    TP_X64_CHECK_CODE_ASSIGN(symbol_table, x64_code_size, tmp_x64_code_size);

                    if ( ! tp_set_function_offset_to_symbol(symbol_table, i, x64_code_size, x64_code_offset)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);
                        goto fail;
                    }

                    goto next;
                }
                break;

            // Call operators
            case TP_WASM_OPCODE_CALL: // function_index: varuint32
                op1 =  (TP_WASM_STACK_ELEMENT){ 0 };
                if ( ! wasm_pop_function_args(
                    symbol_table, x64_code_buffer, &x64_code_size,
                    true/* is_function_index */, opcode.member_function_index)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
                tmp_x64_code_size = tp_encode_call_indirect_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation,
                    opcode.member_function_index, 0, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_CALL_INDIRECT: // type_index: varuint32, reserved: varuint1
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                if ( ! wasm_pop_function_args(
                    symbol_table, x64_code_buffer, &x64_code_size,
                    false/* is_function_index */, opcode.member_function_index)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
                tmp_x64_code_size = tp_encode_call_indirect_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation,
                    opcode.member_type_index, opcode.member_reserved, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;

            // Parametric operators
            case TP_WASM_OPCODE_DROP:
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
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
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_set_local_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, opcode.member_local_index, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_TEE_LOCAL: // local_index: varuint32
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
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
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_set_global_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, opcode.member_global_index, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;

            // Memory-related operators
            case TP_WASM_OPCODE_I32_LOAD: // flags: varuint32, offset: varuint32
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_i32_load_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I32_STORE: // flags: varuint32, offset: varuint32
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_i32_store_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I64_LOAD: // flags: varuint32, offset: varuint32
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_i64_load_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, opcode.member_flags, opcode.member_mem_offset, &op1
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I64_STORE: // flags: varuint32, offset: varuint32
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
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
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_i64_ne_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;

            // Numeric operators(i32)
            // Numeric operators(i64)
            case TP_WASM_OPCODE_I32_ADD:
//              break;
            case TP_WASM_OPCODE_I64_ADD:
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_add_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I32_SUB:
//              break;
            case TP_WASM_OPCODE_I64_SUB:
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_sub_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I32_MUL:
//              break;
            case TP_WASM_OPCODE_I64_MUL:
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_mul_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I32_DIV_S:
//              break;
            case TP_WASM_OPCODE_I64_DIV_S:
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                tmp_x64_code_size = tp_encode_div_code(
                    symbol_table, x64_code_buffer, x64_code_size,
                    opcode.member_wasm_relocation, &op1, &op2
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            case TP_WASM_OPCODE_I32_XOR:
//              break;
            case TP_WASM_OPCODE_I64_XOR:
                op2 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                op1 = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
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
next:
        ++(symbol_table->member_func_local_types_current);
    }

    return x64_code_size;
fail:
    return 0;
}

static uint32_t calc_padding_and_size(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    uint8_t* x64_code_buffer, uint32_t x64_code_size, TP_X64_FUNC_LOCAL* x64_func_local)
{
    // Use non-volatile register.
    uint32_t nv_register_num = 0;
    uint32_t nv_rex_num = 0;

    for (rsize_t i = 0; TP_X64_NV64_REGISTER_NUM > i; ++i){

        TP_X64_NV64_REGISTER use_nv_register = x64_func_local->member_use_nv_register[i];

        if (TP_X64_NV64_REGISTER_NULL != use_nv_register){

            ++nv_register_num;

            if (TP_X64_NV64_REGISTER_R12 <= use_nv_register){

                ++nv_rex_num;
            }
        }
    }

    x64_func_local->member_register_bytes = (nv_register_num * sizeof(uint64_t));

    x64_func_local->member_padding_register_bytes =
        TP_X64_PADDING_16_BYTES_ALIGNMENT(x64_func_local->member_register_bytes);

    if (NULL == x64_code_buffer){

        // See tp_encode_allocate_stack function.
        // x64_code_size += TP_X64_NV64_REGISTER_NUM;
        nv_register_num = TP_X64_NV64_REGISTER_NUM - nv_register_num;

        TP_PUT_LOG_MSG(
            (symbol_table), TP_LOG_TYPE_HIDE,
            TP_MSG_FMT("Prev. x64_code_size -= nv_register_num; // push register."
                "(%1 function: %2): x64_code_size(%3), nv_register_num(%4)"),
            TP_LOG_PARAM_STRING(__func__), TP_LOG_PARAM_UINT64_VALUE(__LINE__),
            TP_LOG_PARAM_UINT64_VALUE(x64_code_size),
            TP_LOG_PARAM_UINT64_VALUE(nv_register_num)
        );

        x64_code_size -= (nv_register_num + nv_rex_num); // push register.
    }

    // Temporary variables.
    {
        int32_t v = x64_func_local->member_register_bytes +
            x64_func_local->member_padding_register_bytes +
            x64_func_local->member_temporary_variable_size;

        x64_func_local->member_padding_temporary_variable_bytes = TP_X64_PADDING_16_BYTES_ALIGNMENT(v);
    }

    // Local variables.
    {
        int32_t v = x64_func_local->member_register_bytes +
            x64_func_local->member_padding_register_bytes +
            x64_func_local->member_temporary_variable_size +
            x64_func_local->member_padding_temporary_variable_bytes +
            x64_func_local->member_local_variable_size;

        x64_func_local->member_padding_local_variable_bytes = TP_X64_PADDING_16_BYTES_ALIGNMENT(v);
    }

    // Use volatile register.
    if (NULL == x64_code_buffer){

        uint32_t v_register_num = 0;

        for (rsize_t j = 0; TP_X64_V64_REGISTER_NUM > j; ++j){

            TP_X64_V64_REGISTER use_v_register = x64_func_local->member_use_v_register[j];

            if (TP_X64_V64_REGISTER_NULL != use_v_register){

                if (TP_X64_V64_REGISTER_R8 <= use_v_register){

                    v_register_num += 2;
                }else{

                    ++v_register_num;
                }
            }
        }

        uint32_t v_register_op_size =
            ((v_register_num * symbol_table->member_x64_call_num) * 2);

        TP_PUT_LOG_MSG(
            (symbol_table), TP_LOG_TYPE_HIDE,
            TP_MSG_FMT("Prev. x64_code_size += v_register_op_size; // push/pop register."
                "(%1 function: %2): x64_code_size(%3), v_register_op_size(%4)"),
            TP_LOG_PARAM_STRING(__func__), TP_LOG_PARAM_UINT64_VALUE(__LINE__),
            TP_LOG_PARAM_UINT64_VALUE(x64_code_size),
            TP_LOG_PARAM_UINT64_VALUE(v_register_op_size)
        );

        // push/pop register.
        x64_code_size += v_register_op_size;
    }

    // Home space of function arguments register.
    const int32_t stack_param_size = (int32_t)(sizeof(uint64_t) * 4);

    x64_func_local->member_stack_imm32 =
        //      x64_func_local->member_register_bytes +
        x64_func_local->member_padding_register_bytes +
        x64_func_local->member_temporary_variable_size +
        x64_func_local->member_padding_temporary_variable_bytes +
        x64_func_local->member_local_variable_size +
        x64_func_local->member_padding_local_variable_bytes +
        stack_param_size;

    if (0 == ((x64_func_local->member_stack_imm32) % 16)){

        x64_func_local->member_last_padding_bytes = sizeof(uint64_t);
        x64_func_local->member_stack_imm32 += sizeof(uint64_t);
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_HIDE,
        TP_MSG_FMT(
            "x64_func_index: %1\n"
            "sub rsp, imm32\n"
            "x64_func_local->member_stack_imm32(member_register_bytes is not included): %2\n"
            "x64_func_local->member_register_bytes: %3\n"
            "x64_func_local->member_padding_register_bytes: %4\n"
            "x64_func_local->member_temporary_variable_size: %5\n"
            "x64_func_local->member_padding_temporary_variable_bytes: %6\n"
            "x64_func_local->member_local_variable_size: %7\n"
            "x64_func_local->member_padding_local_variable_bytes: %8\n"
            "x64_func_local->member_last_padding_bytes: %9\n"
            "stack_param_size: %10"
        ),
        TP_LOG_PARAM_UINT64_VALUE(code_index),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_stack_imm32),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_register_bytes),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_padding_register_bytes),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_temporary_variable_size),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_padding_temporary_variable_bytes),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_local_variable_size),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_padding_local_variable_bytes),
        TP_LOG_PARAM_INT32_VALUE(x64_func_local->member_last_padding_bytes),
        TP_LOG_PARAM_INT32_VALUE(stack_param_size)
    );

    uint32_t x64_padding_16_bytes_alignment =
        (uint32_t)TP_X64_PADDING_16_BYTES_ALIGNMENT(x64_code_size);

    TP_PUT_LOG_MSG(
        (symbol_table), TP_LOG_TYPE_HIDE,
        TP_MSG_FMT("Prev. x64_padding_16_bytes_alignment;"
            "(%1 function: %2): x64_code_size(%3), x64_padding_16_bytes_alignment(%4)"),
        TP_LOG_PARAM_STRING(__func__), TP_LOG_PARAM_UINT64_VALUE(__LINE__),
        TP_LOG_PARAM_UINT64_VALUE(x64_code_size),
        TP_LOG_PARAM_UINT64_VALUE(x64_padding_16_bytes_alignment)
    );

    x64_code_size += x64_padding_16_bytes_alignment;

    return x64_code_size;
}

static uint32_t convert_section_code_content2x64_body_local_var(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    TP_WASM_MODULE_SECTION* code_section, uint8_t* x64_code_buffer, uint32_t x64_code_size,
    uint8_t* payload, uint32_t* offset)
{
    TP_WASM_LOCAL_TYPE** local_types = NULL;
    uint32_t* local_types_num = 0;

    rsize_t wasm_type_index = 0;
    rsize_t wasm_function_index_max = 0;

    if ( ! get_wasm_type_index_function_index_max(
        symbol_table, code_index, &wasm_type_index, &wasm_function_index_max)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    TP_WASM_SECTION_TYPE_VAR* wasm_types = &(symbol_table->member_wasm_types[wasm_type_index]);

    if (TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC != wasm_types->member_form){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    rsize_t param_count = wasm_types->member_param_count;
    uint32_t* param_types = wasm_types->member_param_types;

    if (0 == param_count){

        if (param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }
    }else{

        if (TP_X64_REGISTER_ARGS < param_count){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        if (NULL == param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }
    }

    // Local variables.
    if (NULL == symbol_table->member_func_local_types){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    rsize_t func_local_types_current = symbol_table->member_func_local_types_current;

    if (wasm_function_index_max < func_local_types_current){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    local_types =
        &(symbol_table->member_func_local_types[func_local_types_current].member_local_types);

    local_types_num =
        &(symbol_table->member_func_local_types[func_local_types_current].member_local_types_num);

    if (NULL == x64_code_buffer){

        if ((NULL == *local_types) && param_count){

            // Local variables(wasm type).
            for (rsize_t i = 0; param_count > i; ++i){

                switch (wasm_types->member_param_types[i]){
                case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32:
                    break;
                case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64:
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    goto fail;
                }
            }

            (*local_types_num) = (uint32_t)param_count;

            TP_WASM_LOCAL_TYPE* tmp_arg_types = (TP_WASM_LOCAL_TYPE*)TP_CALLOC(
                symbol_table, (*local_types_num), sizeof(TP_WASM_LOCAL_TYPE)
            );

            if (NULL == tmp_arg_types){

                TP_PRINT_CRT_ERROR(symbol_table);

                goto fail;
            }

            *local_types = tmp_arg_types;

            for (rsize_t i = 0; param_count > i; ++i){

                (*local_types)[i].member_arg_index_1_origin = (uint32_t)(i + 1);
                (*local_types)[i].member_type = wasm_types->member_param_types[i];
                (*local_types)[i].member_offset = (uint32_t)(sizeof(int64_t) * i);
            }
        }
    }else{

        if (NULL == *local_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }
    }

    // Code body size.
    uint32_t body_size = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, *offset, body_size);

    if (0 == body_size){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == body_size")
        );

        goto fail;
    }

    uint32_t wasm_code_body_size = body_size;

    // Local variables(wasm code).
    uint32_t local_count = 0;
    TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, *offset, local_count);

    wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, local_count);

    uint32_t local_variable_size = 0;

    uint32_t local_variable_offset = sizeof(int64_t) * TP_X64_REGISTER_ARGS;

    rsize_t local_variable_index = param_count;

    for (uint32_t i = 0; local_count > i; ++i){

        uint32_t var_count = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, *offset, var_count);

        uint32_t var_type = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, *offset, var_type);

        (*local_types_num) += var_count;

        TP_WASM_LOCAL_TYPE* tmp_local_types = (TP_WASM_LOCAL_TYPE*)TP_REALLOC(
            symbol_table,
            *local_types, sizeof(TP_WASM_LOCAL_TYPE) * (*local_types_num)
        );

        if (NULL == tmp_local_types){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        memset(
            (tmp_local_types + ((*local_types_num) - var_count)), 0,
            sizeof(TP_WASM_LOCAL_TYPE)* var_count
        );

        *local_types = tmp_local_types;

        for (uint32_t j = 0; var_count > j; ++j){

            if (local_variable_index >= (*local_types_num)){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                goto fail;
            }

            (*local_types)[local_variable_index].member_type = var_type;
            (*local_types)[local_variable_index].member_offset = local_variable_offset;

            ++local_variable_index;

            switch (var_type){
            case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32:
                local_variable_offset += sizeof(int32_t);
                local_variable_size += sizeof(int32_t);
                break;
            case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64:
                local_variable_offset += sizeof(int64_t);
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

    uint8_t* wasm_code_body_buffer = payload + (*offset);

    if ( ! wasm_stack_and_use_register_init(
        symbol_table,
        wasm_code_body_buffer, wasm_code_body_size, x64_code_buffer, code_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    x64_code_size = tp_encode_allocate_stack(
        symbol_table, x64_code_buffer, x64_code_size, local_variable_size
    );

    if (0 == x64_code_size){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    return x64_code_size;
fail:
    if (local_types_num){

        TP_FREE(symbol_table, local_types, sizeof(TP_WASM_LOCAL_TYPE) * (*local_types_num));
    }
    return 0;
}

static bool get_wasm_type_index_function_index_max(
    TP_SYMBOL_TABLE* symbol_table, rsize_t code_index,
    rsize_t* wasm_type_index, rsize_t* wasm_function_index_max)
{
    // function section
    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    *wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    if (*wasm_function_index_max < code_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t local_wasm_type_index = symbol_table->member_wasm_functions[code_index];

    // type section
    if (0 == symbol_table->member_wasm_type_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t wasm_type_index_max = symbol_table->member_wasm_type_count - 1;

    if (wasm_type_index_max < local_wasm_type_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    *wasm_type_index = local_wasm_type_index;

    return true;
}

static bool wasm_stack_and_use_register_init(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* wasm_code_body_buffer, uint32_t wasm_code_body_size,
    uint8_t* x64_code_buffer, rsize_t code_index)
{
    // wasm_stack_init

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
    symbol_table->member_wasm_code_body_size += wasm_code_body_size;
    symbol_table->member_wasm_code_body_pos = 0;

    if (symbol_table->member_wasm_code_body_size < (symbol_table->member_wasm_code_body_pos + wasm_code_body_size)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: member_wasm_code_body_size < (member_wasm_code_body_pos + wasm_code_body_size)")
        );

        return false;
    }

    symbol_table->member_wasm_code_body_size_current = wasm_code_body_size;

    symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
    symbol_table->member_stack_size =
        symbol_table->member_stack_size_allocate_unit * sizeof(TP_WASM_STACK_ELEMENT);

    // use_register_init

    if (NULL == x64_code_buffer){

        if (0 == code_index){

            if (0 == symbol_table->member_wasm_function_count){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (NULL == symbol_table->member_x64_func_local){

                TP_X64_FUNC_LOCAL* x64_func_local =
                    (TP_X64_FUNC_LOCAL*)TP_CALLOC(
                        symbol_table,
                        symbol_table->member_wasm_function_count, sizeof(TP_X64_FUNC_LOCAL)
                    );

                if (NULL == x64_func_local){

                    TP_PRINT_CRT_ERROR(symbol_table);

                    return false;
                }

                rsize_t wasm_function_count = symbol_table->member_wasm_function_count;

                for (rsize_t i = 0; wasm_function_count > i; ++i){

                    memset(
                        &(x64_func_local[i].member_use_v_register), TP_X64_V64_REGISTER_NULL,
                        sizeof(x64_func_local[i].member_use_v_register)
                    );
                    memset(
                        &(x64_func_local[i].member_use_nv_register), TP_X64_NV64_REGISTER_NULL,
                        sizeof(x64_func_local[i].member_use_nv_register)
                    );
                }

                symbol_table->member_x64_func_local = x64_func_local;
            }
        }
    }

    memset(
        symbol_table->member_use_X86_32_register, 0,
        sizeof(symbol_table->member_use_X86_32_register)
    );
    memset(
        symbol_table->member_use_X64_32_register, 0,
        sizeof(symbol_table->member_use_X64_32_register)
    );

    if ( ! preset_user_register_of_function_args(symbol_table, x64_code_buffer, code_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool preset_user_register_of_function_args(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, rsize_t code_index)
{
    rsize_t wasm_type_index = 0;
    rsize_t wasm_function_index_max = 0;

    if ( ! get_wasm_type_index_function_index_max(
        symbol_table, code_index, &wasm_type_index, &wasm_function_index_max)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // Local variables.
    if (NULL == symbol_table->member_func_local_types){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t func_local_types_current = symbol_table->member_func_local_types_current;

    if (wasm_function_index_max < func_local_types_current){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_WASM_LOCAL_TYPE* local_types =
        symbol_table->member_func_local_types[func_local_types_current].member_local_types;

    uint32_t* local_types_num =
        &(symbol_table->member_func_local_types[func_local_types_current].member_local_types_num);

    // Local variables(wasm type).
    TP_WASM_SECTION_TYPE_VAR* wasm_types = &(symbol_table->member_wasm_types[wasm_type_index]);

    if (TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC != wasm_types->member_form){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t param_count = wasm_types->member_param_count;
    uint32_t* param_types = wasm_types->member_param_types;

    rsize_t local_types_count = (rsize_t)*local_types_num;

    if (local_types_count < param_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // R10D
    symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R10D] =
        (TP_WASM_STACK_ELEMENT){
            .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
            .member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER
        };
    symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R10D].
        member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R10D;
    // R11D
    symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R11D] =
        (TP_WASM_STACK_ELEMENT){
            .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
            .member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER
        };
    symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R11D].
        member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R11D;

    if (0 == param_count){

        if (param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        return true;
    }else{

        if (TP_X64_REGISTER_ARGS < param_count){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (NULL == param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }

    for (rsize_t arg_index = 0; param_count > arg_index; ++arg_index){

        uint32_t wasm_opcode = TP_WASM_OPCODE_I32_VALUE;

        switch (wasm_types->member_param_types[arg_index]){
        case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32:
            break;
        case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64:
            wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch (arg_index){
        case 0:
            // ECX
            symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_ECX] =
                (TP_WASM_STACK_ELEMENT){
                    .member_wasm_opcode = wasm_opcode,
                    .member_local_index = (uint32_t)arg_index,
                    .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                    .member_offset = (int32_t)local_types[arg_index].member_offset
                };
            symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_ECX].
                member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_ECX;
            symbol_table->member_x64_func_local[code_index].
                member_use_v_register[TP_X64_V64_REGISTER_RCX_INDEX] = TP_X64_V64_REGISTER_RCX;
            break;
        case 1:
            // EDX
            symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX] =
                (TP_WASM_STACK_ELEMENT){
                    .member_wasm_opcode = wasm_opcode,
                    .member_local_index = (uint32_t)arg_index,
                    .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                    .member_offset = (int32_t)local_types[arg_index].member_offset
                };
            symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX].
                member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;
            symbol_table->member_x64_func_local[code_index].
                member_use_v_register[TP_X64_V64_REGISTER_RDX_INDEX] = TP_X64_V64_REGISTER_RDX;
            break;
        case 2:
            // R8D
            symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R8D] =
                (TP_WASM_STACK_ELEMENT){
                    .member_wasm_opcode = wasm_opcode,
                    .member_local_index = (uint32_t)arg_index,
                    .member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER,
                    .member_offset = (int32_t)local_types[arg_index].member_offset
                };
            symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R8D].
                member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R8D;
            symbol_table->member_x64_func_local[code_index].
                member_use_v_register[TP_X64_V64_REGISTER_R8_INDEX] = TP_X64_V64_REGISTER_R8;
            break;
        case 3:
            // R9D
            symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R9D] =
                (TP_WASM_STACK_ELEMENT){
                    .member_wasm_opcode = wasm_opcode,
                    .member_local_index = (uint32_t)arg_index,
                    .member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER,
                    .member_offset = (int32_t)local_types[arg_index].member_offset
                };
            symbol_table->member_use_X64_32_register[TP_X64_32_REGISTER_R9D].
                member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R9D;
            symbol_table->member_x64_func_local[code_index].
                member_use_v_register[TP_X64_V64_REGISTER_R9_INDEX] = TP_X64_V64_REGISTER_R9;
            break;
        default:
            goto out;
        }
    }

out:
    return true;
}

static bool wasm_stack_and_wasm_code_is_empty(TP_SYMBOL_TABLE* symbol_table)
{
    if (TP_WASM_STACK_EMPTY == symbol_table->member_stack_pos){

        if (symbol_table->member_wasm_code_body_pos ==
            symbol_table->member_wasm_code_body_size_current){

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

    // Check of use of volatile register.
    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    switch (value->member_x64_item_kind){
    case TP_X64_ITEM_KIND_X86_32_REGISTER:
        switch (value->member_x64_item.member_x86_32_register){
        case TP_X86_32_REGISTER_ECX:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_RCX_INDEX] = TP_X64_V64_REGISTER_RCX;
            break;
        case TP_X86_32_REGISTER_EDX:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_RDX_INDEX] = TP_X64_V64_REGISTER_RDX;
            break;
        default:
            break;
        }
        break;
    case TP_X64_ITEM_KIND_X64_32_REGISTER:
        switch (value->member_x64_item.member_x64_32_register){
        case TP_X64_32_REGISTER_R8D:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_R8_INDEX] = TP_X64_V64_REGISTER_R8;
            break;
        case TP_X64_32_REGISTER_R9D:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_R9_INDEX] = TP_X64_V64_REGISTER_R9;
            break;
        case TP_X64_32_REGISTER_R10D:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_R10_INDEX] = TP_X64_V64_REGISTER_R10;
            break;
        case TP_X64_32_REGISTER_R11D:
            symbol_table->member_x64_func_local[x64_func_index].
                member_use_v_register[TP_X64_V64_REGISTER_R11_INDEX] = TP_X64_V64_REGISTER_R11;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return true;

fail:

    if (symbol_table->member_stack){

        TP_FREE(symbol_table, &(symbol_table->member_stack), symbol_table->member_stack_size);
    }

    symbol_table->member_stack_pos = TP_WASM_STACK_EMPTY;
    symbol_table->member_stack_size = 0;

    return false;
}

static bool wasm_pop_function_args(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t* x64_code_offset,
    bool is_function_index, rsize_t function_or_type_index)
{
    rsize_t wasm_type_index = 0;

    if (is_function_index){

        rsize_t wasm_function_index_max = 0;

        if ( ! get_wasm_type_index_function_index_max(
            symbol_table, function_or_type_index, &wasm_type_index, &wasm_function_index_max)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else{

        wasm_type_index = function_or_type_index;

        if (wasm_type_index > (symbol_table->member_wasm_type_count - 1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    TP_WASM_SECTION_TYPE_VAR* wasm_types = &(symbol_table->member_wasm_types[wasm_type_index]);

    if (TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC != wasm_types->member_form){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    int64_t param_count = (int64_t)(wasm_types->member_param_count);
    uint32_t* param_types = wasm_types->member_param_types;

    if (0 == param_count){

        if (param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }else{

        if (TP_X64_REGISTER_ARGS < param_count){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (NULL == param_types){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
    }

    if (param_count){

        uint32_t x64_code_size = 0;

        for (int64_t i = param_count - 1; 0 <= i; --i){

            TP_WASM_STACK_ELEMENT arg = { 0 };

            switch (wasm_types->member_param_types[i]){
            case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32:
                arg = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                if (TP_WASM_OPCODE_I32_VALUE != arg.member_wasm_opcode){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
                break;
            case TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64:
                arg = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
                if (TP_WASM_OPCODE_I64_VALUE != arg.member_wasm_opcode){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            if (arg.member_is_arg_of_nested_function){

                TP_X64_64_REGISTER x64_register = TP_X64_64_REGISTER_RCX;

                switch (i){
                case TP_WASM_ARG_INDEX_ECX:
                    break;
                case TP_WASM_ARG_INDEX_EDX:
                    x64_register = TP_X64_64_REGISTER_RDX;
                    break;
                case TP_WASM_ARG_INDEX_R8D:
                    x64_register = TP_X64_64_REGISTER_R8;
                    break;
                case TP_WASM_ARG_INDEX_R9D:
                    x64_register = TP_X64_64_REGISTER_R9;
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }

                // POP – Pop a Value from the Stack
                uint32_t tmp_x64_code_size = tp_encode_x64_pop_reg64(
                    symbol_table, x64_code_buffer, (*x64_code_offset) + x64_code_size,
                    x64_register
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            }

            (*x64_code_offset) += x64_code_size;
        }
    }

    return true;
}

TP_WASM_STACK_ELEMENT tp_wasm_stack_pop(
    TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_POP_MODE pop_mode, uint32_t* offset)
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

            TP_WASM_STACK_ELEMENT control_op = tp_wasm_stack_pop(
                symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL
            );
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
    }else{

        if ((TP_WASM_STACK_POP_MODE_PARAM == pop_mode) ||
            (TP_WASM_STACK_POP_MODE_LABEL == pop_mode)){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }
    }

    result.member_wasm_opcode =
        symbol_table->member_wasm_code_body_buffer[symbol_table->member_wasm_code_body_pos];

    TP_WASM_RELOCATION* wasm_relocation = NULL;

    if ( ! tp_wasm_get_relocation(symbol_table, symbol_table->member_wasm_code_body_pos, &wasm_relocation)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    result.member_wasm_relocation = wasm_relocation;

    ++(symbol_table->member_wasm_code_body_pos);

    if (offset){

        ++(*offset);
    }

    switch (result.member_wasm_opcode){
    // Control flow operators
    case TP_WASM_OPCODE_LOOP: // sig: block_type
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_LOOP", result, result.member_sig, offset
        );
        break;
    case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_BR_IF", result, result.member_relative_depth, offset
        );
        break;

    // Call operators
    case TP_WASM_OPCODE_CALL: // function_index: varuint32
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_CALL", result, result.member_function_index, offset
        );
        break;
    case TP_WASM_OPCODE_CALL_INDIRECT: // type_index: varuint32, reserved: varuint1
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_CALL_INDIRECT", result, result.member_type_index, offset
        );
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_CALL_INDIRECT", result, result.member_reserved, offset
        );
        break;

    // Variable access
    case TP_WASM_OPCODE_GET_LOCAL:
//      break;
    case TP_WASM_OPCODE_SET_LOCAL:
//      break;
    case TP_WASM_OPCODE_TEE_LOCAL:
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_GET/SET/TEE_LOCAL", result, result.member_local_index, offset
        );
        break;
    case TP_WASM_OPCODE_GET_GLOBAL: // global_index: varuint32
//      break;
    case TP_WASM_OPCODE_SET_GLOBAL: // global_index: varuint32
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_GET/SET_GLOBAL", result, result.member_global_index, offset
        );
        break;

    // Memory-related operators
    case TP_WASM_OPCODE_I32_LOAD: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I32_STORE: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I64_LOAD: // flags: varuint32, offset: varuint32
//      break;
    case TP_WASM_OPCODE_I64_STORE: // flags: varuint32, offset: varuint32
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_I32/I64_LOAD/STORE", result, result.member_flags, offset
        );
        TP_WASM_STACK_POP_U_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_I32/I64_LOAD/STORE", result, result.member_mem_offset, offset
        );
        break;

    // Constants
    case TP_WASM_OPCODE_I32_CONST: // value: varint32
        TP_WASM_STACK_POP_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_I32_CONST", result, result.member_i32, offset
        );
        break;
    case TP_WASM_OPCODE_I64_CONST: // value: varint64
        TP_WASM_STACK_POP_DECODE(
            symbol_table, "ERROR: TP_WASM_OPCODE_I64_CONST", result, result.member_i64, offset
        );
        break;
    default:
        break;
    }

    return result;

fail:

    memset(&result, 0, sizeof(result));

    return result;
}

bool tp_allocate_temporary_variable(
    TP_SYMBOL_TABLE* symbol_table, TP_X64_ALLOCATE_MODE allocate_mode,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint32_t* x64_code_size,
    int32_t arg_index, TP_WASM_STACK_ELEMENT* wasm_stack_element)
{
    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[x64_func_index]);

    int32_t offset =
          x64_func_local->member_local_variable_size +
          x64_func_local->member_padding_local_variable_bytes +
          x64_func_local->member_temporary_variable_size;

    int32_t max_size = 
        symbol_table->member_temporary_variable_size_max + symbol_table->member_local_variable_size_max;

    int32_t size = (int32_t)sizeof(int64_t);

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

    x64_func_local->member_temporary_variable_size += size;
    
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
    case 0:
        *is_zero_free_register = false;
        *x86_32_register = TP_X86_32_REGISTER_ECX;
        return true;
    case 1:
        *is_zero_free_register = false;
        *x86_32_register = TP_X86_32_REGISTER_EDX;
        return true;
    case 2:
        *is_zero_free_register = false;
        *x64_32_register = TP_X64_32_REGISTER_R8D;
        return true;
    case 3:
        *is_zero_free_register = false;
        *x64_32_register = TP_X64_32_REGISTER_R9D;
        return true;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    uint32_t local_types_num = symbol_table->member_func_local_types[
        symbol_table->member_func_local_types_current].member_local_types_num;

    uint32_t types_num =
        ((TP_X64_REGISTER_ARGS < local_types_num) ? TP_X64_REGISTER_ARGS : local_types_num);

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

            if (TP_X86_32_REGISTER_EAX > temp_x86_32_register){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_EAX > temp_x86_32_register(%1)"),
                    TP_LOG_PARAM_INT32_VALUE(temp_x86_32_register)
                );

                return false;
            }

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

            if (TP_X64_32_REGISTER_R8D > temp_x64_32_register){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: TP_X64_32_REGISTER_R8D > temp_x64_32_register(%1)"),
                    TP_LOG_PARAM_INT32_VALUE(temp_x64_32_register)
                );

                return false;
            }

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
    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[x64_func_index]);

    switch (x86_32_register){
    case TP_X86_32_REGISTER_EBX:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_RBX_INDEX] = TP_X64_NV64_REGISTER_RBX;
        break;
    case TP_X86_32_REGISTER_ESI:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_RSI_INDEX] = TP_X64_NV64_REGISTER_RSI;
        break;
    case TP_X86_32_REGISTER_EDI:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_RDI_INDEX] = TP_X64_NV64_REGISTER_RDI;
        break;
    default:
        break;
    }

    switch (x64_32_register){
    case TP_X64_32_REGISTER_R12D:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_R12_INDEX] = TP_X64_NV64_REGISTER_R12;
        break;
    case TP_X64_32_REGISTER_R13D:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_R13_INDEX] = TP_X64_NV64_REGISTER_R13;
        break;
    case TP_X64_32_REGISTER_R14D:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_R14_INDEX] = TP_X64_NV64_REGISTER_R14;
        break;
    case TP_X64_32_REGISTER_R15D:
        x64_func_local->member_use_nv_register[TP_X64_NV64_REGISTER_R15_INDEX] = TP_X64_NV64_REGISTER_R15;
        break;
    default:
        break;
    }

    return true;
}

bool tp_free_register(TP_SYMBOL_TABLE* symbol_table, TP_WASM_STACK_ELEMENT* stack_element)
{
    // Local variables(wasm type).
    bool is_arg_ecx = false;
    bool is_arg_edx = false;
    bool is_arg_r8d = false;
    bool is_arg_r9d = false;

    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t wasm_type_index = 0;

    if ( ! get_wasm_type_index_function_index_max(
        symbol_table, x64_func_index, &wasm_type_index, &wasm_function_index_max)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    TP_WASM_SECTION_TYPE_VAR* wasm_types = &(symbol_table->member_wasm_types[wasm_type_index]);

    if (TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC != wasm_types->member_form){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t param_count = wasm_types->member_param_count;

    for (rsize_t arg_index = 0; param_count > arg_index; ++arg_index){

        switch (arg_index){
        case 0: is_arg_ecx = true; break; // ECX
        case 1: is_arg_edx = true; break; // EDX
        case 2: is_arg_r8d = true; break; // R8D
        case 3: is_arg_r9d = true; break; // R9D
        default: goto out;
        }
    }
out:
    ;

    switch (stack_element->member_x64_item_kind){
    case TP_X64_ITEM_KIND_X86_32_REGISTER:{

x86_32:
        ;
        TP_X86_32_REGISTER x86_32_register = stack_element->member_x64_item.member_x86_32_register;

        if (TP_X86_32_REGISTER_EAX > x86_32_register){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_EAX > x86_32_register(%1)"),
                TP_LOG_PARAM_INT32_VALUE(x86_32_register)
            );

            return false;
        }

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
            (TP_X86_32_REGISTER_EBP == x86_32_register) ||
            (stack_element->member_is_tmp_reg)){

            return true;
        }

        if (is_arg_ecx && (TP_X86_32_REGISTER_ECX == x86_32_register)){

            return true;
        }

        if (is_arg_edx && (TP_X86_32_REGISTER_EDX == x86_32_register)){

            return true;
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

        if (TP_X64_32_REGISTER_R8D > x64_32_register){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_X64_32_REGISTER_R8D > x64_32_register(%1)"),
                TP_LOG_PARAM_INT32_VALUE(x64_32_register)
            );

            return false;
        }

        if (TP_X86_32_REGISTER_NULL <= x64_32_register){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_X86_32_REGISTER_NULL <= x64_32_register(%1)"),
                TP_LOG_PARAM_INT32_VALUE(x64_32_register)
            );

            return false;
        }

        if (stack_element->member_is_tmp_reg){

            return true;
        }

        if (is_arg_r8d && (TP_X86_32_REGISTER_ECX == x64_32_register)){

            return true;
        }

        if (is_arg_r9d && (TP_X86_32_REGISTER_EDX == x64_32_register)){

            return true;
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

