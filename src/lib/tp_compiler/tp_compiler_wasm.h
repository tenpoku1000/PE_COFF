
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_WASM_H_)
#define TP_COMPILER_WASM_H_

#include "tp_compiler_semantic_analysis.h"

// ----------------------------------------------------------------------------------------
// wasm section:
#define TP_WASM_RELOCATION_ALLOCATE_UNIT 256

#define TP_WASM_MODULE_MAGIC_NUMBER "\0asm"
#define TP_WASM_MODULE_VERSION 0x1

#define TP_WASM_MODULE_SECTION_TYPE_COUNT_EXPR 1
#define TP_WASM_MODULE_SECTION_TYPE_FORM_FUNC 0x60
#define TP_WASM_MODULE_SECTION_TYPE_PARAM_COUNT_EXPR 0
#define TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I32 0x7f
#define TP_WASM_MODULE_SECTION_TYPE_PARAM_TYPE_I64 0x7e
#define TP_WASM_MODULE_SECTION_TYPE_RETURN_COUNT 1
#define TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I32 0x7f
#define TP_WASM_MODULE_SECTION_TYPE_RETURN_TYPE_I64 0x7e
#define TP_WASM_RETURN_VOID 0x40 // NOTE: In Spec, RETURN_COUNT is 0.

#define TP_WASM_MODULE_SECTION_FUNCTION_COUNT_EXPR 1
#define TP_WASM_MODULE_SECTION_FUNCTION_TYPES_EXPR 0

#define TP_WASM_MODULE_SECTION_EXPORT_NAME_2_EXPR "calc"
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_2_EXPR 4
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_2_UEFI "efi_main"
#define TP_WASM_MODULE_SECTION_EXPORT_NAME_LENGTH_2_UEFI 8

#define TP_WASM_MODULE_SECTION_CODE_COUNT_EXPR 1
#define TP_WASM_MODULE_SECTION_CODE_LOCAL_COUNT_EXPR 1
#define TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I32 0x7f
#define TP_WASM_MODULE_SECTION_CODE_VAR_TYPE_I64 0x7e

#define TP_WASM_MEM_ALIGN_32 2
#define TP_WASM_MEM_ALIGN_64 3

typedef struct TP_WASM_LOCAL_TYPE_{
    uint32_t member_arg_index_1_origin;
    uint32_t member_type;
    uint32_t member_offset;
}TP_WASM_LOCAL_TYPE;

typedef struct TP_WASM_FUNC_LOCAL_TYPE_{
    TP_WASM_LOCAL_TYPE* member_local_types;
    uint32_t member_local_types_num;
}TP_WASM_FUNC_LOCAL_TYPE;

typedef enum TP_WASM_RELOCATION_TYPE_{
    TP_WASM_RELOCATION_TYPE_CODE,            // Not relocation code.
    TP_WASM_RELOCATION_TYPE_PROLOGUE,        // Prologue of function.
    TP_WASM_RELOCATION_TYPE_EPILOGUE,        // Epilogue of function.
    TP_WASM_RELOCATION_TYPE_SKIP,            // Skip code.
    TP_WASM_RELOCATION_TYPE_BASE,            // Transform bottom of linear memory to RBP of x64.
    TP_WASM_RELOCATION_TYPE_BASE_MOV_LESS,   // Transform bottom of linear memory to RBP of x64(mov less).
    TP_WASM_RELOCATION_TYPE_REL64,           // Base of load/store 64 bits offset to offset of COFF(data).
    TP_WASM_RELOCATION_TYPE_REL64_CODE,      // Base of load/store 64 bits offset to offset of COFF(code).
    TP_WASM_RELOCATION_TYPE_STRING_LITERAL64,// 64 bits offset of string literal to offset of COFF.
    TP_WASM_RELOCATION_TYPE_ARG,             // Index of argument of function.
    TP_WASM_RELOCATION_TYPE_ARG_RIP,         // Index of argument of function(RIP call).
}TP_WASM_RELOCATION_TYPE;

typedef enum TP_SECTION_KIND_{
    TP_SECTION_KIND_DATA,
    TP_SECTION_KIND_RDATA,
    TP_SECTION_KIND_TEXT,
    TP_SECTION_KIND_NONE
}TP_SECTION_KIND;

typedef struct TP_WASM_RELOCATION_{
    TP_WASM_RELOCATION_TYPE member_type;
    rsize_t member_begin_index;
    rsize_t member_size;
    uint32_t member_offset32;
    rsize_t member_offset64;
    int32_t member_arg_index;
    int32_t member_type_return;
    uint32_t member_function_call_depth;
    bool member_is_containing_function_call;

    TP_SECTION_KIND member_section_kind;
    uint32_t member_symbol_table_index;
    uint8_t* member_symbol;
    rsize_t member_symbol_length;
}TP_WASM_RELOCATION;

#define TP_WASM_ARG_INDEX_EAX -2
#define TP_WASM_ARG_INDEX_VOID -1
#define TP_WASM_ARG_INDEX_ECX 0
#define TP_WASM_ARG_INDEX_EDX 1
#define TP_WASM_ARG_INDEX_R8D 2
#define TP_WASM_ARG_INDEX_R9D 3
#define TP_WASM_ARG_INDEX(wasm_relocation) \
    (wasm_relocation) ? (wasm_relocation)->member_arg_index : TP_WASM_ARG_INDEX_VOID
#define TP_WASM_CODE_BEGIN size_t tmp_index_ = 0;

#define TP_WASM_CODE(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_CODE, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_PROLOGUE(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_PROLOGUE, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_EPILOGUE(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_EPILOGUE, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_SKIP(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_SKIP, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_BASE(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_BASE, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_BASE_MOV_LESS(symbol_table, index, code) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_BASE_MOV_LESS, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_REL64(symbol_table, index, code, offset, symbol, symbol_length, symbol_index) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_REL64, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = (offset), \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_DATA, \
        .member_symbol_table_index = (symbol_index), \
        .member_symbol = (symbol), \
        .member_symbol_length = (symbol_length) })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_REL64_CODE(symbol_table, index, code, offset, symbol, symbol_length, symbol_index) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_REL64_CODE, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = (offset), \
        .member_arg_index = TP_WASM_ARG_INDEX_VOID, \
        .member_type_return = TP_WASM_RETURN_VOID, \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_CODE, \
        .member_symbol_table_index = (symbol_index), \
        .member_symbol = (symbol), \
        .member_symbol_length = (symbol_length) })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_STRING_LITERAL64( \
    symbol_table, index, code, offset, symbol, symbol_length, symbol_index, \
    arg_index, type_return, is_containing_function_call) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_STRING_LITERAL64, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = (offset), \
        .member_arg_index = (arg_index), \
        .member_type_return = (type_return), \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = (is_containing_function_call), \
\
        .member_section_kind = TP_SECTION_KIND_RDATA, \
        .member_symbol_table_index = (symbol_index), \
        .member_symbol = (symbol), \
        .member_symbol_length = (symbol_length) })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_ARG( \
    symbol_table, index, code, arg_index, type_return, is_containing_function_call) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_ARG, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = (arg_index), \
        .member_type_return = (type_return), \
        .member_function_call_depth = 0, \
        .member_is_containing_function_call = (is_containing_function_call), \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_CALL( \
    symbol_table, index, code, arg_index, type_return, function_call_depth) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_ARG, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = (arg_index), \
        .member_type_return = (type_return), \
        .member_function_call_depth = (function_call_depth), \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_NONE, \
        .member_symbol_table_index = 0, \
        .member_symbol = NULL, \
        .member_symbol_length = 0 })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

#define TP_WASM_CODE_CALL_RIP( \
    symbol_table, index, code, arg_index, type_return, function_call_depth, \
    symbol, symbol_length, symbol_index) \
\
    tmp_index_ = (code); \
\
    if ( ! tp_wasm_add_relocation(symbol_table, &(TP_WASM_RELOCATION){ \
        .member_type = TP_WASM_RELOCATION_TYPE_ARG_RIP, \
        .member_begin_index = (index) - symbol_table->member_wasm_relocation_begin, \
        .member_size = tmp_index_, \
        .member_offset32 = 0, \
        .member_offset64 = 0, \
        .member_arg_index = (arg_index), \
        .member_type_return = (type_return), \
        .member_function_call_depth = (function_call_depth), \
        .member_is_containing_function_call = false, \
\
        .member_section_kind = TP_SECTION_KIND_TEXT, \
        .member_symbol_table_index = (symbol_index), \
        .member_symbol = (symbol), \
        .member_symbol_length = (symbol_length) })){ \
\
        TP_PUT_LOG_MSG_TRACE(symbol_table); \
\
        goto fail; \
    } \
\
    (index) += tmp_index_;

// Control flow operators
#define TP_WASM_OPCODE_LOOP 0x03 // sig: block_type
#define TP_WASM_OPCODE_END 0x0b
#define TP_WASM_OPCODE_BR_IF 0x0d // relative_depth: varuint32
#define TP_WASM_OPCODE_RETURN 0x0f

#define TP_WASM_BLOCK_TYPE_VOID 0x40

// Call operators
#define TP_WASM_OPCODE_CALL 0x10 // function_index: varuint32
#define TP_WASM_OPCODE_CALL_INDIRECT 0x11 // type_index: varuint32, reserved: varuint1

// Parametric operators
#define TP_WASM_OPCODE_DROP 0x1a

// Variable access
#define TP_WASM_OPCODE_GET_LOCAL 0x20 // local_index: varuint32
#define TP_WASM_OPCODE_SET_LOCAL 0x21 // local_index: varuint32
#define TP_WASM_OPCODE_TEE_LOCAL 0x22 // local_index: varuint32
#define TP_WASM_OPCODE_GET_GLOBAL 0x23 // global_index: varuint32
#define TP_WASM_OPCODE_SET_GLOBAL 0x24 // global_index: varuint32

// Memory-related operators
#define TP_WASM_OPCODE_I32_LOAD 0x28 // flags: varuint32, offset: varuint32
#define TP_WASM_OPCODE_I32_STORE 0x36 // flags: varuint32, offset: varuint32
#define TP_WASM_OPCODE_I64_LOAD 0x29 // flags: varuint32, offset: varuint32
#define TP_WASM_OPCODE_I64_STORE 0x37 // flags: varuint32, offset: varuint32

// Constants
#define TP_WASM_OPCODE_I32_CONST 0x41 // value: varint32
#define TP_WASM_OPCODE_I64_CONST 0x42 // value: varint64

// Comparison operators(i32)
#define TP_WASM_OPCODE_I32_NE 0x47 // op1 != op2

// Comparison operators(i64)
#define TP_WASM_OPCODE_I64_NE 0x52 // op1 != op2

// Numeric operators(i32)
#define TP_WASM_OPCODE_I32_ADD 0x6a
#define TP_WASM_OPCODE_I32_SUB 0x6b
#define TP_WASM_OPCODE_I32_MUL 0x6c
#define TP_WASM_OPCODE_I32_DIV_S 0x6d
#define TP_WASM_OPCODE_I32_XOR 0x73

// Numeric operators(i64)
#define TP_WASM_OPCODE_I64_ADD 0x7c
#define TP_WASM_OPCODE_I64_SUB 0x7d
#define TP_WASM_OPCODE_I64_MUL 0x7e
#define TP_WASM_OPCODE_I64_DIV_S 0x7f
#define TP_WASM_OPCODE_I64_XOR 0x85

// Proprietary specification : Value operators(pseudo opcode)
#define TP_WASM_OPCODE_LOOP_LABEL 0xff03
#define TP_WASM_OPCODE_I32_VALUE 0xff41
#define TP_WASM_OPCODE_I64_VALUE 0xff42

typedef struct tp_wasm_section_type_var_{
    uint32_t member_form;
    uint32_t member_param_count;
    uint32_t* member_param_types;
    uint32_t member_return_count;
    uint32_t member_return_type;
}TP_WASM_SECTION_TYPE_VAR;

typedef struct tp_wasm_section_export_var_{
    uint32_t member_name_length;
    uint8_t* member_name;  // NOTE: must not free memory.
    uint8_t member_kind;
    uint32_t member_item_index;
}TP_WASM_SECTION_EXPORT_VAR;

typedef enum tp_wasm_section_type_{
    TP_WASM_SECTION_TYPE_CUSTOM = 0,
    TP_WASM_SECTION_TYPE_TYPE,
    TP_WASM_SECTION_TYPE_IMPORT,
    TP_WASM_SECTION_TYPE_FUNCTION,
    TP_WASM_SECTION_TYPE_TABLE,
    TP_WASM_SECTION_TYPE_MEMORY,
    TP_WASM_SECTION_TYPE_GLOBAL,
    TP_WASM_SECTION_TYPE_EXPORT,
    TP_WASM_SECTION_TYPE_START,
    TP_WASM_SECTION_TYPE_ELEMENT,
    TP_WASM_SECTION_TYPE_CODE,
    TP_WASM_SECTION_TYPE_DATA
}TP_WASM_SECTION_TYPE;

typedef enum tp_wasm_section_kind_{
    TP_WASM_SECTION_KIND_FUNCTION = 0,
    TP_WASM_SECTION_KIND_TABLE,
    TP_WASM_SECTION_KIND_MEMORY,
    TP_WASM_SECTION_KIND_GLOBAL
}TP_WASM_SECTION_KIND;

typedef struct tp_wasm_module_section_{
    uint32_t member_section_size;
    uint32_t member_id;
    uint32_t member_payload_len;
//  NOTE: Not implemented.
//  name_len: 0 == member_id
//  name: 0 == member_id
    uint8_t* member_name_len_name_payload_data;
}TP_WASM_MODULE_SECTION;

typedef struct tp_wasm_module_content_{
    uint32_t member_magic_number;
    uint32_t member_version;
    uint8_t member_payload[];
}TP_WASM_MODULE_CONTENT;

typedef struct tp_wasm_module_{
    uint32_t member_section_num;
    TP_WASM_MODULE_SECTION** member_section;
    uint32_t member_content_size;
    TP_WASM_MODULE_CONTENT* member_module_content;
}TP_WASM_MODULE;

#define TP_MAKE_WASM_SECTION_BUFFER(symbol_table, section, section_buffer, id, payload_len) \
\
    do{ \
        uint32_t size = tp_encode_ui32leb128(NULL, 0, (id)); \
        size += tp_encode_ui32leb128(NULL, 0, payload_len); \
        size += payload_len; \
\
        (section) = \
            (TP_WASM_MODULE_SECTION*)TP_CALLOC(symbol_table, 1, sizeof(TP_WASM_MODULE_SECTION)); \
\
        if (NULL == section){ \
\
            TP_PRINT_CRT_ERROR(symbol_table); \
\
            return NULL; \
        } \
\
        (section)->member_section_size = size; \
        (section)->member_id = (id); \
        (section)->member_payload_len = payload_len; \
\
        (section_buffer) = (section)->member_name_len_name_payload_data = \
            (uint8_t*)TP_CALLOC(symbol_table, size, sizeof(uint8_t)); \
\
        if (NULL == (section_buffer)){ \
\
            TP_PRINT_CRT_ERROR(symbol_table); \
\
            TP_FREE(symbol_table, &section, sizeof(TP_WASM_MODULE_SECTION)); \
\
            return NULL; \
        } \
\
        break; \
\
    }while (false)

#define TP_DECODE_UI32LEB128_GET_VALUE(symbol_table, section, buffer, offset, value) \
\
    do{ \
        uint32_t size = 0; \
\
        value = tp_decode_ui32leb128(((buffer) + (offset)), &size); \
\
        (offset) += size; \
\
        if ((section)->member_section_size < (offset)){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: section->member_section_size(%1) < offset(%2) at %3 function."), \
                TP_LOG_PARAM_UINT64_VALUE((section)->member_section_size), \
                TP_LOG_PARAM_UINT64_VALUE(offset), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return false; \
        } \
    }while (false)

#define TP_DECODE_UI32LEB128_CHECK_VALUE(symbol_table, section, buffer, offset, kind) \
\
    do{ \
        uint32_t size = 0; \
\
        uint32_t value = tp_decode_ui32leb128(((buffer) + (offset)), &size); \
\
        if ((kind) != value){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: kind(%1) != value(%2) at %3 function."), \
                TP_LOG_PARAM_UINT64_VALUE(kind), \
                TP_LOG_PARAM_UINT64_VALUE(value), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return false; \
        } \
\
        (offset) += size; \
\
        if ((section)->member_section_size < (offset)){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: section->member_section_size(%1) < offset(%2) at %3 function."), \
                TP_LOG_PARAM_UINT64_VALUE((section)->member_section_size), \
                TP_LOG_PARAM_UINT64_VALUE(offset), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return false; \
        } \
    }while (false)

#define TP_WASM_CHECK_STRING(symbol_table, section, buffer, offset, name, name_length, is_match) \
\
    do{ \
        (is_match) = (0 == strncmp((name), (buffer) + (offset), (name_length))); \
\
        (offset) += (name_length); \
\
        if ((section)->member_section_size < (offset)){ \
\
            TP_PUT_LOG_MSG( \
                (symbol_table), TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: section->member_section_size(%1) < offset(%2) at %3 function."), \
                TP_LOG_PARAM_UINT64_VALUE((section)->member_section_size), \
                TP_LOG_PARAM_UINT64_VALUE(offset), \
                TP_LOG_PARAM_STRING(__func__) \
            ); \
\
            return false; \
        } \
    }while (false)

#endif

