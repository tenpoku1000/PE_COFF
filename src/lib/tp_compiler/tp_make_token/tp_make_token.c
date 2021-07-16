
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef enum TP_APPEND_KIND_{
    TP_APPEND_LOGICAL_LINE_KIND,
    TP_APPEND_PHYSICAL_LINE_KIND
}TP_APPEND_KIND;

static bool dump_token(TP_SYMBOL_TABLE* symbol_table, char* path, TP_TOKEN* token);
static bool make_logical_lines(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
static bool append_logical_line(
    TP_SYMBOL_TABLE* symbol_table,
    TP_APPEND_KIND append_kind, TP_INPUT_FILE* input_file, TP_CHAR8_T* lines_buffer, rsize_t lines_length
);
static bool get_physical_lines(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);
static bool append_physical_line(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file,
    TP_CHAR8_T* lines_buffer, rsize_t lines_length, long utf_8_restart_pos
);
static bool append_physical_line_content(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_CHAR8_T* line_buffer, rsize_t line_length
);
static bool read_file(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file);

bool tp_make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length)
{
    TP_INPUT_FILE* input_file = &(symbol_table->member_input_file);
    input_file->member_is_from_memory = (string && string_length);

    if (input_file->member_is_from_memory){

        strcpy_s(input_file->member_input_file_path, sizeof(input_file->member_input_file_path), TP_FROM_MEMORY_FILE_NAME);
        input_file->member_string_memory = string;
        input_file->member_string_memory_length = string_length;
    }else{

        if ( ! tp_open_read_file(symbol_table, input_file->member_input_file_path, &(input_file->member_read_file))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        if (input_file->member_is_end_of_file){ 

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }
    }

    if ( ! get_physical_lines(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! input_file->member_is_from_memory){

        if ( ! tp_close_file(symbol_table, &(input_file->member_read_file))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if (NULL == input_file->member_physical_lines){

        return true;
    }

    if ( ! make_logical_lines(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! tp_lexer(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_token_file)){

        if ( ! dump_token(symbol_table, symbol_table->member_pp_token_file_path, input_file->member_tp_pp_token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if ( ! tp_preprocessor(symbol_table, input_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    symbol_table->member_tp_token_position = symbol_table->member_tp_token;

    if (symbol_table->member_nul_num){

        TP_PUT_LOG_MSG(
            symbol_table,
            TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("NOTE: %1 number of NUL characters."),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_nul_num)
        );
    }

    if ((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_token_file)){

        if ( ! dump_token(symbol_table, symbol_table->member_token_file_path, symbol_table->member_tp_token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;

fail:

    if ( ! input_file->member_is_from_memory){

        if ( ! tp_close_file(symbol_table, &(input_file->member_read_file))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }
    }

    return false;
}

static bool dump_token(TP_SYMBOL_TABLE* symbol_table, char* path, TP_TOKEN* token)
{
    FILE* write_file = NULL;

    if ( ! tp_open_write_file(symbol_table, path, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint8_t indent_level = 1;

    for (; TP_SYMBOL_NULL != token->member_symbol; ++token){

        if ( ! tp_dump_token_main(symbol_table, write_file, token, indent_level)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_dump_token_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_TOKEN* token, uint8_t indent_level)
{
    errno_t err = 0;

    if ((UINT8_MAX == indent_level) || (0 == indent_level)){

        fprintf(write_file, "NOTE: Bad indent level(%d).\n", indent_level);
        fprintf(write_file, "NOTE: Stop dump token.\n\n");

        err = _set_errno(0);

        return true;
    }

    if (TP_SYMBOL_WHITE_SPACE == token->member_symbol){

        return true;
    }

    TP_MAKE_INDENT_STRING(indent_level);

    fprintf(write_file, "%s{\n", prev_indent_string);

    switch (token->member_symbol){
    case TP_SYMBOL_NULL:
        fprintf(write_file, "%sTP_SYMBOL_NULL\n", indent_string);
        break;
    case TP_SYMBOL_IDENTIFIER:
        fprintf(write_file, "%sTP_SYMBOL_IDENTIFIER\n", indent_string);
        break;
    case TP_SYMBOL_PP_NUMBER:
        fprintf(write_file, "%sTP_SYMBOL_PP_NUMBER\n", indent_string);
        break;
    case TP_SYMBOL_CHARACTER_CONSTANT:
        fprintf(write_file, "%sTP_SYMBOL_CHARACTER_CONSTANT\n", indent_string);
        break;
    case TP_SYMBOL_STRING_LITERAL:
        fprintf(write_file, "%sTP_SYMBOL_STRING_LITERAL\n", indent_string);
        break;
    case TP_SYMBOL_PUNCTUATOR:
        fprintf(write_file, "%sTP_SYMBOL_PUNCTUATOR\n", indent_string);
        break;
    case TP_SYMBOL_LF:
        fprintf(write_file, "%sTP_SYMBOL_LF\n", indent_string);
        break;
    case TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER:
        fprintf(write_file, "%sTP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER\n", indent_string);
        break;
    case TP_SYMBOL_PLACE_MARKER:
        fprintf(write_file, "%sTP_SYMBOL_PLACE_MARKER\n", indent_string);
        break;
    case TP_SYMBOL_KEYWORD:
        fprintf(write_file, "%sTP_SYMBOL_KEYWORD\n", indent_string);
        break;
    case TP_SYMBOL_CONSTANT:
        fprintf(write_file, "%sTP_SYMBOL_CONSTANT\n", indent_string);
        break;
    default:
        fprintf(write_file, "%sTP_SYMBOL(UNKNOWN_SYMBOL: %d)\n",
            indent_string, token->member_symbol);
        break;
    }

    switch (token->member_symbol_kind){
    case TP_SYMBOL_UNSPECIFIED_KIND:
        fprintf(write_file, "%sTP_SYMBOL_UNSPECIFIED_KIND\n", indent_string);
        break;

    case TP_SYMBOL_KIND_INTEGER_CONSTANT:
        fprintf(write_file, "%sTP_SYMBOL_KIND_INTEGER_CONSTANT\n", indent_string);
        break;
    case TP_SYMBOL_KIND_FLOATING_CONSTANT:
        fprintf(write_file, "%sTP_SYMBOL_KIND_FLOATING_CONSTANT\n", indent_string);
        break;

    case TP_SYMBOL_KIND_LEFT_SQUARE_BRACKET:
        fprintf(write_file, "%sTP_SYMBOL_KIND_LEFT_SQUARE_BRACKET\n", indent_string);
        break;
    case TP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET:
        fprintf(write_file, "%sTP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET\n", indent_string);
        break;
    case TP_SYMBOL_KIND_LEFT_PARENTHESIS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_LEFT_PARENTHESIS\n", indent_string);
        break;
    case TP_SYMBOL_KIND_RIGHT_PARENTHESIS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_RIGHT_PARENTHESIS\n", indent_string);
        break;
    case TP_SYMBOL_KIND_LEFT_CURLY_BRACKET:
        fprintf(write_file, "%sTP_SYMBOL_KIND_LEFT_CURLY_BRACKET\n", indent_string);
        break;
    case TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET:
        fprintf(write_file, "%sTP_SYMBOL_KIND_RIGHT_CURLY_BRACKET\n", indent_string);
        break;

    case TP_SYMBOL_KIND_TRIPLE_PERIOD:
        fprintf(write_file, "%sTP_SYMBOL_KIND_TRIPLE_PERIOD\n", indent_string);
        break;
    case TP_SYMBOL_KIND_PERIOD:
        fprintf(write_file, "%sTP_SYMBOL_KIND_PERIOD\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_AMPERSAND:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_AMPERSAND\n", indent_string);
        break;
    case TP_SYMBOL_KIND_AMPERSAND_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_AMPERSAND_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_AMPERSAND:
        fprintf(write_file, "%sTP_SYMBOL_KIND_AMPERSAND\n", indent_string);
        break;

    case TP_SYMBOL_KIND_ASTERISK_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_ASTERISK_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_ASTERISK:
        fprintf(write_file, "%sTP_SYMBOL_KIND_ASTERISK\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_PLUS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_PLUS\n", indent_string);
        break;
    case TP_SYMBOL_KIND_PLUS_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_PLUS_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_PLUS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_PLUS\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_MINUS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_MINUS\n", indent_string);
        break;
    case TP_SYMBOL_KIND_MINUS_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_MINUS_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_MINUS_GREATER_THAN:
        fprintf(write_file, "%sTP_SYMBOL_KIND_MINUS_GREATER_THAN\n", indent_string);
        break;
    case TP_SYMBOL_KIND_MINUS:
        fprintf(write_file, "%sTP_SYMBOL_KIND_MINUS\n", indent_string);
        break;

    case TP_SYMBOL_KIND_TILDE:
        fprintf(write_file, "%sTP_SYMBOL_KIND_TILDE\n", indent_string);
        break;

    case TP_SYMBOL_KIND_EXCLAMATION_MARK_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_EXCLAMATION_MARK_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_EXCLAMATION_MARK:
        fprintf(write_file, "%sTP_SYMBOL_KIND_EXCLAMATION_MARK\n", indent_string);
        break;

    case TP_SYMBOL_KIND_SLASH_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_SLASH_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_SLASH:
        fprintf(write_file, "%sTP_SYMBOL_KIND_SLASH\n", indent_string);
        break;

    case TP_SYMBOL_KIND_PERCENT_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_PERCENT_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_PERCENT:
        fprintf(write_file, "%sTP_SYMBOL_KIND_PERCENT\n", indent_string);
        break;

    case TP_SYMBOL_KIND_CARET_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_CARET_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_CARET:
        fprintf(write_file, "%sTP_SYMBOL_KIND_CARET\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_VERTICAL_BAR:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_VERTICAL_BAR\n", indent_string);
        break;
    case TP_SYMBOL_KIND_VERTICAL_BAR_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_VERTICAL_BAR_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_VERTICAL_BAR:
        fprintf(write_file, "%sTP_SYMBOL_KIND_VERTICAL_BAR\n", indent_string);
        break;

    case TP_SYMBOL_KIND_QUESTION_MARK:
        fprintf(write_file, "%sTP_SYMBOL_KIND_QUESTION_MARK\n", indent_string);
        break;

    case TP_SYMBOL_KIND_COLON:
        fprintf(write_file, "%sTP_SYMBOL_KIND_COLON\n", indent_string);
        break;

    case TP_SYMBOL_KIND_SEMICOLON:
        fprintf(write_file, "%sTP_SYMBOL_KIND_SEMICOLON\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_EQUAL\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_LESS_THAN_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_LESS_THAN_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_DOUBLE_LESS_THAN:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_LESS_THAN\n", indent_string);
        break;
    case TP_SYMBOL_KIND_LESS_THAN_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_LESS_THAN_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_LESS_THAN:
        fprintf(write_file, "%sTP_SYMBOL_KIND_LESS_THAN\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_GREATER_THAN_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_GREATER_THAN_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_DOUBLE_GREATER_THAN:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_GREATER_THAN\n", indent_string);
        break;
    case TP_SYMBOL_KIND_GREATER_THAN_PERCENT:
        fprintf(write_file, "%sTP_SYMBOL_KIND_GREATER_THAN_PERCENT\n", indent_string);
        break;
    case TP_SYMBOL_KIND_GREATER_THAN_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_KIND_GREATER_THAN_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_KIND_GREATER_THAN:
        fprintf(write_file, "%sTP_SYMBOL_KIND_GREATER_THAN\n", indent_string);
        break;

    case TP_SYMBOL_KIND_COMMA:
        fprintf(write_file, "%sTP_SYMBOL_KIND_COMMA\n", indent_string);
        break;

    case TP_SYMBOL_KIND_DOUBLE_NUMBER:
        fprintf(write_file, "%sTP_SYMBOL_KIND_DOUBLE_NUMBER\n", indent_string);
        break;
    case TP_SYMBOL_KIND_NUMBER:
        fprintf(write_file, "%sTP_SYMBOL_KIND_NUMBER\n", indent_string);
        break;

    default:
        fprintf(write_file, "%sTP_SYMBOL_KIND(UNKNOWN_TP_SYMBOL_KIND: %d)\n",
            indent_string, token->member_symbol_kind);
        break;
    }

    switch (token->member_symbol_type){
    case TP_SYMBOL_UNSPECIFIED_TYPE:
        fprintf(write_file, "%sTP_SYMBOL_UNSPECIFIED_TYPE\n", indent_string);
        break;
    case TP_SYMBOL_ID_INT32:
        fprintf(write_file, "%sTP_SYMBOL_ID_INT32\n", indent_string);
        break;
    case TP_SYMBOL_TYPE_INT32:
        fprintf(write_file, "%sTP_SYMBOL_TYPE_INT32\n", indent_string);
        break;
    default:
        fprintf(write_file, "%sTP_SYMBOL_TYPE(UNKNOWN_SYMBOL_TYPE: %d)\n",
            indent_string, token->member_symbol_type);
        break;
    }

    fprintf(write_file, "%s member_line(%zd)\n", indent_string, token->member_line);
    fprintf(write_file, "%s member_column(%zd)\n", indent_string, token->member_column);

    if (token->member_string){

        fprintf(write_file, "%s member_string(%s)\n", indent_string, token->member_string);
    }

    switch (token->member_value_type){
    case TP_VALUE_TYPE_UTF_8_STRING:
        fprintf(
            write_file, "%s member_utf_8_string(%s)\n", indent_string,
            token->member_value.member_utf_8_string.member_string
        );
        break;
    case TP_VALUE_TYPE_UTF_16_STRING:{

        TP_TOKEN_UTF_8_STRING utf_8_string = { 0 };

        if ( ! tp_decode_utf_16(
            symbol_table, &(token->member_value.member_utf_16_string), &utf_8_string)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(
            write_file, "%s member_utf_16_string(%s)\n", indent_string,
            token->member_value.member_utf_8_string.member_string
        );

        TP_FREE(symbol_table, &(utf_8_string.member_string), utf_8_string.member_string_length);
        break;
    }
    case TP_VALUE_TYPE_UTF_32_STRING:{

        TP_TOKEN_UTF_8_STRING utf_8_string = { 0 };

        if ( ! tp_decode_utf_32(
            symbol_table, &(token->member_value.member_utf_32_string), &utf_8_string)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(
            write_file, "%s member_utf_32_string(%s)\n", indent_string,
            token->member_value.member_utf_8_string.member_string
        );

        TP_FREE(symbol_table, &(utf_8_string.member_string), utf_8_string.member_string_length);
        break;
    }
    case TP_VALUE_TYPE_UTF_16:{

        TP_TOKEN_UTF_8_STRING utf_8_string = { 0 };

        if ( ! tp_decode_ucs_2_char(
            symbol_table, token->member_value.member_utf_16_value, &utf_8_string)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(
            write_file, "%s member_utf_16_value(%s)\n", indent_string,
            token->member_value.member_utf_8_string.member_string
        );

        TP_FREE(symbol_table, &(utf_8_string.member_string), utf_8_string.member_string_length);
        break;
    }
    case TP_VALUE_TYPE_UTF_32:{

        TP_TOKEN_UTF_8_STRING utf_8_string = { 0 };

        if ( ! tp_decode_utf_32_char(
            symbol_table, token->member_value.member_utf_32_value, &utf_8_string)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(
            write_file, "%s member_utf_32_value(%s)\n", indent_string,
            token->member_value.member_utf_8_string.member_string
        );

        TP_FREE(symbol_table, &(utf_8_string.member_string), utf_8_string.member_string_length);
        break;
    }
    case TP_VALUE_TYPE_INT:
        fprintf(
            write_file, "%s member_int_value(%d)\n", indent_string,
            token->member_value.member_int_value
        );
        break;
    case TP_VALUE_TYPE_LONG_INT:
        fprintf(
            write_file, "%s member_long_int_value(%d)\n", indent_string,
            token->member_value.member_long_int_value
        );
        break;
    case TP_VALUE_TYPE_LONG_LONG_INT:
        fprintf(
            write_file, "%s member_long_long_int_value(%zd)\n", indent_string,
            token->member_value.member_long_long_int_value
        );
        break;
    case TP_VALUE_TYPE_UNSIGNED_INT:
        fprintf(
            write_file, "%s member_unsigned_int_value(%u)\n", indent_string,
            token->member_value.member_unsigned_int_value
        );
        break;
    case TP_VALUE_TYPE_UNSIGNED_LONG_INT:
        fprintf(
            write_file, "%s member_unsigned_long_int_value(%u)\n", indent_string,
            token->member_value.member_unsigned_long_int_value
        );
        break;
    case TP_VALUE_TYPE_UNSIGNED_LONG_LONG_INT:
        fprintf(
            write_file, "%s member_unsigned_long_long_int_value(%zu)\n", indent_string,
            token->member_value.member_unsigned_long_long_int_value
        );
        break;
    case TP_VALUE_TYPE_FLOAT:
        fprintf(
            write_file, "%s member_float_value(%f)\n", indent_string,
            token->member_value.member_float_value
        );
        break;
    case TP_VALUE_TYPE_DOUBLE:
        fprintf(
            write_file, "%s member_double_value(%f)\n", indent_string,
            token->member_value.member_double_value
        );
        break;
    case TP_VALUE_TYPE_LONG_DOUBLE:
        fprintf(
            write_file, "%s member_long_double_value(%f)\n", indent_string,
            token->member_value.member_long_double_value
        );
        break;
    default:
        fprintf(write_file, "%sTP_VALUE_TYPE(UNKNOWN_VALUE_TYPE: %d)\n",
            indent_string, token->member_value_type);
        break;
    }

    fprintf(write_file, "%s}\n\n", prev_indent_string);

    err = _set_errno(0);

    return true;
}

static bool make_logical_lines(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    TP_APPEND_KIND append_kind = TP_APPEND_LOGICAL_LINE_KIND;

    for (rsize_t i = 0; input_file->member_physical_lines_pos > i; ++i){

        TP_CHAR8_T* lines_buffer = input_file->member_physical_lines[i];

        rsize_t lines_length = strlen(lines_buffer);

        bool is_lead_line = ((2 <= lines_length) ? (0 == strncmp(lines_buffer + lines_length - 2, "\\\n", 2)) : false);

        if ( ! append_logical_line(
            symbol_table, append_kind, input_file, lines_buffer, (is_lead_line ? (lines_length - 2) : lines_length))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_lead_line){

            append_kind = TP_APPEND_PHYSICAL_LINE_KIND;
        }else{

            append_kind = TP_APPEND_LOGICAL_LINE_KIND;
        }
    }

    return true;
}

static bool append_logical_line(
    TP_SYMBOL_TABLE* symbol_table,
    TP_APPEND_KIND append_kind, TP_INPUT_FILE* input_file, TP_CHAR8_T* lines_buffer, rsize_t lines_length)
{
    switch (append_kind){
    case TP_APPEND_LOGICAL_LINE_KIND:

        if (input_file->member_logical_lines_pos == (input_file->member_logical_lines_size / sizeof(TP_LOGICAL_LINES))){

            rsize_t logical_lines_size_allocate_unit = 
                input_file->member_logical_lines_size_allocate_unit * sizeof(TP_LOGICAL_LINES);

            rsize_t logical_lines_size =  input_file->member_logical_lines_size + logical_lines_size_allocate_unit;

            TP_LOGICAL_LINES* logical_lines = (TP_LOGICAL_LINES*)TP_REALLOC(
                symbol_table, input_file->member_logical_lines, logical_lines_size + sizeof(TP_LOGICAL_LINES)
            );

            if (NULL == logical_lines){

                TP_PRINT_CRT_ERROR(symbol_table);

                return false;
            }

            memset(
                ((TP_CHAR8_T*)logical_lines) + input_file->member_logical_lines_size, 0,
                logical_lines_size_allocate_unit
            );

            input_file->member_logical_lines = logical_lines;
            input_file->member_logical_lines_size = logical_lines_size;
        }

        TP_LOGICAL_LINES* logical_line = &(input_file->member_logical_lines[input_file->member_logical_lines_pos]);

        if ((logical_line->member_physical_lines) || (logical_line->member_physical_lines_size)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: Physical lines exist.")
            );

            return false;
        }

        TP_CHAR8_T** physical_lines = (TP_CHAR8_T**)TP_CALLOC(symbol_table, 2, sizeof(TP_CHAR8_T*));

        if (NULL == physical_lines){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        logical_line->member_physical_lines = physical_lines;

        lines_buffer[lines_length] = '\0';
        logical_line->member_physical_lines[0] = lines_buffer;
        logical_line->member_physical_lines_size = sizeof(TP_CHAR8_T*);

        ++(logical_line->member_physical_lines_pos);

        ++(input_file->member_logical_lines_pos);

        break;
    case TP_APPEND_PHYSICAL_LINE_KIND:

        if ((NULL == input_file->member_logical_lines) || (0 == input_file->member_logical_lines_pos)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                TP_LOG_PARAM_STRING("ERROR: Logical lines does not exist.")
            );

            return false;
        }

        {
            TP_LOGICAL_LINES* logical_lines = &(input_file->member_logical_lines[input_file->member_logical_lines_pos - 1]);

            if (logical_lines->member_physical_lines_pos == (logical_lines->member_physical_lines_size / sizeof(TP_CHAR8_T*))){

                rsize_t physical_lines_allocate_unit = (logical_lines->member_physical_lines_size / sizeof(TP_CHAR8_T*)) +
                    input_file->member_physical_lines_size_allocate_unit + 1;

                TP_CHAR8_T** physical_lines = (TP_CHAR8_T**)TP_CALLOC(symbol_table, physical_lines_allocate_unit, sizeof(TP_CHAR8_T*));

                if (NULL == physical_lines){

                    TP_PRINT_CRT_ERROR(symbol_table);

                    return false;
                }

                memcpy(
                    physical_lines, logical_lines->member_physical_lines,
                    logical_lines->member_physical_lines_pos * sizeof(TP_CHAR8_T*)
                );

                TP_FREE2(
                    symbol_table, &(logical_lines->member_physical_lines),
                    logical_lines->member_physical_lines_pos * sizeof(TP_CHAR8_T*)
                );

                logical_lines->member_physical_lines = physical_lines;

                rsize_t physical_lines_size_allocate_unit = 
                    input_file->member_physical_lines_size_allocate_unit * sizeof(TP_CHAR8_T*);

                rsize_t physical_lines_size = logical_lines->member_physical_lines_size + physical_lines_size_allocate_unit;

                logical_lines->member_physical_lines_size = physical_lines_size;
            }

            lines_buffer[lines_length] = '\0';
            logical_lines->member_physical_lines[logical_lines->member_physical_lines_pos] = lines_buffer;

            ++(logical_lines->member_physical_lines_pos);
        }

        break;
    default:

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
}

static bool get_physical_lines(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    for (;;){

        if (input_file->member_is_from_memory){

            if (false == input_file->member_is_end_of_file){

                input_file->member_is_end_of_file = true;

                if (TP_MAX_LINE_BYTES < input_file->member_string_memory_length){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("ERROR: TP_MAX_LINE_BYTES(%1) < input_file->member_string_memory_length(%2)"),
                        TP_LOG_PARAM_UINT64_VALUE(TP_MAX_LINE_BYTES),
                        TP_LOG_PARAM_UINT64_VALUE(input_file->member_string_memory_length)
                    );

                    return false;
                }

                memcpy(
                    input_file->member_read_lines_buffer,
                    input_file->member_string_memory, input_file->member_string_memory_length
                );

                input_file->member_read_lines_buffer[input_file->member_string_memory_length] = '\0';

                input_file->member_read_lines_length = input_file->member_string_memory_length;
            }
        }else{

            if ( ! read_file(symbol_table, input_file)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        uint8_t* lines_buffer = input_file->member_read_lines_buffer;
        rsize_t lines_length = input_file->member_read_lines_length;

        if (input_file->member_is_end_of_file){

            long utf_8_restart_pos = 0;

            if ( ! tp_is_valid_utf_8(
                symbol_table, lines_buffer, 
                ((input_file->member_is_from_memory) ? (lines_length - 1) : lines_length), &utf_8_restart_pos)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (utf_8_restart_pos){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING("ERROR: Souce code ends in the middle of UTF-8 string.")
                );

                return false;
            }

            if ( ! append_physical_line(
                symbol_table, input_file, lines_buffer, lines_length, utf_8_restart_pos)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if ((false == input_file->member_is_from_memory) &&
                ((input_file->member_physical_lines) && (input_file->member_physical_lines_pos))){

                TP_CHAR8_T* temp_line_buffer =
                    input_file->member_physical_lines[input_file->member_physical_lines_pos - 1];
                rsize_t temp_lines_length = strlen(temp_line_buffer);

                if ((1 <= temp_lines_length) && ('\n' != temp_line_buffer[temp_lines_length - 1])){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                        TP_LOG_PARAM_STRING("ERROR: Not terminated with \\n.")
                    );

                    return false;
                }else if ((2 <= temp_lines_length) &&
                    ('\\' == temp_line_buffer[temp_lines_length - 2])){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                        TP_LOG_PARAM_STRING("ERROR: Terminated with \\.")
                    );

                    return false;
                }
            }

            break;
        }

        if (input_file->member_is_start_of_file){

            // Clear Byte Order Mark.
            static const uint8_t byte_order_mark[] = { 0xEF, 0xBB, 0xBF };

            if (0 == memcmp(lines_buffer, byte_order_mark, sizeof(byte_order_mark))){

                memset(lines_buffer, ' ', sizeof(byte_order_mark));
            }

            input_file->member_is_start_of_file = false;
        }

        long utf_8_restart_pos = 0;

        if ( ! tp_is_valid_utf_8(symbol_table, lines_buffer, 
            ((input_file->member_is_from_memory) ? (lines_length - 1) : lines_length), &utf_8_restart_pos)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! append_physical_line(
            symbol_table, input_file, lines_buffer, lines_length, utf_8_restart_pos)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool append_physical_line(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file,
    TP_CHAR8_T* lines_buffer, rsize_t lines_length, long utf_8_restart_pos)
{
    if (('\0' == *lines_buffer) || (0 == lines_length)){

        return true;
    }

    rsize_t tmp_length = lines_length;

    lines_length -= utf_8_restart_pos;

    if (tmp_length < lines_length){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: Physical line overflow(before length = %1)."),
            TP_LOG_PARAM_UINT64_VALUE(tmp_length)
        );

        return false;
    }

    if (utf_8_restart_pos){

        FILE* stream = input_file->member_read_file;

        long seek_position = 0L;

        if ( ! tp_ftell(symbol_table, stream, &seek_position)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_seek(symbol_table, stream, seek_position, -utf_8_restart_pos)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (input_file->member_is_end_of_file){

            input_file->member_is_end_of_file = false;
        }
    }

    TP_CHAR8_T* start_pos = lines_buffer;
    TP_CHAR8_T* cur_pos = lines_buffer;
    TP_CHAR8_T* end_pos = lines_buffer + (lines_length - 1);

    input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_OTHER;

    bool is_need_append = false;

    for (; *cur_pos; ++cur_pos){

        switch (*cur_pos){
        case '\r':{

            *cur_pos = '\n';

            if (end_pos == cur_pos){

                if (input_file->member_is_end_of_file){

                    input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_NONE;
                }else{

                    input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_CR;
                }
            }else{

                if ('\n' == *(cur_pos + 1)){

                    *(cur_pos + 1) = ' ';
                }

                input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_NONE;
            }

            ++cur_pos;

            rsize_t physical_line_length = cur_pos - start_pos;

            if ( ! append_physical_line_content(symbol_table, input_file, start_pos, physical_line_length)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            start_pos = cur_pos;

            is_need_append = false;

            break;
        }
        case '\n':{

            input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_NONE;

            ++cur_pos;

            rsize_t physical_line_length = cur_pos - start_pos;

            if ( ! append_physical_line_content(symbol_table, input_file, start_pos, physical_line_length)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            start_pos = cur_pos;

            is_need_append = false;

            break;
        }
        default:
            input_file->member_is_ends_in_the_middle = TP_ENDS_IN_THE_MIDDLE_OTHER;
            is_need_append = true;
            break;
        }
    }

    if (is_need_append){

        rsize_t physical_line_length = cur_pos - start_pos - ((input_file->member_is_from_memory) ? 0 : 1);

        if ( ! append_physical_line_content(symbol_table, input_file, start_pos, physical_line_length)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool append_physical_line_content(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_CHAR8_T* line_buffer, rsize_t line_length)
{
    if (input_file->member_physical_lines_pos == (input_file->member_physical_lines_size / sizeof(TP_CHAR8_T*))){

        rsize_t physical_lines_size_allocate_unit = 
            input_file->member_physical_lines_size_allocate_unit * sizeof(TP_CHAR8_T*);

        rsize_t physical_lines_size =  input_file->member_physical_lines_size + physical_lines_size_allocate_unit;

        TP_CHAR8_T** physical_lines = (TP_CHAR8_T**)TP_REALLOC(
            symbol_table, input_file->member_physical_lines, physical_lines_size
        );

        if (NULL == physical_lines){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            physical_lines + input_file->member_physical_lines_size, 0,
            physical_lines_size_allocate_unit
        );

        input_file->member_physical_lines = physical_lines;
        input_file->member_physical_lines_size = physical_lines_size;
    }

    TP_CHAR8_T* temp_line_buffer = NULL;

    if (NULL == input_file->member_physical_lines[input_file->member_physical_lines_pos]){

        temp_line_buffer = (TP_CHAR8_T*)TP_CALLOC(symbol_table, line_length + 1, sizeof(TP_CHAR8_T));

        if (NULL == temp_line_buffer){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memcpy(temp_line_buffer, line_buffer, line_length);
    }else{

        rsize_t prev_length = strlen(input_file->member_physical_lines[input_file->member_physical_lines_pos]);
        rsize_t length = prev_length + line_length + 1;

        temp_line_buffer = TP_REALLOC(
            symbol_table,
            input_file->member_physical_lines[input_file->member_physical_lines_pos],
            length
        );

        if (NULL == temp_line_buffer){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memcpy(temp_line_buffer + prev_length, line_buffer, line_length);
        temp_line_buffer[length] = '\0';
    }

    input_file->member_physical_lines[input_file->member_physical_lines_pos] = temp_line_buffer;

    if ((TP_ENDS_IN_THE_MIDDLE_NONE == input_file->member_is_ends_in_the_middle) ||
        input_file->member_is_from_memory){

        ++(input_file->member_physical_lines_pos);
    }

    return true;
}

static bool read_file(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    FILE* stream = input_file->member_read_file;
    int count = TP_MAX_LINE_BYTES;

    size_t fread_bytes = fread(
        input_file->member_read_lines_buffer,
        sizeof(uint8_t), count, stream
    );

    if (count > fread_bytes){

        int ferror_error = ferror(stream);

        if (ferror_error){

            TP_PRINT_CRT_ERROR(symbol_table);

            clearerr(stream);

            return false;
        }

        int feof_error = feof(stream);

        if (feof_error){

            input_file->member_is_end_of_file = true;
        }
    }

    input_file->member_read_lines_buffer[
        fread_bytes ? (TP_BUFFER_SIZE - fread_bytes) : 0] = '\0';
    input_file->member_read_lines_length = fread_bytes;

    return true;
}

