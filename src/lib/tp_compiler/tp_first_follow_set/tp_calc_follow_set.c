
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool append_follow_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer, TP_GRAMMER_TERM* grammer_term, bool* is_change
);
static bool calc_follow_set_element(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, TP_GRAMMER* grammer, bool* is_change
);
static bool print_follow_set(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_GRAMMER* grammer);
static bool append_follow_set_from_first_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer_from, TP_GRAMMER* grammer_to, bool* is_change
);
static bool append_follow_set_from_follow_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer_from, TP_GRAMMER* grammer_to, bool* is_change
);

bool tp_calc_follow_set(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, rsize_t grammer_num, bool is_generate)
{
    TP_GRAMMER_TERM grammer_term_eof = { .member_symbol = TP_SYMBOL_EOF, .member_string = "EOF" };
    bool is_change_start_symbol = false;

    if ( ! append_follow_set(symbol_table, grammer_tbl[0], &grammer_term_eof, &is_change_start_symbol) ||
        (false == is_change_start_symbol)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    for (bool is_change = true; is_change; ){

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("\n=== calc_follow_set_loop ===")
        );

        is_change = false;

        for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

            if ( ! calc_follow_set_element(symbol_table, grammer_tbl, grammer_tbl[i], &is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    if (is_generate){

        FILE* write_file_follow_set_1 = NULL;

        if ( ! tp_open_write_file_text(
            symbol_table,
            symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1],
            &write_file_follow_set_1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

            if ( ! print_follow_set(symbol_table, write_file_follow_set_1, grammer_tbl[i])){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
        }

        if ( ! tp_close_file(symbol_table, &write_file_follow_set_1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        FILE* write_file_follow_set_1_max_num = NULL;

        if ( ! tp_open_write_file_text(
            symbol_table,
            symbol_table->member_follow_set_path[TP_WRITE_FIRST_FOLLOW_SET_PATH_INDEX_1_MAX_NUM],
            &write_file_follow_set_1_max_num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        rsize_t follow_set_1_max_num = 0;

        for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

            if (follow_set_1_max_num < grammer_tbl[i]->member_follow_set_num){

                follow_set_1_max_num = grammer_tbl[i]->member_follow_set_num;
            }
        }

        if (0 == follow_set_1_max_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        fprintf_s(write_file_follow_set_1_max_num, "%zd\n", follow_set_1_max_num + 1);

        if ( ! tp_close_file(symbol_table, &write_file_follow_set_1_max_num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        return true;

fail:

        if ( ! tp_close_file(symbol_table, &write_file_follow_set_1)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }

        if ( ! tp_close_file(symbol_table, &write_file_follow_set_1_max_num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }

        return false;
    }else{

        for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

            if ( ! print_follow_set(symbol_table, NULL, grammer_tbl[i])){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    return true;
}

static bool append_follow_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer, TP_GRAMMER_TERM* grammer_term, bool* is_change)
{
    for (rsize_t i = 0; grammer->member_follow_set_num > i; ++i){

        if (0 == strcmp(grammer_term->member_string, grammer->member_follow_set[i].member_string)){

            return true;
        }
    }

    TP_GRAMMER_TERM* tmp_grammer_term = (TP_GRAMMER_TERM*)TP_REALLOC(symbol_table,
        grammer->member_follow_set, sizeof(TP_GRAMMER_TERM) * (grammer->member_follow_set_num + 1)
    );

    if (NULL == tmp_grammer_term){

        return false;
    }

    grammer->member_follow_set = tmp_grammer_term;

    ++(grammer->member_follow_set_num);

    grammer->member_follow_set[grammer->member_follow_set_num - 1] = *grammer_term;

    *is_change = true;

    return true;
}

static bool calc_follow_set_element(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER** grammer_tbl, TP_GRAMMER* grammer, bool* is_change)
{
    for (rsize_t i = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[i]->member_grammer; ++i){

        rsize_t element_num = grammer_tbl[i]->member_element_num;
        TP_GRAMMER_ELEMENT* grammer_element = grammer_tbl[i]->member_element;

        for (rsize_t j = 0; element_num > j; ++j){

            switch (grammer_element[j].member_type){
            case TP_GRAMMER_TYPE_TERM:
                break;
            case TP_GRAMMER_TYPE_NONTERM:{

                if (element_num == (j + 1)){

                    return false;
                }

                if (grammer->member_grammer != grammer_element[j].member_body.member_nonterm){

                    break;
                }

                switch (grammer_element[j + 1].member_type){
                case TP_GRAMMER_TYPE_NULL:

                    if ( ! append_follow_set_from_follow_set(symbol_table, grammer_tbl[i], grammer, is_change)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }

                    break;
                case TP_GRAMMER_TYPE_NONTERM:

                    if (grammer_element[j + 1].member_is_empty){

                        if ( ! append_follow_set_from_follow_set(symbol_table, grammer_tbl[i], grammer, is_change)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);

                            return false;
                        }
                    }else{

                        TP_PARSE_TREE_GRAMMER nonterm = grammer_element[j + 1].member_body.member_nonterm;

                        for (rsize_t k = 0; TP_PARSE_TREE_GRAMMER_NULL != grammer_tbl[k]->member_grammer; ++k){

                            if (nonterm == grammer_tbl[k]->member_grammer){

                                if ( ! append_follow_set_from_first_set(symbol_table, grammer_tbl[k], grammer, is_change)){

                                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                                    return false;
                                }

                                if (tp_is_contained_empty_first_set(symbol_table, grammer_tbl[k])){

                                    if ( ! append_follow_set_from_follow_set(symbol_table, grammer_tbl[i], grammer, is_change)){

                                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                                        return false;
                                    }
                                }

                                goto out;
                            }
                        }

                        return false;
                    }
out:
                    break;
                case TP_GRAMMER_TYPE_TERM:{

                    TP_GRAMMER_TERM* grammer_term = &(grammer_element[j + 1].member_body.member_term);

                    if (TP_SYMBOL_PIPE == grammer_term->member_symbol){

                        if ( ! append_follow_set_from_follow_set(symbol_table, grammer_tbl[i], grammer, is_change)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);

                            return false;
                        }
                    }else{

                        if ( ! append_follow_set(symbol_table, grammer, grammer_term, is_change)){

                            TP_PUT_LOG_MSG_TRACE(symbol_table);

                            return false;
                        }
                    }

                    break;
                }
                default:
                    return false;
                }

                break;
            }
            case TP_GRAMMER_TYPE_NULL:

                if (element_num != (j + 1)){

                    return false;
                }

                break;
            default:
                return false;
            }
        }
    }

    return true;
}

static bool print_follow_set(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_GRAMMER* grammer)
{
    if (write_file){ fprintf_s(write_file, "{ "); }

    if ( ! tp_print_grammer(symbol_table, write_file, grammer->member_grammer)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (write_file){ fprintf_s(write_file, ", {\n"); }

    for (rsize_t i = 0; grammer->member_follow_set_num > i; ++i){

        if (write_file){ fprintf_s(write_file, "    { "); }

        if ( ! tp_print_symbol(symbol_table, write_file, grammer->member_follow_set[i].member_symbol)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (write_file){ fprintf_s(write_file, ", \"%s\" },\n", grammer->member_follow_set[i].member_string); }

        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(grammer->member_follow_set[i].member_string)
        );
    }

    if (write_file){ fprintf_s(write_file, "    { TP_SYMBOL_NULL, NULL }, }\n},\n"); }

    TP_PUT_LOG_PRINT(
        symbol_table,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING(" ")
    );

    return true;
}

static bool append_follow_set_from_first_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer_from, TP_GRAMMER* grammer_to, bool* is_change)
{
    for (rsize_t i = 0; grammer_from->member_first_set_num > i; ++i){

        if (TP_SYMBOL_EMPTY == grammer_from->member_first_set[i].member_symbol){

            continue;
        }

        if (0 == grammer_to->member_follow_set_num){

            if ( ! append_follow_set(symbol_table, grammer_to, &(grammer_from->member_first_set[i]), is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        for (rsize_t j = 0; grammer_to->member_follow_set_num > j; ++j){

            if (0 == strcmp(grammer_from->member_first_set[i].member_string, grammer_to->member_follow_set[j].member_string)){

                continue;
            }

            if ( ! append_follow_set(symbol_table, grammer_to, &(grammer_from->member_first_set[i]), is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    return true;
}

static bool append_follow_set_from_follow_set(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER* grammer_from, TP_GRAMMER* grammer_to, bool* is_change)
{
    for (rsize_t i = 0; grammer_from->member_follow_set_num > i; ++i){

        if (0 == grammer_to->member_follow_set_num){

            if ( ! append_follow_set(symbol_table, grammer_to, &(grammer_from->member_follow_set[i]), is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        for (rsize_t j = 0; grammer_to->member_follow_set_num > j; ++j){

            if (0 == strcmp(grammer_from->member_follow_set[i].member_string, grammer_to->member_follow_set[j].member_string)){

                continue;
            }

            if ( ! append_follow_set(symbol_table, grammer_to, &(grammer_from->member_follow_set[i]), is_change)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    return true;
}

