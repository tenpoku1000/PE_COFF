
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static int compare_grammer_term(const void* param1, const void* param2);

TP_GRAMMER** tp_make_grammer(
    TP_SYMBOL_TABLE* symbol_table, rsize_t* grammer_num_out, TP_GRAMMER** grammer, rsize_t grammer_num)
{
    TP_GRAMMER** grammer_buffer = (TP_GRAMMER**)TP_CALLOC(symbol_table, grammer_num, sizeof(TP_GRAMMER*));

    if (NULL == grammer_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        *grammer_num_out = 0;

        return NULL;
    }

    memcpy(
        grammer_buffer, grammer,
        sizeof(TP_GRAMMER*) * grammer_num
    );
    *grammer_num_out = grammer_num;

    return grammer_buffer;
}

TP_GRAMMER* tp_make_grammer_element(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER grammer, TP_GRAMMER_ELEMENT* grammer_element, rsize_t grammer_element_num)
{
    TP_GRAMMER* grammer_element_buffer = (TP_GRAMMER*)TP_CALLOC(symbol_table, 1, sizeof(TP_GRAMMER));

    if (NULL == grammer_element_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    TP_GRAMMER_ELEMENT* element_buffer = (TP_GRAMMER_ELEMENT*)TP_CALLOC(
        symbol_table, grammer_element_num, sizeof(TP_GRAMMER_ELEMENT)
    );

    if (NULL == element_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(symbol_table, &grammer_element_buffer, sizeof(TP_GRAMMER));

        return NULL;
    }

    grammer_element_buffer->member_grammer = grammer;
    grammer_element_buffer->member_is_empty = false;
    grammer_element_buffer->member_element_num = grammer_element_num;
    grammer_element_buffer->member_element = element_buffer;
    memcpy(
        grammer_element_buffer->member_element, grammer_element,
        sizeof(TP_GRAMMER_ELEMENT) * grammer_element_num
    );

    return grammer_element_buffer;
}

void tp_free_grammer(TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER*** grammer, rsize_t grammer_element_num)
{
    if (NULL == grammer){

        return;
    }

    TP_GRAMMER** p = *grammer;

    if (p){

        for (rsize_t i = 0; grammer_element_num > i; ++i){

            TP_FREE(
                symbol_table, &(p[i]->member_first_set),
                sizeof(TP_GRAMMER_ELEMENT) * p[i]->member_first_set_num
            );

            TP_FREE(
                symbol_table, &(p[i]->member_follow_set),
                sizeof(TP_GRAMMER_ELEMENT) * p[i]->member_follow_set_num
            );

            TP_FREE(
                symbol_table, &(p[i]->member_element),
                sizeof(TP_GRAMMER_ELEMENT) * p[i]->member_element_num
            );

            TP_FREE(symbol_table, &p[i], sizeof(TP_GRAMMER*));
        }

        TP_FREE2(symbol_table, grammer, sizeof(TP_GRAMMER**));
    }
}

bool tp_print_grammer(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_PARSE_TREE_GRAMMER grammer)
{
    switch (grammer){
    case TP_PARSE_TREE_GRAMMER_PROGRAM:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_PROGRAM"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_PROGRAM:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_STATEMENT"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_STATEMENT:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_TYPE:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_TYPE"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_TYPE:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_EXPRESSION"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_EXPRESSION:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_TERM:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_TERM"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_TERM:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_TMP1:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_TERM_TMP1"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_TERM_TMP1:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_TMP2:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_TERM_TMP2"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_TERM_TMP2:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_FACTOR"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_FACTOR:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_TMP1:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_FACTOR_TMP1"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_FACTOR_TMP1:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_TMP2:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_FACTOR_TMP2"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_FACTOR_TMP2:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_E:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_E"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_E:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_E_TMP:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_E_TMP"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_E_TMP:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_T:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_T"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_T:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_T_TMP:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_T_TMP"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_T_TMP:")
        );
        break;
    case TP_PARSE_TREE_GRAMMER_F:
        if (write_file){ fprintf_s(write_file, "TP_PARSE_TREE_GRAMMER_F"); }
        TP_PUT_LOG_PRINT(
            symbol_table,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("TP_PARSE_TREE_GRAMMER_F:")
        );
        break;
    default:
        return false;
    }

    return true;
}

bool tp_print_symbol(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SYMBOL symbol)
{
    if (NULL == write_file){

        return true;
    }

    switch (symbol){
    case TP_SYMBOL_NULL:
        fprintf_s(write_file, "TP_SYMBOL_NULL");
        break;
    case TP_SYMBOL_PIPE: // |
        fprintf_s(write_file, "TP_SYMBOL_PIPE");
        break;
    case TP_SYMBOL_EMPTY: // ε
        fprintf_s(write_file, "TP_SYMBOL_EMPTY");
        break;
    case TP_SYMBOL_EOF:
        fprintf_s(write_file, "TP_SYMBOL_EOF");
        break;
    case TP_SYMBOL_HEADER_NAME:
        fprintf_s(write_file, "TP_SYMBOL_HEADER_NAME");
        break;
    case TP_SYMBOL_IDENTIFIER:
        fprintf_s(write_file, "TP_SYMBOL_IDENTIFIER");
        break;
    case TP_SYMBOL_PP_NUMBER:
        fprintf_s(write_file, "TP_SYMBOL_PP_NUMBER");
        break;
    case TP_SYMBOL_CHARACTER_CONSTANT:
        fprintf_s(write_file, "TP_SYMBOL_CHARACTER_CONSTANT");
        break;
    case TP_SYMBOL_STRING_LITERAL:
        fprintf_s(write_file, "TP_SYMBOL_STRING_LITERAL");
        break;
    case TP_SYMBOL_PUNCTUATOR:
        fprintf_s(write_file, "TP_SYMBOL_PUNCTUATOR");
        break;
    case TP_SYMBOL_WHITE_SPACE:
        fprintf_s(write_file, "TP_SYMBOL_WHITE_SPACE");
        break;
    case TP_SYMBOL_LF:
        fprintf_s(write_file, "TP_SYMBOL_LF");
        break;
    case TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER:
        fprintf_s(write_file, "TP_SYMBOL_OTHER_NON_WHITE_SPACE_CHARACTER");
        break;
    case TP_SYMBOL_PLACE_MARKER:
        fprintf_s(write_file, "TP_SYMBOL_PLACE_MARKER");
        break;
    case TP_SYMBOL_KEYWORD:
        fprintf_s(write_file, "TP_SYMBOL_KEYWORD");
        break;
    case TP_SYMBOL_CONSTANT:
        fprintf_s(write_file, "TP_SYMBOL_CONSTANT");
        break;
    default:
        return false;
    }

    return true;
}

bool tp_compare_first_or_follow_set(
    TP_GRAMMER_TERM* test_first_or_follow_set, rsize_t test_first_or_follow_set_num,
    TP_GRAMMER_TERM* first_or_follow_set)
{
    qsort(test_first_or_follow_set, test_first_or_follow_set_num, sizeof(TP_GRAMMER_TERM), compare_grammer_term);

    qsort(first_or_follow_set, test_first_or_follow_set_num, sizeof(TP_GRAMMER_TERM), compare_grammer_term);

    for (rsize_t i = 0; test_first_or_follow_set_num > i; ++i){

        if ((test_first_or_follow_set[i].member_symbol != first_or_follow_set[i].member_symbol) ||
            (0 != strcmp(test_first_or_follow_set[i].member_string, first_or_follow_set[i].member_string))){

            return false;
        }
    }

    return true;
}

static int compare_grammer_term(const void* param1, const void* param2)
{
    TP_GRAMMER_TERM* first_or_follow_set1 = (TP_GRAMMER_TERM*)param1;

    TP_GRAMMER_TERM* first_or_follow_set2 = (TP_GRAMMER_TERM*)param2;

    if (first_or_follow_set1->member_symbol < first_or_follow_set2->member_symbol){

        return -1;
    }

    if (first_or_follow_set1->member_symbol > first_or_follow_set2->member_symbol){

        return 1;
    }

    return strcmp(first_or_follow_set1->member_string, first_or_follow_set2->member_string);
}

