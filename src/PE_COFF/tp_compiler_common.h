
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_COMMON_H_)
#define TP_COMPILER_COMMON_H_

#include <windows.h>
#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <errno.h>
#include <intrin.h>

// ----------------------------------------------------------------------------------------
// config section:

typedef struct test_case_table_{
    uint8_t* member_source_code;
    int32_t member_return_value;
}TEST_CASE_TABLE;

TEST_CASE_TABLE tp_test_case_table[];

#define TP_CONFIG_OPTION_IS_32 '3'
#define TP_CONFIG_OPTION_IS_TEST_DISASM_X64 'a'
#define TP_CONFIG_OPTION_IS_OUTPUT_CURRENT_DIR 'c'
#define TP_CONFIG_OPTION_IS_OUTPUT_LOG_FILE 'l'
#define TP_CONFIG_OPTION_IS_NO_OUTPUT_MESSAGES 'm'
#define TP_CONFIG_OPTION_IS_NO_OUTPUT_FILES 'n'
#define TP_CONFIG_OPTION_IS_ORIGIN_WASM 'r'
#define TP_CONFIG_OPTION_IS_SOURCE_CMD_PARAM 's'
#define TP_CONFIG_OPTION_IS_TEST_MODE 't'
#define TP_CONFIG_OPTION_IS_OUTPUT_WASM_FILE 'w'
#define TP_CONFIG_OPTION_IS_OUTPUT_X64_FILE 'x'
#define TP_CONFIG_OPTION_IS_OUTPUT_WASM_TEXT_FILE 'y'
#define TP_CONFIG_OPTION_IS_OUTPUT_PE_COFF_FILE 'z'

#define TP_SOURCE_CODE_STRING_BUFFER_SIZE 256
#define TP_SOURCE_CODE_STRING_LENGTH_MAX (TP_SOURCE_CODE_STRING_BUFFER_SIZE - 1)

// ----------------------------------------------------------------------------------------
// message section:

#define TP_MESSAGE_BUFFER_SIZE 1024
#define TP_TEST_FNAME_NUM_MAX 999

typedef enum TP_LOG_TYPE_
{
    TP_LOG_TYPE_DEFAULT,         // PRINT & LOG
    TP_LOG_TYPE_HIDE_AFTER_DISP, // ERROR
    TP_LOG_TYPE_DISP_FORCE,      // ERROR
    TP_LOG_TYPE_DISP_WARNING,    // WARNING
    TP_LOG_TYPE_HIDE,            // LOG
}TP_LOG_TYPE;

typedef enum TP_ERROR_TYPE_
{
    TP_ERROR_TYPE_ABORT,
    TP_ERROR_TYPE_CONTINUE
}TP_ERROR_TYPE;

typedef enum TP_LOG_PARAM_TYPE_
{
    TP_LOG_PARAM_TYPE_STRING,
    TP_LOG_PARAM_TYPE_INT32_VALUE,
    TP_LOG_PARAM_TYPE_UINT64_VALUE
}TP_LOG_PARAM_TYPE;

typedef union tp_log_param_element_union{
    uint8_t* member_string;
    int32_t member_int32_value;
    size_t member_uint64_value;
}TP_LOG_PARAM_ELEMENT_UNION;

typedef struct tp_put_log_element_{
    TP_LOG_PARAM_TYPE member_type;
    TP_LOG_PARAM_ELEMENT_UNION member_body;
}TP_LOG_PARAM_ELEMENT;

#define TP_PUT_LOG_MSG(symbol_table, log_type, format_string, ...) \
    tp_put_log_msg( \
        (symbol_table), (log_type), TP_ERROR_TYPE_CONTINUE, (format_string), __FILE__, __func__, __LINE__, \
        (TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }, \
        sizeof((TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_LOG_PARAM_ELEMENT) \
    )
#define TP_PUT_LOG_ABORT(symbol_table, log_type, format_string, ...) \
    tp_put_log_msg( \
        (symbol_table), (log_type), TP_ERROR_TYPE_ABORT, (format_string), __FILE__, __func__, __LINE__, \
        (TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }, \
        sizeof((TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_LOG_PARAM_ELEMENT) \
    )
#define TP_PUT_LOG_WARNING(symbol_table, format_string, ...) \
    tp_put_log_msg( \
        (symbol_table), TP_LOG_TYPE_DISP_WARNING, TP_ERROR_TYPE_CONTINUE, (format_string), __FILE__, __func__, __LINE__, \
        (TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }, \
        sizeof((TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_LOG_PARAM_ELEMENT) \
    )
#define TP_PUT_LOG_PRINT(symbol_table, format_string, ...) \
    tp_put_log_msg( \
        (symbol_table), TP_LOG_TYPE_DEFAULT, TP_ERROR_TYPE_CONTINUE, (format_string), __FILE__, __func__, __LINE__, \
        (TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }, \
        sizeof((TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_LOG_PARAM_ELEMENT) \
    )
#define TP_PUT_LOG(symbol_table, format_string, ...) \
    tp_put_log_msg( \
        (symbol_table), TP_LOG_TYPE_HIDE, TP_ERROR_TYPE_CONTINUE, (format_string), __FILE__, __func__, __LINE__, \
        (TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }, \
        sizeof((TP_LOG_PARAM_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_LOG_PARAM_ELEMENT) \
    )
#define TP_MSG_FMT(format_string) (format_string)
#define TP_LOG_PARAM_STRING(string) (TP_LOG_PARAM_ELEMENT){ \
    .member_type = TP_LOG_PARAM_TYPE_STRING, \
    .member_body.member_string = (string) \
}
#define TP_LOG_PARAM_INT32_VALUE(value) (TP_LOG_PARAM_ELEMENT){ \
    .member_type = TP_LOG_PARAM_TYPE_INT32_VALUE, \
    .member_body.member_int32_value = (value) \
}
#define TP_LOG_PARAM_UINT64_VALUE(value) (TP_LOG_PARAM_ELEMENT){ \
    .member_type = TP_LOG_PARAM_TYPE_UINT64_VALUE, \
    .member_body.member_uint64_value = (value) \
}
#define TP_LOG_MSG_ICE "Internal compiler error."
#define TP_PUT_LOG_MSG_ICE(symbol_table) \
    TP_PUT_LOG_ABORT( \
        (symbol_table), TP_LOG_TYPE_HIDE_AFTER_DISP, \
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(TP_LOG_MSG_ICE) \
    );
#define TP_LOG_MSG_IRE "Internal disassembler error."
#define TP_PUT_LOG_MSG_IRE(symbol_table) \
    TP_PUT_LOG_ABORT( \
        (symbol_table), TP_LOG_TYPE_HIDE_AFTER_DISP, \
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(TP_LOG_MSG_IRE) \
    );
#define TP_LOG_MSG_ILE "Internal linker error."
#define TP_PUT_LOG_MSG_ILE(symbol_table) \
    TP_PUT_LOG_ABORT( \
        (symbol_table), TP_LOG_TYPE_HIDE_AFTER_DISP, \
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(TP_LOG_MSG_ILE) \
    );
#define TP_PUT_LOG_MSG_TRACE(symbol_table) \
    TP_PUT_LOG_MSG( \
        (symbol_table), TP_LOG_TYPE_HIDE, \
        TP_MSG_FMT("TRACE: %1 function"), TP_LOG_PARAM_STRING(__func__) \
    );
#define TP_GET_LAST_ERROR(symbol_table) \
    tp_get_last_error((symbol_table), TP_ERROR_TYPE_ABORT, __FILE__, __func__, __LINE__);
#define TP_PRINT_CRT_ERROR(symbol_table) \
    tp_print_crt_error((symbol_table), TP_ERROR_TYPE_ABORT, __FILE__, __func__, __LINE__);
#define TP_PRINT_CRT_ERROR_CONTINUE(symbol_table) \
    tp_print_crt_error((symbol_table), TP_ERROR_TYPE_CONTINUE, __FILE__, __func__, __LINE__)

#define TP_CALLOC(symbol_table, num, size) calloc((num), (size))
#define TP_REALLOC(symbol_table, ptr, size) realloc((ptr), (size))
#define TP_FREE(symbol_table, ptr, size) tp_free((symbol_table), (ptr), (size), __FILE__, __func__, __LINE__)
#define TP_FREE2(symbol_table, ptr, size) tp_free2((symbol_table), (ptr), (size), __FILE__, __func__, __LINE__)
// ----------------------------------------------------------------------------------------
// output file section:

#define TP_LOG_FILE_PREFIX "PE_COFF"

#define TP_WRITE_LOG_DEFAULT_FILE_NAME "log"
#define TP_WRITE_LOG_DEFAULT_EXT_NAME "log"

#define TP_TOKEN_DEFAULT_FILE_NAME "token"
#define TP_TOKEN_DEFAULT_EXT_NAME "log"

#define TP_PARSE_TREE_DEFAULT_FILE_NAME "parse_tree"
#define TP_PARSE_TREE_DEFAULT_EXT_NAME "log"

#define TP_OBJECT_HASH_DEFAULT_FILE_NAME "object_hash"
#define TP_OBJECT_HASH_DEFAULT_EXT_NAME "log"

#define TP_WASM_DEFAULT_FILE_NAME "bootx64"
#define TP_WASM_DEFAULT_EXT_NAME "wasm"

#define TP_WASM_TEXT_DEFAULT_FILE_NAME "bootx64_wasm"
#define TP_WASM_TEXT_DEFAULT_EXT_NAME "txt"

#define TP_X64_DEFAULT_FILE_NAME "bootx64_obj"
#define TP_X64_DEFAULT_EXT_NAME "bin"

#define TP_X64_TEXT_DEFAULT_FILE_NAME "bootx64_obj"
#define TP_X64_TEXT_DEFAULT_EXT_NAME "txt"

#define TP_COFF_CODE_TEXT_DEFAULT_FILE_NAME "efi_main_obj"
#define TP_COFF_CODE_TEXT_DEFAULT_EXT_NAME "txt"

#define TP_PE_CODE_TEXT_DEFAULT_FILE_NAME "bootx64_bin"
#define TP_PE_CODE_TEXT_DEFAULT_EXT_NAME "txt"

#define TP_INDENT_UNIT 4
#define TP_INDENT_FORMAT_BUFFER_SIZE 32
#define TP_INDENT_STRING_BUFFER_SIZE 4096

#define TP_MAKE_INDENT_STRING(indent_level) \
\
    uint8_t temp_prev_indent_string[TP_INDENT_FORMAT_BUFFER_SIZE]; \
    memset(temp_prev_indent_string, 0, sizeof(temp_prev_indent_string)); \
\
    sprintf_s( \
        temp_prev_indent_string, sizeof(temp_prev_indent_string), \
        "%% %dc", (indent_level * TP_INDENT_UNIT) - 1 \
    ); \
\
    uint8_t temp_indent_string[TP_INDENT_FORMAT_BUFFER_SIZE]; \
    memset(temp_indent_string, 0, sizeof(temp_indent_string)); \
    sprintf_s( \
        temp_indent_string, sizeof(temp_indent_string), \
        "%% %dc", (indent_level * TP_INDENT_UNIT)); \
\
    uint8_t prev_indent_string[TP_INDENT_STRING_BUFFER_SIZE]; \
    memset(prev_indent_string, 0, sizeof(prev_indent_string)); \
    sprintf_s(prev_indent_string, sizeof(prev_indent_string), temp_prev_indent_string, ' '); \
\
    uint8_t indent_string[TP_INDENT_STRING_BUFFER_SIZE]; \
    memset(indent_string, 0, sizeof(indent_string)); \
    sprintf_s(indent_string, sizeof(indent_string), temp_indent_string, ' '); \
\
    { \
        errno_t err = _set_errno(0); \
    }

#endif

