
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_PUNCTUATOR_MAX 3
#define TP_PUNCTUATOR_NUM (TP_PUNCTUATOR_MAX + 2)

bool tp_make_pp_token_punctuator(
    TP_SYMBOL_TABLE* symbol_table, TP_INPUT_FILE* input_file, bool* is_match, bool* is_end_of_source_code)
{
    typedef struct TP_PUNCTUATOR_{
        TP_SYMBOL_KIND symbol_kind;
        TP_CHAR8_T punctuator[TP_PUNCTUATOR_NUM];
    }TP_PUNCTUATOR;

    static TP_PUNCTUATOR punctuator_table[] = {
        { TP_SYMBOL_KIND_LEFT_SQUARE_BRACKET, { '[', 0, 0, 0, 0 } },
        { TP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET, { ']', 0, 0, 0, 0 } },
        { TP_SYMBOL_KIND_LEFT_PARENTHESIS, { '(', 0, 0, 0, 0 } },
        { TP_SYMBOL_KIND_RIGHT_PARENTHESIS, { ')', 0, 0, 0, 0 } },
        { TP_SYMBOL_KIND_LEFT_CURLY_BRACKET, { '{', 0, 0, 0, 0 } },
        { TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET, { '}', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_TRIPLE_PERIOD, { '.', '.', '.', 0, 0 } },
        { TP_SYMBOL_KIND_PERIOD, { '.', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_AMPERSAND, { '&', '&', 0, 0, 0 } },
        { TP_SYMBOL_KIND_AMPERSAND_EQUAL, { '&', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_AMPERSAND, { '&', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_ASTERISK_EQUAL, { '*', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_ASTERISK, { '*', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_PLUS, { '+', '+', 0, 0, 0 } },
        { TP_SYMBOL_KIND_PLUS_EQUAL, { '+', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_PLUS, { '+', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_MINUS, { '-', '-', 0, 0, 0 } },
        { TP_SYMBOL_KIND_MINUS_EQUAL, { '-', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_MINUS_GREATER_THAN, { '-', '>', 0, 0, 0 } },
        { TP_SYMBOL_KIND_MINUS, { '-', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_TILDE, { '~', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_EXCLAMATION_MARK_EQUAL, { '!', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_EXCLAMATION_MARK, { '!', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_SLASH_EQUAL, { '/', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_SLASH, { '/', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_NUMBER, { '%', ':', '%', ':', 0 } },   // ##
        { TP_SYMBOL_KIND_NUMBER, { '%', ':', 0, 0, 0 } },              // #
        { TP_SYMBOL_KIND_RIGHT_CURLY_BRACKET, { '%', '>', 0, 0, 0 } }, // }
        { TP_SYMBOL_KIND_PERCENT_EQUAL, { '%', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_PERCENT, { '%', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_CARET_EQUAL, { '^', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_CARET, { '^', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_VERTICAL_BAR, { '|', '|', 0, 0, 0 } },
        { TP_SYMBOL_KIND_VERTICAL_BAR_EQUAL, { '|', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_VERTICAL_BAR, { '|', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_QUESTION_MARK, { '?', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_RIGHT_SQUARE_BRACKET, { ':', '>', 0, 0, 0 } }, // ]
        { TP_SYMBOL_KIND_COLON, { ':', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_SEMICOLON, { ';', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_EQUAL, { '=', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_EQUAL, { '=', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_LESS_THAN_EQUAL, { '<', '<', '=', 0, 0 } },
        { TP_SYMBOL_KIND_DOUBLE_LESS_THAN, { '<', '<', 0, 0, 0 } },
        { TP_SYMBOL_KIND_LEFT_CURLY_BRACKET, { '<', '%', 0, 0, 0 } },   // {
        { TP_SYMBOL_KIND_LEFT_SQUARE_BRACKET, { '<', ':', 0, 0, 0 } },  // [
        { TP_SYMBOL_KIND_LESS_THAN_EQUAL, { '<', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_LESS_THAN, { '<', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_GREATER_THAN_EQUAL, { '>', '>', '=', 0, 0 } },
        { TP_SYMBOL_KIND_DOUBLE_GREATER_THAN, { '>', '>', 0, 0, 0 } },
        { TP_SYMBOL_KIND_GREATER_THAN_PERCENT, { '>', '%', 0, 0, 0 } },
        { TP_SYMBOL_KIND_GREATER_THAN_EQUAL, { '>', '=', 0, 0, 0 } },
        { TP_SYMBOL_KIND_GREATER_THAN, { '>', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_COMMA, { ',', 0, 0, 0, 0 } },

        { TP_SYMBOL_KIND_DOUBLE_NUMBER, { '#', '#', 0, 0, 0 } },
        { TP_SYMBOL_KIND_NUMBER, { '#', 0, 0, 0, 0 } },

        { TP_SYMBOL_UNSPECIFIED_KIND, { 0, 0, 0, 0, 0 } },
    };

    TP_ENTER_CHAR_POSITION(symbol_table, input_file);
    TP_BACKUP_CHAR_POSITION(symbol_table, input_file);

    TP_CHAR8_T* punctuator_pos = NULL;
    bool is_end_logical_line = false;

    if ( ! tp_getchar_pos_of_physical_line(symbol_table, input_file, &punctuator_pos, &is_end_logical_line)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_end_logical_line){

        TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

        return true;
    }

    TP_INIT_STRING(symbol_table, input_file);

    TP_CHAR8_T punctuator[TP_PUNCTUATOR_NUM] = { 0 };
    int32_t punctuator_state = 0;
    TP_SYMBOL_KIND symbol_kind = TP_SYMBOL_UNSPECIFIED_KIND;
    bool is_local_match = false;

    for (rsize_t i = 0; 0 != punctuator_table[i].punctuator[0]; ++i){

        for (int32_t punctuator_state_index = 0; TP_PUNCTUATOR_MAX >= punctuator_state_index; ++punctuator_state_index){

            if (punctuator_table[i].punctuator[punctuator_state_index] == (*punctuator_pos)){

                symbol_kind = punctuator_table[i].symbol_kind;

                punctuator[punctuator_state_index] = (*punctuator_pos);

                punctuator_state = punctuator_state_index;

                is_local_match = true;

                if (TP_PUNCTUATOR_MAX == punctuator_state_index){

                    TP_APPEND_STRING_CHAR(symbol_table, input_file, *punctuator_pos);

                    goto out;
                }else{

                    ++punctuator_state_index;

                    if (0 != punctuator_table[i].punctuator[punctuator_state_index]){

                        if ( ! tp_getchar_pos_of_physical_line(
                            symbol_table, input_file, &punctuator_pos, &is_end_logical_line)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);

                            return false;
                        }
                    }else{

                        TP_APPEND_STRING_CHAR(symbol_table, input_file, *punctuator_pos);

                        goto out;
                    }

                    if (is_end_logical_line){

                        if (0 != punctuator_table[i].punctuator[punctuator_state_index]){

                            TP_PUT_LOG_MSG(
                                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                                TP_MSG_FMT("ERROR: Souce code ends in the middle of preprocessor token(%1, kind=%2:%3)."),
                                TP_LOG_PARAM_STRING(punctuator),
                                TP_LOG_PARAM_INT32_VALUE(symbol_kind),
                                TP_LOG_PARAM_STRING(punctuator_table[i].punctuator)
                            );

                            return false;
                        }

                        is_local_match = false;

                        goto out;
                    }else{

                        if (punctuator_table[i].punctuator[punctuator_state_index] != (*punctuator_pos)){

                            is_local_match = false;

                            TP_RESTORE_CHAR_POSITION(symbol_table, input_file);

                            if ( ! tp_getchar_pos_of_physical_line(
                                symbol_table, input_file, &punctuator_pos, &is_end_logical_line)){

                                TP_PUT_LOG_MSG_TRACE(symbol_table);

                                return false;
                            }

                            if (is_end_logical_line){

                                TP_CLEAR_CHAR_POSITION(symbol_table, input_file);

                                return true;
                            }

                            break;
                        }

                        TP_APPEND_STRING_CHAR(symbol_table, input_file, *punctuator_pos);
                    }
                }
            }else{

                if (is_local_match){

                    goto out;
                }

                break;
            }
        }
    }

out:

    if (is_local_match){

        TP_TOKEN pp_token = global_pp_token_template;
        pp_token.member_file = input_file->member_input_file_path;
        pp_token.member_line = input_file->member_physical_line;
        pp_token.member_column = input_file->member_physical_column;
        pp_token.member_symbol = TP_SYMBOL_PUNCTUATOR;
        pp_token.member_symbol_kind = symbol_kind;
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

