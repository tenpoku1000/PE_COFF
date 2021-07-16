
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool convert_escape_sequence_to_char(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool convert_simple_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index
);
static bool convert_octal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index
);
static bool convert_hexadecimal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index
);
static bool convert_universal_character_name(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index
);

bool tp_convert_escape_sequence(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    for (rsize_t i = 0; input_file->member_tp_pp_token_pos > i; ++i){

        TP_TOKEN* token = &(input_file->member_tp_pp_token[i]);

        switch (token->member_symbol){
        case TP_SYMBOL_CHARACTER_CONSTANT:
//          break;
        case TP_SYMBOL_STRING_LITERAL:
            if ( ! convert_escape_sequence_to_char(symbol_table, token)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                if (symbol_table->member_is_error_abort){

                    return false;
                }
            }
            break;
        default:
            break;
        }
    }

    return true;
}

static bool convert_escape_sequence_to_char(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    TP_CHAR8_T* from = token->member_string;
    rsize_t length = token->member_string_length;

    TP_CHAR8_T* to = TP_CALLOC(symbol_table, length * 4 + 1 /* for UTF-8 encode. */, sizeof(TP_CHAR8_T));

    if (NULL == to){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    rsize_t to_index = 0;

    for (rsize_t from_index = 0; length > from_index; ++from_index){

        if ('\\' != from[from_index]){

            ++to_index;

            continue;
        }

        if ( ! convert_simple_escape_sequence(symbol_table, from, &from_index, to, &to_index)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! convert_octal_escape_sequence(symbol_table, from, &from_index, to, &to_index)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! convert_hexadecimal_escape_sequence(symbol_table, from, &from_index, to, &to_index)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! convert_universal_character_name(symbol_table, from, &from_index, to, &to_index)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    TP_FREE(symbol_table, &from, length);

    token->member_string = to;
    token->member_string_length = to_index;

    return true;
}

static bool convert_simple_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index)
{
    rsize_t local_from_index = *from_index;

    if ('\\' != from[local_from_index]){

        return true;
    }

    ++local_from_index;

    switch (from[local_from_index]){
    case '\'': *to = '\''; ++(*to_index); break;
    case '"': *to = '"'; ++(*to_index); break;
    case '?': *to = '?'; ++(*to_index); break;
    case '\\': *to = '\\'; ++(*to_index); break;
    case 'a': *to = '\a'; ++(*to_index); break;
    case 'b': *to = '\b'; ++(*to_index); break;
    case 'f': *to = '\f'; ++(*to_index); break;
    case 'n': *to = '\n'; ++(*to_index); break;
    case 'r': *to = '\r'; ++(*to_index); break;
    case 't': *to = '\t'; ++(*to_index); break;
    case 'v': *to = '\v'; ++(*to_index); break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    *from_index = local_from_index;

    return true;
}

static bool convert_octal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index)
{
    rsize_t local_from_index = *from_index;

    if ('\\' != from[local_from_index]){

        return true;
    }

    ++local_from_index;

    TP_CHAR8_T octal_digit[4] = { 0 };

    // \ octal-digit
    // \ octal-digit octal-digit
    // \ octal-digit octal-digit octal-digit
    for (int32_t i = 0; 3 > i; ++i){

        TP_CHAR8_T c = from[local_from_index];

        if ( ! IS_TP_OCTAL_DIGIT(c)){

            --local_from_index;

            break;
        }

        octal_digit[i] = c;

        ++local_from_index;
    }

    char* error_first_char = NULL;

    uint32_t value = (uint32_t)strtoul(octal_digit, &error_first_char, 8);

    if (octal_digit == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
            TP_LOG_PARAM_STRING(octal_digit)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    rsize_t total_bytes = 0;

    if ( ! tp_encode_utf_8(symbol_table, value, to, &total_bytes)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *from_index = local_from_index;

    *to_index += total_bytes;

    return true;
}

static bool convert_hexadecimal_escape_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index)
{
    rsize_t local_from_index = *from_index;

    if ('\\' != from[local_from_index]){

        return true;
    }

    ++local_from_index;

    rsize_t start_index = local_from_index;

    int32_t hex_num = 0;

    // \x hexadecimal-digit+
    for (;;){

        if ( ! isxdigit(from[local_from_index])){

            --local_from_index;
            --hex_num;

            break;
        }

        ++local_from_index;
        ++hex_num;
    }

    if (hex_num > 8){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: Hexadecimal-escape-sequence overflow value = %1."),
            TP_LOG_PARAM_INT32_VALUE(hex_num)
        );

        return false;
    }

    TP_CHAR8_T hexadecimal[9] = { 0 };

    memcpy(hexadecimal, from + start_index, hex_num);

    char* error_first_char = NULL;

    uint32_t value = (uint32_t)strtoul(hexadecimal, &error_first_char, 16);

    if (hexadecimal == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
            TP_LOG_PARAM_STRING(hexadecimal)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    rsize_t total_bytes = 0;

    if ( ! tp_encode_utf_8(symbol_table, value, to, &total_bytes)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *from_index = local_from_index;

    *to_index += total_bytes;

    return true;
}

static bool convert_universal_character_name(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR8_T* from, rsize_t* from_index, TP_CHAR8_T* to, rsize_t* to_index)
{
    rsize_t local_from_index = *from_index;

    if ('\\' != from[local_from_index]){

        return true;
    }

    ++local_from_index;

    int32_t hex_num = 0;

    switch (from[local_from_index]){
    case 'u':
        hex_num = 4;
        break;
    case 'U':
        hex_num = 8;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_CHAR8_T universal_character_name[9] = { 0 };

    for (int32_t i = 0; hex_num > i; ++i){

        ++local_from_index;

        TP_CHAR8_T c = from[local_from_index];

        if ( ! isxdigit(c)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: isxdigit(%1) is false."),
                TP_LOG_PARAM_UINT64_VALUE(c)
            );

            return false;
        }

        universal_character_name[i] = c;
    }

    char* error_first_char = NULL;

    uint32_t value = (uint32_t)strtoul(universal_character_name, &error_first_char, 16);

    if (universal_character_name == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
            TP_LOG_PARAM_STRING(universal_character_name)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

        return false;
    }

    rsize_t total_bytes = 0;

    if ( ! tp_encode_utf_8(symbol_table, value, to, &total_bytes)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    *from_index = local_from_index;

    *to_index += total_bytes;

    return true;
}

