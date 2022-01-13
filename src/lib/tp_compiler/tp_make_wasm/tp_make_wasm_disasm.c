
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool disasm_wasm_body(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_WASM_MODULE_SECTION* code_section,
    uint32_t code_count, uint8_t* payload, uint32_t offset, uint32_t payload_len
);
static uint8_t* get_opcode_string(uint32_t wasm_opcode);

bool tp_disasm_wasm(TP_SYMBOL_TABLE* symbol_table, uint8_t* entry_point_symbol)
{
    TP_WASM_MODULE_SECTION* code_section = NULL;
    uint32_t return_type = 0;

    if ( ! tp_get_wasm_export_code_section(
        symbol_table, entry_point_symbol, &code_section, &return_type)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

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

        return false;
    }

    FILE* write_file = NULL;

    if ( ! tp_open_write_file(symbol_table, symbol_table->member_wasm_text_file_path, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! disasm_wasm_body(
        symbol_table, write_file,
        code_section, code_count, payload, offset, payload_len)){

        if ( ! tp_close_file(symbol_table, &write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

//          return false;
        }

        return false;
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

//      return false;
    }

    return true;
}

static bool disasm_wasm_body(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_WASM_MODULE_SECTION* code_section,
    uint32_t code_count, uint8_t* payload, uint32_t offset, uint32_t payload_len)
{
    for (uint32_t i = 0; code_count > i; ++i){

        uint32_t body_size = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, body_size);

        if (0 == body_size){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: 0 == body_size")
            );

            return false;
        }

        uint32_t wasm_code_body_size = body_size;

        uint32_t local_count = 0;
        TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, local_count);

        wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, local_count);

        for (uint32_t i = 0; local_count > i; ++i){

            uint32_t var_count = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, var_count);

            uint32_t var_type = 0;
            TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, code_section, payload, offset, var_type);

            wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, var_count);
            wasm_code_body_size -= tp_encode_ui32leb128(NULL, 0, var_type);
        }

        uint8_t* wasm_code_body_buffer = payload + offset;

        uint32_t wasm_code_body_pos = 0;

        do{

            uint32_t wasm_opcode = wasm_code_body_buffer[wasm_code_body_pos];

            ++wasm_code_body_pos;
            ++offset;

            if (wasm_code_body_size < wasm_code_body_pos){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: wasm_code_body_size < wasm_code_body_pos")
                );

                return false;
            }

            uint32_t param1 = 0;
            uint32_t param2 = 0;
            int32_t param32 = 0;
            int64_t param64 = 0;
            uint32_t param_size = 0;

            switch (wasm_opcode){
            // Control flow operators
            case TP_WASM_OPCODE_LOOP: // sig: block_type

                param32 = tp_decode_si32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s %d\n", get_opcode_string(wasm_opcode), param32);
                break;
            case TP_WASM_OPCODE_BR_IF: // relative_depth: varuint32
//              break;

            // Variable access
            case TP_WASM_OPCODE_GET_LOCAL: // local_index: varuint32
//              break;
            case TP_WASM_OPCODE_SET_LOCAL: // local_index: varuint32
//              break;
            case TP_WASM_OPCODE_TEE_LOCAL: // local_index: varuint32
//              break;
            case TP_WASM_OPCODE_GET_GLOBAL: // global_index: varuint32
//              break;
            case TP_WASM_OPCODE_SET_GLOBAL: // global_index: varuint32

                param1 = tp_decode_ui32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s 0x%x\n", get_opcode_string(wasm_opcode), param1);
                break;

            // Constants
            case TP_WASM_OPCODE_I32_CONST: // value: varint32

                param32 = tp_decode_si32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s %d\n", get_opcode_string(wasm_opcode), param32);
                break;
            case TP_WASM_OPCODE_I64_CONST: // value: varint64

                param64 = tp_decode_si64leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s %zd\n", get_opcode_string(wasm_opcode), param64);
                break;

            // Call operators
            case TP_WASM_OPCODE_CALL: // function_index: varuint32

                param1 = tp_decode_ui32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s 0x%x\n", get_opcode_string(wasm_opcode), param1);
                break;
            case TP_WASM_OPCODE_CALL_INDIRECT: // type_index: varuint32, reserved: varuint1
//              break;

            // Memory-related operators
            case TP_WASM_OPCODE_I32_LOAD: // flags: varuint32, offset: varuint32
//              break;
            case TP_WASM_OPCODE_I32_STORE: // flags: varuint32, offset: varuint32
//              break;
            case TP_WASM_OPCODE_I64_LOAD: // flags: varuint32, offset: varuint32
//              break;
            case TP_WASM_OPCODE_I64_STORE: // flags: varuint32, offset: varuint32

                param1 = tp_decode_ui32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                param2 = tp_decode_ui32leb128(&(wasm_code_body_buffer[wasm_code_body_pos]), &param_size);

                if ((wasm_code_body_pos + param_size) >= wasm_code_body_size){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                wasm_code_body_pos += param_size;
                offset += param_size;

                fprintf(write_file, "%s 0x%x 0x%x\n", get_opcode_string(wasm_opcode), param1, param2);
                break;

            // Numeric operators(i32)
            case TP_WASM_OPCODE_I32_ADD:
//              break;
            case TP_WASM_OPCODE_I32_SUB:
//              break;
            case TP_WASM_OPCODE_I32_MUL:
//              break;
            case TP_WASM_OPCODE_I32_DIV_S:
//              break;
            case TP_WASM_OPCODE_I32_XOR:
//              break;

            // Numeric operators(i64)
            case TP_WASM_OPCODE_I64_ADD:
//              break;
            case TP_WASM_OPCODE_I64_SUB:
//              break;
            case TP_WASM_OPCODE_I64_MUL:
//              break;
            case TP_WASM_OPCODE_I64_DIV_S:
//              break;
            case TP_WASM_OPCODE_I64_XOR:
//              break;

            // Parametric operators
            case TP_WASM_OPCODE_DROP:
//              break;

            // Comparison operators(i32)
            case TP_WASM_OPCODE_I32_NE:  // op1 != op2
//              break;

            // Comparison operators(i64)
            case TP_WASM_OPCODE_I64_NE:  // op1 != op2
//              break;

            // Control flow operators
            case TP_WASM_OPCODE_RETURN:
//              break;
            case TP_WASM_OPCODE_END:

                fprintf(write_file, "%s\n", get_opcode_string(wasm_opcode));

                if (TP_WASM_OPCODE_END == wasm_opcode){

                    if (wasm_code_body_size <= wasm_code_body_pos){

                        fprintf(write_file, "\n");

                        goto next;
                    }
                }
                break;
            default:

                fprintf(write_file, "Unknown opcode: 0x%x\n", wasm_opcode);

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }
        }while (true);
next:
        ;
    }

    return true;
}

static uint8_t* get_opcode_string(uint32_t wasm_opcode)
{
    switch (wasm_opcode){
    // Control flow operators
    case TP_WASM_OPCODE_LOOP: return "loop";
    case TP_WASM_OPCODE_BR_IF: return "br_if";
    // Variable access
    case TP_WASM_OPCODE_GET_LOCAL: return "get_local";
    case TP_WASM_OPCODE_SET_LOCAL: return "set_local";
    case TP_WASM_OPCODE_TEE_LOCAL: return "tee_local";
    case TP_WASM_OPCODE_GET_GLOBAL: return "get_global";
    case TP_WASM_OPCODE_SET_GLOBAL: return "set_global";
    // Constants
    case TP_WASM_OPCODE_I32_CONST: return "i32_const";
    case TP_WASM_OPCODE_I64_CONST: return "i64_const";
    // Call operators
    case TP_WASM_OPCODE_CALL: return "call";
    case TP_WASM_OPCODE_CALL_INDIRECT: return "call_indirect";
    // Memory-related operators
    case TP_WASM_OPCODE_I32_LOAD: return "i32_load";
    case TP_WASM_OPCODE_I32_STORE: return "i32_store";
    case TP_WASM_OPCODE_I64_LOAD: return "i64_load";
    case TP_WASM_OPCODE_I64_STORE: return "i64_store";
    // Numeric operators(i32)
    case TP_WASM_OPCODE_I32_ADD: return "i32_add";
    case TP_WASM_OPCODE_I32_SUB: return "i32_sub";
    case TP_WASM_OPCODE_I32_MUL: return "i32_mul";
    case TP_WASM_OPCODE_I32_DIV_S: return "i32_div_s";
    case TP_WASM_OPCODE_I32_XOR: return "i32_xor";
    // Numeric operators(i64)
    case TP_WASM_OPCODE_I64_ADD: return "i64_add";
    case TP_WASM_OPCODE_I64_SUB: return "i64_sub";
    case TP_WASM_OPCODE_I64_MUL: return "i64_mul";
    case TP_WASM_OPCODE_I64_DIV_S: return "i64_div_s";
    case TP_WASM_OPCODE_I64_XOR: return "i64_xor";
    // Parametric operators
    case TP_WASM_OPCODE_DROP: return "drop";
    // Comparison operators
    case TP_WASM_OPCODE_I32_NE: return "i32_ne";  // op1 != op2
    case TP_WASM_OPCODE_I64_NE: return "i64_ne";  // op1 != op2
    // Control flow operators
    case TP_WASM_OPCODE_RETURN: return "return";
    case TP_WASM_OPCODE_END: return "end";
    default:
        break;
    }

    return NULL;
}

