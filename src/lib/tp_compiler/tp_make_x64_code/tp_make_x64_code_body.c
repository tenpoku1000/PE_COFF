
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation) \
( \
    (wasm_relocation) && \
    ((wasm_relocation)->member_is_containing_function_call) && \
    (TP_X64_REGISTER_ARGS > ((wasm_relocation)->member_arg_index)) && \
    (0 <= ((wasm_relocation)->member_arg_index)) \
)

#define TP_X64_PUSH_VALUE( \
    symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation, \
    x64_code_size, tmp_x64_code_size, is_containing_function_call) \
{ \
    if (is_containing_function_call){ \
\
        TP_X64_64_REGISTER x64_register = TP_X64_64_REGISTER_RCX; \
\
        switch ((wasm_relocation)->member_arg_index){ \
        case TP_WASM_ARG_INDEX_ECX: \
            break; \
        case TP_WASM_ARG_INDEX_EDX: \
            x64_register = TP_X64_64_REGISTER_RDX; \
            break; \
        case TP_WASM_ARG_INDEX_R8D: \
            x64_register = TP_X64_64_REGISTER_R8; \
            break; \
        case TP_WASM_ARG_INDEX_R9D: \
            x64_register = TP_X64_64_REGISTER_R9; \
            break; \
        default: \
            TP_PUT_LOG_MSG_ICE(symbol_table); \
            return false; \
        } \
\
        (tmp_x64_code_size) = tp_encode_x64_push_reg64( \
            (symbol_table), (x64_code_buffer), (x64_code_offset) + (x64_code_size), \
            x64_register \
        ); \
        TP_X64_CHECK_CODE_SIZE((symbol_table), (x64_code_size), (tmp_x64_code_size)); \
    } \
}

static uint32_t encode_x64_2_operand_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

uint32_t tp_encode_allocate_stack(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    uint32_t local_variable_size)
{
    uint32_t x64_code_size = 0;

#if TP_DEBUG_BREAK
    // int 3
    if (x64_code_buffer){
 
        x64_code_buffer[x64_code_offset + x64_code_size] = 0xcc;
    }

    ++x64_code_size;
#endif
    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[x64_func_index]);

    // Return Address
    x64_func_local->member_register_bytes = (int32_t)sizeof(uint64_t);

    // PUSH – Push Operand onto the Stack
    uint32_t tmp_x64_code_size = tp_encode_x64_push_reg64(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, TP_X64_64_REGISTER_RBP
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    // RBP register.
    x64_func_local->member_register_bytes += (int32_t)sizeof(uint64_t);

    // Other non-volatile registers.
    int32_t nv_register_bytes = 0;

    if (NULL == x64_code_buffer){

        nv_register_bytes = TP_X64_NV64_REGISTER_NUM;

        x64_code_size += TP_X64_NV64_REGISTER_NUM;
    }else{

        for (int32_t i = TP_X64_NV64_REGISTER_NUM - 1; 0 <= i; --i){

            switch (x64_func_local->member_use_nv_register[i]){
            case TP_X64_NV64_REGISTER_NULL:
                break;
            case TP_X64_NV64_REGISTER_RBX:
//              break;
            case TP_X64_NV64_REGISTER_RSI:
//              break;
            case TP_X64_NV64_REGISTER_RDI:
//              break;
            case TP_X64_NV64_REGISTER_R12:
//              break;
            case TP_X64_NV64_REGISTER_R13:
//              break;
            case TP_X64_NV64_REGISTER_R14:
//              break;
            case TP_X64_NV64_REGISTER_R15:{
                // PUSH – Push Operand onto the Stack
                tmp_x64_code_size = tp_encode_x64_push_reg64(
                    symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                    (TP_X64_64_REGISTER)(x64_func_local->member_use_nv_register[i])
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                ++nv_register_bytes;
                break;
            }
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }
    }

    x64_func_local->member_register_bytes += (nv_register_bytes * sizeof(uint64_t));

    x64_func_local->member_padding_register_bytes =
        TP_X64_PADDING_16_BYTES_ALIGNMENT(x64_func_local->member_register_bytes);

    // Temporary variables.
    {
        // See tp_allocate_temporary_variable function.
        // x64_func_local->member_local_variable_size;

        int32_t v = x64_func_local->member_register_bytes +
            x64_func_local->member_padding_register_bytes +
            x64_func_local->member_temporary_variable_size;

        x64_func_local->member_padding_temporary_variable_bytes = TP_X64_PADDING_16_BYTES_ALIGNMENT(v);
    }

    // Local variables.
    {
        x64_func_local->member_local_variable_size = (int32_t)local_variable_size;

        int32_t v = x64_func_local->member_register_bytes +
            x64_func_local->member_padding_register_bytes +
            x64_func_local->member_temporary_variable_size +
            x64_func_local->member_padding_temporary_variable_bytes +
            x64_func_local->member_local_variable_size;

        x64_func_local->member_padding_local_variable_bytes = TP_X64_PADDING_16_BYTES_ALIGNMENT(v);
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

    // Last padding bytes.
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
        TP_LOG_PARAM_UINT64_VALUE(x64_func_index),
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

    // SUB – Integer Subtraction
    tmp_x64_code_size = tp_encode_x64_add_sub_imm(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        TP_X64_SUB, TP_X64_64_REGISTER_RSP, x64_func_local->member_stack_imm32, TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    // LEA - Load Effective Address
    // lea rbp, QWORD PTR [rsp+32]
    tmp_x64_code_size = tp_encode_x64_lea(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        TP_X64_64_REGISTER_RBP, TP_X64_64_REGISTER_INDEX_NONE,
        TP_X64_64_REGISTER_RSP, stack_param_size
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

#if TP_DEBUG_BREAK
    // int 3
    if (x64_code_buffer){

        x64_code_buffer[x64_code_offset + x64_code_size] = 0xcc;
    }

    ++x64_code_size;
#endif

    return x64_code_size;
}

uint32_t tp_encode_relocation_type_base(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset, 
    TP_WASM_RELOCATION* wasm_relocation, bool* status)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    *status = false;

    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    dst.member_is_stack_base = true;

    TP_WASM_STACK_ELEMENT src = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
        .member_wasm_relocation = NULL,
        .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
        .member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL,
        .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_NONE,
        .member_offset = 0
    };

    src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EBP;

    if (TP_WASM_RELOCATION_TYPE_BASE == wasm_relocation->member_type){

        // mov dst, rbp
        // NOTE: Not free RBP register.
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else{

        dst = src; // rbp
    }

    // push dst
    if ( ! tp_wasm_stack_push(symbol_table, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    *status = true;

    return x64_code_size;
}

// Control flow operators

bool tp_encode_loop_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset, 
    TP_WASM_RELOCATION* wasm_relocation, int32_t sig)
{
    TP_WASM_STACK_ELEMENT dst = {
        .member_wasm_opcode = TP_WASM_OPCODE_LOOP_LABEL,
        .member_sig = sig,
        .member_x64_code_offset = x64_code_offset
    };

    if ( ! tp_wasm_stack_push(symbol_table, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

uint32_t tp_encode_br_if_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t relative_depth, TP_WASM_STACK_ELEMENT* label,
    TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    if (TP_WASM_OPCODE_LOOP_LABEL != label->member_wasm_opcode){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: TP_WASM_OPCODE_LOOP_LABEL(%1) != op1->member_wasm_opcode(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(TP_WASM_OPCODE_LOOP_LABEL),
            TP_LOG_PARAM_UINT64_VALUE(op1->member_wasm_opcode)
        );

        return 0;
    }

    if (TP_WASM_BLOCK_TYPE_VOID != label->member_sig){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    uint32_t x64_code_size = 0;

    // JNE - Jump short if not equal (ZF=0).
    uint32_t tmp_x64_code_size = tp_encode_x64_jne(
        symbol_table, x64_code_buffer, x64_code_offset, label
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

uint32_t tp_encode_return_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[x64_func_index]);

    for (rsize_t i = 0; TP_X64_NV64_REGISTER_NUM > i; ++i){

        switch (x64_func_local->member_use_nv_register[i]){
        case TP_X64_NV64_REGISTER_NULL:
            break;
        case TP_X64_NV64_REGISTER_RBX:
//          break;
        case TP_X64_NV64_REGISTER_RSI:
//          break;
        case TP_X64_NV64_REGISTER_RDI:
//          break;
        case TP_X64_NV64_REGISTER_R12:
//          break;
        case TP_X64_NV64_REGISTER_R13:
//          break;
        case TP_X64_NV64_REGISTER_R14:
//          break;
        case TP_X64_NV64_REGISTER_R15:{
            // POP – Pop a Value from the Stack
            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                (TP_X64_64_REGISTER)(x64_func_local->member_use_nv_register[i])
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        }
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
    }

//  // LEA - Load Effective Address
//  // lea rsp, QWORD PTR [rbp+32]
//  x64_code_size += encode_x64_lea(
//      symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
//      TP_X64_64_REGISTER_RSP, TP_X64_64_REGISTER_INDEX_NONE, TP_X64_64_REGISTER_RBP, stack_param_size
//  );

    // ADD
    tmp_x64_code_size = tp_encode_x64_add_sub_imm(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        TP_X64_ADD, TP_X64_64_REGISTER_RSP, x64_func_local->member_stack_imm32, TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    // POP – Pop a Value from the Stack
    tmp_x64_code_size = tp_encode_x64_pop_reg64(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, TP_X64_64_REGISTER_RBP
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    // RET - Return from Procedure(near return)
    tmp_x64_code_size = tp_encode_x64_1_opcode(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, TP_X64_OPCODE_RET/* 0xc3 */
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

// Call operators

uint32_t tp_encode_call_indirect_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t function_or_type_index, uint32_t reserved, TP_WASM_STACK_ELEMENT* op1)
{
    if (0 == symbol_table->member_wasm_function_count){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

    rsize_t x64_func_index = symbol_table->member_x64_func_index;

    if (wasm_function_index_max < x64_func_index){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_X64_FUNC_LOCAL* x64_func_local = &(symbol_table->member_x64_func_local[x64_func_index]);

    bool is_nested_function = (
        (wasm_relocation->member_function_call_depth) &&
        (TP_X64_REGISTER_ARGS > (wasm_relocation->member_arg_index)) &&
        (0 <= (wasm_relocation->member_arg_index))
    );

    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
        .member_is_arg_of_nested_function = is_nested_function
    };

    bool is_need_return_value = false;
    int32_t type_return = TP_WASM_ARG_INDEX_VOID;

    if (wasm_relocation){

        switch (wasm_relocation->member_type_return){
        case TP_WASM_RETURN_VOID:
            break;
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32:
            type_return = TP_WASM_ARG_INDEX_EAX;
            is_need_return_value = true;
            break;
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64:
            type_return = TP_WASM_ARG_INDEX_EAX;
            is_need_return_value = true;
            result.member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
    }

    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (is_need_return_value){

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, type_return, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    if (x64_code_buffer){

        for (int32_t i = 0; TP_X64_V64_REGISTER_NUM > i; ++i){

            switch (x64_func_local->member_use_v_register[i]){
            case TP_X64_V64_REGISTER_NULL:
                break;
            case TP_X64_V64_REGISTER_RCX:
//              break;
            case TP_X64_V64_REGISTER_RDX:
//              break;
            case TP_X64_V64_REGISTER_R8:
//              break;
            case TP_X64_V64_REGISTER_R9:
//              break;
            case TP_X64_V64_REGISTER_R10:
//              break;
            case TP_X64_V64_REGISTER_R11:{
                // PUSH – Push Operand onto the Stack
                tmp_x64_code_size = tp_encode_x64_push_reg64(
                    symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                    (TP_X64_64_REGISTER)(x64_func_local->member_use_v_register[i])
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            }
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }
    }

    tmp_x64_code_size = tp_encode_x64_call(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, wasm_relocation, op1
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    if (x64_code_buffer){

        for (int32_t i = TP_X64_V64_REGISTER_NUM - 1; 0 <= i; --i){

            switch (x64_func_local->member_use_v_register[i]){
            case TP_X64_V64_REGISTER_NULL:
                break;
            case TP_X64_V64_REGISTER_RCX:
//              break;
            case TP_X64_V64_REGISTER_RDX:
//              break;
            case TP_X64_V64_REGISTER_R8:
//              break;
            case TP_X64_V64_REGISTER_R9:
//              break;
            case TP_X64_V64_REGISTER_R10:
//              break;
            case TP_X64_V64_REGISTER_R11:{
                // POP – Pop a Value from the Stack
                tmp_x64_code_size = tp_encode_x64_pop_reg64(
                    symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                    (TP_X64_64_REGISTER)(x64_func_local->member_use_v_register[i])
                );
                TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
                break;
            }
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }
    }

    if (is_need_return_value){

        if (is_nested_function){

            // PUSH – Push Operand onto the Stack
            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if ( ! tp_wasm_stack_push(symbol_table, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    ++(symbol_table->member_x64_call_num);

    return x64_code_size;
}

// Variable access

uint32_t tp_encode_get_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, bool* status)
{
    *status = false;

    TP_WASM_FUNC_LOCAL_TYPE* func_local_types = &(symbol_table->member_func_local_types[
        symbol_table->member_func_local_types_current]);

    if (local_index >= func_local_types->member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_WASM_LOCAL_TYPE* local_types = &(func_local_types->member_local_types[local_index]);

    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (NULL == wasm_relocation){

        TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        TP_WASM_STACK_ELEMENT src = {
            .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL,
            .member_offset = (int32_t)local_types->member_offset
        };

        // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
        uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_wasm_stack_push(symbol_table, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }else{

        uint32_t wasm_opcode =
            (TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32 == local_types->member_type) ?
            TP_WASM_OPCODE_I32_VALUE : TP_WASM_OPCODE_I64_VALUE;
        bool is_arg_register = false;

        if (TP_X64_REGISTER_ARGS < local_types->member_arg_index_1_origin){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return 0;
        }

        bool is_containing_function_call =
            TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation);

        TP_WASM_STACK_ELEMENT src = {
            .member_wasm_opcode = wasm_opcode,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL,
            .member_offset = (int32_t)local_types->member_offset,
            .member_is_arg_of_nested_function = is_containing_function_call
        };

        switch (wasm_relocation->member_arg_index){
        case TP_WASM_ARG_INDEX_VOID:
            break;
        case TP_WASM_ARG_INDEX_EAX:
            src.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
            src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX;
            is_arg_register = true;
            break;
        case TP_WASM_ARG_INDEX_ECX:
            src.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
            src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_ECX;
            is_arg_register = true;
            break;
        case TP_WASM_ARG_INDEX_EDX:
            src.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
            src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;
            is_arg_register = true;
            break;
        case TP_WASM_ARG_INDEX_R8D:
            src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
            src.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R8D;
            is_arg_register = true;
            break;
        case TP_WASM_ARG_INDEX_R9D:
            src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
            src.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R9D;
            is_arg_register = true;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        if (is_arg_register){

            TP_X64_PUSH_VALUE(
                symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
                x64_code_size, tmp_x64_code_size, is_containing_function_call
            );

            if ( ! tp_wasm_stack_push(symbol_table, &src)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }
        }else{

            rsize_t local_types_num = func_local_types->member_local_types_num;
            uint32_t var_local_index_base = 0; // base_local_index

            for (rsize_t i = 0; local_types_num > i; ++i){

                uint32_t arg_index_1_origin =
                    func_local_types->member_local_types[i].member_arg_index_1_origin;

                if ((TP_X64_REGISTER_ARGS >= arg_index_1_origin) &&
                    (0 < arg_index_1_origin)){

                    ++var_local_index_base;
                }else{

                    break;
                }
            }

            uint32_t var_local_index_i64 = var_local_index_base + 1; // base_local_index + 1
            uint32_t var_local_index_i32 = var_local_index_base + 2; // base_local_index + 2

            switch (wasm_opcode){
            case TP_WASM_OPCODE_I32_VALUE:
                if (local_index == var_local_index_i32){

                    src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                    src.member_x64_item.member_x86_32_register = TP_X64_32_REGISTER_R10D;
                }
                break;
            case TP_WASM_OPCODE_I64_VALUE:
                if (local_index == var_local_index_i64){

                    src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                    src.member_x64_item.member_x86_32_register = TP_X64_32_REGISTER_R11D;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }

            TP_WASM_STACK_ELEMENT dst = {
                .member_wasm_opcode = wasm_opcode,
                .member_is_arg_of_nested_function = is_containing_function_call
            };

            if ( ! tp_allocate_temporary_variable(
                symbol_table, TP_X64_ALLOCATE_DEFAULT,
                x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &dst)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }

            // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
            uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_MOV, &dst, &src
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            TP_X64_PUSH_VALUE(
                symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
                x64_code_size, tmp_x64_code_size, is_containing_function_call
            );

            if ( ! tp_wasm_stack_push(symbol_table, &dst)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }
        }
    }

    *status = true;

    return x64_code_size;
}

uint32_t tp_encode_set_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, TP_WASM_STACK_ELEMENT* op1)
{
    TP_WASM_FUNC_LOCAL_TYPE* func_local_types = &(symbol_table->member_func_local_types[
        symbol_table->member_func_local_types_current]);

    if (local_index >= func_local_types->member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_WASM_LOCAL_TYPE* local_types = &(func_local_types->member_local_types[local_index]);

    uint32_t x64_code_size = 0;

    if (NULL == wasm_relocation){

        if (TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: TP_WASM_OPCODE_I32_VALUE(%1) != op1->member_wasm_opcode(%2)"),
                TP_LOG_PARAM_UINT64_VALUE(TP_WASM_OPCODE_I32_VALUE),
                TP_LOG_PARAM_UINT64_VALUE(op1->member_wasm_opcode)
            );

            return 0;
        }

        TP_WASM_STACK_ELEMENT dst = {
            .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL,
            .member_offset = (int32_t)local_types->member_offset
        };

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else{

        uint32_t wasm_opcode =
            (TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32 == local_types->member_type) ?
                TP_WASM_OPCODE_I32_VALUE : TP_WASM_OPCODE_I64_VALUE;
        bool is_arg_register = false;

        if (TP_X64_REGISTER_ARGS < local_types->member_arg_index_1_origin){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return 0;
        }

        if ((TP_WASM_ARG_INDEX_EAX == wasm_relocation->member_arg_index) ||
            ((0 <= wasm_relocation->member_arg_index) &&
            (TP_X64_REGISTER_ARGS > wasm_relocation->member_arg_index))){

            is_arg_register = true;
        }

        TP_WASM_STACK_ELEMENT dst = {
            .member_wasm_opcode = wasm_opcode,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL,
            .member_offset = (int32_t)local_types->member_offset
        };

        if (false == is_arg_register){

            rsize_t local_types_num = func_local_types->member_local_types_num;
            uint32_t var_local_index_base = 0; // base_local_index

            for (rsize_t i = 0; local_types_num > i; ++i){

                uint32_t arg_index_1_origin =
                    func_local_types->member_local_types[i].member_arg_index_1_origin;

                if ((TP_X64_REGISTER_ARGS >= arg_index_1_origin) &&
                    (0 < arg_index_1_origin)){

                    ++var_local_index_base;
                }else{

                    break;
                }
            }

            uint32_t var_local_index_i64 = var_local_index_base + 1; // base_local_index + 1
            uint32_t var_local_index_i32 = var_local_index_base + 2; // base_local_index + 2

            switch (wasm_opcode){
            case TP_WASM_OPCODE_I32_VALUE:
                if (TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return 0;
                }
                if (local_index == var_local_index_i32){

                    dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                    dst.member_x64_item.member_x86_32_register = TP_X64_32_REGISTER_R10D;
                }
                break;
            case TP_WASM_OPCODE_I64_VALUE:
                if (TP_WASM_OPCODE_I64_VALUE != op1->member_wasm_opcode){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return 0;
                }
                if (local_index == var_local_index_i64){

                    dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                    dst.member_x64_item.member_x86_32_register = TP_X64_32_REGISTER_R11D;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }else{

            if (0 == symbol_table->member_wasm_function_count){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return 0;
            }

            rsize_t wasm_function_index_max = symbol_table->member_wasm_function_count - 1;

            rsize_t x64_func_index = symbol_table->member_x64_func_index;

            if (wasm_function_index_max < x64_func_index){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return 0;
            }

            switch (wasm_relocation->member_arg_index){
            case TP_WASM_ARG_INDEX_VOID:
                break;
            case TP_WASM_ARG_INDEX_EAX:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                dst.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX;
                break;
            case TP_WASM_ARG_INDEX_ECX:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                dst.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_ECX;
                symbol_table->member_x64_func_local[x64_func_index].
                    member_use_v_register[TP_X64_V64_REGISTER_RCX_INDEX] = TP_X64_V64_REGISTER_RCX;
                break;
            case TP_WASM_ARG_INDEX_EDX:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                dst.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;
                symbol_table->member_x64_func_local[x64_func_index].
                    member_use_v_register[TP_X64_V64_REGISTER_RDX_INDEX] = TP_X64_V64_REGISTER_RDX;
                break;
            case TP_WASM_ARG_INDEX_R8D:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                dst.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R8D;
                symbol_table->member_x64_func_local[x64_func_index].
                    member_use_v_register[TP_X64_V64_REGISTER_R8_INDEX] = TP_X64_V64_REGISTER_R8;
                break;
            case TP_WASM_ARG_INDEX_R9D:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                dst.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R9D;
                symbol_table->member_x64_func_local[x64_func_index].
                    member_use_v_register[TP_X64_V64_REGISTER_R9_INDEX] = TP_X64_V64_REGISTER_R9;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    return x64_code_size;
}

uint32_t tp_encode_tee_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index,
    TP_WASM_STACK_ELEMENT* op1, bool* status)
{
    if (local_index >= symbol_table->member_func_local_types[
        symbol_table->member_func_local_types_current].member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    uint32_t x64_code_size = 0;

    uint32_t tmp_x64_code_size = tp_encode_set_local_code(
        symbol_table, x64_code_buffer, x64_code_offset,
        wasm_relocation, local_index, op1
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    tmp_x64_code_size = tp_encode_get_local_code(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        wasm_relocation, local_index, status
    );
    TP_X64_CHECK_CODE_STATUS(symbol_table, status, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

uint32_t tp_encode_get_global_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t global_index)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return 0;
}

uint32_t tp_encode_set_global_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t global_index, TP_WASM_STACK_ELEMENT* op1)
{
    TP_PUT_LOG_MSG_ICE(symbol_table);
    return 0;
}

// Memory-related operators

uint32_t tp_encode_i32_load_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    bool is_i64 = false;

    switch (op1->member_wasm_opcode){
    case TP_WASM_OPCODE_I32_VALUE:
        break;
    case TP_WASM_OPCODE_I64_VALUE:
        is_i64 = true;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    TP_WASM_RELOCATION* op1_wasm_relocation = op1->member_wasm_relocation;
    bool is_rel = false;

    if (op1_wasm_relocation &&
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type)){

        is_rel = true;
    }

    bool is_containing_function_call =
        TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation);

    TP_WASM_STACK_ELEMENT dst = {
        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
        .member_is_arg_of_nested_function = is_containing_function_call
    };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    // mov src, op1;
    TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if (is_i64){

        src = *op1; 
    }else{

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &src)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        src.member_wasm_relocation = op1_wasm_relocation;

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, &src, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    src.member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE;
    src.member_x64_memory_item_kind = src.member_x64_item_kind;
    src.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
    src.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
    src.member_offset_backup = src.member_offset;
    src.member_offset = (int32_t)offset;

    if (is_rel && offset){

        TP_WASM_STACK_ELEMENT tmp = { .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // mov tmp, src
        // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_RIP, &tmp, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        tmp.member_x64_memory_item_kind = tmp.member_x64_item_kind;
        tmp.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        tmp.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
        tmp.member_offset_backup = tmp.member_offset;
        tmp.member_offset = (int32_t)offset;

        // mov dst, tmp
        // NOTE: Call tp_free_register(tmp) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, &tmp
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else{

        // mov dst, src
        // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            (is_rel ? TP_X64_MOV_RIP : TP_X64_MOV), &dst, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    // push dst
    dst.member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE;

    if (wasm_relocation){

        TP_X64_PUSH_VALUE(
            symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
            x64_code_size, tmp_x64_code_size, is_containing_function_call
        );
    }
    if ( ! tp_wasm_stack_push(symbol_table, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    return x64_code_size;
}

uint32_t tp_encode_i32_store_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    bool is_i64 = false;

    switch (op1->member_wasm_opcode){
    case TP_WASM_OPCODE_I32_VALUE:
        break;
    case TP_WASM_OPCODE_I64_VALUE:
        is_i64 = true;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    TP_WASM_RELOCATION* op1_wasm_relocation = op1->member_wasm_relocation;
    bool is_rel = false;

    if (op1_wasm_relocation &&
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type)){

        is_rel = true;
    }

    // mov dst, op1;
    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };
    TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if (is_i64){

        dst = *op1; 
        dst.member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE;
        src = *op2; 
    }else{

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        dst.member_wasm_relocation = op1_wasm_relocation;

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, &dst, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &src)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        src.member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
        src.member_wasm_relocation = op2->member_wasm_relocation;

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, &src, op2
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    dst.member_x64_memory_item_kind = dst.member_x64_item_kind;
    dst.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
    dst.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
    dst.member_offset_backup = dst.member_offset;
    dst.member_offset = (int32_t)offset;

    if (is_rel && offset){

        TP_WASM_STACK_ELEMENT tmp = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        TP_X64_64_REGISTER reg64_dst_reg = TP_X64_64_REGISTER_NULL;

        if ( ! tp_convert_to_x64_reg64(symbol_table, &reg64_dst_reg, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // mov tmp, dst
        // NOTE: Call tp_free_register(dst) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_RIP, &tmp, &dst
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        // add tmp, imm
        tmp_x64_code_size = tp_encode_x64_add_sub_imm(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_ADD, reg64_dst_reg, (int32_t)offset, TP_X64_ADD_SUB_IMM_MODE_DEFAULT
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        // mov tmp, op2
        // NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &tmp, op2
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_free_register(symbol_table, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }else{

        // mov dst, op2
        // NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            (is_rel ? TP_X64_MOV_RIP : TP_X64_MOV), &dst, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_free_register(symbol_table, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    return x64_code_size;
}

uint32_t tp_encode_i64_load_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    bool is_i64 = false;

    switch (op1->member_wasm_opcode){
    case TP_WASM_OPCODE_I32_VALUE:
        break;
    case TP_WASM_OPCODE_I64_VALUE:
        is_i64 = true;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    TP_WASM_RELOCATION* op1_wasm_relocation = op1->member_wasm_relocation;
    bool is_rel = false;

    if (op1_wasm_relocation &&
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type)){

        is_rel = true;
    }

    bool is_containing_function_call =
        TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation);

    TP_WASM_STACK_ELEMENT dst = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
        .member_is_arg_of_nested_function = is_containing_function_call
    };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    // mov src, op1;
    TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if (is_i64){

        src = *op1; 
    }else{

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &src)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        src.member_wasm_relocation = op1_wasm_relocation;

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, &src, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    src.member_x64_memory_item_kind = src.member_x64_item_kind;
    src.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
    src.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
    src.member_offset_backup = src.member_offset;
    src.member_offset = (int32_t)offset;

    if (is_rel && offset){

        TP_WASM_STACK_ELEMENT tmp = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // mov tmp, src
        // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_RIP, &tmp, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        tmp.member_x64_memory_item_kind = tmp.member_x64_item_kind;
        tmp.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        tmp.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
        tmp.member_offset_backup = tmp.member_offset;
        tmp.member_offset = (int32_t)offset;

        // mov dst, tmp
        // NOTE: Call tp_free_register(tmp) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, &tmp
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else{

        // mov dst, src
        // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            (is_rel ? TP_X64_MOV_RIP : TP_X64_MOV), &dst, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    // push dst

    if (wasm_relocation){

        TP_X64_PUSH_VALUE(
            symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
            x64_code_size, tmp_x64_code_size, is_containing_function_call
        );
    }

    if ( ! tp_wasm_stack_push(symbol_table, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    return x64_code_size;
}

uint32_t tp_encode_i64_store_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation,
    uint32_t flags, uint32_t offset, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    bool is_i64 = false;

    switch (op1->member_wasm_opcode){
    case TP_WASM_OPCODE_I32_VALUE:
        break;
    case TP_WASM_OPCODE_I64_VALUE:
        is_i64 = true;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    TP_WASM_RELOCATION* op1_wasm_relocation = op1->member_wasm_relocation;
    bool is_rel = false;

    if (op1_wasm_relocation &&
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type)){

        is_rel = true;
    }

    // mov dst, op1;
    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if (is_i64){

        dst = *op1; 
    }else{

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        dst.member_wasm_relocation = op1_wasm_relocation;

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, &dst, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    dst.member_x64_memory_item_kind = dst.member_x64_item_kind;
    dst.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
    dst.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
    dst.member_offset_backup = dst.member_offset;
    dst.member_offset = (int32_t)offset;

    if (is_rel && offset){

        TP_WASM_STACK_ELEMENT tmp = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        TP_X64_64_REGISTER reg64_dst_reg = TP_X64_64_REGISTER_NULL;

        if ( ! tp_convert_to_x64_reg64(symbol_table, &reg64_dst_reg, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // mov tmp, dst
        // NOTE: Call tp_free_register(dst) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_RIP, &tmp, &dst
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        // add tmp, imm
        tmp_x64_code_size = tp_encode_x64_add_sub_imm(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_ADD, reg64_dst_reg, (int32_t)offset, TP_X64_ADD_SUB_IMM_MODE_DEFAULT
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        // mov tmp, op2
        // NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &tmp, op2
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_free_register(symbol_table, &tmp)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }else{

        // mov dst, op2
        // NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            (is_rel ? TP_X64_MOV_RIP : TP_X64_MOV), &dst, op2
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if ( ! tp_free_register(symbol_table, &dst)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    return x64_code_size;
}

// Constants

uint32_t tp_encode_i32_const_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, int32_t value, bool* status)
{
    *status = false;

    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (wasm_relocation && (TP_X64_REGISTER_ARGS < (wasm_relocation->member_arg_index + 1))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    bool is_containing_function_call =
        (wasm_relocation) ? TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation) : false;

    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE,
        .member_is_arg_of_nested_function = is_containing_function_call
    };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    result.member_wasm_relocation = wasm_relocation;

    tmp_x64_code_size = tp_encode_x64_mov_imm(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        value, TP_X64_MOV_IMM_MODE_FORCE_IMM32, &result
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    TP_X64_PUSH_VALUE(
        symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
        x64_code_size, tmp_x64_code_size, is_containing_function_call
    );

    if ( ! tp_wasm_stack_push(symbol_table, &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    *status = true;

    return x64_code_size;
}

uint32_t tp_encode_i64_const_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, int64_t value, bool* status)
{
    *status = false;

    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (wasm_relocation && (TP_X64_REGISTER_ARGS < (wasm_relocation->member_arg_index + 1))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    bool is_containing_function_call =
        (wasm_relocation) ? TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation) : false;

    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
        .member_is_arg_of_nested_function = is_containing_function_call
    };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    result.member_wasm_relocation = wasm_relocation;

    if ((NULL == wasm_relocation) ||
        (wasm_relocation && ((TP_WASM_RELOCATION_TYPE_REL64 != wasm_relocation->member_type) &&
        (TP_WASM_RELOCATION_TYPE_STRING_LITERAL64 != wasm_relocation->member_type)))){

        tmp_x64_code_size = tp_encode_x64_mov_imm(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            value, TP_X64_MOV_IMM_MODE_DEFAULT, &result
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else if (wasm_relocation && (TP_WASM_RELOCATION_TYPE_STRING_LITERAL64 == wasm_relocation->member_type)){

        TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &src)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        result.member_wasm_relocation = NULL;

        src.member_wasm_relocation = wasm_relocation;
        src.member_x64_memory_item_kind = src.member_x64_item_kind;
        src.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
        src.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
        src.member_offset_backup = src.member_offset;
        src.member_offset = (int32_t)(wasm_relocation->member_offset64);

        TP_X64_64_REGISTER reg64_dst_reg = TP_X64_64_REGISTER_NULL;

        if ( ! tp_convert_to_x64_reg64(symbol_table, &reg64_dst_reg, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // lea result, src
        // NOTE: Call tp_free_register(src) in tp_encode_x64_lea_rel_mem().
        tmp_x64_code_size = tp_encode_x64_lea_rel_mem(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            reg64_dst_reg, &src
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    TP_X64_PUSH_VALUE(
        symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
        x64_code_size, tmp_x64_code_size, is_containing_function_call
    );

    if ( ! tp_wasm_stack_push(symbol_table, &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    *status = true;

    return x64_code_size;
}

// Comparison operators(i32)

uint32_t tp_encode_i32_ne_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;

    // NOTE: Call tp_free_register(op2) in tp_encode_sub_code().
    uint32_t tmp_x64_code_size = tp_encode_sub_code(
        symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation, op1, op2
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

// Comparison operators(i64)

uint32_t tp_encode_i64_ne_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;
    
    // NOTE: Call tp_free_register(op2) in tp_encode_sub_code().
    uint32_t tmp_x64_code_size = tp_encode_sub_code(
        symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation, op1, op2
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

// Numeric operators(i32)
// Numeric operators(i64)

uint32_t tp_encode_add_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    return encode_x64_2_operand_code(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_ADD, wasm_relocation, op1, op2
    );
}

uint32_t tp_encode_sub_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    return encode_x64_2_operand_code(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_SUB, wasm_relocation, op1, op2
    );
}

uint32_t tp_encode_mul_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    return encode_x64_2_operand_code(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_IMUL, wasm_relocation, op1, op2
    );
}

uint32_t tp_encode_div_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    return encode_x64_2_operand_code(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_IDIV, wasm_relocation, op1, op2
    );
}

uint32_t tp_encode_xor_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    return encode_x64_2_operand_code(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_XOR, wasm_relocation, op1, op2
    );
}

static uint32_t encode_x64_2_operand_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if (((TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode) &&
        (TP_WASM_OPCODE_I64_VALUE != op1->member_wasm_opcode)) ||
        (op1->member_wasm_opcode != op2->member_wasm_opcode)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    bool is_containing_function_call =
        TP_X64_IS_CONTAINING_FUNCTION_CALL(wasm_relocation);

    TP_WASM_STACK_ELEMENT left_op = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
        .member_is_arg_of_nested_function = is_containing_function_call
    };
    TP_WASM_STACK_ELEMENT right_op = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE
    };

    left_op = *op1;
    right_op = *op2;

    /* NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand(). */
    tmp_x64_code_size = tp_encode_x64_2_operand(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        x64_op, &left_op, &right_op
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    if (TP_X64_IDIV == x64_op){

        bool is_dst_x86_32_register = (TP_X64_ITEM_KIND_X86_32_REGISTER == left_op.member_x64_item_kind);
        bool is_dst_EAX_register = (is_dst_x86_32_register &&
            (TP_X86_32_REGISTER_EAX == left_op.member_x64_item.member_x86_32_register)
        );

        if (false == is_dst_EAX_register){

            left_op = tp_wasm_stack_pop(symbol_table, TP_WASM_STACK_POP_MODE_PARAM, NULL);
        }
    }

    int32_t arg_index = TP_WASM_ARG_INDEX(wasm_relocation);

    if (TP_WASM_ARG_INDEX_VOID != arg_index){

        if ( ! ((TP_X64_ADD == x64_op) && (left_op.member_is_stack_base))){

            switch (left_op.member_x64_item_kind){
            case TP_X64_ITEM_KIND_X86_32_REGISTER:
                switch (left_op.member_x64_item.member_x86_32_register){
                case TP_X86_32_REGISTER_ECX:
                    if (TP_WASM_ARG_INDEX_ECX == arg_index){ goto skip_1; }
                    break;
                case TP_X86_32_REGISTER_EDX:
                    if (TP_WASM_ARG_INDEX_EDX == arg_index){ goto skip_1; }
                    break;
                default:
                    break;
                }
                break;
            case TP_X64_ITEM_KIND_X64_32_REGISTER:
                switch (left_op.member_x64_item.member_x64_32_register){
                case TP_X64_32_REGISTER_R8D:
                    if (TP_WASM_ARG_INDEX_R8D == arg_index){ goto skip_1; }
                    break;
                case TP_X64_32_REGISTER_R9D:
                    if (TP_WASM_ARG_INDEX_R9D == arg_index){ goto skip_1; }
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
            goto skip_2;
skip_1:
            TP_X64_PUSH_VALUE(
                symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
                x64_code_size, tmp_x64_code_size, is_containing_function_call
            );
            if ( ! tp_wasm_stack_push(symbol_table, &left_op)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }
            return x64_code_size;
        }
skip_2:
        ;

        TP_WASM_STACK_ELEMENT result = {
            .member_wasm_opcode = left_op.member_wasm_opcode,
            .member_is_arg_of_nested_function = is_containing_function_call
        };

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size, arg_index, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        if ((TP_X64_ADD == x64_op) && (left_op.member_is_stack_base)){

            left_op.member_x64_memory_item_kind = left_op.member_x64_item_kind;
            left_op.member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY;
            left_op.member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_GLOBAL;
            left_op.member_offset_backup = left_op.member_offset;
            left_op.member_offset = 0;

            TP_X64_64_REGISTER reg64_dst_reg = TP_X64_64_REGISTER_NULL;

            if ( ! tp_convert_to_x64_reg64(symbol_table, &reg64_dst_reg, &result)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }

            // lea result, src
            // NOTE: Call tp_free_register(left_op) in tp_encode_x64_lea_rel_mem().
            tmp_x64_code_size = tp_encode_x64_lea_rel_mem(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                reg64_dst_reg, &left_op
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }else{

            /* NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand(). */
            tmp_x64_code_size = tp_encode_x64_2_operand(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_MOV, &result, &left_op
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        TP_X64_PUSH_VALUE(
            symbol_table, x64_code_buffer, x64_code_offset, wasm_relocation,
            x64_code_size, tmp_x64_code_size, is_containing_function_call
        );

        if ( ! tp_wasm_stack_push(symbol_table, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }else{

        if ( ! tp_wasm_stack_push(symbol_table, &left_op)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    return x64_code_size;
}

