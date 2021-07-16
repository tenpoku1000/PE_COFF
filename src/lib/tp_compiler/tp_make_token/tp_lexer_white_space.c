
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool comment(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code
);

bool tp_make_pp_token_white_space(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file,  bool* is_match, bool* is_end_of_source_code)
{
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    uint8_t* white_space_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &white_space_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_INIT_STRING(symbol_table, input_file);
    bool is_local_match = false;

    switch (*white_space_pos){
    case '\n':
        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);
        return true;
    case ' ':
    case '\t':
    case '\v':
    case '\f':
        is_local_match = true;
        break;
    case '/':
        if ( ! comment(symbol_table, input_file, &is_local_match, is_end_of_source_code)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    case '*':
        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &white_space_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ((false == is_end_logical_line) && ('/' == *white_space_pos)){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("WARNING: Found orphan end of comment(%1:%2)."),
                TP_LOG_PARAM_STRING(input_file->member_input_file_path),
                TP_LOG_PARAM_UINT64_VALUE(input_file->member_physical_line)
            );
        }
//      break;
    default:
        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);
        return true;
    }

    if (is_local_match){

        TP_TOKEN pp_token = global_pp_token_template;
        pp_token.member_file = input_file->member_input_file_path;
        pp_token.member_line = input_file->member_physical_line;
        pp_token.member_column = input_file->member_physical_column;
        pp_token.member_symbol = TP_SYMBOL_WHITE_SPACE;
        TP_APPEND_STRING_CHAR(symbol_table, input_file, ' ');
        TP_STRING_MOVE_TO_PP_TOKEN(&pp_token, input_file);

        if ( ! tp_append_pp_token(symbol_table, input_file, &pp_token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        TP_APPEND_CHAR_POSITION(input_file);

        *is_match = true;

        tp_check_end_of_source_code(input_file, is_end_logical_line, is_end_of_source_code);

        TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    *is_match = false;

    TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

    return true;
}

static bool comment(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code)
{
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    uint8_t* comment_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &comment_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    if ('/' == *comment_pos){

        for (;;){

            if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &comment_pos, &is_end_logical_line)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (is_end_logical_line){

                TP_APPEND_CHAR_POSITION(input_file);

                *is_match = true;

                tp_check_end_of_source_code(input_file, is_end_logical_line, is_end_of_source_code);

                TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

                return true;
            }
        }
    }

    if ('*' == *comment_pos){

        for (bool is_local_match = false; ; ){

            if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &comment_pos, &is_end_logical_line)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (is_end_logical_line){

                tp_check_end_of_source_code(input_file, is_end_logical_line, is_end_of_source_code);

                if (is_end_of_source_code){

                    goto fail;
                }
            }

            if ('*' == *comment_pos){

                if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &comment_pos, &is_end_logical_line)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return false;
                }

                if (is_end_logical_line){

                    tp_check_end_of_source_code(input_file, is_end_logical_line, is_end_of_source_code);

                    if (is_end_of_source_code){

                        goto fail;
                    }
                }

                if ('/' == *comment_pos){

                    TP_APPEND_CHAR_POSITION(input_file);

                    *is_match = true;

                    TP_LEAVE_CHAR_POSITION(symbol_table, input_file);

                    return true;
                }
            }

            if ('/' == *comment_pos){

                if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &comment_pos, &is_end_logical_line)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return false;
                }

                if (is_end_logical_line){

                    tp_check_end_of_source_code(input_file, is_end_logical_line, is_end_of_source_code);

                    if (is_end_of_source_code){

                        goto fail;
                    }
                }

                if ('*' == *comment_pos){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("WARNING: Nesting of comment(%1:%2)."),
                        TP_LOG_PARAM_STRING(input_file->member_input_file_path),
                        TP_LOG_PARAM_UINT64_VALUE(input_file->member_physical_line)
                    );
                }
            }
        }
    }

    TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

    return true;

fail:

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: Souce code ends in the middle of comment(%1:%2)."),
        TP_LOG_PARAM_STRING(input_file->member_input_file_path),
        TP_LOG_PARAM_UINT64_VALUE(input_file->member_physical_line)
    );

    return false;
}

