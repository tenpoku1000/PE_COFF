
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// int_calc_compiler

static bool variable_reference_check_grammer_statement_1(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree
);
static bool variable_reference_check_grammer_statement_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree
);
static bool variable_reference_check_grammer_factor_2(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree
);
static bool variable_reference_check_grammer_factor_3(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree
);

bool tp_variable_reference_check_expr(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context)
{
    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_PROGRAM:
        break;
    case TP_PARSE_TREE_GRAMMER_STATEMENT_1:
        return variable_reference_check_grammer_statement_1(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_STATEMENT_2:
        return variable_reference_check_grammer_statement_2(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_1:
        break;
    case TP_PARSE_TREE_GRAMMER_EXPRESSION_2:
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_1:
        break;
    case TP_PARSE_TREE_GRAMMER_TERM_2:
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_1:
        break;
    case TP_PARSE_TREE_GRAMMER_FACTOR_2:
        return variable_reference_check_grammer_factor_2(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_FACTOR_3:
        return variable_reference_check_grammer_factor_3(symbol_table, parse_tree);
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool variable_reference_check_grammer_statement_1(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Statement -> variable '=' Expression ';'

    if (symbol_table->member_grammer_statement_1_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_STATEMENT_1][0]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_1 = parse_tree->member_element[0].member_body.member_tp_token;

    if ( ! (IS_TOKEN_ID(token_1) && IS_TOKEN_TYPE_ID_INT32(token_1))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    REGISTER_OBJECT object = { 0 };

    if ( ! tp_register_undefined_variable(symbol_table, token_1, TP_C_NAMESPACE_NONE, &object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    symbol_table->member_last_statement = parse_tree;

    return true;
}

static bool variable_reference_check_grammer_statement_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Statement -> Type variable '=' Expression ';'

    if (symbol_table->member_grammer_statement_2_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_STATEMENT_2][1]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_2 = parse_tree->member_element[1].member_body.member_tp_token;

    if ( ! (IS_TOKEN_ID(token_2) && IS_TOKEN_TYPE_ID_INT32(token_2))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! tp_register_defined_variable(
        symbol_table, TP_GRAMMER_CONTEXT_NULL, token_2, TP_C_NAMESPACE_OTHER, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    ++(symbol_table->member_var_count);

    symbol_table->member_last_statement = parse_tree;

    return true;
}

static bool variable_reference_check_grammer_factor_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Factor -> ('+' | '-') (variable | constant)

    if (symbol_table->member_grammer_factor_2_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_2][1]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_2 = parse_tree->member_element[1].member_body.member_tp_token;

    if ( ! IS_TOKEN_TYPE_ID_INT32(token_2) || IS_TOKEN_TYPE_CONST_VALUE_INT32(token_2)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (IS_TOKEN_TYPE_CONST_VALUE_INT32(token_2)){ return true; }

    REGISTER_OBJECT object = { 0 };

    return tp_register_undefined_variable(symbol_table, token_2, TP_C_NAMESPACE_NONE, &object);
}

static bool variable_reference_check_grammer_factor_3(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Factor -> variable | constant

    if (symbol_table->member_grammer_factor_3_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_3][0]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_1 = parse_tree->member_element[0].member_body.member_tp_token;

    if ( ! (IS_TOKEN_TYPE_ID_INT32(token_1) ||
            IS_TOKEN_TYPE_CONST_VALUE_INT32(token_1))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (IS_TOKEN_TYPE_CONST_VALUE_INT32(token_1)){ return true; }

    REGISTER_OBJECT object = { 0 };

    return tp_register_undefined_variable(symbol_table, token_1, TP_C_NAMESPACE_NONE, &object);
}

