
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_make_pp_token_pp_number(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code)
{
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    TP_CHAR8_T* pp_number_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &pp_number_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_INIT_STRING(symbol_table, input_file);

    // digit
    if (isdigit(*pp_number_pos)){

        TP_APPEND_STRING_CHAR(symbol_table, input_file, *pp_number_pos);
    }else if ('.' == *pp_number_pos){

        // . digit

        TP_APPEND_STRING_CHAR(symbol_table, input_file, '.');

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &pp_number_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

            return true;
        }

        if (isdigit(*pp_number_pos)){

            TP_APPEND_STRING_CHAR(symbol_table, input_file, *pp_number_pos);
        }else{

            TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

            return true;
        }
    }else{

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

    bool is_local_match = false;

    // (digit | identifier-nondigit | e sign | E sign | p sign | P sign | . )*
    // sign: (+ | -)
    for (;;){

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &pp_number_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            break;
        }

        // digit
        if (isdigit(*pp_number_pos)){

            TP_APPEND_STRING_CHAR(symbol_table, input_file, *pp_number_pos);
            is_local_match = true;
        }else{

            if (is_local_match){

                TP_RESTORE_CHAR_POSITION(symbol_table, input_file);

                goto out;
            }
        }

        if (is_local_match){

            TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

            continue;
        }

        // identifier_nondigit
        if ( ! tp_identifier_nondigit(symbol_table, input_file, TP_SYMBOL_PP_NUMBER,
            false/* bool is_start_char */, *pp_number_pos, &is_local_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_local_match){

            TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

            continue;
        }

        // (e sign | E sign | p sign | P sign | . )*
        // sign: (+ | -)
        switch (*pp_number_pos){
        case 'e':
        case 'E':
        case 'p':
        case 'P':
        {
            TP_CHAR8_T c2[2] = { 0 };
            c2[0] = *pp_number_pos;

            if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &pp_number_pos, &is_end_logical_line)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (is_end_logical_line){

                TP_APPEND_STRING_CHAR(symbol_table, input_file, c2[0]);

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(PP_NUMBER: %1)."),
                    TP_LOG_PARAM_STRING(input_file->member_string)
                );

                return false;
            }

            c2[1] = *pp_number_pos;

            if (('+' != c2[1]) && ('-' != c2[1])){

                TP_RESTORE_CHAR_POSITION(symbol_table, input_file);

                goto out;
            }

            TP_APPEND_STRING_CHAR(symbol_table, input_file, c2[0]);
            TP_APPEND_STRING_CHAR(symbol_table, input_file, c2[1]);
            is_local_match = true;
            break;
        }
        case '.':
            TP_APPEND_STRING_CHAR(symbol_table, input_file, '.');
            is_local_match = true;
            break;
        default:
            TP_RESTORE_CHAR_POSITION(symbol_table, input_file);
            goto out;
        }

        TP_BACKUP_CHAR_POSITION(symbol_table, input_file);
    }

out:
    ;

    TP_TOKEN pp_token = global_pp_token_template;
    pp_token.member_file = input_file->member_input_file_path;
    pp_token.member_line = input_file->member_physical_line;
    pp_token.member_column = input_file->member_physical_column;
    pp_token.member_symbol = TP_SYMBOL_PP_NUMBER;
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

