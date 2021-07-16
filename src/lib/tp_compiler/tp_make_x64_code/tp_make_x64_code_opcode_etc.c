
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

uint32_t tp_encode_x64_mov_imm(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    int64_t imm, TP_X64_MOV_IMM_MODE x64_mov_imm_mode, TP_WASM_STACK_ELEMENT* result)
{
    uint32_t x64_code_size = 0;

    bool is_disp8 = (
        (INT8_MIN <= result->member_offset) &&
        (INT8_MAX >= result->member_offset)
    );

    bool is_i64 = (TP_WASM_OPCODE_I64_VALUE == result->member_wasm_opcode);

    bool is_imm8 = ((INT8_MIN <= imm) && (INT8_MAX >= imm));
    bool is_imm32 = ((false == is_imm8) && (INT32_MIN <= imm) && (INT32_MAX >= imm));
    bool is_imm64 = ((false == is_imm8) && (false == is_imm32) && (INT64_MIN <= imm) && (INT64_MAX >= imm));

    if (is_imm64 &&
        ((false == is_i64) || (TP_X64_ITEM_KIND_MEMORY == result->member_x64_item_kind))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    switch (x64_mov_imm_mode){
    case TP_X64_MOV_IMM_MODE_DEFAULT:
        break;
    case TP_X64_MOV_IMM_MODE_FORCE_IMM32:
        if (is_imm64){
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }else if (is_imm8){
            is_imm8 = false;
            is_imm32 = true;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    // MOV – Move Data
    if (x64_code_buffer){

        switch (result->member_x64_item_kind){
        case TP_X64_ITEM_KIND_X86_32_REGISTER:
            if (is_imm64){

                // immediate64 to qwordregister 0100 100B : 1011 1 reg : imm64
                x64_code_buffer[x64_code_offset] = 0x48;
                x64_code_buffer[x64_code_offset + 1] = (
                    TP_X64_OPCODE_MOV_IMM_32_64/* 0xb8 */ | (result->member_x64_item.member_x86_32_register & 0x07)
                );

                x64_code_size = 2;
            }else{

                if (is_i64){

                    is_imm8 = false;
                    is_imm32 = false;
                    is_imm64 = true;

                    x64_code_buffer[x64_code_offset] = 0x48;

                    ++x64_code_size;
                }

                // immediate to register (alternate encoding) 1011 w reg : imm
                x64_code_buffer[x64_code_offset + x64_code_size] = (
                    TP_X64_OPCODE_MOV_IMM/* 0xb0 */ | (is_imm8 ? 0x00 : 0x08) |
                    (result->member_x64_item.member_x86_32_register & 0x07)
                );

                ++x64_code_size;
            }
            break;
        case TP_X64_ITEM_KIND_X64_32_REGISTER:
            if (is_imm64){

                // immediate64 to qwordregister 0100 100B : 1011 1 reg : imm64
                x64_code_buffer[x64_code_offset] = 0x49;
                x64_code_buffer[x64_code_offset + 1] = (
                    TP_X64_OPCODE_MOV_IMM_32_64/* 0xb8 */ | (result->member_x64_item.member_x64_32_register & 0x07)
                );
            }else{

                if (is_i64){

                    is_imm8 = false;
                    is_imm32 = false;
                    is_imm64 = true;
                }

                // immediate to register (alternate encoding) 0100 000B : 1011 w reg : imm
                x64_code_buffer[x64_code_offset] = (is_i64 ? 0x49 : 0x41);
                x64_code_buffer[x64_code_offset + 1] = (
                    TP_X64_OPCODE_MOV_IMM/* 0xb0 */ | (is_imm8 ? 0x00 : 0x08) |
                    (result->member_x64_item.member_x64_32_register & 0x07)
                );
            }
            x64_code_size = 2;
            break;
        case TP_X64_ITEM_KIND_MEMORY:

            // immediate to memory 1100 011w : mod 000 r/m : immediate data
            x64_code_buffer[x64_code_offset] = TP_X64_OPCODE_MOV_IMM_MEM/* 0xc7 */;

            // ModR/M
            if (is_disp8){

                // disp8
                x64_code_buffer[x64_code_offset + 1] = 0x44;
            }else{

                // disp32
                x64_code_buffer[x64_code_offset + 1] = 0x84;
            }

            // SIB
            if (TP_X64_ITEM_MEMORY_KIND_GLOBAL == result->member_x64_memory_kind){

                x64_code_buffer[x64_code_offset + 2] = (
                    ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) |
                    (((TP_X64_ITEM_KIND_X86_32_REGISTER == result->member_x64_memory_item_kind) ?
                    result->member_x64_item.member_x86_32_register :
                    result->member_x64_item.member_x64_32_register) & 0x07)
                );
            }else{

                x64_code_buffer[x64_code_offset + 2] = (
                    ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) | (TP_X64_64_REGISTER_RBP & 0x07)
                );
            }

            x64_code_size = 3;

            // Address displacement
            if (is_disp8){

                x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)(result->member_offset);

                ++x64_code_size;
            }else{

                memcpy(
                    &(x64_code_buffer[x64_code_offset + x64_code_size]), &(result->member_offset),
                    sizeof(result->member_offset)
                );

                x64_code_size += 4;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        // Immediate data
        if (is_imm8){

            x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)imm;

            ++x64_code_size;
        }else if (is_imm32){

            int32_t imm32 = (int32_t)imm;

            memcpy(&(x64_code_buffer[x64_code_offset + x64_code_size]), &imm32, sizeof(imm32));

            x64_code_size += 4;
        }else{

            memcpy(&(x64_code_buffer[x64_code_offset + x64_code_size]), &imm, sizeof(imm));

            x64_code_size += 8;
        }
    }else{

        switch (result->member_x64_item_kind){
        case TP_X64_ITEM_KIND_X86_32_REGISTER:
            if (is_imm64){

                x64_code_size = 2;
            }else{

                if (is_i64){

                    is_imm8 = false;
                    is_imm32 = false;
                    is_imm64 = true;

                    ++x64_code_size;
                }

                ++x64_code_size;
            }
            break;
        case TP_X64_ITEM_KIND_X64_32_REGISTER:

            if (is_i64){

                is_imm8 = false;
                is_imm32 = false;
                is_imm64 = true;
            }

            x64_code_size = 2;
            break;
        case TP_X64_ITEM_KIND_MEMORY:
            x64_code_size = 3;

            if (is_disp8){

                ++x64_code_size;
            }else{

                x64_code_size += 4;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }

        if (is_imm8){

            ++x64_code_size;
        }else if (is_imm32){

            x64_code_size += 4;
        }else{

            x64_code_size += 8;
        }
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_add_sub_imm(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64 x64_op, TP_X64_64_REGISTER reg64,
    int32_t imm, TP_X64_ADD_SUB_IMM_MODE x64_add_sub_imm_mode)
{
    uint32_t x64_code_size = 0;

    bool is_add = false;

    switch (x64_op){
    case TP_X64_ADD:
        is_add = true;
        break;
    case TP_X64_SUB:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    bool is_imm8 = false;

    switch (x64_add_sub_imm_mode){
    case TP_X64_ADD_SUB_IMM_MODE_DEFAULT:
        is_imm8 = ((INT8_MIN <= imm) && (INT8_MAX >= imm));
        break;
    case TP_X64_ADD_SUB_IMM_MODE_FORCE_IMM32:
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return 0;
    }

    bool is_dst_x64_32_register = (TP_X64_64_REGISTER_R8 <= reg64);

    TP_X64_64_REGISTER reg = (is_dst_x64_32_register ? (reg64 - TP_X86_32_REGISTER_NUM) : reg64);

    if (x64_code_buffer){

        x64_code_buffer[x64_code_offset] = (0x48 | (is_dst_x64_32_register ? 0x01 : 0x00));

        // SUB – Integer Subtraction
        if (is_imm8){

            // ADD: immediate8 to qwordregister   0100 000B : 1000 0011 : 11 000 qwordreg : imm8
            // SUB: immediate8 from qwordregister 0100 100B : 1000 0011 : 11 101 qwordreg : imm8
            x64_code_buffer[x64_code_offset + 1] = TP_X64_OPCODE_ADD_SUB_IMM8_1; // 0x83
            x64_code_buffer[x64_code_offset + 2] =
                ((is_add ? TP_X64_OPCODE_ADD_IMM8_IMM32_2/* 0xc0 */ :
                    TP_X64_OPCODE_SUB_IMM8_IMM32_2/* 0xe8 */) | (reg & 0x07));
            x64_code_buffer[x64_code_offset + 3] = (uint8_t)imm;

            x64_code_size += 4;
        }else{

            // ADD: immediate32 to qwordregister   0100 100B : 1000 0001 : 11 000 qwordreg : imm32
            // SUB: immediate32 from qwordregister 0100 100B : 1000 0001 : 11 101 qwordreg : imm32
            x64_code_buffer[x64_code_offset + 1] = TP_X64_OPCODE_ADD_SUB_IMM32_1; // 0x81
            x64_code_buffer[x64_code_offset + 2] =
                ((is_add ? TP_X64_OPCODE_ADD_IMM8_IMM32_2/* 0xc0 */ :
                    TP_X64_OPCODE_SUB_IMM8_IMM32_2/* 0xe8 */) | (reg & 0x07));
            memcpy(&(x64_code_buffer[x64_code_offset + 3]), &imm, sizeof(imm));

            x64_code_size += 7;
        }
    }else{

        if (is_imm8){

            // imm8
            x64_code_size += 4;
        }else{

            // imm32
            x64_code_size += 7;
        }
    }

    return x64_code_size;
}

bool tp_convert_to_x64_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    TP_X64_64_REGISTER* reg64_dst_reg, TP_WASM_STACK_ELEMENT* op1)
{
    switch (op1->member_x64_item_kind){
    case TP_X64_ITEM_KIND_X86_32_REGISTER:
        if (TP_X86_32_REGISTER_NULL == op1->member_x64_item.member_x86_32_register){

           TP_PUT_LOG_MSG_ICE(symbol_table);
           return false;
        }
        *reg64_dst_reg = op1->member_x64_item.member_x86_32_register;
        break;
    case TP_X64_ITEM_KIND_X64_32_REGISTER:
        if (TP_X64_32_REGISTER_NULL == op1->member_x64_item.member_x64_32_register){

           TP_PUT_LOG_MSG_ICE(symbol_table);
           return false;
        }
        *reg64_dst_reg =
            op1->member_x64_item.member_x64_32_register + TP_X86_32_REGISTER_NUM;
        break;
    case TP_X64_ITEM_KIND_MEMORY:
//      break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

uint32_t tp_encode_x64_lea_rel_mem(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64_64_REGISTER reg64_dst_reg, TP_WASM_STACK_ELEMENT* src)
{
    uint32_t x64_code_size = 0;

    if ((TP_X64_ITEM_KIND_MEMORY != src->member_x64_item_kind) ||
        (TP_X64_ITEM_MEMORY_KIND_GLOBAL != src->member_x64_memory_kind)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return 0;
    }

    bool is_dst_x64_32_register = (TP_X64_64_REGISTER_R8 <= reg64_dst_reg);

    TP_X64_64_REGISTER reg = (is_dst_x64_32_register ? (reg64_dst_reg - TP_X86_32_REGISTER_NUM) : reg64_dst_reg);

    bool is_src_x64_32_register = (TP_X64_ITEM_KIND_X64_32_REGISTER == src->member_x64_item_kind);

    TP_X64_64_REGISTER sib_reg = (is_src_x64_32_register ?
        src->member_x64_item.member_x64_32_register : src->member_x64_item.member_x86_32_register);

    int32_t offset = src->member_offset;

    bool is_disp8 = ((INT8_MIN <= offset) && (INT8_MAX >= offset));

    if (x64_code_buffer){

        // LEA - Load Effective Address : 8D /r LEA r64,m 
        // in qwordregister 0100 1RXB : 1000 1101 : modA qwordreg r/m
        x64_code_buffer[x64_code_offset] = (0x48 |
            /* R */ (is_dst_x64_32_register ? 0x04 : 0x00) |
            /* B */ (is_src_x64_32_register ? 0x01 : 0x00)
        );
        x64_code_buffer[x64_code_offset + 1] = TP_X64_OPCODE_LEA; // 0x8d

        x64_code_size = 2;

        if (false == src->member_is_stack_base){

            x64_code_size += 5;

            // ModR/M
            x64_code_buffer[x64_code_offset + 2] = (((reg & 0x07) << 3) | 0x05);

            // Address displacement
            TP_WASM_RELOCATION* wasm_relocation = src->member_wasm_relocation;

            bool is_text = (wasm_relocation && (wasm_relocation->member_symbol_table_index) &&
                (TP_WASM_RELOCATION_TYPE_REL64 == wasm_relocation->member_type));

            bool is_rdata = (wasm_relocation && (wasm_relocation->member_symbol_table_index) &&
                (TP_WASM_RELOCATION_TYPE_STRING_LITERAL64 == wasm_relocation->member_type));

            if (is_text || is_rdata){

                uint32_t virtual_address = x64_code_offset + 3;
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

            int32_t offset = 0;

            memcpy(
                &(x64_code_buffer[x64_code_offset + 3]), &offset,
                sizeof(offset)
            );
        }else{

            x64_code_size += 2;

            // ModR/M
            if (0 == offset){

                // disp0
                x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x04);
            }else if (is_disp8){

                // disp8
                x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x44);
            }else{

                // disp32
                x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x84);
            }

            // SIB
            x64_code_buffer[x64_code_offset + 3] = (((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) | (sib_reg & 0x07));

            if (0 == offset){

                // disp0
                ;
            }else if (is_disp8){

                // disp8
                x64_code_buffer[x64_code_offset + 4] = (uint8_t)offset;

                ++x64_code_size;
            }else{

                // disp32
                memcpy(&(x64_code_buffer[x64_code_offset + 4]), &offset, sizeof(offset));

                x64_code_size += 4;
            }
        }
    }else{

        x64_code_size = 2;

        if (false == src->member_is_stack_base){

            x64_code_size += 5;
        }else{

            x64_code_size += 2;

            if (0 == offset){

                ;
            }else if (is_disp8){

                ++x64_code_size;
            }else{

                x64_code_size += 4;
            }
        }
    }

    if ( ! tp_free_register(symbol_table, src)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return 0;
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_lea(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_X64_64_REGISTER reg64_dst_reg,
    TP_X64_64_REGISTER reg64_src_index, TP_X64_64_REGISTER reg64_src_base, int32_t offset)
{
    uint32_t x64_code_size = 4;

    bool is_disp8 = ((INT8_MIN <= offset) && (INT8_MAX >= offset));

    if (x64_code_buffer){

        // LEA - Load Effective Address : 8D /r LEA r64,m 
        // in qwordregister 0100 1RXB : 1000 1101 : modA qwordreg r/m
        x64_code_buffer[x64_code_offset] = (0x48 |
            /* R */ ((TP_X64_64_REGISTER_R8 <= reg64_dst_reg) ? 0x04 : 0x00) |
            /* X */ ((TP_X64_64_REGISTER_R8 <= reg64_src_index) ? 0x02 : 0x00) |
            /* B */ ((TP_X64_64_REGISTER_R8 <= reg64_src_base) ? 0x01 : 0x00)
        );
        x64_code_buffer[x64_code_offset + 1] = TP_X64_OPCODE_LEA; // 0x8d

        // ModR/M
        if (0 == offset){

            // disp0
            x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x04);
        }else if (is_disp8){

            // disp8
            x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x44);
        }else{

            // disp32
            x64_code_buffer[x64_code_offset + 2] = (((reg64_dst_reg & 0x07) << 3) | 0x84);
        }

        // SIB
        x64_code_buffer[x64_code_offset + 3] = (((reg64_src_index & 0x07) << 3) | (reg64_src_base & 0x07));

        if (0 == offset){

            // disp0
            ;
        }else if (is_disp8){

            // disp8
            x64_code_buffer[x64_code_offset + 4] = (uint8_t)offset;

            ++x64_code_size;
        }else{

            // disp32
            memcpy(&(x64_code_buffer[x64_code_offset + 4]), &offset, sizeof(offset));

            x64_code_size += 4;
        }
    }else{

        if (0 == offset){

            // disp0
            ;
        }else if (is_disp8){

            // disp8
            ++x64_code_size;
        }else{

            // disp32
            x64_code_size += 4;
        }
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_push_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_X64_64_REGISTER reg64)
{
    uint32_t x64_code_size = 0;

    if (x64_code_buffer){

        // PUSH – Push Operand onto the Stack : 50+rd push r64
        // qwordregister (alternate encoding) 0100 W00B : 0101 0 reg64
        if (TP_X64_64_REGISTER_R8 <= reg64){

            x64_code_buffer[x64_code_offset] = (0x40 | 0x01);

            ++x64_code_size;
        }

        x64_code_buffer[x64_code_offset + x64_code_size] = (TP_X64_OPCODE_PUSH/* 0x50 */ | (reg64 & 0x07));

        ++x64_code_size;
    }else{

        if (TP_X64_64_REGISTER_R8 <= reg64){

            ++x64_code_size;
        }

        ++x64_code_size;
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_pop_reg64(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_X64_64_REGISTER reg64)
{
    uint32_t x64_code_size = 0;

    if (x64_code_buffer){

        // POP – Pop a Value from the Stack : REX.W + 58+rd pop r64
        // qwordregister (alternate encoding) 0100 W00B : 0101 1 reg64
        if (TP_X64_64_REGISTER_R8 <= reg64){

            x64_code_buffer[x64_code_offset] = (0x40 | 0x01);

            ++x64_code_size;
        }

        x64_code_buffer[x64_code_offset + x64_code_size] = (TP_X64_OPCODE_POP/* 0x58 */ | (reg64 & 0x07));

        ++x64_code_size;
    }else{

        if (TP_X64_64_REGISTER_R8 <= reg64){

            ++x64_code_size;
        }

        ++x64_code_size;
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_jne(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, TP_WASM_STACK_ELEMENT* label)
{
    if (TP_WASM_OPCODE_LOOP_LABEL != label->member_wasm_opcode){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: TP_WASM_OPCODE_LOOP_LABEL(%1) != op1->member_wasm_opcode(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(TP_WASM_OPCODE_LOOP_LABEL),
            TP_LOG_PARAM_UINT64_VALUE(label->member_wasm_opcode)
        );

        return 0;
    }

    uint32_t x64_code_size = 0;

    int32_t jmp_offset = (int32_t)(label->member_x64_code_offset);
    int32_t ip_offset = (int32_t)x64_code_offset;
    int32_t offset = jmp_offset - (ip_offset + 2);

    bool is_disp8 = ((INT8_MIN <= offset) && (INT8_MAX >= offset));

    if (false == is_disp8){

        offset = jmp_offset - (ip_offset + 6);
    }

    if (x64_code_buffer){

        // JNE - Jump short if not equal (ZF=0).
        if (is_disp8){

            // JNE rel8 : tttn = 5
            // 8-bit displacement 0111 tttn : 8-bit displacement
            x64_code_buffer[x64_code_offset] = TP_X64_OPCODE_JNE_REL8; // 0x75
            // disp8
            x64_code_buffer[x64_code_offset + 1] = (uint8_t)offset;

            x64_code_size += 2;
        }else{

            // JNE rel32 : tttn = 5
            // displacements (excluding 16-bit relative offsets) 0000 1111 : 1000 tttn : displacement32
            x64_code_buffer[x64_code_offset] = TP_X64_OPCODE_JNE_REL32_1; // 0x0f
            x64_code_buffer[x64_code_offset + 1] = TP_X64_OPCODE_JNE_REL32_2; // 0x85
            // disp32
            memcpy(&(x64_code_buffer[x64_code_offset + 2]), &offset, sizeof(offset));

            x64_code_size += 6;
        }
    }else{

        if (is_disp8){

            x64_code_size += 2;
        }else{

            x64_code_size += 6;
        }
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_call(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* x64_code_buffer, uint32_t x64_code_offset,
    TP_WASM_RELOCATION* wasm_relocation, TP_WASM_STACK_ELEMENT* op1)
{
    uint32_t x64_code_size = 0;

    bool is_op1_local_memory = (TP_X64_ITEM_KIND_MEMORY == op1->member_x64_item_kind) &&
        (TP_X64_ITEM_MEMORY_KIND_LOCAL == op1->member_x64_memory_kind);
    bool is_op1_global_memory = (TP_X64_ITEM_KIND_MEMORY == op1->member_x64_item_kind) &&
        (TP_X64_ITEM_MEMORY_KIND_GLOBAL == op1->member_x64_memory_kind);

    bool is_op1_register = ((false == is_op1_local_memory) && (false == is_op1_global_memory));

    TP_X64 x64_op = TP_X64_NULL;

    if (wasm_relocation){

        switch (wasm_relocation->member_type){
        case TP_WASM_RELOCATION_TYPE_ARG:
            x64_op = TP_X64_CALL;
            break;
        case TP_WASM_RELOCATION_TYPE_ARG_RIP:
            x64_op = TP_X64_CALL_RIP;
            is_op1_register = false;
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return 0;
        }
    }else{

        if (is_op1_register){

            x64_op = TP_X64_CALL;
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return 0;
        }
    }

    bool is_op1_x86_32_register = (is_op1_global_memory ?
        (TP_X64_ITEM_KIND_X86_32_REGISTER == op1->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X86_32_REGISTER == op1->member_x64_item_kind)
    );
    bool is_op1_x64_32_register = (is_op1_global_memory ?
        (TP_X64_ITEM_KIND_X64_32_REGISTER == op1->member_x64_memory_item_kind) :
        (TP_X64_ITEM_KIND_X64_32_REGISTER == op1->member_x64_item_kind)
    );

    TP_X86_32_REGISTER x86_32_register = op1->member_x64_item.member_x86_32_register;
    TP_X64_32_REGISTER x64_32_register = op1->member_x64_item.member_x64_32_register;
 
    int32_t offset = op1->member_offset;

    bool is_disp8 = (
        (INT8_MIN <= op1->member_offset) &&
        (INT8_MAX >= op1->member_offset)
    );

    if (x64_code_buffer){

        // CALL – Call Procedure (in same segment)
        if (is_op1_register){

            switch (x64_op){
            case TP_X64_CALL:
                break;
            case TP_X64_CALL_RIP:
//              break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }

            // register indirect 0100 000B 1111 1111 : 11 010 reg
            if (is_op1_x64_32_register){

                x64_code_buffer[x64_code_offset] = 0x41;

                ++x64_code_size;
            }

            x64_code_buffer[x64_code_offset + x64_code_size] = TP_X64_OPCODE_CALL_INDIRECT; // 0xff

            ++x64_code_size;

            // ModR/M
            x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)(
                (0x03 << 6) | (0x02 << 3) |
                (is_op1_x86_32_register ? x86_32_register : x64_32_register)
            );

            ++x64_code_size;
        }else{

            switch (x64_op){
            case TP_X64_CALL_RIP:
                break;
            case TP_X64_CALL:
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return 0;
            }

            // memory indirect 0100 000B 1111 1111 : mod 010 r/m
            // rel32 1110 1000
            if (is_op1_x64_32_register){

                x64_code_buffer[x64_code_offset] = 0x41;

                ++x64_code_size;
            }

            x64_code_buffer[x64_code_offset + x64_code_size] =
                (TP_X64_CALL_RIP != x64_op) ?
                    TP_X64_OPCODE_CALL_INDIRECT : // 0xff
                    TP_X64_OPCODE_CALL_RIP;       // 0xe8

            ++x64_code_size;

            // ModR/M
            if (TP_X64_CALL_RIP != x64_op){

                x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)(
                    ((is_disp8 ? 0x01 : 0x02) << 6) | (0x02 << 3) | 0x04
                );

                ++x64_code_size;
            }

            // SIB
            if (TP_X64_CALL_RIP != x64_op){

                if (is_op1_global_memory){

                    x64_code_buffer[x64_code_offset + x64_code_size] = (
                        ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) |
                        ((is_op1_x86_32_register ? x86_32_register : x64_32_register) & 0x07)
                    );
                }else{

                    x64_code_buffer[x64_code_offset + x64_code_size] = (
                        ((TP_X64_64_REGISTER_INDEX_NONE & 0x07) << 3) | (TP_X64_64_REGISTER_RBP & 0x07)
                    );
                }

                ++x64_code_size;
            }

            // Address displacement
            if (is_disp8 && (TP_X64_CALL_RIP != x64_op)){

                x64_code_buffer[x64_code_offset + x64_code_size] = (uint8_t)offset;

                ++x64_code_size;
            }else{

                if (TP_X64_CALL_RIP == x64_op){

                    uint32_t virtual_address = x64_code_offset + x64_code_size;
                    uint32_t symbol_table_index = wasm_relocation->member_symbol_table_index;

                    if ( ! tp_append_coff_relocation(
                        symbol_table, TP_SECTION_KIND_TEXT, virtual_address, symbol_table_index)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

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
        }
    }else{

        if (is_op1_register){

            if (is_op1_x64_32_register){

                ++x64_code_size;
            }

            x64_code_size += 2;
        }else{

            if (is_op1_x64_32_register){

                ++x64_code_size;
            }

            ++x64_code_size;

            if (TP_X64_CALL_RIP != x64_op){

                x64_code_size += 2;
            }

            if (is_disp8 && (TP_X64_CALL_RIP != x64_op)){

                ++x64_code_size;
            }else{

                x64_code_size += 4;
            }
        }
    }

    return x64_code_size;
}

uint32_t tp_encode_x64_1_opcode(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_offset, uint8_t opcode)
{
    uint32_t x64_code_size = 1;

    if (x64_code_buffer){

        x64_code_buffer[x64_code_offset] = opcode;
    }

    return x64_code_size;
}

