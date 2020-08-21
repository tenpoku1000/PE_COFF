
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static uint32_t encode_x64_2_operand_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2
);

uint32_t tp_encode_allocate_stack(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    uint32_t local_variable_size)
{
    uint32_t x64_code_size = 0;

    // Return Address
    symbol_table->member_register_bytes = (int32_t)sizeof(uint64_t);

    // PUSH – Push Operand onto the Stack
    uint32_t tmp_x64_code_size = tp_encode_x64_push_reg64(
        symbol_table, x64_code_buffer, x64_code_offset, TP_X64_64_REGISTER_RBP
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    {
        // RBP register.
        symbol_table->member_register_bytes += (int32_t)sizeof(uint64_t);

        int32_t nv_register_bytes = 0;

        for (int32_t i = TP_X64_NV64_REGISTER_NUM - 1; 0 <= i; --i){

            switch (symbol_table->member_use_nv_register[i]){
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
                    (TP_X64_64_REGISTER)(symbol_table->member_use_nv_register[i])
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

        // Other non-volatile registers.
        symbol_table->member_register_bytes += (nv_register_bytes * sizeof(uint64_t));

        symbol_table->member_padding_register_bytes =
            ((-(symbol_table->member_register_bytes)) & TP_PADDING_MASK);
    }

    // Temporary variables(see tp_make_x64_code function).

    // Local variables.
    {
        symbol_table->member_local_variable_size = (int32_t)local_variable_size;

        int32_t v = symbol_table->member_register_bytes +
            symbol_table->member_padding_register_bytes +
            symbol_table->member_temporary_variable_size +
            symbol_table->member_padding_temporary_variable_bytes +
            symbol_table->member_local_variable_size;

        symbol_table->member_padding_local_variable_bytes = ((-v) & TP_PADDING_MASK);
    }

    // Home space of function arguments register.
    const int32_t stack_param_size = (int32_t)(sizeof(uint64_t) * 4);

    symbol_table->member_stack_imm32 =
//      symbol_table->member_register_bytes +
        symbol_table->member_padding_register_bytes +
        symbol_table->member_temporary_variable_size +
        symbol_table->member_padding_temporary_variable_bytes +
        symbol_table->member_local_variable_size +
        symbol_table->member_padding_local_variable_bytes +
        stack_param_size;

    if (x64_code_buffer){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_HIDE,
            TP_MSG_FMT(
                "sub rsp, imm32\n"
                "symbol_table->member_stack_imm32(member_register_bytes is not included): %1\n"
                "symbol_table->member_register_bytes: %2\n"
                "symbol_table->member_padding_register_bytes: %3\n"
                "symbol_table->member_temporary_variable_size: %4\n"
                "symbol_table->member_padding_temporary_variable_bytes: %5\n"
                "symbol_table->member_local_variable_size: %6\n"
                "symbol_table->member_padding_local_variable_bytes: %7\n"
                "stack_param_size: %8"
            ),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_stack_imm32),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_register_bytes),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_padding_register_bytes),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_temporary_variable_size),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_padding_temporary_variable_bytes),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_local_variable_size),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_padding_local_variable_bytes),
            TP_LOG_PARAM_INT32_VALUE(stack_param_size)
        );
    }

    // SUB – Integer Subtraction
    tmp_x64_code_size = tp_encode_x64_add_sub_imm(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        TP_X64_SUB, TP_X64_64_REGISTER_RSP, symbol_table->member_stack_imm32, TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32
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

//  // int 3
//  if (x64_code_buffer){
//
//      x64_code_buffer[x64_code_offset + x64_code_size] = 0xcc;
//  }
//
//  ++x64_code_size;

    return x64_code_size;
}

uint32_t tp_encode_relocation_type_base(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset, 
    TP_WASM_RELOCATION* wasm_relocation)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

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

    // mov dst, rbp
    // NOTE: Not free RBP register.
    tmp_x64_code_size = tp_encode_x64_2_operand(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        TP_X64_MOV, &dst, &src
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    // push dst
    if ( ! tp_wasm_stack_push(symbol_table, &dst)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

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

    for (int32_t i = 0; TP_X64_NV64_REGISTER_NUM > i; ++i){

        switch (symbol_table->member_use_nv_register[i]){
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
                (TP_X64_64_REGISTER)(symbol_table->member_use_nv_register[i])
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
        TP_X64_ADD, TP_X64_64_REGISTER_RSP, symbol_table->member_stack_imm32, TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32
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
    uint32_t type_index, uint32_t reserved, TP_WASM_STACK_ELEMENT* op1)
{
    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE
    };

    bool is_need_return_value = false;
    int32_t arg_index = TP_WASM_ARG_INDEX_VOID;

    if (wasm_relocation){

        switch (wasm_relocation->member_type_return){
        case TP_WASM_RETURN_VOID:
            break;
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32:
            arg_index = TP_WASM_ARG_INDEX_EAX;
            is_need_return_value = true;
            break;
        case TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64:
            arg_index = TP_WASM_ARG_INDEX_EAX;
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
            x64_code_buffer, x64_code_offset, &x64_code_size, arg_index, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    // PUSH – Push Operand onto the Stack
    tmp_x64_code_size = tp_encode_x64_push_reg64(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, TP_X64_64_REGISTER_R10
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    symbol_table->member_use_v_register[TP_X64_V64_REGISTER_R10_INDEX] = TP_X64_V64_REGISTER_R10;

    // PUSH – Push Operand onto the Stack
    tmp_x64_code_size = tp_encode_x64_push_reg64(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, TP_X64_64_REGISTER_R11
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    symbol_table->member_use_v_register[TP_X64_V64_REGISTER_R11_INDEX] = TP_X64_V64_REGISTER_R11;

    tmp_x64_code_size = tp_encode_x64_call(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size, op1
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    for (int32_t i = TP_X64_V64_REGISTER_NUM - 1; 0 <= i; --i){

        switch (symbol_table->member_use_v_register[i]){
        case TP_X64_V64_REGISTER_NULL:
            break;
        case TP_X64_V64_REGISTER_R11:
//          break;
        case TP_X64_V64_REGISTER_R10:
//          break;
        case TP_X64_V64_REGISTER_R9:
//          break;
        case TP_X64_V64_REGISTER_R8:
//          break;
        case TP_X64_V64_REGISTER_RDX:
//          break;
        case TP_X64_V64_REGISTER_RCX:{
            // POP – Pop a Value from the Stack
            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                (TP_X64_64_REGISTER)(symbol_table->member_use_v_register[i])
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            symbol_table->member_use_v_register[i] = TP_X64_V64_REGISTER_NULL;
            break;
        }
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
    }

    if (is_need_return_value){

        if ( ! tp_wasm_stack_push(symbol_table, &result)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    return x64_code_size;
}

// Variable access

uint32_t tp_encode_get_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, bool* status)
{
    *status = false;

    if (local_index >= symbol_table->member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    uint32_t x64_code_size = 0;

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
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL
        };

        if ( ! tp_get_local_variable_offset(symbol_table, local_index, &(src.member_offset))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

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

        uint32_t wasm_opcode = TP_WASM_OPCODE_I32_VALUE;
        bool is_register = false;

        switch (symbol_table->member_local_types[local_index].member_type){
        case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32:
            break;
        case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64:
            wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            break;
        case TP_WASM_VAR_TYPE_PSEUDO_I32:
            if (4 <= local_index){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            is_register = true;
            break;
        case TP_WASM_VAR_TYPE_PSEUDO_I64:
            if (4 <= local_index){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            is_register = true;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        TP_WASM_STACK_ELEMENT src = {
            .member_wasm_opcode = wasm_opcode,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL
        };

        if (is_register){

            switch (local_index){
            case 0:
                src.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_ECX;
                break;
            case 1:
                src.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                src.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;
                break;
            case 2:
                src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                src.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R8D;
                break;
            case 3:
                src.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                src.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R9D;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }

        if ( ! tp_get_local_variable_offset(symbol_table, local_index, &(src.member_offset))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = wasm_opcode };

        if (false == is_register){

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
        }

        if ( ! tp_wasm_stack_push(symbol_table, (is_register ? &src : &dst))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }
    }

    *status = true;

    return x64_code_size;
}

uint32_t tp_encode_set_local_code(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, uint32_t local_index, TP_WASM_STACK_ELEMENT* op1)
{
    if (local_index >= symbol_table->member_local_types_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

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
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL
        };

        if ( ! tp_get_local_variable_offset(symbol_table, local_index, &(dst.member_offset))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV, &dst, op1
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else{

        uint32_t wasm_opcode = TP_WASM_OPCODE_I32_VALUE;
        bool is_register = false;

        switch (symbol_table->member_local_types[local_index].member_type){
        case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32:
            if (TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            break;
        case TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64:
            if (TP_WASM_OPCODE_I64_VALUE != op1->member_wasm_opcode){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            break;
        case TP_WASM_VAR_TYPE_PSEUDO_I32:
            if (TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            if (4 <= local_index){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            is_register = true;
            break;
        case TP_WASM_VAR_TYPE_PSEUDO_I64:
            if (TP_WASM_OPCODE_I64_VALUE != op1->member_wasm_opcode){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            if (4 <= local_index){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
            wasm_opcode = TP_WASM_OPCODE_I64_VALUE;
            is_register = true;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        TP_WASM_STACK_ELEMENT dst = {
            .member_wasm_opcode = wasm_opcode,
            .member_local_index = local_index,
            .member_x64_item_kind = TP_X64_ITEM_KIND_MEMORY,
            .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_LOCAL
        };

        if (is_register){

            switch (local_index){
            case 0:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                dst.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_ECX;
                break;
            case 1:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER;
                dst.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;
                break;
            case 2:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                dst.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R8D;
                break;
            case 3:
                dst.member_x64_item_kind = TP_X64_ITEM_KIND_X64_32_REGISTER;
                dst.member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_R9D;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }
        }

        if ( ! tp_get_local_variable_offset(symbol_table, local_index, &(dst.member_offset))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
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
        ((TP_WASM_RELOCATION_TYPE_REL32 == op1_wasm_relocation->member_type) ||
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type))){

        is_rel = true;
    }

    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

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
    dst.member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE;

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
        ((TP_WASM_RELOCATION_TYPE_REL32 == op1_wasm_relocation->member_type) ||
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type))){

        is_rel = true;
    }

    // mov dst, op1;
    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };
    TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

    if (is_i64){

        dst = *op1; 
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

        if (!tp_allocate_temporary_variable(
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
            (is_rel ? TP_X64_MOV_RIP : TP_X64_MOV), &dst, (is_i64 ? op2 : &src)
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
        ((TP_WASM_RELOCATION_TYPE_REL32 == op1_wasm_relocation->member_type) ||
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type))){

        is_rel = true;
    }

    TP_WASM_STACK_ELEMENT dst = { .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE };

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
        ((TP_WASM_RELOCATION_TYPE_REL32 == op1_wasm_relocation->member_type) ||
        (TP_WASM_RELOCATION_TYPE_REL64 == op1_wasm_relocation->member_type))){

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

    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE
    };

    if ( ! tp_allocate_temporary_variable(
        symbol_table, TP_X64_ALLOCATE_DEFAULT,
        x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX(wasm_relocation), &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    result.member_wasm_relocation = wasm_relocation;

    if ((NULL == wasm_relocation) ||
        (wasm_relocation && (TP_WASM_RELOCATION_TYPE_REL32 != wasm_relocation->member_type))){

        tmp_x64_code_size = tp_encode_x64_mov_imm(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            value, TP_X64_MOV_IMM_MODE_FORCE_IMM32, &result
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }else if (wasm_relocation && (TP_WASM_RELOCATION_TYPE_STRING_LITERAL32 == wasm_relocation->member_type)){

        TP_WASM_STACK_ELEMENT src = { .member_wasm_opcode = TP_WASM_OPCODE_I32_VALUE };

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
        src.member_offset = (int32_t)(wasm_relocation->member_offset32);

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

    TP_WASM_STACK_ELEMENT result = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE
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

    if ( ! tp_wasm_stack_push(symbol_table, &result)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    *status = true;

    return x64_code_size;
}

// Comparison operators

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

    if ((TP_WASM_OPCODE_I32_VALUE != (op1)->member_wasm_opcode) &&
        (TP_WASM_OPCODE_I64_VALUE != (op1)->member_wasm_opcode)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    TP_WASM_STACK_ELEMENT left_op = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE
    };
    TP_WASM_STACK_ELEMENT right_op = {
        .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE
    };

    if (symbol_table->member_is_32){

        TP_WASM_RELOCATION* op1_relocation = op1->member_wasm_relocation;
        TP_WASM_RELOCATION* op2_relocation = op2->member_wasm_relocation;

        bool is_op1_32 = true;

        if (TP_WASM_OPCODE_I64_VALUE == op1->member_wasm_opcode){

            if (TP_WASM_OPCODE_I64_VALUE == op2->member_wasm_opcode){

                goto common;
            }

            is_op1_32 = false;
        }else if (TP_WASM_OPCODE_I64_VALUE == op2->member_wasm_opcode){

            if (TP_WASM_OPCODE_I64_VALUE == op1->member_wasm_opcode){

                goto common;
            }
        }else{

            goto common;
        }

        if ( ! tp_allocate_temporary_variable(
            symbol_table, TP_X64_ALLOCATE_DEFAULT,
            x64_code_buffer, x64_code_offset, &x64_code_size,
            TP_WASM_ARG_INDEX_VOID, (is_op1_32 ? &left_op : &right_op))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return 0;
        }

        if (is_op1_32){

            left_op.member_wasm_relocation = op1_relocation;
        }else{

            right_op.member_wasm_relocation = op2_relocation;
        }

        // NOTE: Call tp_free_register(op1) in tp_encode_x64_2_operand().
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_MOV_32_TO_64, (is_op1_32 ? &left_op : &right_op), (is_op1_32 ? op1 : op2)
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

        if (is_op1_32){

            right_op = *op2;
        }else{

            left_op = *op1;
        }
    }else{

common:
        ;

        if (op1->member_wasm_opcode != op2->member_wasm_opcode){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return 0;
        }

        left_op = *op1;
        right_op = *op2;
    }

    /* NOTE: Call tp_free_register(op2) in tp_encode_x64_2_operand(). */
    tmp_x64_code_size = tp_encode_x64_2_operand(
        symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
        x64_op, &left_op, &right_op
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    int32_t arg_index = TP_WASM_ARG_INDEX(wasm_relocation);

    if (TP_WASM_ARG_INDEX_VOID != arg_index){

        TP_WASM_STACK_ELEMENT result = {
            .member_wasm_opcode = left_op.member_wasm_opcode
        };

        if (!tp_allocate_temporary_variable(
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

