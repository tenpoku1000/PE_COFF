
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool calc_empty_element(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, TP_GRAMMER* grammer, bool* is_change
);

bool tp_calc_empty(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num)
{
    for (bool is_change = true; is_change; ){

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== calc_empty_loop ===")
        );

        is_change = false;

        for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

            if ( ! calc_empty_element(symbol_table, grammer_tbl, grammer_tbl[i], &is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            for (rsize_t j = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[j]->member_grammer; ++j){

                grammer_tbl[j]->member_is_visit = false;
            }
        }
    }

    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

        if (grammer_tbl[i]->member_is_empty){

            if ( ! tp_print_grammer(symbol_table, NULL, grammer_tbl[i]->member_grammer)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    return true;
}

static bool calc_empty_element(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, TP_GRAMMER* grammer, bool* is_change)
{
    if (grammer->member_is_visit){

        return true;
    }

    grammer->member_is_visit = true;

    rsize_t element_num = grammer->member_element_num;
    TP_GRAMMER_ELEMENT* grammer_element = grammer->member_element;

    rsize_t empty_num = 0;
    rsize_t non_empty_num = 0;
    bool is_current_empty = true;
    bool is_prev_pipe = false;

    for (rsize_t i = 0; element_num > i; ++i){

        switch (grammer_element[i].member_type){
        case TP_GRAMMER_TYPE_TERM:{

            TP_GRAMMER_TERM* grammer_term = &(grammer_element[i].member_body.member_term);

            switch (grammer_term->member_symbol){
            case TP_SYMBOL_EMPTY:
//              break;
            case TP_SYMBOL_IDENTIFIER:
//              break;
            case TP_SYMBOL_PUNCTUATOR:
//              break;
            case TP_SYMBOL_KEYWORD:
//              break;
            case TP_SYMBOL_CONSTANT:

                if (false == grammer_element[i].member_is_empty){

                    is_current_empty = false;
                }

                is_prev_pipe = false;

                break;
            case TP_SYMBOL_PIPE:

                if (0 == i){

                    return false;
                }else if (element_num == (i + 1)){

                    return false;
                }else if (is_prev_pipe){

                    return false;
                }

                if (is_current_empty){

                    ++empty_num;
                }else{

                    ++non_empty_num;

                    is_current_empty = true;
                }

                is_prev_pipe = true;

                break;
            case TP_SYMBOL_NULL:
                break;
            default:
                return false;
            }

            break;
        }
        case TP_GRAMMER_TYPE_NONTERM:{

            TP_PARSE_TREE_GRAMMER nonterm = grammer_element[i].member_body.member_nonterm;

            for (rsize_t j = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[j]->member_grammer; ++j){

                if (nonterm == grammer_tbl[j]->member_grammer){

                    if ( ! calc_empty_element(symbol_table, grammer_tbl, grammer_tbl[j], is_change)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }

                    if (grammer_tbl[j]->member_is_empty){

                        grammer_element[i].member_is_empty = true;
                    }else{

                        is_current_empty = false;
                    }

                    break;
                }
            }

            is_prev_pipe = false;

            break;
        }
        case TP_GRAMMER_TYPE_NULL:

            if (element_num != (i + 1)){

                return false;
            }

            if (is_current_empty){

                ++empty_num;
            }else{

                ++non_empty_num;
            }

            if (empty_num && (0 == non_empty_num)){

                if (false == grammer->member_is_empty){

                    grammer->member_is_empty = true;

                    *is_change = true;
                }
            }else{

                if (grammer->member_is_empty){

                    grammer->member_is_empty = false;

                    *is_change = true;
                }
            }

            return true;
        default:
            return false;
        }
    }

    return false;
}

