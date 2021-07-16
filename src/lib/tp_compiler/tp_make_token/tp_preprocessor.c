
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_GET_PREFIX_OF_STRING_LITERAL(symbol_table, string_literal, string_literal_prefix, prefix_length) \
\
    switch ((string_literal_prefix)[0]){ \
    case 'L': \
        if ((symbol_table)->member_is_wchar_t_UTF_32){ \
\
            (string_literal_prefix)[0] = 'U'; \
        }else{ \
\
            (string_literal_prefix)[0] = 'u'; \
        } \
\
        (prefix_length) = 1; \
        break; \
    case 'u': \
        if ((2 <= (string_literal)->member_string_length) && ('8' == (string_literal)->member_string[1])){ \
\
            (string_literal_prefix)[0] = 0; \
            (prefix_length) = 0; \
        }else{ \
\
            (prefix_length) = 1; \
        } \
        break; \
    case '"': \
        (string_literal_prefix)[0] = 0; \
        (prefix_length) = 0; \
        break; \
    default: \
        (prefix_length) = 1; \
        break; \
    }

static TP_CHAR8_T* keyword[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do", "double",
    "else", "enum", "extern", "float", "for", "goto", "if", "inline", "int", "long",
    "register", "restrict", "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "_Alignas",
    "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn",
    "_Static_assert", "_Thread_local", NULL
};

static bool concatenate_string_literal(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
static bool concat_string_literal(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* prev_string_literal, TP_TOKEN* token);

static bool convert_pp_token_to_token(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
static bool convert_character_constant_code(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool convert_string_literali_code(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool append_token(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);

bool tp_preprocessor(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    if ( ! tp_do_preprocess(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_convert_escape_sequence(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! concatenate_string_literal(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! convert_pp_token_to_token(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool concatenate_string_literal(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    TP_TOKEN* prev_string_literal = NULL;

    for (rsize_t i = 0; input_file->member_tp_pp_token_pos > i; ++i){

        TP_TOKEN* token = &(input_file->member_tp_pp_token[i]);

        if (NULL == token){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (TP_SYMBOL_STRING_LITERAL == token->member_symbol){

            if (prev_string_literal){

                if ( ! concat_string_literal(symbol_table, prev_string_literal, token)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    if (symbol_table->member_is_error_abort){

                        return false;
                    }
                }

                prev_string_literal->member_symbol = TP_SYMBOL_WHITE_SPACE;
                memset(
                    prev_string_literal->member_string, ' ',
                    prev_string_literal->member_string_length
                );
            }

            prev_string_literal = token;
        }else{

            prev_string_literal = NULL;
        }
    }

    return true;
}

static bool concat_string_literal(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* prev_string_literal, TP_TOKEN* token)
{
    rsize_t prev_prefix_length = 0;
    TP_CHAR8_T prev_string_literal_prefix[2] = { 0 };
    prev_string_literal_prefix[0] = prev_string_literal->member_string[0];

    TP_GET_PREFIX_OF_STRING_LITERAL(
        symbol_table, prev_string_literal,
        prev_string_literal_prefix, prev_prefix_length
    );

    rsize_t prev_string_literal_length =
        prev_string_literal->member_string_length - prev_prefix_length - 1;

    rsize_t current_prefix_length = 0;
    TP_CHAR8_T current_prefix[2] = { 0 };
    current_prefix[0] = token->member_string[0];

    TP_GET_PREFIX_OF_STRING_LITERAL(
        symbol_table, token, current_prefix, current_prefix_length
    );

    rsize_t current_length =
        token->member_string_length - current_prefix_length - 1;

    rsize_t prefix_length = 0;
    TP_CHAR8_T prefix[2] = { 0 };

    if (0 == strncmp(prev_string_literal_prefix, current_prefix, 2)){

        prefix_length = current_prefix_length;
        prefix[0] = current_prefix[0];
        prefix[1] = current_prefix[1];
    }else{

        bool is_UTF_32 = false;

        if (('U' == prev_string_literal_prefix[0]) || ('U' == current_prefix[0])){

            is_UTF_32 = true;
        }

        prefix_length = 1;
        prefix[0] = is_UTF_32 ? 'U' : 'u';
        prefix[1] = 0;
    }

    rsize_t string_length = prefix_length + prev_string_literal_length + current_length + 1;

    TP_CHAR8_T* tmp_string = TP_CALLOC(symbol_table, string_length, sizeof(TP_CHAR8_T));

    if (NULL == tmp_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    if (prefix_length){

        memcpy(tmp_string, prefix, prefix_length);
    }

    memcpy(
        tmp_string + prefix_length,
        prev_string_literal->member_string + prev_prefix_length,
        prev_string_literal_length
    );

    memcpy(
        tmp_string + prefix_length + prev_string_literal_length,
        token->member_string + current_prefix_length + 1,
        current_length
    );

    TP_FREE(symbol_table, &(token->member_string), token->member_string_length);

    token->member_string = tmp_string;
    token->member_string_length = string_length;

    return true;
}

static bool convert_pp_token_to_token(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    for (rsize_t i = 0; input_file->member_tp_pp_token_pos > i; ++i){

        TP_TOKEN* token = &(input_file->member_tp_pp_token[i]);

        switch (token->member_symbol){
        case TP_SYMBOL_CHARACTER_CONSTANT:
            token->member_symbol = TP_SYMBOL_CONSTANT;
            if ( ! convert_character_constant_code(symbol_table, token)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                if (symbol_table->member_is_error_abort){

                    return false;
                }
            }
            break;
        case TP_SYMBOL_STRING_LITERAL:
            if ( ! convert_string_literali_code(symbol_table, token)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                if (symbol_table->member_is_error_abort){

                    return false;
                }
            }
            break;
        case TP_SYMBOL_PP_NUMBER:
            if ( ! tp_convert_pp_number(symbol_table, token)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                if (symbol_table->member_is_error_abort){

                    return false;
                }
            }
            break;
        case TP_SYMBOL_IDENTIFIER:
            for (rsize_t j = 0; NULL != keyword[j]; ++j){

                if (0 == strcmp(keyword[j], token->member_string)){

                    token->member_symbol = TP_SYMBOL_KEYWORD;
                    break;
                }
            }
            break;
        case TP_SYMBOL_PUNCTUATOR:
            break;
        case TP_SYMBOL_WHITE_SPACE:
//          break;
        case TP_SYMBOL_LF:
            goto next;
        case TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER:
            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Other-non-white-space-character(%1) convert failed."),
                TP_LOG_PARAM_STRING(token->member_string)
            );
//          return false;
            break;
        case TP_SYMBOL_NULL:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ( ! append_token(symbol_table, token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                return false;
            }
        }
next:
        ;
    }

    return true;
}

static bool convert_character_constant_code(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    TP_CHAR8_T* from = token->member_string;
    rsize_t length = token->member_string_length;

    if (3 > length){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (*from){
    case 'L':
        ++from; if ('\'' != *from){  return false; }
        ++from;
        if (symbol_table->member_is_wchar_t_UTF_32){

            token->member_value_type = TP_VALUE_TYPE_UTF_32;
        }else{

            token->member_value_type = TP_VALUE_TYPE_UTF_16;
        }
        length -= 3;
        break;
    case 'u':
        ++from; if ('\'' != *from){  return false; }
        ++from;
        length -= 3;
        token->member_value_type = TP_VALUE_TYPE_UTF_16;
        break;
    case 'U':
        ++from; if ('\'' != *from){  return false; }
        ++from;
        length -= 3;
        token->member_value_type = TP_VALUE_TYPE_UTF_32;
        break;
    case '\'':
        ++from;
        length -= 2;
        token->member_value_type = TP_VALUE_TYPE_INT;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (0 == length){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (token->member_value_type){
    case TP_VALUE_TYPE_UTF_32:
        if ( ! tp_decode_utf_8_char(symbol_table, &(token->member_value.member_utf_32_value), from, length)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    case TP_VALUE_TYPE_UTF_16:
        if ( ! tp_encode_ucs_2_char(symbol_table, &(token->member_value.member_utf_16_value), from, length)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    case TP_VALUE_TYPE_INT:
        if (length > sizeof(int)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Character constant overflow length = %1."),
                TP_LOG_PARAM_UINT64_VALUE(length)
            );

            return false;
        }else{

            memcpy(&(token->member_value.member_int_value), from, sizeof(int));
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool convert_string_literali_code(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    TP_CHAR8_T* from = token->member_string;
    rsize_t length = token->member_string_length;

    if (2 > length){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t prefix_length = 0;
    TP_CHAR8_T prefix[2] = { 0 };
    prefix[0] = token->member_string[0];

    TP_GET_PREFIX_OF_STRING_LITERAL(symbol_table, token, prefix, prefix_length);

    switch (prefix[0]){
    case 'L':
        ++from; if ('"' != *from){  return false; }
        ++from;
        if (symbol_table->member_is_wchar_t_UTF_32){

            token->member_value_type = TP_VALUE_TYPE_UTF_32_STRING;
        }else{

            token->member_value_type = TP_VALUE_TYPE_UTF_16_STRING;
        }
        length -= 3;
        break;
    case 'u':
        ++from; if ('"' != *from){  return false; }
        ++from;
        length -= 3;
        token->member_value_type = TP_VALUE_TYPE_UTF_16_STRING;
        break;
    case 'U':
        ++from; if ('"' != *from){  return false; }
        ++from;
        length -= 3;
        token->member_value_type = TP_VALUE_TYPE_UTF_32_STRING;
        break;
    case '\0':
        ++from;
        length -= 2;
        token->member_value_type = TP_VALUE_TYPE_UTF_8_STRING;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (length > token->member_string_length){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (token->member_value_type){
    case TP_VALUE_TYPE_UTF_32_STRING:
        if ( ! tp_decode_utf_8(symbol_table, &(token->member_value.member_utf_32_string), from, length)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    case TP_VALUE_TYPE_UTF_16_STRING:
        if ( ! tp_encode_utf_16(symbol_table, &(token->member_value.member_utf_16_string), from, length)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    case TP_VALUE_TYPE_UTF_8_STRING:{

        if (0 == length){

            ++length;
        }

        TP_CHAR8_T* tmp_string = TP_CALLOC(symbol_table, length, sizeof(TP_CHAR8_T));

        if (NULL == tmp_string){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memcpy(tmp_string, from, length - 1);

        token->member_value.member_utf_8_string.member_string = tmp_string;
        token->member_value.member_utf_8_string.member_string_length = length;
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool append_token(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    if (symbol_table->member_tp_token_pos == (symbol_table->member_tp_token_size / sizeof(TP_TOKEN))){

        rsize_t tp_token_size_allocate_unit = 
            symbol_table->member_tp_token_size_allocate_unit * sizeof(TP_TOKEN);

        rsize_t tp_token_size =  symbol_table->member_tp_token_size + tp_token_size_allocate_unit;

        TP_TOKEN* tp_token = (TP_TOKEN*)TP_REALLOC(
            symbol_table, symbol_table->member_tp_token, tp_token_size
        );

        if (NULL == tp_token){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tp_token) + symbol_table->member_tp_token_size, 0,
            tp_token_size_allocate_unit
        );

        symbol_table->member_tp_token = tp_token;
        symbol_table->member_tp_token_size = tp_token_size;
    }

    symbol_table->member_tp_token[symbol_table->member_tp_token_pos] = *token;
    token->member_string = NULL;
    token->member_string_length = 0;

    ++(symbol_table->member_tp_token_pos);

    return true;
}

