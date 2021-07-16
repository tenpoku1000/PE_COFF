
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static uint32_t encode_x64_32_register_to_x64_32_register(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src
);
static uint32_t mov_src_reg_to_dst_reg(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src
);
static uint32_t encode_x64_32_register_to_memory_offset(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src
);
static uint32_t encode_x64_32_memory_offset_to_register(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src
);
static uint32_t encode_x64_32_memory_offset_common(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_X64_DIRECTION x64_direction, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src
);

uint32_t tp_encode_x64_2_operand(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* op1, TP_WASM_STACK_ELEMENT* op2)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if ((TP_X64_MOV_32_TO_64 != x64_op) &&
        (op1->member_wasm_opcode != op2->member_wasm_opcode)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    if ((TP_WASM_OPCODE_I32_VALUE != op1->member_wasm_opcode) &&
        (TP_WASM_OPCODE_I64_VALUE != op1->member_wasm_opcode)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    switch (op1->member_x64_item_kind){
    case TP_X64_ITEM_KIND_X86_32_REGISTER:
//      break;
    case TP_X64_ITEM_KIND_X64_32_REGISTER:
        switch (op2->member_x64_item_kind){
        case TP_X64_ITEM_KIND_X86_32_REGISTER:
//          break;
        case TP_X64_ITEM_KIND_X64_32_REGISTER:
            x64_code_size = encode_x64_32_register_to_x64_32_register(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                x64_op, op1, op2
            );
            break;
        case TP_X64_ITEM_KIND_MEMORY:
            x64_code_size = encode_x64_32_memory_offset_to_register(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                x64_op, op1, op2
            );
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
        break;
    case TP_X64_ITEM_KIND_MEMORY:
        switch (op2->member_x64_item_kind){
        case TP_X64_ITEM_KIND_X86_32_REGISTER:
//          break;
        case TP_X64_ITEM_KIND_X64_32_REGISTER:
            x64_code_size = encode_x64_32_register_to_memory_offset(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                x64_op, op1, op2
            );
            break;
        case TP_X64_ITEM_KIND_MEMORY:
        {
            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            TP_WASM_STACK_ELEMENT eax_op2 = {
                .member_wasm_opcode = op2->member_wasm_opcode,
                .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                .member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX,
                .member_is_tmp_reg = true
            };

            tmp_x64_code_size = encode_x64_32_memory_offset_to_register(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_MOV, &eax_op2, op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            tmp_x64_code_size = encode_x64_32_register_to_memory_offset(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                x64_op, op1, &eax_op2
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
            break;
        }
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    if ( ! tp_free_register(symbol_table, op2)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    return x64_code_size;
}

static uint32_t encode_x64_32_register_to_x64_32_register(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    bool is_i64 = (TP_WASM_OPCODE_I64_VALUE == dst->member_wasm_opcode);

    bool is_dst_x86_32_register = (TP_X64_ITEM_KIND_X86_32_REGISTER == dst->member_x64_item_kind);
    bool is_dst_EAX_register = (is_dst_x86_32_register &&
        (TP_X86_32_REGISTER_EAX == dst->member_x64_item.member_x86_32_register)
    );
    bool is_dst_x64_32_register = (TP_X64_ITEM_KIND_X64_32_REGISTER == dst->member_x64_item_kind);

    bool is_use_EDX_register = (TP_X64_ITEM_KIND_X86_32_REGISTER ==
        symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX].member_x64_item_kind
    );

    bool is_src_x86_32_register = (TP_X64_ITEM_KIND_X86_32_REGISTER == src->member_x64_item_kind);
    bool is_src_EDX_register = (is_src_x86_32_register &&
        (TP_X86_32_REGISTER_EDX == src->member_x64_item.member_x86_32_register)
    );
    bool is_src_x64_32_register = (TP_X64_ITEM_KIND_X64_32_REGISTER == src->member_x64_item_kind);

    if (TP_X64_IDIV == x64_op){

        if (false == is_dst_EAX_register){

            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            TP_WASM_STACK_ELEMENT dst_eax = {
                .member_wasm_opcode = dst->member_wasm_opcode,
                .member_wasm_relocation = NULL,
                .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                .member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL,
                .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_NONE
            };

            dst_eax.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX;
            dst_eax.member_offset = dst->member_offset;

            // NOTE: Call tp_free_register(src) in tp_encode_x64_2_operand().
            tmp_x64_code_size = tp_encode_x64_2_operand(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_MOV, &dst_eax, dst/* src */
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (is_use_EDX_register){

            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RDX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (is_src_EDX_register){

            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RBX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (is_src_EDX_register){

            TP_WASM_STACK_ELEMENT dst_rbx = {
                .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
                .member_wasm_relocation = NULL,
                .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                .member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL,
                .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_NONE,
                .member_offset = 0,
                .member_is_tmp_reg = true
            };
            dst_rbx.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EBX;

            TP_WASM_STACK_ELEMENT src_rdx = {
                .member_wasm_opcode = TP_WASM_OPCODE_I64_VALUE,
                .member_wasm_relocation = NULL,
                .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                .member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL,
                .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_NONE,
                .member_offset = 0,
                .member_is_tmp_reg = true
            };
            src_rdx.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX;

            // mov rbx, rdx
            // NOTE: Not free RDX register.
            tmp_x64_code_size = mov_src_reg_to_dst_reg(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                &dst_rbx, &src_rdx
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        TP_WASM_STACK_ELEMENT edx_op = {
            .member_wasm_opcode = dst->member_wasm_opcode,
            .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
            .member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX
        };

        // xor edx, edx
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_XOR, &edx_op, &edx_op
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    bool is_need_rex = (((TP_X64_MOV_32_TO_64 == x64_op) ? false : is_i64) ||
        is_dst_x64_32_register || is_src_x64_32_register);

    if (x64_code_buffer){

        if (is_need_rex){

            switch (x64_op){
            case TP_X64_IMUL:
                if ( ! is_dst_EAX_register){

                    goto rex_common;
                }
//              break;
            case TP_X64_IDIV:
                x64_code_buffer[x64_code_offset + x64_code_size] = (0x40 |
                    /* W */ (is_i64 ? 0x08 : 0x00) |
                    /* B */ (is_src_x64_32_register ? 0x01 : 0x00)
                );
                break;
            default:
rex_common:
                x64_code_buffer[x64_code_offset + x64_code_size] = (0x40 |
                    /* W */ ((TP_X64_MOV_32_TO_64 == x64_op) ? 0x00 : (is_i64 ? 0x08 : 0x00)) |
                    /* R */ (is_dst_x64_32_register ? 0x04 : 0x00) |
                    /* B */ (is_src_x64_32_register ? 0x01 : 0x00)
                );
                break;
            }

            ++x64_code_size;
        }

        switch (x64_op){
        case TP_X64_MOV_32_TO_64:
//          break;
        case TP_X64_MOV:
            // MOV – Move Data
            // register2 to register1 1000 101w : 11 reg1 reg2
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_MOV_REG; // 0x8b
            break;
        case TP_X64_ADD:
            // ADD
            // register2 to register1 0000 001w : 11 reg1 reg2
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_ADD_REG; // 0x03
            break;
        case TP_X64_SUB:
            // SUB – Integer Subtraction
            // register2 to register1 0010 101w : 11 reg1 reg2
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_SUB; // 0x2b
            break;
        case TP_X64_IMUL:
            // IMUL – Signed Multiply
            if (is_dst_EAX_register){

                // AL, AX, or EAX with register 1111 011w : 11 101 reg
                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IDIV_IMUL_EAX_1; // 0xf7
            }else{

                // register1 with register2 0000 1111 : 1010 1111 : 11 : reg1 reg2
                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IMUL_1; // 0x0f

                ++x64_code_size;

                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IMUL_2; // 0xaf
            }
            break;
        case TP_X64_IDIV:
            // IDIV – Signed Divide
            // AL, AX, or EAX by register 1111 011w : 11 111 reg
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IDIV_IMUL_EAX_1; // 0xf7
            break;
        case TP_X64_XOR:
            // XOR – Logical Exclusive OR
            // register2 to register1 0011 001w : 11 reg1 reg2
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_XOR; // 0x33
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        ++x64_code_size;

        // ModR/M
        switch (x64_op){
        case TP_X64_IMUL:
            if (is_dst_EAX_register){

                // AL, AX, or EAX with register : 11 101 reg
                x64_code_buffer[x64_code_offset + x64_code_size] = ((0x3 << 6) |
                    ((TP_X64_OPCODE_IMUL_EAX_2/* 0x5 */ & 0x07) << 3) |
                    ((is_src_x86_32_register ? src->member_x64_item.member_x86_32_register :
                    src->member_x64_item.member_x64_32_register) & 0x07)
                );
            }else{

                goto mod_rm_common;
            }
            break;
        case TP_X64_IDIV:
            if (is_src_EDX_register){

                // AL, AX, or EAX by register : 11 111 EBX
                x64_code_buffer[x64_code_offset + x64_code_size] = ((0x3 << 6) |
                    ((TP_X64_OPCODE_IDIV_2/* 0x7 */ & 0x07) << 3) |
                    (TP_X86_32_REGISTER_EBX & 0x07)
                );
            }else{

                // AL, AX, or EAX by register : 11 111 reg
                x64_code_buffer[x64_code_offset + x64_code_size] = ((0x3 << 6) |
                        ((TP_X64_OPCODE_IDIV_2/* 0x7 */ & 0x07) << 3) |
                    ((is_src_x86_32_register ? src->member_x64_item.member_x86_32_register :
                    src->member_x64_item.member_x64_32_register) & 0x07)
                );
            }
            break;
        default:
mod_rm_common:
            // register2 to register1 : 11 reg1 reg2
            x64_code_buffer[x64_code_offset + x64_code_size] = ((0x03 << 6) |
                (((is_dst_x86_32_register ? dst->member_x64_item.member_x86_32_register :
                dst->member_x64_item.member_x64_32_register) & 0x07) << 3) |
                ((is_src_x86_32_register ? src->member_x64_item.member_x86_32_register :
                src->member_x64_item.member_x64_32_register) & 0x07)
            );
            break;
        }

        ++x64_code_size;
    }else{

        if (is_need_rex){

            ++x64_code_size;
        }

        if ((TP_X64_IMUL == x64_op) && (false == is_dst_EAX_register)){

            ++x64_code_size;
        }

        x64_code_size += 2;
    }

    if (TP_X64_IDIV == x64_op){

        if (false == is_dst_EAX_register){

            TP_WASM_STACK_ELEMENT result = {
                .member_wasm_opcode = dst->member_wasm_opcode
            };

            if ( ! tp_allocate_temporary_variable(
                symbol_table, TP_X64_ALLOCATE_MEMORY,
                x64_code_buffer, x64_code_offset, &x64_code_size, TP_WASM_ARG_INDEX_VOID, &result)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }

            TP_WASM_STACK_ELEMENT src_eax = {
                .member_wasm_opcode = dst->member_wasm_opcode,
                .member_wasm_relocation = NULL,
                .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
                .member_x64_item.member_x64_32_register = TP_X64_32_REGISTER_NULL,
                .member_x64_memory_kind = TP_X64_ITEM_MEMORY_KIND_NONE,
                .member_offset = 0
            };

            src_eax.member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EAX;
            src_eax.member_is_tmp_reg = true;

            // NOTE: Not free src register(for src_eax.member_is_tmp_reg = true).
            tmp_x64_code_size = tp_encode_x64_2_operand(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_MOV, &result, &src_eax
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

            if ( ! tp_wasm_stack_push(symbol_table, &result)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return 0;
            }
        }

        if (is_src_EDX_register){

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RBX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (is_use_EDX_register){

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RDX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (false == is_dst_EAX_register){

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }
    }

    return x64_code_size;
}

static uint32_t mov_src_reg_to_dst_reg(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src)
{
    uint32_t x64_code_size = 0;

    // NOTE: Not free src register(for src.member_is_tmp_reg = true).
    uint32_t tmp_x64_code_size = tp_encode_x64_2_operand(
        symbol_table, x64_code_buffer, x64_code_offset,
        TP_X64_MOV, dst, src
    );
    TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);

    return x64_code_size;
}

static uint32_t encode_x64_32_register_to_memory_offset(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src)
{
    return encode_x64_32_memory_offset_common(
        symbol_table, x64_code_buffer, x64_code_offset,
        x64_op, TP_X64_DIRECTION_SOURCE_REGISTER, dst, src
    );
}

static uint32_t encode_x64_32_memory_offset_to_register(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src)
{
    return encode_x64_32_memory_offset_common(
        symbol_table, x64_code_buffer, x64_code_offset,
        x64_op, TP_X64_DIRECTION_SOURCE_MEMORY, dst, src
    );
}

static uint32_t encode_x64_32_memory_offset_common(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_X64_DIRECTION x64_direction, TP_WASM_STACK_ELEMENT* dst, TP_WASM_STACK_ELEMENT* src)
{
    uint32_t x64_code_size = 0;
    uint32_t tmp_x64_code_size = 0;

    if ((TP_X64_ITEM_KIND_MEMORY != dst->member_x64_item_kind) &&
        (TP_X64_ITEM_KIND_MEMORY != src->member_x64_item_kind)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("TP_X64_ITEM_KIND_MEMORY(%1) != dst_op(%2), src_op(%3)"),
            TP_LOG_PARAM_INT32_VALUE(TP_X64_ITEM_KIND_MEMORY),
            TP_LOG_PARAM_INT32_VALUE(dst->member_x64_item_kind),
            TP_LOG_PARAM_INT32_VALUE(src->member_x64_item_kind)
        );

        return 0;
    }

    bool is_i64 = (TP_WASM_OPCODE_I64_VALUE == dst->member_wasm_opcode);

    bool is_source_memory = (TP_X64_DIRECTION_SOURCE_MEMORY == x64_direction);

    bool is_global = (is_source_memory ?
        (TP_X64_ITEM_MEMORY_KIND_GLOBAL == src->member_x64_memory_kind) :
        (TP_X64_ITEM_MEMORY_KIND_GLOBAL == dst->member_x64_memory_kind));

    int32_t offset = (is_source_memory ? src->member_offset : dst->member_offset);

    bool is_disp8 = (is_source_memory ? (
        (INT8_MIN <= src->member_offset) &&
        (INT8_MAX >= src->member_offset)
    ) : (
        (INT8_MIN <= dst->member_offset) &&
        (INT8_MAX >= dst->member_offset)
    ));

    bool is_dst_x86_32_register = ((TP_X64_ITEM_MEMORY_KIND_GLOBAL == dst->member_x64_memory_kind) ?
        (TP_X64_ITEM_KIND_X86_32_REGISTER == dst->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X86_32_REGISTER == dst->member_x64_item_kind)
    );
    bool is_dst_EAX_register = (is_dst_x86_32_register &&
        (TP_X86_32_REGISTER_EAX == dst->member_x64_item.member_x86_32_register)
    );
    bool is_use_EDX_register = (TP_X64_ITEM_KIND_X86_32_REGISTER ==
        symbol_table->member_use_X86_32_register[TP_X86_32_REGISTER_EDX].member_x64_item_kind
    );
    bool is_dst_x64_32_register = ((TP_X64_ITEM_MEMORY_KIND_GLOBAL == dst->member_x64_memory_kind) ?
        (TP_X64_ITEM_KIND_X64_32_REGISTER == dst->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X64_32_REGISTER == dst->member_x64_item_kind)
    );

    bool is_src_x86_32_register = ((TP_X64_ITEM_MEMORY_KIND_GLOBAL == src->member_x64_memory_kind) ?
        (TP_X64_ITEM_KIND_X86_32_REGISTER == src->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X86_32_REGISTER == src->member_x64_item_kind)
    );
    bool is_src_x64_32_register = ((TP_X64_ITEM_MEMORY_KIND_GLOBAL == src->member_x64_memory_kind) ?
        (TP_X64_ITEM_KIND_X64_32_REGISTER == src->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X64_32_REGISTER == src->member_x64_item_kind)
    );

    if (TP_X64_IDIV == x64_op){

        if (false == is_dst_EAX_register){

            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (is_use_EDX_register){

            tmp_x64_code_size = tp_encode_x64_push_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RDX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        TP_WASM_STACK_ELEMENT edx_op = {
            .member_wasm_opcode = dst->member_wasm_opcode,
            .member_x64_item_kind = TP_X64_ITEM_KIND_X86_32_REGISTER,
            .member_x64_item.member_x86_32_register = TP_X86_32_REGISTER_EDX
        };

        // xor edx, edx
        tmp_x64_code_size = tp_encode_x64_2_operand(
            symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
            TP_X64_XOR, &edx_op, &edx_op
        );
        TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
    }

    bool is_need_rex = (((TP_X64_MOV_32_TO_64 == x64_op) ? false : is_i64) ||
        is_dst_x64_32_register || is_src_x64_32_register);

    if (x64_code_buffer){

        if (is_need_rex){

            switch (x64_op){
            case TP_X64_IMUL:
                if ( ! is_dst_EAX_register){

                    goto rex_common;
                }
//              break;
            case TP_X64_IDIV:
                x64_code_buffer[x64_code_offset + x64_code_size] = (0x40 |
                    /* W */ (is_i64 ? 0x08 : 0x00) |
                    /* B */ (is_src_x64_32_register ? 0x01 : 0x00)
                );
                break;
            default:
rex_common:
                x64_code_buffer[x64_code_offset + x64_code_size] = (0x40 |
                    /* W */ ((TP_X64_MOV_32_TO_64 == x64_op) ? 0x00 : (is_i64 ? 0x08 : 0x00)) |
                    /* R */ (is_source_memory ? 
                                (is_dst_x64_32_register ? 0x04 : 0x00) : 
                                (is_src_x64_32_register ? 0x04 : 0x00)) |
                    /* B */ ((TP_X64_MOV_RIP != x64_op) ?
                            (is_source_memory ? 
                                (is_src_x64_32_register ? 0x01 : 0x00) : 
                                (is_dst_x64_32_register ? 0x01 : 0x00))
                            : 0x00)
                );
                break;
            }

            ++x64_code_size;
        }

        switch (x64_op){
        case TP_X64_MOV_32_TO_64:
//          break;
        case TP_X64_MOV:
//          break;
        case TP_X64_MOV_RIP:
            // MOV – Move Data
            // memory to reg 1000 101w : mod reg r/m
            // reg to memory 1000 100w : mod reg r/m
            x64_code_buffer[x64_code_offset + x64_code_size] =
                (TP_X64_OPCODE_MOV_DST_MEM/* 0x89 */ | (is_source_memory ? 0x02 : 0x00));
            break;
        case TP_X64_ADD:
            // ADD
            // memory to register 0000 001w : mod reg r/m
            // register to memory 0000 000w : mod reg r/m
            x64_code_buffer[x64_code_offset + x64_code_size] =
                (TP_X64_OPCODE_ADD_DST_MEM/* 0x01 */ | (is_source_memory ? 0x02 : 0x00));
            break;
        case TP_X64_SUB:
            // SUB – Integer Subtraction
            // memory to register 0010 101w : mod reg r/m
            // register to memory 0010 100w : mod reg r/m
            x64_code_buffer[x64_code_offset + x64_code_size] =
                (TP_X64_OPCODE_SUB_DST_MEM/* 0x29 */ | (is_source_memory ? 0x02 : 0x00));
            break;
        case TP_X64_IMUL:
            // IMUL – Signed Multiply
            // AL, AX, or EAX with memory 1111 011w : mod 101 reg
            // register with memory 0000 1111 : 1010 1111 : mod reg r/m
            if (is_dst_EAX_register){

                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IDIV_IMUL_EAX_1; // 0xf7
            }else{

                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IMUL_1; // 0x0f

                ++x64_code_size;

                x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IMUL_2; // 0xaf
            }
            break;
        case TP_X64_IDIV:
            // IDIV – Signed Divide
            // AL, AX, or EAX by memory 1111 011w : mod 111 r/m
            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_IDIV_IMUL_EAX_1; // 0xf7
            break;
        case TP_X64_XOR:
            // XOR – Logical Exclusive OR
            // memory to register 0011 001w : mod reg r/m
            // register to memory 0011 000w : mod reg r/m
            x64_code_buffer[x64_code_offset + x64_code_size] =
                (TP_X64_OPCODE_XOR_DST_MEM/* 0x31 */ | (is_source_memory ? 0x02 : 0x00));
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        ++x64_code_size;

        // ModR/M
        switch (x64_op){
        case TP_X64_IMUL:
            if (is_dst_EAX_register){

                // AL, AX, or EAX with memory 1111 011w : mod 101 reg
                x64_code_buffer[x64_code_offset + x64_code_size] = (
                    ((TP_X64_MOV_RIP != x64_op) ? (is_disp8 ? 0x44 : 0x84) : 0x05) |
                    ((TP_X64_OPCODE_IMUL_EAX_2/* 0x5 */ & 0x07) << 3)
                );
            }else{

                goto mod_rm_common;
            }
            break;
        case TP_X64_IDIV:
            // AL, AX, or EAX by memory 1111 011w : mod 111 r/m
            x64_code_buffer[x64_code_offset + x64_code_size] = (
                ((TP_X64_MOV_RIP != x64_op) ? (is_disp8 ? 0x44 : 0x84) : 0x05) |
                ((TP_X64_OPCODE_IDIV_2/* 0x07 */ & 0x07) << 3)
            );
            break;
        default:
mod_rm_common:
            // mod reg r/m
            x64_code_buffer[x64_code_offset + x64_code_size] = (
                ((TP_X64_MOV_RIP != x64_op) ? (is_disp8 ? 0x44 : 0x84) : 0x05) |
                (is_source_memory ?
                    (((is_dst_x86_32_register ? dst->member_x64_item.member_x86_32_register :
                    dst->member_x64_item.member_x64_32_register) & 0x07) << 3)
                    :
                    (((is_src_x86_32_register ? src->member_x64_item.member_x86_32_register :
                    src->member_x64_item.member_x64_32_register) & 0x07) << 3)
                )
            );
            break;
        }

        ++x64_code_size;

        // SIB
        if (TP_X64_MOV_RIP != x64_op){

            if (is_global){

                x64_code_buffer[x64_code_offset + x64_code_size] = (
                    ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) |
                    (is_source_memory ?
                        ((is_src_x86_32_register ? src->member_x64_item.member_x86_32_register :
                        src->member_x64_item.member_x64_32_register) & 0x07)
                        :
                        ((is_dst_x86_32_register ? dst->member_x64_item.member_x86_32_register :
                        dst->member_x64_item.member_x64_32_register) & 0x07)
                    )
                );
            }else{

                x64_code_buffer[x64_code_offset + x64_code_size] = (
                    ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) | (TP_X64_64_REGISTER_RBP & 0x07)
                );
            }

            ++x64_code_size;
        }

        // Address displacement
        if (is_disp8 && (TP_X64_MOV_RIP != x64_op)){

            x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)offset;

            ++x64_code_size;
        }else{

            if (TP_X64_MOV_RIP == x64_op){

                TP_WASM_RELOCATION* wasm_relocation =
                    (is_source_memory ? src->member_wasm_relocation : dst->member_wasm_relocation);

                bool is_code = (wasm_relocation && (wasm_relocation->member_symbol_table_index) &&
                    (TP_WASM_RELOCATION_TYPE_REL64_CODE == wasm_relocation->member_type));

                bool is_data = (wasm_relocation && (wasm_relocation->member_symbol_table_index) &&
                    (TP_WASM_RELOCATION_TYPE_REL64 == wasm_relocation->member_type));

                bool is_rdata = (wasm_relocation && (wasm_relocation->member_symbol_table_index) &&
                    (TP_WASM_RELOCATION_TYPE_STRING_LITERAL64 == wasm_relocation->member_type));

                if (is_code || is_data || is_rdata){

                    uint32_t virtual_address = x64_code_offset + x64_code_size;
                    uint32_t symbol_table_index = wasm_relocation->member_symbol_table_index;

                    if ( ! tp_append_coff_relocation(
                        symbol_table, TP_SECTION_KIND_TEXT, virtual_address, symbol_table_index)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return 0;
                    }
                }else{

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return 0;
                }

                offset = 0;
            }

            memcpy(
                &(x64_code_buffer[x64_code_offset + x64_code_size]), &offset,
                sizeof(offset)
            );

            x64_code_size += 4;
        }
    }else{

        if (is_need_rex){

            ++x64_code_size;
        }

        if ((TP_X64_IMUL == x64_op) && (false == is_dst_EAX_register)){

            ++x64_code_size;
        }

        x64_code_size += 2;

        if (TP_X64_MOV_RIP != x64_op){

            ++x64_code_size;
        }

        if (is_disp8 && (TP_X64_MOV_RIP != x64_op)){

            ++x64_code_size;
        }else{

            x64_code_size += 4;
        }
    }

    if (TP_X64_IDIV == x64_op){

        if (is_use_EDX_register){

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RDX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }

        if (false == is_dst_EAX_register){

            tmp_x64_code_size = tp_encode_x64_pop_reg64(
                symbol_table, x64_code_buffer, x64_code_offset + x64_code_size,
                TP_X64_64_REGISTER_RAX
            );
            TP_X64_CHECK_CODE_SIZE(symbol_table, x64_code_size, tmp_x64_code_size);
        }
    }

    return x64_code_size;
}

