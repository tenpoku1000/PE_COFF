
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "tp_compiler.h"

// Example:
// int32_t value1 = (1 + 2) * 3;
// int32_t value2 = 2 + (3 * value1);
// value1 = value2 + 100;
//
// Grammer:
// Program -> Statement+
// Statement -> Type? variable '=' Expression ';'
// Expression -> Term (('+' | '-') Term)*
// Term -> Factor (('*' | '/') Factor)*
// Factor -> '(' Expression ')' | ('+' | '-')? (variable | constant)
// Type -> int32_t
//
// Note:
//  (1) Nesting level of expression maximum 63.

#define TP_POS(symbol_table) ((symbol_table)->member_tp_token_position)

#define IS_END_OF_TOKEN(token) ((NULL == (token)) || ((token) && (TP_SYMBOL_NULL == (token)->member_symbol)))

#define MAKE_PARSE_SUBTREE(symbol_table, grammer, ...) \
  make_parse_subtree( \
    (symbol_table), (grammer), \
    (TP_PARSE_TREE_ELEMENT[]){ __VA_ARGS__ }, \
    sizeof((TP_PARSE_TREE_ELEMENT[]){ __VA_ARGS__ }) / sizeof(TP_PARSE_TREE_ELEMENT) \
  )
#define TP_TREE_TOKEN(token) (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_TOKEN, \
    .member_body.member_tp_token = (token) \
}
#define TP_TREE_TOKEN_NULL (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_NULL, \
    .member_body.member_tp_token = NULL \
}
#define TP_TREE_NODE(child) (TP_PARSE_TREE_ELEMENT){ \
    .member_type = TP_PARSE_TREE_TYPE_NODE, \
    .member_body.member_child = (child) \
}

static const uint8_t NESTING_LEVEL_OF_EXPRESSION_MAXIMUM = 63;

static TP_PARSE_TREE* make_parse_subtree(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE_GRAMMER grammer,
    TP_PARSE_TREE_ELEMENT* parse_tree_element, size_t parse_tree_element_num
);
static bool dump_parse_tree_open(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool dump_parse_tree_main(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, uint8_t indent_level);
static bool dump_parse_tree_close(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_program(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_term(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_factor(TP_SYMBOL_TABLE* symbol_table);
static bool calc_const_value(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);

bool tp_make_parse_tree(TP_SYMBOL_TABLE* symbol_table)
{
    if (symbol_table->member_is_output_log_file){

        if ( ! dump_parse_tree_open(symbol_table, symbol_table->member_parse_tree_file_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(symbol_table->member_parse_tree_file, "=== Dump parse subtree. ===\n\n");
    }

    TP_PARSE_TREE* tmp_program = parse_program(symbol_table);

    if (tmp_program && IS_END_OF_TOKEN(symbol_table->member_tp_token_position)){

        symbol_table->member_tp_parse_tree = tmp_program;

        if (symbol_table->member_is_output_log_file){

            fprintf(symbol_table->member_parse_tree_file, "=== Dump all parse tree. ===\n\n");

            uint8_t indent_level = 1;

            if ( ! dump_parse_tree_main(symbol_table, symbol_table->member_tp_parse_tree, indent_level)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        if (symbol_table->member_is_output_log_file){

            if ( ! dump_parse_tree_close(symbol_table)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        return true;
    }

    if (symbol_table->member_is_output_log_file){

        fprintf(symbol_table->member_parse_tree_file, "=== Syntax error. It is not dump all parse tree. ===\n\n");

        if ( ! dump_parse_tree_close(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: syntax error.")
    );

    return false;
}

static TP_PARSE_TREE* make_parse_subtree(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE_GRAMMER grammer,
    TP_PARSE_TREE_ELEMENT* parse_tree_element, size_t parse_tree_element_num)
{
    TP_PARSE_TREE* parse_subtree = (TP_PARSE_TREE*)calloc(1, sizeof(TP_PARSE_TREE));

    if (NULL == parse_subtree){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    parse_subtree->member_grammer = grammer;
    parse_subtree->member_element_num = parse_tree_element_num;
    parse_subtree->member_element = (TP_PARSE_TREE_ELEMENT*)calloc(
        parse_tree_element_num + 1, sizeof(TP_PARSE_TREE)
    );

    if (NULL == parse_subtree->member_element){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(symbol_table, &parse_subtree, sizeof(TP_PARSE_TREE));

        return NULL;
    }

    memcpy(
        parse_subtree->member_element, parse_tree_element,
        sizeof(TP_PARSE_TREE_ELEMENT) * parse_tree_element_num
    );
    TP_PARSE_TREE_ELEMENT null = TP_TREE_TOKEN_NULL;
    memcpy(
        parse_subtree->member_element + parse_tree_element_num, &null,
        sizeof(TP_PARSE_TREE_ELEMENT)
    );

    if (symbol_table->member_is_output_log_file){

        uint8_t indent_level = 1;

        if ( ! dump_parse_tree_main(symbol_table, parse_subtree, indent_level)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return NULL;
        }

        fprintf(symbol_table->member_parse_tree_file, "---------------------------\n\n");
    }

    return parse_subtree;
}

void tp_free_parse_subtree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE** parse_subtree)
{
    if (NULL == parse_subtree){

        return;
    }

    if (*parse_subtree){

        size_t element_num = (*parse_subtree)->member_element_num;

        for (size_t i = 0; element_num > i; ++i){

            // NOTE: TP_PARSE_TREE_TYPE_TOKEN must not free memory.

            if (TP_PARSE_TREE_TYPE_NODE == (*parse_subtree)->member_element[i].member_type){

                tp_free_parse_subtree(
                    symbol_table, &((*parse_subtree)->member_element[i].member_body.member_child)
                );
            }
        }

        if ((*parse_subtree)->member_element){

            TP_FREE(symbol_table, &((*parse_subtree)->member_element),
                ((*parse_subtree)->member_element_num + 1) * sizeof(TP_PARSE_TREE));
        }

        TP_FREE(symbol_table, parse_subtree, sizeof(TP_PARSE_TREE));
    }
}

static bool dump_parse_tree_open(TP_SYMBOL_TABLE* symbol_table, char* path)
{
    if ( ! tp_open_write_file(symbol_table, path, &(symbol_table->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool dump_parse_tree_main(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, uint8_t indent_level)
{
    errno_t err = 0;

    if (NULL == parse_tree){

        return true;
    }

    FILE* write_file = symbol_table->member_parse_tree_file;

    if ((UINT8_MAX == indent_level) || (0 == indent_level)){

        fprintf(write_file, "NOTE: Bad indent level(%d).\n", indent_level);
        fprintf(write_file, "NOTE: Stop dump parse tree.\n\n");

        err = _set_errno(0);

        return true;
    }

    TP_MAKE_INDENT_STRING(indent_level);

    fprintf(write_file, "%s{\n", prev_indent_string);

    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_PROGRAM:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_PROGRAM\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_1:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_STATEMENT_1\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_2:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_STATEMENT_2\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_1:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_EXPRESSION_1\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_2:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_EXPRESSION_2\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_1:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_TERM_1\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_2:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_TERM_2\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_1:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_FACTOR_1\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_2:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_FACTOR_2\n", indent_string);
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_3:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER_FACTOR_3\n", indent_string);
        break;
    default:
        fprintf(write_file, "%sTP_PARSE_TREE_GRAMMER(UNKNOWN_GRAMMER: %d)\n",
            indent_string, parse_tree->member_grammer);
        break;
    }

    TP_PARSE_TREE_ELEMENT* parse_tree_element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    for (size_t i = 0; element_num > i; ++i){

        switch (parse_tree_element[i].member_type){
        case TP_PARSE_TREE_TYPE_NULL:
            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_NULL\n", indent_string);
            break;
        case TP_PARSE_TREE_TYPE_TOKEN:

            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_TOKEN\n", indent_string);

            if ( ! tp_dump_token_main(
                symbol_table, write_file,
                parse_tree_element[i].member_body.member_tp_token, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            break;
        case TP_PARSE_TREE_TYPE_NODE:

            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_NODE\n", indent_string);

            if ( ! dump_parse_tree_main(
                symbol_table, (TP_PARSE_TREE*)parse_tree_element[i].member_body.member_child, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            break;
        default:
            fprintf(write_file, "%sTP_PARSE_TREE_TYPE(UNKNOWN_TYPE: %d)\n",
                indent_string, parse_tree_element[i].member_type);
            break;
        }
    }

    fprintf(write_file, "%s}\n\n", prev_indent_string);

    err = _set_errno(0);

    return true;
}

static bool dump_parse_tree_close(TP_SYMBOL_TABLE* symbol_table)
{
    if ( ! tp_close_file(symbol_table, &(symbol_table->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

//      return false;
    }

    return true;
}

static TP_PARSE_TREE* parse_program(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Program -> Statement+
    {
        TP_PARSE_TREE* tmp_statement_1 = NULL;
        TP_PARSE_TREE* tmp_statement_2 = NULL;

        if ( ! (tmp_statement_1 = parse_statement(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip;
        }

        while (tmp_statement_2 = parse_statement(symbol_table)){

            tmp_statement_1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PROGRAM,
                TP_TREE_NODE(tmp_statement_1),
                TP_TREE_NODE(tmp_statement_2)
            );
        }

        return tmp_statement_1;
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_statement(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Statement -> Type? variable '=' Expression ';'
    {
        TP_TOKEN* tmp_type = NULL;
        TP_TOKEN* tmp_variable = NULL;
        TP_TOKEN* tmp_equal = NULL;
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_TOKEN* tmp_semicolon = NULL;

        if (IS_TOKEN_ID(TP_POS(symbol_table))){

            tmp_type = TP_POS(symbol_table)++;

            if (IS_TOKEN_STRING_ID_INT32(tmp_type)){

                tmp_type->member_symbol_type = TP_SYMBOL_TYPE_INT32;

                if ( ! (IS_TOKEN_ID(TP_POS(symbol_table)))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip;
                }

                tmp_variable = TP_POS(symbol_table)++;

                tmp_variable->member_symbol_type = TP_SYMBOL_ID_INT32;

            }else{

                tmp_variable = tmp_type;

                tmp_type = NULL;

                tmp_variable->member_symbol_type = TP_SYMBOL_ID_INT32;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip;
        }

        if ( ! (IS_TOKEN_EQUAL(TP_POS(symbol_table)))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip;
        }

        tmp_equal = TP_POS(symbol_table)++;

        if (tmp_expression = parse_expression(symbol_table)){

            if ( ! (IS_TOKEN_SEMICOLON(TP_POS(symbol_table)))){

                tp_free_parse_subtree(symbol_table, &tmp_expression);

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip;
            }

            tmp_semicolon = TP_POS(symbol_table)++;

            if (NULL == tmp_type){

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STATEMENT_1,
                    TP_TREE_TOKEN(tmp_variable),
                    TP_TREE_TOKEN(tmp_equal),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STATEMENT_2,
                    TP_TREE_TOKEN(tmp_type),
                    TP_TREE_TOKEN(tmp_variable),
                    TP_TREE_TOKEN(tmp_equal),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_expression(TP_SYMBOL_TABLE* symbol_table)
{
    if (NESTING_LEVEL_OF_EXPRESSION_MAXIMUM <=
        symbol_table->member_nesting_level_of_expression){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: %1 over NESTING_LEVEL_OF_EXPRESSION_MAXIMUM(%2) error."),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_nesting_level_of_expression),
            TP_LOG_PARAM_INT32_VALUE(NESTING_LEVEL_OF_EXPRESSION_MAXIMUM)
        );

        return NULL;
    }

    ++(symbol_table->member_nesting_level_of_expression);

    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Expression -> Term (('+' | '-') Term)*
    // Example: ((1 + 2) - 3) + 4)
    {
        TP_PARSE_TREE* tmp_expression_1 = NULL;

        TP_PARSE_TREE* tmp_term_1 = NULL;
        TP_TOKEN* tmp_plus_or_minus = NULL;
        TP_PARSE_TREE* tmp_term_2 = NULL;

        if (tmp_term_1 = parse_term(symbol_table)){

            while (IS_TOKEN_PLUS(TP_POS(symbol_table)) || IS_TOKEN_MINUS(TP_POS(symbol_table))){

                tmp_plus_or_minus = TP_POS(symbol_table)++;

                if ( ! (tmp_term_2 = parse_term(symbol_table))){

                    tp_free_parse_subtree(symbol_table, &tmp_expression_1);
                    tp_free_parse_subtree(symbol_table, &tmp_term_1);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip;
                }

                if (NULL == tmp_expression_1){

                    tmp_expression_1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EXPRESSION_1,
                        TP_TREE_NODE(tmp_term_1),
                        TP_TREE_TOKEN(tmp_plus_or_minus),
                        TP_TREE_NODE(tmp_term_2)
                    );

                    if (NULL == tmp_expression_1){

                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }
                }else{

                    TP_PARSE_TREE* tmp_expression_2 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EXPRESSION_2,
                        TP_TREE_NODE(tmp_expression_1),
                        TP_TREE_TOKEN(tmp_plus_or_minus),
                        TP_TREE_NODE(tmp_term_2)
                    );

                    if (NULL == tmp_expression_2){

                        tp_free_parse_subtree(symbol_table, &tmp_expression_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }

                    tmp_expression_1 = tmp_expression_2;
                }
            }

            if (tmp_expression_1){

                return tmp_expression_1;
            }else{

                return tmp_term_1;
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_term(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Term -> Factor (('*' | '/') Factor)*
    // Example: ((1 * 2) / 3) * 4)
    {
        TP_PARSE_TREE* tmp_term_1 = NULL;

        TP_PARSE_TREE* tmp_factor_1 = NULL;
        TP_TOKEN* tmp_mul_or_div = NULL;
        TP_PARSE_TREE* tmp_factor_2 = NULL;

        if (tmp_factor_1 = parse_factor(symbol_table)){

            while (IS_TOKEN_MUL(TP_POS(symbol_table)) || IS_TOKEN_DIV(TP_POS(symbol_table))){

                tmp_mul_or_div = TP_POS(symbol_table)++;

                if ( ! (tmp_factor_2 = parse_factor(symbol_table))){

                    tp_free_parse_subtree(symbol_table, &tmp_factor_1);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip;
                }

                if (NULL == tmp_term_1){

                    tmp_term_1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_TERM_1,
                        TP_TREE_NODE(tmp_factor_1),
                        TP_TREE_TOKEN(tmp_mul_or_div),
                        TP_TREE_NODE(tmp_factor_2)
                    );

                    if (NULL == tmp_term_1){

                        tp_free_parse_subtree(symbol_table, &tmp_factor_1);
                        tp_free_parse_subtree(symbol_table, &tmp_factor_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }
                }else{

                    TP_PARSE_TREE* tmp_term_2 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_TERM_2,
                        TP_TREE_NODE(tmp_term_1),
                        TP_TREE_TOKEN(tmp_mul_or_div),
                        TP_TREE_NODE(tmp_factor_2)
                    );

                    if (NULL == tmp_term_2){

                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_factor_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }

                    tmp_term_1 =  tmp_term_2;
                }
            }

            if (tmp_term_1){

                return tmp_term_1;
            }else{

                return tmp_factor_1;
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_factor(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: Factor -> '(' Expression ')' | ('+' | '-')? (variable | constant)
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Factor -> '(' Expression ')'
    {
        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_expression = NULL;

            if (tmp_expression = parse_expression(symbol_table)){

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if ( ! IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    tp_free_parse_subtree(symbol_table, &tmp_expression);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_FACTOR_1,
                    TP_TREE_TOKEN(tmp_left_paren),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_right_paren)
                );
            }
        }
skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    // Factor -> ('+' | '-')? (variable | constant)
    {
        if (IS_TOKEN_PLUS(TP_POS(symbol_table)) || IS_TOKEN_MINUS(TP_POS(symbol_table))){

            TP_TOKEN* tmp_plus_or_minus = TP_POS(symbol_table)++;

            bool is_const = IS_TOKEN_CONST_VALUE(TP_POS(symbol_table));

            if ( ! (IS_TOKEN_ID(TP_POS(symbol_table)) || is_const)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }

            TP_TOKEN* tmp_variable_or_constant = TP_POS(symbol_table)++;

            if (is_const){

                if ( ! calc_const_value(symbol_table, tmp_variable_or_constant)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return NULL;
                }
            }else{

                tmp_variable_or_constant->member_symbol_type = TP_SYMBOL_ID_INT32;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FACTOR_2,
                TP_TREE_TOKEN(tmp_plus_or_minus),
                TP_TREE_TOKEN(tmp_variable_or_constant)
            );
        }else if (IS_TOKEN_ID(TP_POS(symbol_table)) || IS_TOKEN_CONST_VALUE(TP_POS(symbol_table))){

            bool is_const = IS_TOKEN_CONST_VALUE(TP_POS(symbol_table));

            TP_TOKEN* tmp_variable_or_constant = TP_POS(symbol_table)++;

            if (is_const){

                if ( ! calc_const_value(symbol_table, tmp_variable_or_constant)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return NULL;
                }
            }else{

                tmp_variable_or_constant->member_symbol_type = TP_SYMBOL_ID_INT32;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FACTOR_3,
                TP_TREE_TOKEN(tmp_variable_or_constant)
            );
        }
skip_2:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static bool calc_const_value(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    char* error_first_char = NULL;

    long value = strtol(token->member_string, &error_first_char, 0);

    if (NULL == error_first_char){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: strtol(%1) convert failed."),
            TP_LOG_PARAM_STRING(token->member_string)
        );

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    token->member_i32_value = value;

    token->member_symbol_type = TP_SYMBOL_CONST_VALUE_INT32;

    return true;
}

