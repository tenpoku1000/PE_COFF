
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_MAKE_ULEB128_CODE(buffer, offset, opcode, value) \
\
    do{ \
        uint32_t size = 0; \
\
        if (buffer){ \
\
            ((buffer) + (offset))[0] = (opcode); \
            size = tp_encode_ui32leb128((buffer), (offset) + 1, (value)) + sizeof(uint8_t); \
        }else{ \
\
            size = tp_encode_ui32leb128(NULL, 0, (value)) + sizeof(uint8_t); \
        } \
\
        return size; \
\
    }while (false)

#define TP_MAKE_ULEB128_CODE_2(buffer, offset, opcode, value1, value2) \
\
    do{ \
        uint32_t size = 0; \
\
        if (buffer){ \
\
            ((buffer) + (offset))[0] = (opcode); \
            size = tp_encode_ui32leb128((buffer), (offset) + 1, (value1)) + sizeof(uint8_t); \
            size += tp_encode_ui32leb128((buffer), (offset) + size, (value2)); \
        }else{ \
\
            size = tp_encode_ui32leb128(NULL, 0, (value1)) + sizeof(uint8_t); \
            size += tp_encode_ui32leb128(NULL, 0, (value2)); \
        } \
\
        return size; \
\
    }while (false)

#define TP_MAKE_SLEB128_CODE(buffer, offset, opcode, value) \
\
    do{ \
        uint32_t size = 0; \
\
        if (buffer){ \
\
            ((buffer) + (offset))[0] = (opcode); \
            size = tp_encode_si64leb128((buffer), (offset) + 1, (value)) + sizeof(uint8_t); \
        }else{ \
\
            size = tp_encode_si64leb128(NULL, 0, (value)) + sizeof(uint8_t); \
        } \
\
        return size; \
\
    }while (false)

#define TP_MAKE_OPCODE(buffer, offset, opcode) \
\
    do{ \
        uint32_t size = 1; \
\
        if (buffer){ \
\
            ((buffer) + (offset))[0] = (opcode); \
        } \
\
        return size; \
\
    }while (false)

// Control flow operators

uint32_t tp_make_loop_code(uint8_t* buffer, size_t offset, int32_t sig)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_LOOP, sig);
}

uint32_t tp_make_end_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_END);
}

uint32_t tp_make_br_if_code(uint8_t* buffer, size_t offset, uint32_t relative_depth)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_BR_IF, relative_depth);
}

uint32_t tp_make_ret_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_RETURN);
}

// Call operators

uint32_t tp_make_call_code(uint8_t* buffer, size_t offset, uint32_t function_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_CALL, function_index);
}

uint32_t tp_make_call_indirect_code(uint8_t* buffer, size_t offset, uint32_t type_index, uint32_t reserved)
{
    TP_MAKE_ULEB128_CODE_2(buffer, offset, TP_WASM_OPCODE_CALL_INDIRECT, type_index, reserved);
}

// Parametric operators

uint32_t tp_make_drop_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_DROP);
}

// Variable access

uint32_t tp_make_get_local_code(uint8_t* buffer, size_t offset, uint32_t local_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_GET_LOCAL, local_index);
}

uint32_t tp_make_set_local_code(uint8_t* buffer, size_t offset, uint32_t local_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_SET_LOCAL, local_index);
}

uint32_t tp_make_tee_local_code(uint8_t* buffer, size_t offset, uint32_t local_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_TEE_LOCAL, local_index);
}

uint32_t tp_make_get_global_code(uint8_t* buffer, size_t offset, uint32_t global_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_GET_GLOBAL, global_index);
}

uint32_t tp_make_set_global_code(uint8_t* buffer, size_t offset, uint32_t global_index)
{
    TP_MAKE_ULEB128_CODE(buffer, offset, TP_WASM_OPCODE_SET_GLOBAL, global_index);
}

// Memory-related operators

uint32_t tp_make_i32_load_code(uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset)
{
    TP_MAKE_ULEB128_CODE_2(buffer, buffer_offset, TP_WASM_OPCODE_I32_LOAD, flags, offset);
}

uint32_t tp_make_i32_store_code(uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset)
{
    TP_MAKE_ULEB128_CODE_2(buffer, buffer_offset, TP_WASM_OPCODE_I32_STORE, flags, offset);
}

uint32_t tp_make_i64_load_code(uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset)
{
    TP_MAKE_ULEB128_CODE_2(buffer, buffer_offset, TP_WASM_OPCODE_I64_LOAD, flags, offset);
}

uint32_t tp_make_i64_store_code(uint8_t* buffer, size_t buffer_offset, uint32_t flags, uint32_t offset)
{
    TP_MAKE_ULEB128_CODE_2(buffer, buffer_offset, TP_WASM_OPCODE_I64_STORE, flags, offset);
}

// Constants

uint32_t tp_make_i32_const_code(uint8_t* buffer, size_t offset, int32_t value)
{
    TP_MAKE_SLEB128_CODE(buffer, offset, TP_WASM_OPCODE_I32_CONST, value);
}

uint32_t tp_make_i64_const_code(uint8_t* buffer, size_t offset, int64_t value)
{
    TP_MAKE_SLEB128_CODE(buffer, offset, TP_WASM_OPCODE_I64_CONST, value);
}

// Comparison operators

uint32_t tp_make_i64_ne_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_NE); // op1 != op2
}

// Numeric operators(i32)

uint32_t tp_make_i32_add_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I32_ADD);
}

uint32_t tp_make_i32_sub_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I32_SUB);
}

uint32_t tp_make_i32_mul_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I32_MUL);
}

uint32_t tp_make_i32_div_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I32_DIV_S);
}

uint32_t tp_make_i32_xor_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I32_XOR);
}

// Numeric operators(i64)

uint32_t tp_make_i64_add_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_ADD);
}

uint32_t tp_make_i64_sub_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_SUB);
}

uint32_t tp_make_i64_mul_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_MUL);
}

uint32_t tp_make_i64_div_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_DIV_S);
}

uint32_t tp_make_i64_xor_code(uint8_t* buffer, size_t offset)
{
    TP_MAKE_OPCODE(buffer, offset, TP_WASM_OPCODE_I64_XOR);
}

