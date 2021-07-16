
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool simple_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match
);
static bool octal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match
);
static bool hexadecimal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match
);

bool tp_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match)
{
    bool is_local_match = false;

    if ( ! simple_escape_sequence(symbol_table, input_file, symbol, current_char, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        // simple_escape_sequence:
        *is_match = true;

        return true;
    }

    if ( ! octal_escape_sequence(symbol_table, input_file, symbol, current_char, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        // octal_escape_sequence:
        *is_match = true;

        return true;
    }

    if ( ! hexadecimal_escape_sequence(symbol_table, input_file, symbol, current_char, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        // hexadecimal_escape_sequence:
        *is_match = true;

        return true;
    }

    if ( ! tp_universal_character_name(
        symbol_table, input_file, symbol, false/* bool is_start_char */, current_char, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        // universal_character_name:
        *is_match = true;

        return true;
    }

    *is_match = false;

    return true;
}

static bool simple_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match)
{
    if ('\\' != current_char){

        return true;
    }

    TP_ENTER_CHAR_POSITION(symbol_table, input_file);
    TP_APPEND_STRING_CHAR(symbol_table, input_file, '\\');
    TP_CHAR8_T* simple_escape_sequence_pos = NULL;
    bool is_end_logical_line = false;
    bool is_local_match = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &simple_escape_sequence_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(%1)."),
            TP_LOG_PARAM_STRING(input_file->member_string)
        );

        return false;
    }

    current_char = *simple_escape_sequence_pos;

    switch (current_char){
    case '\'':
    case '"':
    case '?':
    case '\\':
    case 'a':
    case 'b':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);
        break;
    default:
        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);
        return true;
    }

    *is_match = is_local_match;

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    return true;
}

static bool octal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match)
{
    if ('\\' != current_char){

        return true;
    }

    TP_ENTER_CHAR_POSITION(symbol_table, input_file);
    TP_APPEND_STRING_CHAR(symbol_table, input_file, '\\');
    TP_CHAR8_T* octal_escape_sequence_pos = NULL;
    bool is_end_logical_line = false;
    bool is_local_match = false;

    // \ octal-digit
    // \ octal-digit octal-digit
    // \ octal-digit octal-digit octal-digit
    for (int32_t i = 0; 3 > i; ++i){

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &octal_escape_sequence_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(%1)."),
                TP_LOG_PARAM_STRING(input_file->member_string)
            );

            return false;
        }

        current_char = *octal_escape_sequence_pos;

        if ( ! IS_TP_OCTAL_DIGIT(current_char)){

            TP_RESTORE_CHAR_POSITION(symbol_table, input_file);
            break;
        }

        TP_BACKUP_CHAR_POSITION(symbol_table, input_file);
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);
        is_local_match = true;
    }

    *is_match = is_local_match;

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    return true;
}

static bool hexadecimal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match)
{
    if ('\\' != current_char){

        return true;
    }

    TP_ENTER_CHAR_POSITION(symbol_table, input_file);
    TP_APPEND_STRING_CHAR(symbol_table, input_file, '\\');
    TP_CHAR8_T* hexadecimal_escape_sequence_pos = NULL;
    bool is_end_logical_line = false;
    bool is_local_match = false;

    // \x hexadecimal-digit+
    for (;;){

        if ( ! tp_getchar_pos_of_physical_line(
            symbol_table, input_file, &hexadecimal_escape_sequence_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(%1)."),
                TP_LOG_PARAM_STRING(input_file->member_string)
            );

            return false;
        }

        current_char = *hexadecimal_escape_sequence_pos;

        if ( ! isxdigit(current_char)){

            TP_RESTORE_CHAR_POSITION(symbol_table, input_file);
            break;
        }

        TP_BACKUP_CHAR_POSITION(symbol_table, input_file);
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);
        is_local_match = true;
    }

    *is_match = is_local_match;

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    return true;
}

bool tp_universal_character_name(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    bool is_start_char, TP_CHAR8_T current_char, bool* is_match)
{
    if ('\\' != current_char){

        return true;
    }

    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    TP_APPEND_STRING_CHAR(symbol_table, input_file, '\\');

    TP_CHAR8_T* ucn_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &ucn_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    int32_t hex_num = 0;

    switch (*ucn_pos){
    case 'u':
        hex_num = 4;
        TP_APPEND_STRING_CHAR(symbol_table, input_file, 'u');
        break;
    case 'U':
        hex_num = 8;
        TP_APPEND_STRING_CHAR(symbol_table, input_file, 'U');
        break;
    default:
        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);
        return true;
    }

    for (int32_t i = 0; hex_num > i; ++i){

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &ucn_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(UNC: %1)."),
                TP_LOG_PARAM_STRING(input_file->member_string)
            );

            return false;
        }

        if ( ! isxdigit(*ucn_pos)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: isxdigit(%1) is false."),
                TP_LOG_PARAM_UINT64_VALUE(*ucn_pos)
            );

            return false;
        }

        TP_APPEND_STRING_CHAR(symbol_table, input_file, *ucn_pos);
    }

    char* error_first_char = NULL;

    TP_CHAR32_T code_point = (TP_CHAR32_T)strtoul(input_file->member_string, &error_first_char, 16);

    if (input_file->member_string == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
            TP_LOG_PARAM_STRING(input_file->member_string)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    if ((0xA0 > code_point) && ('$' != code_point) && ('@' != code_point) && ('`' != code_point)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT(
                "ERROR: (0xA0 > code_point) && "
                "('$' != code_point) && ('@' != code_point) && ('`' != code_point): code_point = %1."
            ),
            TP_LOG_PARAM_UINT64_VALUE(code_point)
        );

        return false;
    }

    if ((0xD800 <= code_point) && (0xDFFF >= code_point)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: (0xD800 <= code_point) && (0xDFFF >= code_point): code_point = %1."),
            TP_LOG_PARAM_UINT64_VALUE(code_point)
        );

        return false;
    }

    bool is_symbol_identifier = (TP_SYMBOL_IDENTIFIER == symbol);

    if (is_symbol_identifier & is_start_char){

        bool is_numeric = false;

        if ( ! tp_is_numeric_code_point(symbol_table, code_point, &is_numeric)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_numeric){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: First universal character name is numeric(code_point = %1)."),
                TP_LOG_PARAM_UINT64_VALUE(code_point)
            );

            return false;
        }
    }

    *is_match = true;

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    return true;
}

