
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool c_char_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match
);

bool tp_make_pp_token_character_constant(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code)
{
    TP_ENTER_CHAR_POSITION(symbol_table, input_file);

    TP_CHAR8_T* character_constant_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &character_constant_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_CHAR8_T current_char[2] = { 0 };
    current_char[0] = *character_constant_pos;

    switch (current_char[0]){
    case 'u':
    case 'L':
    case 'U':
        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &character_constant_pos, &is_end_logical_line)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_end_logical_line){

            TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

            return true;
        }
        current_char[1] = *character_constant_pos;
        break;
    default:
        break;
    }

    TP_INIT_STRING(symbol_table, input_file);
    TP_CHAR8_T current = '\0';
    bool is_local_match = false;

    switch (current_char[0]){
    case 'L':
    case 'U':
    case 'u':
        if ('\'' != current_char[1]){

            break;
        }
//      break;
    case '\'':
        current = current_char[0];
        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char[0]);
        if (0 != current_char[1]){

            current = current_char[1];
            TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char[1]);
        }
        if ( ! c_char_sequence(
            symbol_table, input_file, TP_SYMBOL_CHARACTER_CONSTANT,
            current, &is_local_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    default:
        break;
    }

    if (is_local_match){

        TP_TOKEN pp_token = global_pp_token_template;
        pp_token.member_file = input_file->member_input_file_path;
        pp_token.member_line = input_file->member_physical_line;
        pp_token.member_column = input_file->member_physical_column;
        pp_token.member_symbol = TP_SYMBOL_CHARACTER_CONSTANT;
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

static bool c_char_sequence(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, TP_SYMBOL symbol,
    TP_CHAR8_T current_char, bool* is_match)
{
    if ('\'' != current_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: \' != current_char")
        );

        return false;
    }

    TP_CHAR8_T* c_char_sequence_pos = NULL;
    bool is_end_logical_line = false;
    bool is_local_match = false;
    bool is_empty = true;

    // c_char+
    for (;;){

        if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &c_char_sequence_pos, &is_end_logical_line)){

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

        current_char = *c_char_sequence_pos;

        if ( ! tp_escape_sequence(symbol_table, input_file, symbol, current_char, &is_local_match)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (is_local_match){

            is_local_match = false;
            is_empty = false;
            continue;
        }

        TP_APPEND_STRING_CHAR(symbol_table, input_file, current_char);

        if ('\\' == current_char){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: \\ in preprocessor token(%1)."),
                TP_LOG_PARAM_STRING(input_file->member_string)
            );

            return false;
        }

        if ('\'' == current_char){

            *is_match = true;
            break;
        }else{

            is_empty = false;
        }
    }

    if (is_empty){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: Preprocessor token(%1) is empty."),
            TP_LOG_PARAM_STRING(input_file->member_string)
        );

        return false;
    }

    return true;
}

