
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_TOKEN_H_)
#define TP_COMPILER_TOKEN_H_

#include "tp_compiler_common.h"

// ----------------------------------------------------------------------------------------
// input file section:

#define TP_MAX_LINE_BYTES 4095
#define TP_BUFFER_SIZE (TP_MAX_LINE_BYTES + 1)
#define TP_MAX_FILE_BYTES (TP_MAX_LINE_BYTES * 4096)

// ----------------------------------------------------------------------------------------
// token section:

#define TP_TOKEN_SIZE_ALLOCATE_UNIT 256

typedef enum TP_SYMBOL_
{
    TP_SYMBOL_NULL,
    TP_SYMBOL_ID,
    TP_SYMBOL_CONST_VALUE,
    TP_SYMBOL_PLUS,
    TP_SYMBOL_MINUS,
    TP_SYMBOL_MUL,
    TP_SYMBOL_DIV,
    TP_SYMBOL_LEFT_PAREN,
    TP_SYMBOL_RIGHT_PAREN,
    TP_SYMBOL_EQUAL,
    TP_SYMBOL_SEMICOLON
}TP_SYMBOL;

typedef enum TP_SYMBOL_TYPE_
{
    TP_SYMBOL_UNSPECIFIED_TYPE,
    TP_SYMBOL_ID_INT32,
    TP_SYMBOL_TYPE_INT32,
    TP_SYMBOL_CONST_VALUE_INT32
}TP_SYMBOL_TYPE;

#define TP_MAX_ID_BYTES 63
#define TP_ID_SIZE (TP_MAX_ID_BYTES + 1)
#define TP_MAX_ID_NUM 4095

typedef uint8_t TP_CHAR8_T;
typedef uint_least16_t TP_CHAR16_T;
typedef uint_least32_t TP_CHAR32_T;

typedef struct tp_token_{
    TP_SYMBOL member_symbol;
    TP_SYMBOL_TYPE member_symbol_type;
    rsize_t member_line;
    rsize_t member_column;
    uint8_t member_string[TP_ID_SIZE];
    int32_t member_i32_value;
}TP_TOKEN;

#endif

