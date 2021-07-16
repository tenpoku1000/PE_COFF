
// (C) Shin'ichi Ichikawa. Released under the MIT license.

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

#define IS_TP_OCTAL_DIGIT(c) (('0' <= (c)) && ('7' >= (c)))

typedef enum TP_SYMBOL_
{
    TP_SYMBOL_NULL = 0,

    TP_SYMBOL_PIPE,  // |
    TP_SYMBOL_EMPTY, // ε
    TP_SYMBOL_EOF,

    TP_SYMBOL_HEADER_NAME,
    TP_SYMBOL_IDENTIFIER,
    TP_SYMBOL_PP_NUMBER,
    TP_SYMBOL_CHARACTER_CONSTANT,
    TP_SYMBOL_STRING_LITERAL,
    TP_SYMBOL_PUNCTUATOR,
    TP_SYMBOL_WHITE_SPACE,
    TP_SYMBOL_LF,
    TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER,
    TP_SYMBOL_PLACE_MARKER,

    TP_SYMBOL_KEYWORD,
    TP_SYMBOL_CONSTANT
}TP_SYMBOL;

typedef enum TP_SYMBOL_KIND_
{
    TP_SYMBOL_UNSPECIFIED_KIND,

    TP_SYMBOL_KIND_INTEGER_CONSTANT,
    TP_SYMBOL_KIND_FLOATING_CONSTANT,

    TP_SYMBOL_KIND_LEFT_SQUARE_BRACKET,
    TP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET,
    TP_SYMBOL_KIND_LEFT_PARENTHESIS,
    TP_SYMBOL_KIND_RIGHT_PARENTHESIS,
    TP_SYMBOL_KIND_LEFT_CURLY_BRACKET,
    TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET,

    TP_SYMBOL_KIND_TRIPLE_PERIOD,
    TP_SYMBOL_KIND_PERIOD,

    TP_SYMBOL_KIND_DOUBLE_AMPERSAND,
    TP_SYMBOL_KIND_AMPERSAND_EQUAL,
    TP_SYMBOL_KIND_AMPERSAND,

    TP_SYMBOL_KIND_ASTERISK_EQUAL,
    TP_SYMBOL_KIND_ASTERISK,

    TP_SYMBOL_KIND_DOUBLE_PLUS,
    TP_SYMBOL_KIND_PLUS_EQUAL,
    TP_SYMBOL_KIND_PLUS,

    TP_SYMBOL_KIND_DOUBLE_MINUS,
    TP_SYMBOL_KIND_MINUS_EQUAL,
    TP_SYMBOL_KIND_MINUS_GREATER_THAN,
    TP_SYMBOL_KIND_MINUS,

    TP_SYMBOL_KIND_TILDE,

    TP_SYMBOL_KIND_EXCLAMATION_MARK_EQUAL,
    TP_SYMBOL_KIND_EXCLAMATION_MARK,

    TP_SYMBOL_KIND_SLASH_EQUAL,
    TP_SYMBOL_KIND_SLASH,

    TP_SYMBOL_KIND_PERCENT_EQUAL,
    TP_SYMBOL_KIND_PERCENT,

    TP_SYMBOL_KIND_CARET_EQUAL,
    TP_SYMBOL_KIND_CARET,

    TP_SYMBOL_KIND_DOUBLE_VERTICAL_BAR,
    TP_SYMBOL_KIND_VERTICAL_BAR_EQUAL,
    TP_SYMBOL_KIND_VERTICAL_BAR,

    TP_SYMBOL_KIND_QUESTION_MARK,

    TP_SYMBOL_KIND_COLON,

    TP_SYMBOL_KIND_SEMICOLON,

    TP_SYMBOL_KIND_DOUBLE_EQUAL,
    TP_SYMBOL_KIND_EQUAL,

    TP_SYMBOL_KIND_DOUBLE_LESS_THAN_EQUAL,
    TP_SYMBOL_KIND_DOUBLE_LESS_THAN,
    TP_SYMBOL_KIND_LESS_THAN_EQUAL,
    TP_SYMBOL_KIND_LESS_THAN,

    TP_SYMBOL_KIND_DOUBLE_GREATER_THAN_EQUAL,
    TP_SYMBOL_KIND_DOUBLE_GREATER_THAN,
    TP_SYMBOL_KIND_GREATER_THAN_PERCENT,
    TP_SYMBOL_KIND_GREATER_THAN_EQUAL,
    TP_SYMBOL_KIND_GREATER_THAN,

    TP_SYMBOL_KIND_COMMA,

    TP_SYMBOL_KIND_DOUBLE_NUMBER,
    TP_SYMBOL_KIND_NUMBER
}TP_SYMBOL_KIND;

typedef enum TP_SYMBOL_TYPE_
{
    TP_SYMBOL_UNSPECIFIED_TYPE,
    TP_SYMBOL_ID_INT32,
    TP_SYMBOL_TYPE_INT32
}TP_SYMBOL_TYPE;

typedef enum TP_VALUE_TYPE_
{
    TP_VALUE_TYPE_NULL = 0,
    TP_VALUE_TYPE_UTF_8_STRING,
    TP_VALUE_TYPE_UTF_16_STRING,
    TP_VALUE_TYPE_UTF_32_STRING,
    TP_VALUE_TYPE_UTF_16,
    TP_VALUE_TYPE_UTF_32,
    TP_VALUE_TYPE_INT,
    TP_VALUE_TYPE_LONG_INT,
    TP_VALUE_TYPE_LONG_LONG_INT,
    TP_VALUE_TYPE_UNSIGNED_INT,
    TP_VALUE_TYPE_UNSIGNED_LONG_INT,
    TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT,
    TP_VALUE_TYPE_FLOAT,
    TP_VALUE_TYPE_DOUBLE,
    TP_VALUE_TYPE_LONG_DOUBLE
}TP_VALUE_TYPE;

typedef uint8_t TP_CHAR8_T;
typedef uint_least16_t TP_CHAR16_T;
typedef uint_least32_t TP_CHAR32_T;

typedef struct tp_token_utf_8_string{
    TP_CHAR8_T* member_string;
    rsize_t member_string_length;
}TP_TOKEN_UTF_8_STRING;

typedef struct tp_token_utf_16_string{
    TP_CHAR16_T* member_string;
    rsize_t member_string_length;
}TP_TOKEN_UTF_16_STRING;

typedef struct tp_token_utf_32_string{
    TP_CHAR32_T* member_string;
    rsize_t member_string_length;
}TP_TOKEN_UTF_32_STRING;

typedef struct tp_token_value{
    TP_TOKEN_UTF_8_STRING member_utf_8_string;
    TP_TOKEN_UTF_16_STRING member_utf_16_string;
    TP_TOKEN_UTF_32_STRING member_utf_32_string;
    TP_CHAR16_T member_utf_16_value;
    TP_CHAR32_T member_utf_32_value;
    int member_int_value;
    long int member_long_int_value;
    long long int member_long_long_int_value;
    unsigned int member_unsigned_int_value;
    unsigned long int member_unsigned_long_int_value;
    unsigned long long int member_unsigned_long_long_int_value;
    float member_float_value;
    double member_double_value;
    long double member_long_double_value;
}TP_TOKEN_VALUE_UNION;

typedef struct tp_token_{
    TP_SYMBOL member_symbol;
    TP_SYMBOL_KIND member_symbol_kind;
    TP_SYMBOL_TYPE member_symbol_type;
    TP_CHAR8_T* member_file; // NOTE: member_file must not free memory.
    rsize_t member_line;
    rsize_t member_column;
    TP_CHAR8_T* member_string;
    rsize_t member_string_length;
    TP_VALUE_TYPE member_value_type;
    TP_TOKEN_VALUE_UNION member_value;
}TP_TOKEN;

// input file section:

#define TP_MAX_LINE_BYTES 4095
#define TP_BUFFER_SIZE (TP_MAX_LINE_BYTES + 1)
#define TP_MAX_FILE_BYTES (TP_MAX_LINE_BYTES * 4096)

#define TP_FROM_MEMORY_FILE_NAME "from_memory"

#define TP_LOGICAL_LINES_SIZE_ALLOCATE_UNIT 256
#define TP_PHYSICAL_LINES_SIZE_ALLOCATE_UNIT 256

typedef struct tp_logical_lines_{
    // NOTE: member_physical_lines release memory by double pointer only.
    TP_CHAR8_T** member_physical_lines;
    rsize_t member_physical_lines_pos;
    rsize_t member_physical_lines_size;
}TP_LOGICAL_LINES;

typedef enum TP_ENDS_IN_THE_MIDDLE_
{
    TP_ENDS_IN_THE_MIDDLE_NONE,
    TP_ENDS_IN_THE_MIDDLE_CR,
    TP_ENDS_IN_THE_MIDDLE_OTHER
}TP_ENDS_IN_THE_MIDDLE;

typedef struct tp_lexer_stack_{
    rsize_t member_physical_line_position;
    rsize_t member_physical_column_position;
}TP_LEXER_STACK;

typedef struct tp_input_file_{
    uint8_t member_input_file_path[_MAX_PATH];
    FILE* member_read_file;
    bool member_is_start_of_file;
    bool member_is_end_of_file;
    uint8_t member_read_lines_buffer[TP_BUFFER_SIZE];
    rsize_t member_read_lines_length;

    // from memory:
    bool member_is_from_memory;
    uint8_t* member_string_memory;
    rsize_t member_string_memory_length;

    // physical line:
    TP_CHAR8_T** member_physical_lines;
    rsize_t member_physical_lines_pos;
    rsize_t member_physical_lines_size;
    rsize_t member_physical_lines_size_allocate_unit;
    TP_ENDS_IN_THE_MIDDLE member_is_ends_in_the_middle;

    // logical line:
    TP_LOGICAL_LINES* member_logical_lines;
    rsize_t member_logical_lines_pos;
    rsize_t member_logical_lines_size;
    rsize_t member_logical_lines_size_allocate_unit;

    rsize_t member_logical_lines_current_position;

    // lexer:
    rsize_t member_logical_line;
    rsize_t member_physical_line;
    rsize_t member_physical_column;

    rsize_t member_physical_lines_current_line_position;
    rsize_t member_physical_lines_current_column_position;

    TP_LEXER_STACK* member_lexer_stack;
    int32_t member_lexer_stack_pos;
    int32_t member_lexer_stack_size;
    int32_t member_lexer_stack_size_allocate_unit;

    TP_CHAR8_T* member_string;
    rsize_t member_string_pos;
    rsize_t member_string_size;
    rsize_t member_string_size_allocate_unit;

    // preprocessor token:
    TP_TOKEN* member_tp_pp_token;
    rsize_t member_tp_pp_token_pos;
    rsize_t member_tp_pp_token_size;
    rsize_t member_tp_pp_token_size_allocate_unit;
}TP_INPUT_FILE;

// lexer section:

#define TP_LEXER_STACK_EMPTY -1
#define TP_LEXER_STACK_SIZE_ALLOCATE_UNIT 256
#define TP_STRING_SIZE_ALLOCATE_UNIT 256

#define TP_INIT_CHAR_POSITION(symbol_table, input_file) \
\
    if ((input_file)->member_lexer_stack){ \
\
        TP_FREE((symbol_table), &((input_file)->member_lexer_stack), (input_file)->member_lexer_stack_pos); \
    } \
\
    (input_file)->member_lexer_stack_pos = TP_LEXER_STACK_EMPTY; \
    (input_file)->member_lexer_stack_size = 0; \
    (input_file)->member_physical_lines_current_line_position = 0; \
    (input_file)->member_physical_lines_current_column_position = 0;

#define TP_ENTER_CHAR_POSITION(symbol_table, input_file) \
{ \
    if (NULL == (input_file)){ \
\
        TP_PUT_LOG_MSG_ICE(symbol_table); \
\
        return false; \
    } \
\
    if ((input_file)->member_lexer_stack_pos == \
        (((input_file)->member_lexer_stack_size / sizeof(TP_LEXER_STACK)) - 1)){ \
\
        int32_t lexer_stack_size_allocate_unit = \
            (input_file)->member_lexer_stack_size_allocate_unit * sizeof(TP_LEXER_STACK); \
\
        int32_t lexer_stack_size = (input_file)->member_lexer_stack_size + lexer_stack_size_allocate_unit; \
\
        if ((input_file)->member_lexer_stack_size > lexer_stack_size){ \
\
            TP_PUT_LOG_MSG( \
                symbol_table, TP_LOG_TYPE_DISP_FORCE, \
                TP_MSG_FMT("ERROR: input_file->member_lexer_stack_size(%1) > lexer_stack_size(%2)"), \
                TP_LOG_PARAM_UINT64_VALUE((input_file)->member_lexer_stack_size), \
                TP_LOG_PARAM_UINT64_VALUE(lexer_stack_size) \
            ); \
\
            goto error_out_enter_char_position; \
        } \
\
        TP_LEXER_STACK* lexer_stack = (TP_LEXER_STACK*)TP_REALLOC( \
            symbol_table, \
            (input_file)->member_lexer_stack, lexer_stack_size \
        ); \
\
        if (NULL == lexer_stack){ \
\
            TP_PRINT_CRT_ERROR(symbol_table); \
\
            goto error_out_enter_char_position; \
        } \
\
        memset( \
            ((uint8_t*)lexer_stack) + (input_file)->member_lexer_stack_size, 0, \
            lexer_stack_size_allocate_unit \
        ); \
\
        (input_file)->member_lexer_stack = lexer_stack; \
        (input_file)->member_lexer_stack_size = lexer_stack_size; \
    } \
\
    ++((input_file)->member_lexer_stack_pos); \
\
    if ((input_file)->member_lexer_stack){ \
\
        (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos].member_physical_line_position = \
            (input_file)->member_physical_lines_current_line_position; \
        (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos].member_physical_column_position = \
            (input_file)->member_physical_lines_current_column_position; \
\
        goto out_enter_char_position; \
    } \
\
error_out_enter_char_position: \
\
    if ((input_file)->member_lexer_stack){ \
\
        TP_FREE(symbol_table, &((input_file)->member_lexer_stack), (input_file)->member_lexer_stack_size); \
    } \
\
    (input_file)->member_lexer_stack_pos = TP_LEXER_STACK_EMPTY; \
    (input_file)->member_lexer_stack_size = 0; \
\
    return false; \
\
out_enter_char_position: \
    ; \
}

#define TP_BACKUP_CHAR_POSITION(symbol_table, input_file) \
\
    if (TP_LEXER_STACK_EMPTY == (input_file)->member_lexer_stack_pos){ \
\
        TP_PUT_LOG_MSG_ICE(symbol_table); \
\
        return false; \
    } \
\
   (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos]\
.member_physical_line_position = (input_file)->member_physical_lines_current_line_position; \
\
    (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos]\
.member_physical_column_position = (input_file)->member_physical_lines_current_column_position;

#define TP_RESTORE_CHAR_POSITION(symbol_table, input_file) \
\
    if (TP_LEXER_STACK_EMPTY == (input_file)->member_lexer_stack_pos){ \
\
        TP_PUT_LOG_MSG_ICE(symbol_table); \
\
        return false; \
    } \
\
    (input_file)->member_physical_lines_current_line_position = \
        (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos].member_physical_line_position; \
\
    (input_file)->member_physical_lines_current_column_position = \
        (input_file)->member_lexer_stack[(input_file)->member_lexer_stack_pos].member_physical_column_position;

#define TP_LEAVE_CHAR_POSITION(symbol_table, input_file) \
\
    if (TP_LEXER_STACK_EMPTY == (input_file)->member_lexer_stack_pos){ \
\
        TP_PUT_LOG_MSG_ICE(symbol_table); \
\
        return false; \
    } \
\
    memset(&((input_file)->member_lexer_stack\
[(input_file)->member_lexer_stack_pos]), 0, sizeof(TP_LEXER_STACK)); \
\
    --((input_file)->member_lexer_stack_pos);

#define TP_CLEAR_CHAR_POSITION(symbol_table, input_file) \
    TP_RESTORE_CHAR_POSITION((symbol_table), (input_file)); \
    TP_LEAVE_CHAR_POSITION((symbol_table), (input_file));

#define TP_APPEND_CHAR_POSITION(input_file) \
    (input_file)->member_physical_column = (input_file)->member_physical_lines_current_column_position;

#define TP_INIT_STRING(symbol_table, input_file) \
\
    if ((input_file)->member_string){ \
\
        TP_FREE((symbol_table), &((input_file)->member_string), (input_file)->member_string_pos); \
    } \
\
    (input_file)->member_string_pos = 0; \
    (input_file)->member_string_size = 0;

#define TP_APPEND_STRING_CHAR(symbol_table, input_file, c) \
\
    if ((input_file)->member_string_pos == (input_file)->member_string_size){ \
\
        rsize_t string_size =  (input_file)->member_string_size + \
            (input_file)->member_string_size_allocate_unit; \
\
        TP_CHAR8_T* string = (TP_CHAR8_T*)TP_REALLOC( \
            symbol_table, \
            (input_file)->member_string, string_size \
        ); \
\
        if (NULL == string){ \
\
            TP_PRINT_CRT_ERROR(symbol_table); \
\
            return false; \
        } \
\
        memset( \
            string + (input_file)->member_string_size, 0, \
            (input_file)->member_string_size_allocate_unit \
        ); \
\
        (input_file)->member_string = string; \
        (input_file)->member_string_size = string_size; \
    } \
\
    (input_file)->member_string[(input_file)->member_string_pos] = c; \
    ++((input_file)->member_string_pos);

#define TP_STRING_MOVE_TO_PP_TOKEN(pp_token, input_file) \
\
    (pp_token)->member_string = (input_file)->member_string; \
    (pp_token)->member_string_length = (input_file)->member_string_pos; \
    (input_file)->member_string = NULL; \
    (input_file)->member_string_pos = 0; \
    (input_file)->member_string_size = 0;

#endif

