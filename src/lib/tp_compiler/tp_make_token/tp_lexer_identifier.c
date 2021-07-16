
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_make_pp_token_identifier(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code)
{
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    TP_CHAR8_T* id_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &id_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_INIT_STRING(symbol_table, input_file);
    bool is_local_match = false;

    // identifier_nondigit
    if ( ! tp_identifier_nondigit(
        symbol_table, input_file, TP_SYMBOL_IDENTIFIER, true/* bool is_start_char */, *id_pos, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (false == is_local_match){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

    // (identifier_nondigit | digit)*
    for (;;){

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &id_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            break;
        }

        // identifier_nondigit
        if ( ! tp_identifier_nondigit(
            symbol_table, input_file, TP_SYMBOL_IDENTIFIER, false/* bool is_start_char */, *id_pos, &is_local_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_local_match){

            TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

            continue;
        }

        // digit
        if (isdigit(*id_pos)){

            TP_APPEND_STRING_CHAR(symbol_table, input_file, *id_pos);
            is_local_match = true;
        }else{

            TP_RESTORE_CHAR_POSITION(symbol_table, input_file);

            break;
        }

        TP_BACKUP_CHAR_POSITION(symbol_table, input_file);
    }

    if ((input_file && (NULL == input_file->member_string)) ||
        (NULL == input_file)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! isascii(input_file->member_string[0])){

        TP_CHAR32_T code_point = 0;

        if ( ! tp_decode_utf_8_char(
            symbol_table, &code_point, input_file->member_string, input_file->member_string_pos)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        bool is_numeric = false;

        if ( ! tp_is_numeric_code_point(symbol_table, code_point, &is_numeric)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_numeric){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: First character of identifier is numeric(code_point = %1)."),
                TP_LOG_PARAM_UINT64_VALUE(code_point)
            );

            return false;
        }
    }

    TP_TOKEN pp_token = global_pp_token_template;
    pp_token.member_file = input_file->member_input_file_path;
    pp_token.member_line = input_file->member_physical_line;
    pp_token.member_column = input_file->member_physical_column;
    pp_token.member_symbol = TP_SYMBOL_IDENTIFIER;
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

bool tp_identifier_nondigit(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    bool is_start_char, TP_CHAR8_T current_char, bool* is_match)
{
    if (isalpha(current_char) || ('_' == current_char)){

        // nondigit:
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);
        *is_match = true;

        return true;
    }

    bool is_local_match = false;

    if ( ! tp_universal_character_name(
        symbol_table, input_file, symbol,
        is_start_char, current_char, &is_local_match)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_local_match){

        // universal_character_name:
        *is_match = true;

        return true;
    }

    if ( ! isascii(current_char)){

        // other_implementation_defined_characters:
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);
        *is_match = true;

        return true;
    }

    *is_match = false;

    return true;
}

