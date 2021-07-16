
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

const TP_TOKEN global_pp_token_template = {
    .member_symbol = TP_SYMBOL_IDENTIFIER,
    .member_symbol_kind = TP_SYMBOL_UNSPECIFIED_KIND,
    .member_symbol_type = TP_SYMBOL_UNSPECIFIED_TYPE,
    .member_file = NULL,
    .member_line = 0,
    .member_column = 0,
    .member_string = NULL,
    .member_string_length = 0,
    .member_value_type = TP_VALUE_TYPE_NULL,
    .member_value = { 0 },
};

bool tp_lexer(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    if ((NULL == input_file->member_logical_lines) || (0 == input_file->member_logical_lines_pos)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: Logical lines is zero.")
        );

        return false;
    }

    TP_INIT_CHAR_POSITION(symbol_table, input_file);
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    for (bool is_end_of_source_code = false; false == is_end_of_source_code; ){

        bool is_match = false;

        // TP_SYMBOL_WHITE_SPACE
        if (tp_make_pp_token_white_space(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        is_match = false;

        // TP_SYMBOL_CHARACTER_CONSTANT
        if (tp_make_pp_token_character_constant(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        is_match = false;

        // TP_SYMBOL_STRING_LITERAL
        if (tp_make_pp_token_string_literal(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        is_match = false;

        // TP_SYMBOL_IDENTIFIER
        if (tp_make_pp_token_identifier(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        is_match = false;

        // TP_SYMBOL_PP_NUMBER
        if (tp_make_pp_token_pp_number(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        is_match = false;

        // TP_SYMBOL_PUNCTUATOR
        if (tp_make_pp_token_punctuator(symbol_table, input_file, &is_match, &is_end_of_source_code)){

            if (is_match){

                continue;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        TP_CHAR8_T* char_pos = NULL;
        bool is_end_logical_line = false;

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &char_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }

            continue;
        }

        if (is_end_logical_line){

            tp_check_end_of_source_code(input_file, is_end_logical_line, &is_end_of_source_code);

            continue;
        }

        TP_TOKEN pp_token = global_pp_token_template;
        pp_token.member_file = input_file->member_input_file_path;
        pp_token.member_line = input_file->member_physical_line;
        pp_token.member_column = input_file->member_physical_column;
        TP_APPEND_STRING_CHAR(symbol_table, input_file, *char_pos);
        TP_STRING_MOVE_TO_PP_TOKEN(&pp_token, input_file);

        switch (*char_pos){
        case '\n':
            pp_token.member_symbol = TP_SYMBOL_LF;
            break;
        case ' ':
        case '\t':
        case '\v':
        case '\f':
            pp_token.member_symbol = TP_SYMBOL_WHITE_SPACE;
            break;
        default:
            pp_token.member_symbol = TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER;
            break;
        }

        if ( ! tp_append_pp_token(symbol_table, input_file, &pp_token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if (symbol_table->member_is_error_abort){

                goto fail;
            }
        }

        TP_APPEND_CHAR_POSITION(input_file);
    }

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    TP_TOKEN pp_token = global_pp_token_template;
    pp_token.member_symbol = TP_SYMBOL_NULL;

    if ( ! tp_append_pp_token(symbol_table, input_file, &pp_token)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        if (symbol_table->member_is_error_abort){

            goto fail;
        }
    }

    return true;

fail:

    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

    return false;
}

bool tp_append_pp_token(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_TOKEN* pp_token)
{
    if (input_file->member_tp_pp_token_pos == (input_file->member_tp_pp_token_size / sizeof(TP_TOKEN))){

        rsize_t tp_pp_token_size_allocate_unit =
            input_file->member_tp_pp_token_size_allocate_unit * sizeof(TP_TOKEN);

        rsize_t tp_pp_token_size = input_file->member_tp_pp_token_size + tp_pp_token_size_allocate_unit;

        TP_TOKEN* tp_pp_token = (TP_TOKEN*)TP_REALLOC(
            symbol_table, input_file->member_tp_pp_token, tp_pp_token_size
        );

        if (NULL == tp_pp_token){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tp_pp_token) + input_file->member_tp_pp_token_size, 0,
            tp_pp_token_size_allocate_unit
        );

        input_file->member_tp_pp_token = tp_pp_token;
        input_file->member_tp_pp_token_size = tp_pp_token_size;
    }

    input_file->member_tp_pp_token[input_file->member_tp_pp_token_pos] = *pp_token;

    ++(input_file->member_tp_pp_token_pos);

    return true;
}

bool tp_getchar_pos_of_physical_line(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_CHAR8_T** char_pos, bool* is_end_of_logical_line)
{
    TP_LOGICAL_LINES* logical_line = tp_get_logical_line(symbol_table, input_file);

    if (NULL == logical_line){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (NULL == logical_line->member_physical_lines){

        *is_end_of_logical_line = true;

        return true;
    }

    TP_CHAR8_T* local_physical_line =
        logical_line->member_physical_lines[input_file->member_physical_lines_current_line_position];

    if (NULL == local_physical_line){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: Physical line is NULL.")
        );

        return false;
    }

    if ('\0' == local_physical_line[input_file->member_physical_lines_current_column_position]){

        input_file->member_logical_line = input_file->member_logical_lines_current_position;

        ++(input_file->member_physical_line);

        ++(input_file->member_physical_lines_current_line_position);
        input_file->member_physical_lines_current_column_position = 0;

        if (logical_line->member_physical_lines_pos > input_file->member_physical_lines_current_line_position){

            local_physical_line =
                logical_line->member_physical_lines[input_file->member_physical_lines_current_line_position];

            if (NULL == local_physical_line){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING("ERROR: Physical line of logical lines is overflow.")
                );

                return false;
            }

            *char_pos = &(local_physical_line[input_file->member_physical_lines_current_column_position]);
            ++(input_file->member_physical_lines_current_column_position);
        }else{

            *is_end_of_logical_line = true;

            input_file->member_physical_lines_current_line_position = 0;

            if (input_file->member_logical_lines_pos >= (input_file->member_logical_lines_current_position + 1)){

                ++(input_file->member_logical_lines_current_position);
            }
        }

        TP_BACKUP_CHAR_POSITION(symbol_table, input_file);
    }else{

        *char_pos = &(local_physical_line[input_file->member_physical_lines_current_column_position]);
        ++(input_file->member_physical_lines_current_column_position);
    }

    return true;
}

TP_LOGICAL_LINES* tp_get_logical_line(TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file)
{
    if ((NULL == input_file->member_logical_lines->member_physical_lines) ||
        (0 == input_file->member_logical_lines->member_physical_lines_size)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: Physical lines is zero in logical line.")
        );

        return NULL;
    }

    if (input_file->member_logical_lines_pos <
        input_file->member_logical_lines_current_position){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: Logical lines is overflow.")
        );

        return NULL;
    }

    return &(input_file->member_logical_lines[input_file->member_logical_lines_current_position]);
}

void tp_check_end_of_source_code(
    TP_INPUT_FILE* input_file, bool is_end_logical_line, bool* is_end_of_source_code)
{
    if (is_end_logical_line){

        ++(input_file->member_logical_lines_current_position);

        if (input_file->member_logical_lines_pos <= input_file->member_logical_lines_current_position){

            *is_end_of_source_code = true;
        }
    }
}

