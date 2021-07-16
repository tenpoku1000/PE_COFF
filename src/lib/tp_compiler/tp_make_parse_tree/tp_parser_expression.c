
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Grammer(Expressions):
//
// expression -> (assignment-expression ,)* assignment-expression
// 
// constant-expression -> conditional-expression
// 
// assignment-expression -> unary-expression assignment-operator assignment-expression |
//     conditional-expression
// 
// assignment-operator: one of
//     = *= /= %= += -= <<= >>= &= ^= |=
// 
// conditional-expression -> logical-OR-expression (? expression : conditional-expression)?
// 
// primary-expression -> identifier |
//     constant | string-literal | ( expression ) | generic-selection
// 
// generic-selection -> _Generic ( assignment-expression , generic-assoc-list )
// 
// generic-assoc-list -> (generic-association ,)* generic-association
// 
// generic-association -> type-name : assignment-expression |
//     default : assignment-expression
// 
// postfix-expression -> primary-expression PostfixExpressionTmp1* |
//     ( type-name ) { initializer-list ,? }
// 
// PostfixExpressionTmp1 -> [ expression ] |
//     ( argument-expression-list? ) |
//     (. | ->) identifier |
//     (++ | --)
// 
// argument-expression-list -> (assignment-expression ,)* assignment-expression
// 
// unary-expression -> postfix-expression |
//     (++ | --) unary-expression |
//     unary-operator cast-expression |
//     sizeof (unary-expression | ( type-name )) |
//     _Alignof ( type-name )
// 
// unary-operator: one of
//     &*+-~!
// 
// cast-expression -> unary-expression |
//     ( type-name ) cast-expression
// 
// multiplicative-expression -> (cast-expression (* | / | %))* cast-expression
// 
// additive-expression -> (multiplicative-expression (+ | -))* multiplicative-expression
// 
// shift-expression -> (additive-expression (<< | >>))* additive-expression
// 
// relational-expression -> (shift-expression (< | > | <= | >=))* shift-expression
// 
// equality-expression -> (relational-expression (== | !=))* relational-expression
// 
// AND-expression -> (equality-expression &)* equality-expression
// 
// exclusive-OR-expression -> (AND-expression ^)* AND-expression
// 
// inclusive-OR-expression -> (exclusive-OR-expression |)* exclusive-OR-expression
// 
// logical-AND-expression -> (inclusive-OR-expression &&)* inclusive-OR-expression
// 
// logical-OR-expression -> (logical-AND-expression ||)* logical-AND-expression

static TP_PARSE_TREE* parse_conditional_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_primary_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_generic_selection(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_generic_assoc_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_generic_association(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_postfix_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_postfix_expression_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_paren
);
static TP_PARSE_TREE* parse_postfix_expression_tmp1(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_argument_expression_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_unary_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_cast_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_multiplicative_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_additive_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_shift_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_relational_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_equality_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_AND_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_exclusive_OR_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_inclusive_OR_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_logical_AND_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_logical_OR_expression(TP_SYMBOL_TABLE* symbol_table);

TP_PARSE_TREE* tp_parse_expression(TP_SYMBOL_TABLE* symbol_table)
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

    // Grammer: expression -> (assignment-expression ,)* assignment-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_assignment_expression1 = NULL;
        TP_PARSE_TREE* tmp_assignment_expression2 = NULL;

        if ( ! (tmp_assignment_expression1 = tp_parse_assignment_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_assignment_expression2 = tp_parse_assignment_expression(symbol_table)){

                    is_single = false;

                    // Grammer: expression -> (assignment-expression ,)+ assignment-expression
                    tmp_assignment_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_C_EXPRESSION_1,
                        TP_TREE_NODE(tmp_assignment_expression1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_assignment_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: expression -> assignment-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2,
                TP_TREE_NODE(tmp_assignment_expression1)
            );
        }

        return tmp_assignment_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_constant_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: constant-expression -> conditional-expression
    {
        TP_PARSE_TREE* tmp_conditional_expression = parse_conditional_expression(symbol_table);

        if (tmp_conditional_expression){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_CONSTANT_EXPRESSION_1,
                TP_TREE_NODE(tmp_conditional_expression)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_assignment_expression(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression |
    //     conditional-expression
    // assignment-operator: one of
    //     = *= /= %= += -= <<= >>= &= ^= |=
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
    // 
    // assignment-operator: one of
    //     = *= /= %= += -= <<= >>= &= ^= |=
    {
        TP_PARSE_TREE* tmp_unary_expression = NULL;
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        if ( ! (tmp_unary_expression = parse_unary_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_EQUAL(tmp_token) || IS_TOKEN_ASTERISK_EQUAL(tmp_token) ||
            IS_TOKEN_SLASH_EQUAL(tmp_token) || IS_TOKEN_PERCENT_EQUAL(tmp_token) ||
            IS_TOKEN_PLUS_EQUAL(tmp_token) || IS_TOKEN_MINUS_EQUAL(tmp_token) ||
            IS_TOKEN_DOUBLE_LESS_THAN_EQUAL(tmp_token) || IS_TOKEN_DOUBLE_GREATER_THAN_EQUAL(tmp_token) ||
            IS_TOKEN_AMPERSAND_EQUAL(tmp_token) || IS_TOKEN_CARET_EQUAL(tmp_token) ||
            IS_TOKEN_VERTICAL_BAR_EQUAL(tmp_token)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_assignment_expression = tp_parse_assignment_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1,
                TP_TREE_NODE(tmp_unary_expression),
                TP_TREE_TOKEN(tmp_token),
                TP_TREE_NODE(tmp_assignment_expression)
            );
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_unary_expression);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: assignment-expression -> conditional-expression
    {
        TP_PARSE_TREE* tmp_conditional_expression = parse_conditional_expression(symbol_table);

        if (tmp_conditional_expression){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2,
                TP_TREE_NODE(tmp_conditional_expression)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_conditional_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: conditional-expression -> logical-OR-expression (? expression : conditional-expression)?
    {
        TP_PARSE_TREE* tmp_logical_OR_expression = NULL;
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_PARSE_TREE* tmp_conditional_expression = NULL;

        if ( ! (tmp_logical_OR_expression = parse_logical_OR_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_question_mark = TP_POS(symbol_table);

        if (IS_TOKEN_QUESTION_MARK(tmp_question_mark)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_expression = tp_parse_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_colon = TP_POS(symbol_table);

            if (IS_TOKEN_COLON(tmp_colon)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_conditional_expression =  parse_conditional_expression(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_CONDITIONAL_EXPRESSION_1,
                    TP_TREE_NODE(tmp_logical_OR_expression),
                    TP_TREE_TOKEN(tmp_question_mark),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_colon),
                    TP_TREE_NODE(tmp_conditional_expression)
                );
            }

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }else{

            return tmp_logical_OR_expression;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_logical_OR_expression);
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_conditional_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_primary_expression(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: primary-expression -> identifier |
    //     constant | string-literal | ( expression ) | generic-selection
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: primary-expression -> identifier
    {
        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1,
                TP_TREE_TOKEN(tmp_identifier)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: primary-expression -> constant
    {
        TP_TOKEN* tmp_constant = TP_POS(symbol_table);

        if (IS_TOKEN_CONSTANT(tmp_constant)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_2,
                TP_TREE_TOKEN(tmp_constant)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: primary-expression -> string-literal
    {
        TP_TOKEN* tmp_string_literal = TP_POS(symbol_table);

        if (IS_TOKEN_STRING_LITERAL(tmp_string_literal)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_3,
                TP_TREE_TOKEN(tmp_string_literal)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: primary-expression -> ( expression )
    {
        TP_PARSE_TREE* tmp_expression = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_expression = tp_parse_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_4,
                    TP_TREE_TOKEN(tmp_left_paren),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_right_paren)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: primary-expression -> generic-selection
    {
        TP_PARSE_TREE* tmp_generic_selection = parse_generic_selection(symbol_table);

        if (tmp_generic_selection){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_5,
                TP_TREE_NODE(tmp_generic_selection)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_generic_selection(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: generic-selection -> _Generic ( assignment-expression , generic-assoc-list )
    {
        TP_PARSE_TREE* tmp_assignment_expression = NULL;
        TP_PARSE_TREE* tmp_generic_assoc_list = NULL;

        TP_TOKEN* tmp_generic = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_GENERIC(tmp_generic)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_assignment_expression = tp_parse_assignment_expression(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                TP_TOKEN* tmp_comma = TP_POS(symbol_table);

                if (IS_TOKEN_COMMA(tmp_comma)){

                    ++TP_POS(symbol_table);

                    tmp_generic_assoc_list = parse_generic_assoc_list(symbol_table);

                    TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                    if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                        ++TP_POS(symbol_table);

                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_GENERIC_SELECTION_1,
                            TP_TREE_TOKEN(tmp_generic),
                            TP_TREE_TOKEN(tmp_left_paren),
                            TP_TREE_NODE(tmp_assignment_expression),
                            TP_TREE_TOKEN(tmp_comma),
                            TP_TREE_NODE(tmp_generic_assoc_list),
                            TP_TREE_TOKEN(tmp_right_paren)
                        );
                    }else{

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip_1;
                    }
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);
        tp_free_parse_subtree(symbol_table, &tmp_generic_assoc_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_generic_assoc_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: generic-assoc-list -> (generic-association ,)* generic-association
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_generic_association1 = NULL;
        TP_PARSE_TREE* tmp_generic_association2 = NULL;

        if ( ! (tmp_generic_association1 = parse_generic_association(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_generic_association2 = parse_generic_association(symbol_table)){

                    is_single = false;

                    // Grammer: generic-assoc-list -> (generic-association ,)+ generic-association
                    tmp_generic_association1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_1,
                        TP_TREE_NODE(tmp_generic_association1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_generic_association2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: generic-assoc-list -> generic-association
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_2,
                TP_TREE_NODE(tmp_generic_association1)
            );
        }

        return tmp_generic_association1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_generic_association1);
        tp_free_parse_subtree(symbol_table, &tmp_generic_association2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_generic_association(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: generic-association -> type-name : assignment-expression |
    //     default : assignment-expression
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: generic-association -> type-name : assignment-expression
    {
        TP_PARSE_TREE* tmp_parse_type_name = NULL;
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        if ( ! (tmp_parse_type_name = tp_parse_type_name(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_colon = TP_POS(symbol_table);

        if (IS_TOKEN_COLON(tmp_colon)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_assignment_expression =  tp_parse_assignment_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_1,
                TP_TREE_NODE(tmp_parse_type_name),
                TP_TREE_TOKEN(tmp_colon),
                TP_TREE_NODE(tmp_assignment_expression)
            );
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_parse_type_name);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: generic-association -> default : assignment-expression
    {
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        TP_TOKEN* tmp_default = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_DEFAULT(tmp_default)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_colon = TP_POS(symbol_table);

            if (IS_TOKEN_COLON(tmp_colon)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_assignment_expression =  tp_parse_assignment_expression(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_2;
                }

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_2,
                    TP_TREE_TOKEN(tmp_default),
                    TP_TREE_TOKEN(tmp_colon),
                    TP_TREE_NODE(tmp_assignment_expression)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }
        }
skip_2:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_postfix_expression(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1* |
    //     ( type-name ) { initializer-list ,? }
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_postfix_expression1 = NULL;
        TP_PARSE_TREE* tmp_postfix_expression2 = NULL;

        if ( ! (tmp_postfix_expression1 = parse_primary_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_postfix_expression2 = parse_postfix_expression_tmp1(symbol_table)){

            is_single = false;

            // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1+
            tmp_postfix_expression1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_1,
                TP_TREE_NODE(tmp_postfix_expression1),
                TP_TREE_NODE(tmp_postfix_expression2)
            );
        }

        if (is_single){

            // Grammer: postfix-expression -> primary-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_2,
                TP_TREE_NODE(tmp_postfix_expression1)
            );
        }

        return tmp_postfix_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_postfix_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_postfix_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: postfix-expression -> ( type-name ) { initializer-list ,? }
    {
        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_type_name = NULL;

            if (tmp_type_name = parse_postfix_expression_paren(symbol_table, tmp_left_paren)){

                return tmp_type_name;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_postfix_expression_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_paren)
{
    // Grammer: postfix-expression -> ( type-name ) { initializer-list ,? }
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_type_name = NULL;
        TP_PARSE_TREE* tmp_initializer_list = NULL;

        if ( ! (tmp_type_name = tp_parse_type_name(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_curly_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_CURLY_BRACKET(tmp_left_curly_bracket)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_initializer_list = tp_parse_initializer_list(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                TP_TOKEN* tmp_comma = TP_POS(symbol_table);

                if (IS_TOKEN_COMMA(tmp_comma)){

                    ++TP_POS(symbol_table);

                    TP_TOKEN* tmp_right_curly_bracket = TP_POS(symbol_table);

                    if (IS_TOKEN_RIGHT_CURLY_BRACKET(tmp_right_curly_bracket)){

                        ++TP_POS(symbol_table);

                        // Grammer: postfix-expression -> ( type-name ) { initializer-list , }
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_3,
                            TP_TREE_TOKEN(tmp_left_paren),
                            TP_TREE_NODE(tmp_type_name),
                            TP_TREE_TOKEN(tmp_right_paren),
                            TP_TREE_TOKEN(tmp_left_curly_bracket),
                            TP_TREE_NODE(tmp_initializer_list),
                            TP_TREE_TOKEN(tmp_comma),
                            TP_TREE_TOKEN(tmp_right_curly_bracket)
                        );
                    }
                }else{

                    TP_TOKEN* tmp_right_curly_bracket = TP_POS(symbol_table);

                    if (IS_TOKEN_RIGHT_CURLY_BRACKET(tmp_right_curly_bracket)){

                        ++TP_POS(symbol_table);

                        // Grammer: postfix-expression -> ( type-name ) { initializer-list }
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_4,
                            TP_TREE_TOKEN(tmp_left_paren),
                            TP_TREE_NODE(tmp_type_name),
                            TP_TREE_TOKEN(tmp_right_paren),
                            TP_TREE_TOKEN(tmp_left_curly_bracket),
                            TP_TREE_NODE(tmp_initializer_list),
                            TP_TREE_TOKEN(tmp_right_curly_bracket)
                        );
                    }
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_type_name);
        tp_free_parse_subtree(symbol_table, &tmp_initializer_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_postfix_expression_tmp1(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: PostfixExpressionTmp1 -> [ expression ] |
    //     ( argument-expression-list? ) |
    //     (. | ->) identifier |
    //     (++ | --)
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_PARSE_TREE* tmp_argument_expression_list = NULL;

        // Grammer: PostfixExpressionTmp1 -> [ expression ]
        TP_TOKEN* tmp_left_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_SQUARE_BRACKET(tmp_left_square_bracket)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_expression = tp_parse_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
             }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }

        // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list? )
        TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

            ++TP_POS(symbol_table);

            tmp_argument_expression_list = parse_argument_expression_list(symbol_table);

            TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                ++TP_POS(symbol_table);

                if (tmp_argument_expression_list){

                    // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list )
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2,
                        TP_TREE_TOKEN(tmp_left_parenthesis),
                        TP_TREE_NODE(tmp_argument_expression_list),
                        TP_TREE_TOKEN(tmp_right_parenthesis)
                    );
                }else{

                    // Grammer: PostfixExpressionTmp1 -> ( )
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_3,
                        TP_TREE_TOKEN(tmp_left_parenthesis),
                        TP_TREE_TOKEN(tmp_right_parenthesis)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }

        // Grammer: PostfixExpressionTmp1 -> (. | ->) identifier
        TP_TOKEN* tmp_period_or_minus_greater_than = TP_POS(symbol_table);

        if (IS_TOKEN_PERIOD(tmp_period_or_minus_greater_than) ||
            IS_TOKEN_MINUS_GREATER_THAN(tmp_period_or_minus_greater_than)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

            if (IS_TOKEN_ID(tmp_identifier)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_4,
                    TP_TREE_TOKEN(tmp_period_or_minus_greater_than),
                    TP_TREE_TOKEN(tmp_identifier)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }

        // Grammer: PostfixExpressionTmp1 -> ++ | --
        TP_TOKEN* tmp_double_plus_or_double_minus = TP_POS(symbol_table);

        if (IS_TOKEN_DOUBLE_PLUS(tmp_double_plus_or_double_minus) ||
            IS_TOKEN_DOUBLE_MINUS(tmp_double_plus_or_double_minus)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_5,
                TP_TREE_TOKEN(tmp_double_plus_or_double_minus)
            );
        }

        TP_PUT_LOG_MSG_TRACE(symbol_table);
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_argument_expression_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_argument_expression_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: argument-expression-list -> (assignment-expression ,)* assignment-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_assignment_expression1 = NULL;
        TP_PARSE_TREE* tmp_assignment_expression2 = NULL;

        if ( ! (tmp_assignment_expression1 = tp_parse_assignment_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_assignment_expression2 = tp_parse_assignment_expression(symbol_table)){

                    is_single = false;

                    // Grammer: argument-expression-list -> (assignment-expression ,)+ assignment-expression
                    tmp_assignment_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_1,
                        TP_TREE_NODE(tmp_assignment_expression1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_assignment_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: argument-expression-list -> assignment-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_2,
                TP_TREE_NODE(tmp_assignment_expression1)
            );
        }

        return tmp_assignment_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_unary_expression(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: unary-expression -> postfix-expression |
    //     (++ | --) unary-expression |
    //     unary-operator cast-expression |
    //     sizeof (unary-expression | ( type-name )) |
    //     _Alignof ( type-name )
    // 
    // unary-operator: one of
    //     &*+-~!
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: unary-expression -> postfix-expression
    {
        TP_PARSE_TREE* tmp_postfix_expression = NULL;

        if (tmp_postfix_expression = parse_postfix_expression(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_1,
                TP_TREE_NODE(tmp_postfix_expression)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: unary-expression -> (++ | --) unary-expression
    {
        TP_PARSE_TREE* tmp_unary_expression = NULL;

        TP_TOKEN* tmp_double_plus_or_double_minus = TP_POS(symbol_table);

        if (IS_TOKEN_DOUBLE_PLUS(tmp_double_plus_or_double_minus) ||
            IS_TOKEN_DOUBLE_MINUS(tmp_double_plus_or_double_minus)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_unary_expression = parse_unary_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_2,
                TP_TREE_TOKEN(tmp_double_plus_or_double_minus),
                TP_TREE_NODE(tmp_unary_expression)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_unary_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: unary-expression -> unary-operator cast-expression
    // 
    // unary-operator: one of
    //     &*+-~!
    {
        TP_PARSE_TREE* tmp_cast_expression = NULL;

        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_AMPERSAND(tmp_token) || IS_TOKEN_ASTERISK(tmp_token) ||
            IS_TOKEN_PLUS(tmp_token) || IS_TOKEN_MINUS(tmp_token) ||
            IS_TOKEN_TILDE(tmp_token) || IS_TOKEN_EXCLAMATION_MARK(tmp_token)){

            tmp_cast_expression = parse_cast_expression(symbol_table);

            if (tmp_cast_expression){

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_3,
                    TP_TREE_TOKEN(tmp_token),
                    TP_TREE_NODE(tmp_cast_expression)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }
        }
skip_2:
        tp_free_parse_subtree(symbol_table, &tmp_cast_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: unary-expression -> sizeof (unary-expression | ( type-name ))
    {
        TP_PARSE_TREE* tmp_unary_expression = NULL;
        TP_PARSE_TREE* tmp_type_name = NULL;

        TP_TOKEN* tmp_sizeof = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_SIZEOF(tmp_sizeof)){

            ++TP_POS(symbol_table);

            tmp_unary_expression = parse_unary_expression(symbol_table);

            if (tmp_unary_expression){

                // Grammer: unary-expression -> sizeof unary-expression
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_4,
                    TP_TREE_TOKEN(tmp_sizeof),
                    TP_TREE_NODE(tmp_unary_expression),
                );
            }

            TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_type_name = tp_parse_type_name(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_3;
                }

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    // Grammer: unary-expression -> sizeof ( type-name )
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_5,
                        TP_TREE_TOKEN(tmp_sizeof),
                        TP_TREE_TOKEN(tmp_left_paren),
                        TP_TREE_NODE(tmp_type_name),
                        TP_TREE_TOKEN(tmp_right_paren)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_3;
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_3;
            }
        }
skip_3:
        tp_free_parse_subtree(symbol_table, &tmp_unary_expression);
        tp_free_parse_subtree(symbol_table, &tmp_type_name);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: unary-expression -> _Alignof ( type-name )
    {
        TP_PARSE_TREE* tmp_type_name = NULL;

        TP_TOKEN* tmp_alignof = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_ALIGNOF(tmp_alignof)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_type_name = tp_parse_type_name(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_4;
                }

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_6,
                        TP_TREE_TOKEN(tmp_alignof),
                        TP_TREE_TOKEN(tmp_left_paren),
                        TP_TREE_NODE(tmp_type_name),
                        TP_TREE_TOKEN(tmp_right_paren)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_4;
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_4;
            }
        }
skip_4:
        tp_free_parse_subtree(symbol_table, &tmp_type_name);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_cast_expression(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: cast-expression -> unary-expression |
    //     ( type-name ) cast-expression
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: cast-expression -> unary-expression
    {
        TP_PARSE_TREE* tmp_unary_expression = NULL;

        if (tmp_unary_expression = parse_unary_expression(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_1,
                TP_TREE_NODE(tmp_unary_expression)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: cast-expression -> ( type-name ) cast-expression
    {
        TP_PARSE_TREE* tmp_type_name = NULL;
        TP_PARSE_TREE* tmp_cast_expression = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_type_name = tp_parse_type_name(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_cast_expression = parse_cast_expression(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_2,
                    TP_TREE_TOKEN(tmp_left_paren),
                    TP_TREE_NODE(tmp_type_name),
                    TP_TREE_TOKEN(tmp_right_paren),
                    TP_TREE_NODE(tmp_cast_expression)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_type_name);
        tp_free_parse_subtree(symbol_table, &tmp_cast_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_multiplicative_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: multiplicative-expression -> (cast-expression (* | / | %))* cast-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_cast_expression1 = NULL;
        TP_PARSE_TREE* tmp_cast_expression2 = NULL;

        if ( ! (tmp_cast_expression1 = parse_cast_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_asterisk_or_slash_or_percent = TP_POS(symbol_table);

            if (IS_TOKEN_ASTERISK(tmp_asterisk_or_slash_or_percent) ||
                IS_TOKEN_SLASH(tmp_asterisk_or_slash_or_percent) ||
                IS_TOKEN_PERCENT(tmp_asterisk_or_slash_or_percent)){

                ++TP_POS(symbol_table);

                if (tmp_cast_expression2 = parse_cast_expression(symbol_table)){

                    is_single = false;

                    // Grammer: multiplicative-expression -> (cast-expression (* | / | %))+ cast-expression
                    tmp_cast_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_1,
                        TP_TREE_NODE(tmp_cast_expression1),
                        TP_TREE_TOKEN(tmp_asterisk_or_slash_or_percent),
                        TP_TREE_NODE(tmp_cast_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: multiplicative-expression -> cast-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_2,
                TP_TREE_NODE(tmp_cast_expression1)
            );
        }

        return tmp_cast_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_cast_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_cast_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_additive_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: additive-expression -> (multiplicative-expression (+ | -))* multiplicative-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_multiplicative_expression1 = NULL;
        TP_PARSE_TREE* tmp_multiplicative_expression2 = NULL;

        if ( ! (tmp_multiplicative_expression1 = parse_multiplicative_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_plus_or_minus = TP_POS(symbol_table);

            if (IS_TOKEN_PLUS(tmp_plus_or_minus) ||
                IS_TOKEN_MINUS(tmp_plus_or_minus)){

                ++TP_POS(symbol_table);

                if (tmp_multiplicative_expression2 = parse_multiplicative_expression(symbol_table)){

                    is_single = false;

                    // Grammer: additive-expression -> (multiplicative-expression (+ | -))+ multiplicative-expression
                    tmp_multiplicative_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_1,
                        TP_TREE_NODE(tmp_multiplicative_expression1),
                        TP_TREE_TOKEN(tmp_plus_or_minus),
                        TP_TREE_NODE(tmp_multiplicative_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: additive-expression -> multiplicative-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_2,
                TP_TREE_NODE(tmp_multiplicative_expression1)
            );
        }

        return tmp_multiplicative_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_multiplicative_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_multiplicative_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_shift_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: shift-expression -> (additive-expression (<< | >>))* additive-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_additive_expression1 = NULL;
        TP_PARSE_TREE* tmp_additive_expression2 = NULL;

        if ( ! (tmp_additive_expression1 = parse_additive_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_double_less_than_or_double_greater_than = TP_POS(symbol_table);

            if (IS_TOKEN_DOUBLE_LESS_THAN(tmp_double_less_than_or_double_greater_than) ||
                IS_TOKEN_DOUBLE_GREATER_THAN(tmp_double_less_than_or_double_greater_than)){

                ++TP_POS(symbol_table);

                if (tmp_additive_expression2 = parse_additive_expression(symbol_table)){

                    is_single = false;

                    // Grammer: shift-expression -> (additive-expression (<< | >>))+ additive-expression
                    tmp_additive_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_1,
                        TP_TREE_NODE(tmp_additive_expression1),
                        TP_TREE_TOKEN(tmp_double_less_than_or_double_greater_than),
                        TP_TREE_NODE(tmp_additive_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: shift-expression -> additive-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_2,
                TP_TREE_NODE(tmp_additive_expression1)
            );
        }

        return tmp_additive_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_additive_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_additive_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_relational_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: relational-expression -> (shift-expression (< | > | <= | >=))* shift-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_shift_expression1 = NULL;
        TP_PARSE_TREE* tmp_shift_expression2 = NULL;

        if ( ! (tmp_shift_expression1 = parse_shift_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_token = TP_POS(symbol_table);

            if (IS_TOKEN_LESS_THAN(tmp_token) ||
                IS_TOKEN_GREATER_THAN(tmp_token) ||
                IS_TOKEN_LESS_THAN_EQUAL(tmp_token) ||
                IS_TOKEN_GREATER_THAN_EQUAL(tmp_token)){

                ++TP_POS(symbol_table);

                if (tmp_shift_expression2 = parse_shift_expression(symbol_table)){

                    is_single = false;

                    // Grammer: relational-expression -> (shift-expression (< | > | <= | >=))+ shift-expression
                    tmp_shift_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_1,
                        TP_TREE_NODE(tmp_shift_expression1),
                        TP_TREE_TOKEN(tmp_token),
                        TP_TREE_NODE(tmp_shift_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: relational-expression -> shift-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_2,
                TP_TREE_NODE(tmp_shift_expression1)
            );
        }

        return tmp_shift_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_shift_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_shift_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_equality_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: equality-expression -> (relational-expression (== | !=))* relational-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_relational_expression1 = NULL;
        TP_PARSE_TREE* tmp_relational_expression2 = NULL;

        if ( ! (tmp_relational_expression1 = parse_relational_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_double_equal_or_exclamation_mark_equal = TP_POS(symbol_table);

            if (IS_TOKEN_DOUBLE_EQUAL(tmp_double_equal_or_exclamation_mark_equal) ||
                IS_TOKEN_EXCLAMATION_MARK_EQUAL(tmp_double_equal_or_exclamation_mark_equal)){

                ++TP_POS(symbol_table);

                if (tmp_relational_expression2 = parse_relational_expression(symbol_table)){

                    is_single = false;

                    // Grammer: equality-expression -> (relational-expression (== | !=))+ relational-expression
                    tmp_relational_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_1,
                        TP_TREE_NODE(tmp_relational_expression1),
                        TP_TREE_TOKEN(tmp_double_equal_or_exclamation_mark_equal),
                        TP_TREE_NODE(tmp_relational_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: equality-expression -> relational-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_2,
                TP_TREE_NODE(tmp_relational_expression1)
            );
        }

        return tmp_relational_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_relational_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_relational_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_AND_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: AND-expression -> (equality-expression &)* equality-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_equality_expression1 = NULL;
        TP_PARSE_TREE* tmp_equality_expression2 = NULL;

        if ( ! (tmp_equality_expression1 = parse_equality_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_ampersand = TP_POS(symbol_table);

            if (IS_TOKEN_AMPERSAND(tmp_ampersand)){

                ++TP_POS(symbol_table);

                if (tmp_equality_expression2 = parse_equality_expression(symbol_table)){

                    is_single = false;

                    // Grammer: AND-expression -> (equality-expression &)+ equality-expression
                    tmp_equality_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_1,
                        TP_TREE_NODE(tmp_equality_expression1),
                        TP_TREE_TOKEN(tmp_ampersand),
                        TP_TREE_NODE(tmp_equality_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: AND-expression -> equality-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_2,
                TP_TREE_NODE(tmp_equality_expression1)
            );
        }

        return tmp_equality_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_equality_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_equality_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_exclusive_OR_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: exclusive-OR-expression -> (AND-expression ^)* AND-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_AND_expression1 = NULL;
        TP_PARSE_TREE* tmp_AND_expression2 = NULL;

        if ( ! (tmp_AND_expression1 = parse_AND_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_caret = TP_POS(symbol_table);

            if (IS_TOKEN_CARET(tmp_caret)){

                ++TP_POS(symbol_table);

                if (tmp_AND_expression2 = parse_AND_expression(symbol_table)){

                    is_single = false;

                    // Grammer: exclusive-OR-expression -> (AND-expression ^)+ AND-expression
                    tmp_AND_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_1,
                        TP_TREE_NODE(tmp_AND_expression1),
                        TP_TREE_TOKEN(tmp_caret),
                        TP_TREE_NODE(tmp_AND_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: exclusive-OR-expression -> AND-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_2,
                TP_TREE_NODE(tmp_AND_expression1)
            );
        }

        return tmp_AND_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_AND_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_AND_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_inclusive_OR_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: inclusive-OR-expression -> (exclusive-OR-expression |)* exclusive-OR-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_exclusive_OR_expression1 = NULL;
        TP_PARSE_TREE* tmp_exclusive_OR_expression2 = NULL;

        if ( ! (tmp_exclusive_OR_expression1 = parse_exclusive_OR_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_vertical_bar = TP_POS(symbol_table);

            if (IS_TOKEN_VERTICAL_BAR(tmp_vertical_bar)){

                ++TP_POS(symbol_table);

                if (tmp_exclusive_OR_expression2 = parse_exclusive_OR_expression(symbol_table)){

                    is_single = false;

                    // Grammer: inclusive-OR-expression -> (exclusive-OR-expression |)+ exclusive-OR-expression
                    tmp_exclusive_OR_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_1,
                        TP_TREE_NODE(tmp_exclusive_OR_expression1),
                        TP_TREE_TOKEN(tmp_vertical_bar),
                        TP_TREE_NODE(tmp_exclusive_OR_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: inclusive-OR-expression -> exclusive-OR-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_2,
                TP_TREE_NODE(tmp_exclusive_OR_expression1)
            );
        }

        return tmp_exclusive_OR_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_exclusive_OR_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_exclusive_OR_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_logical_AND_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: logical-AND-expression -> (inclusive-OR-expression &&)* inclusive-OR-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_inclusive_OR_expression1 = NULL;
        TP_PARSE_TREE* tmp_inclusive_OR_expression2 = NULL;

        if ( ! (tmp_inclusive_OR_expression1 = parse_inclusive_OR_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_double_ampersand = TP_POS(symbol_table);

            if (IS_TOKEN_DOUBLE_AMPERSAND(tmp_double_ampersand)){

                ++TP_POS(symbol_table);

                if (tmp_inclusive_OR_expression2 = parse_inclusive_OR_expression(symbol_table)){

                    is_single = false;

                    // Grammer: logical-AND-expression -> (inclusive-OR-expression &&)+ inclusive-OR-expression
                    tmp_inclusive_OR_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_1,
                        TP_TREE_NODE(tmp_inclusive_OR_expression1),
                        TP_TREE_TOKEN(tmp_double_ampersand),
                        TP_TREE_NODE(tmp_inclusive_OR_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: logical-AND-expression -> inclusive-OR-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_2,
                TP_TREE_NODE(tmp_inclusive_OR_expression1)
            );
        }

        return tmp_inclusive_OR_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_inclusive_OR_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_inclusive_OR_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_logical_OR_expression(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: logical-OR-expression -> (logical-AND-expression ||)* logical-AND-expression
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_logical_AND_expression1 = NULL;
        TP_PARSE_TREE* tmp_logical_AND_expression2 = NULL;

        if ( ! (tmp_logical_AND_expression1 = parse_logical_AND_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_double_vertical_bar = TP_POS(symbol_table);

            if (IS_TOKEN_DOUBLE_VERTICAL_BAR(tmp_double_vertical_bar)){

                ++TP_POS(symbol_table);

                if (tmp_logical_AND_expression2 = parse_logical_AND_expression(symbol_table)){

                    is_single = false;

                    // Grammer: logical-OR-expression -> (logical-AND-expression ||)+ logical-AND-expression
                    tmp_logical_AND_expression1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_1,
                        TP_TREE_NODE(tmp_logical_AND_expression1),
                        TP_TREE_TOKEN(tmp_double_vertical_bar),
                        TP_TREE_NODE(tmp_logical_AND_expression2)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                break;
            }
        }

        if (is_single){

            // Grammer: logical-OR-expression -> logical-AND-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_2,
                TP_TREE_NODE(tmp_logical_AND_expression1)
            );
        }

        return tmp_logical_AND_expression1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_logical_AND_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_logical_AND_expression2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

