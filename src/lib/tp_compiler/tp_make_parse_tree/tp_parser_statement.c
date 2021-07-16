
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Grammer(Statements):
//
// statement -> labeled-statement |
//     compound-statement | expression-statement | selection-statement |
//     iteration-statement | jump-statement
// 
// labeled-statement -> identifier : statement |
//     case constant-expression : statement |
//     default : statement
// 
// compound-statement -> { block-item-list? }
// 
// block-item-list ->  block-item+
// 
// block-item -> declaration | statement
// 
// expression-statement -> expression? ;
// 
// selection-statement -> 
//     if ( expression ) statement (else statement)? |
//     switch ( expression ) statement
// 
// iteration-statement -> 
//     while ( expression ) statement |
//     do statement while ( expression ) ; |
//     for ( expression? ; expression? ; expression? ) statement |
//     for ( declaration expression? ; expression? ) statement
// 
// jump-statement -> 
//     goto identifier ; |
//     continue ; |
//     break ; |
//     return expression? ;

static TP_PARSE_TREE* parse_labeled_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_block_item_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_block_item(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_expression_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_selection_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_selection_statement_if(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_if
);
static TP_PARSE_TREE* parse_selection_statement_switch(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_switch
);
static TP_PARSE_TREE* parse_iteration_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_iteration_statement_while(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_while);
static TP_PARSE_TREE* parse_iteration_statement_do(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_do);
static TP_PARSE_TREE* parse_iteration_statement_for(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_for);
static TP_PARSE_TREE* parse_jump_statement(TP_SYMBOL_TABLE* symbol_table);

TP_PARSE_TREE* tp_parse_statement(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: statement -> labeled-statement |
    //     compound-statement | expression-statement | selection-statement |
    //     iteration-statement | jump-statement
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: statement -> labeled-statement
    {
        TP_PARSE_TREE* tmp_labeled_statement = parse_labeled_statement(symbol_table);

        if (tmp_labeled_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_1,
                TP_TREE_NODE(tmp_labeled_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: statement -> compound-statement
    {
        TP_PARSE_TREE* tmp_compound_statement = tp_parse_compound_statement(symbol_table);

        if (tmp_compound_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_2,
                TP_TREE_NODE(tmp_compound_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: statement -> expression-statement
    {
        TP_PARSE_TREE* tmp_expression_statement = parse_expression_statement(symbol_table);

        if (tmp_expression_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_3,
                TP_TREE_NODE(tmp_expression_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: statement -> selection-statement
    {
        TP_PARSE_TREE* tmp_selection_statement = parse_selection_statement(symbol_table);

        if (tmp_selection_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_4,
                TP_TREE_NODE(tmp_selection_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: statement -> iteration-statement
    {
        TP_PARSE_TREE* tmp_iteration_statement = parse_iteration_statement(symbol_table);

        if (tmp_iteration_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_5,
                TP_TREE_NODE(tmp_iteration_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: statement -> jump-statement
    {
        TP_PARSE_TREE* tmp_jump_statement = parse_jump_statement(symbol_table);

        if (tmp_jump_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_C_STATEMENT_6,
                TP_TREE_NODE(tmp_jump_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_compound_statement(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: compound-statement -> { block-item-list? }
    {
        TP_PARSE_TREE* tmp_block_item_list = NULL;

        TP_TOKEN* tmp_left_curly_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_CURLY_BRACKET(tmp_left_curly_bracket)){

            ++TP_POS(symbol_table);

            tmp_block_item_list = parse_block_item_list(symbol_table);

            TP_TOKEN* tmp_right_curly_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_CURLY_BRACKET(tmp_right_curly_bracket)){

                ++TP_POS(symbol_table);

                if (tmp_block_item_list){

                    // Grammer: compound-statement -> { block-item-list }
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1,
                        TP_TREE_TOKEN(tmp_left_curly_bracket),
                        TP_TREE_NODE(tmp_block_item_list),
                        TP_TREE_TOKEN(tmp_right_curly_bracket)
                    );
                }else{

                    // Grammer: compound-statement -> { }
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2,
                        TP_TREE_TOKEN(tmp_left_curly_bracket),
                        TP_TREE_TOKEN(tmp_right_curly_bracket)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_block_item_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_labeled_statement(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: labeled-statement -> identifier : statement |
    //     case constant-expression : statement |
    //     default : statement
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: labeled-statement -> identifier : statement
    {
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_colon = TP_POS(symbol_table);

            if (IS_TOKEN_COLON(tmp_colon)){

                ++TP_POS(symbol_table);

                tmp_statement = tp_parse_statement(symbol_table);

                if (tmp_statement){

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_1,
                        TP_TREE_TOKEN(tmp_identifier),
                        TP_TREE_TOKEN(tmp_colon),
                        TP_TREE_NODE(tmp_statement)
                    );
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
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: labeled-statement -> case constant-expression : statement
    {
        TP_PARSE_TREE* tmp_constant_expression = NULL;
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_case = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_CASE(tmp_case)){

            ++TP_POS(symbol_table);

            if (tmp_constant_expression = tp_parse_constant_expression(symbol_table)){

                TP_TOKEN* tmp_colon = TP_POS(symbol_table);

                if (IS_TOKEN_COLON(tmp_colon)){

                    ++TP_POS(symbol_table);

                    if (tmp_statement = tp_parse_statement(symbol_table)){

                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_2,
                            TP_TREE_TOKEN(tmp_case),
                            TP_TREE_NODE(tmp_constant_expression),
                            TP_TREE_TOKEN(tmp_colon),
                            TP_TREE_NODE(tmp_statement)
                        );
                    }else{

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip_2;
                    }
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_2;
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }
        }
skip_2:
        tp_free_parse_subtree(symbol_table, &tmp_constant_expression);
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: labeled-statement -> default : statement
    {
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_default = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_DEFAULT(tmp_default)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_colon = TP_POS(symbol_table);

            if (IS_TOKEN_COLON(tmp_colon)){

                ++TP_POS(symbol_table);

                if (tmp_statement = tp_parse_statement(symbol_table)){

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_3,
                        TP_TREE_TOKEN(tmp_default),
                        TP_TREE_TOKEN(tmp_colon),
                        TP_TREE_NODE(tmp_statement)
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
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_block_item_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: block-item-list ->  block-item+
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_block_item1 = NULL;
        TP_PARSE_TREE* tmp_block_item2 = NULL;

        if ( ! (tmp_block_item1 = parse_block_item(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_block_item2 = parse_block_item(symbol_table)){

            is_single = false;

            // Grammer: block-item-list -> block-item block-item+
            tmp_block_item1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1,
                TP_TREE_NODE(tmp_block_item1),
                TP_TREE_NODE(tmp_block_item2)
            );
        }

        if (is_single){

            // Grammer: block-item-list -> block-item
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_2,
                TP_TREE_NODE(tmp_block_item1)
            );
        }

        return tmp_block_item1;
skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_block_item(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: block-item -> declaration | statement
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: block-item -> declaration
    {
        TP_PARSE_TREE* tmp_declaration = tp_parse_declaration(symbol_table);

        if (tmp_declaration){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1,
                TP_TREE_NODE(tmp_declaration)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: block-item -> statement
    {
        TP_PARSE_TREE* tmp_statement = tp_parse_statement(symbol_table);

        if (tmp_statement){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2,
                TP_TREE_NODE(tmp_statement)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_expression_statement(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: expression-statement -> expression? ;
    {
        TP_PARSE_TREE* tmp_expression = tp_parse_expression(symbol_table);

        TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

        if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

            ++TP_POS(symbol_table);

            if (tmp_expression){

                // Grammer: expression-statement -> expression ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_1,
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                // Grammer: expression-statement -> ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_2,
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_selection_statement(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: selection-statement -> 
    //     if ( expression ) statement (else statement)? |
    //     switch ( expression ) statement
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: if ( expression ) statement (else statement)?
    {
        TP_TOKEN* tmp_if = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_IF(tmp_if)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_selection_statement_if
                = parse_selection_statement_if(symbol_table, tmp_if);

            if (tmp_selection_statement_if){

                return tmp_selection_statement_if;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: selection-statement -> switch ( expression ) statement
    {
        TP_TOKEN* tmp_switch = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_SWITCH(tmp_switch)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_selection_statement_switch
                = parse_selection_statement_switch(symbol_table, tmp_switch);

            if (tmp_selection_statement_switch){

                return tmp_selection_statement_switch;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_selection_statement_if(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_if)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: if ( expression ) statement (else statement)?
    {
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_PARSE_TREE* tmp_statement1 = NULL;
        TP_PARSE_TREE* tmp_statement2 = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            tmp_expression = tp_parse_expression(symbol_table);

            if (tmp_expression){

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    tmp_statement1 = tp_parse_statement(symbol_table);

                    if (tmp_statement1){

                        TP_TOKEN* tmp_else = TP_POS(symbol_table);

                        if (IS_TOKEN_KEYWORD_ELSE(tmp_else)){

                            ++TP_POS(symbol_table);

                            tmp_statement2 = tp_parse_statement(symbol_table);

                            if (tmp_statement2){

                                // Grammer: if ( expression ) statement else statement
                                return MAKE_PARSE_SUBTREE(
                                    symbol_table,
                                    TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_1,
                                    TP_TREE_TOKEN(tmp_if),
                                    TP_TREE_TOKEN(tmp_left_paren),
                                    TP_TREE_NODE(tmp_expression),
                                    TP_TREE_TOKEN(tmp_right_paren),
                                    TP_TREE_NODE(tmp_statement1),
                                    TP_TREE_TOKEN(tmp_else),
                                    TP_TREE_NODE(tmp_statement2)
                                );
                            }else{

                                goto skip_1;
                            }
                        }else{

                            // Grammer: if ( expression ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_2,
                                TP_TREE_TOKEN(tmp_if),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_NODE(tmp_expression),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement1)
                            );
                        }
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
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_statement1);
        tp_free_parse_subtree(symbol_table, &tmp_statement2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_selection_statement_switch(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_switch)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: selection-statement -> switch ( expression ) statement
    {
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            tmp_expression = tp_parse_expression(symbol_table);

            if (tmp_expression){

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    tmp_statement = tp_parse_statement(symbol_table);

                    if (tmp_statement){

                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_SWITCH_1,
                            TP_TREE_TOKEN(tmp_switch),
                            TP_TREE_TOKEN(tmp_left_paren),
                            TP_TREE_NODE(tmp_expression),
                            TP_TREE_TOKEN(tmp_right_paren),
                            TP_TREE_NODE(tmp_statement)
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
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_iteration_statement(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: iteration-statement -> 
    //     while ( expression ) statement |
    //     do statement while ( expression ) ; |
    //     for ( expression? ; expression? ; expression? ) statement |
    //     for ( declaration expression? ; expression? ) statement
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: iteration-statement -> while ( expression ) statement
    {
        TP_TOKEN* tmp_while = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_WHILE(tmp_while)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_iteration_statement_while
                = parse_iteration_statement_while(symbol_table, tmp_while);

            if (tmp_iteration_statement_while){

                return tmp_iteration_statement_while;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: iteration-statement -> do statement while ( expression ) ;
    {
        TP_TOKEN* tmp_do = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_DO(tmp_do)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_iteration_statement_do
                = parse_iteration_statement_while(symbol_table, tmp_do);

            if (tmp_iteration_statement_do){

                return tmp_iteration_statement_do;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: iteration-statement -> for ( expression? ; expression? ; expression? ) statement
    // Grammer: iteration-statement -> for ( declaration expression? ; expression? ) statement
    {
        TP_TOKEN* tmp_for = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_FOR(tmp_for)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_iteration_statement_for
                = parse_iteration_statement_while(symbol_table, tmp_for);

            if (tmp_iteration_statement_for){

                return tmp_iteration_statement_for;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_iteration_statement_while(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_while)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: iteration-statement -> while ( expression ) statement
    {
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            tmp_expression = tp_parse_expression(symbol_table);

            if (tmp_expression){

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    tmp_statement = tp_parse_statement(symbol_table);

                    if (tmp_statement){

                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_WHILE_1,
                            TP_TREE_TOKEN(tmp_while),
                            TP_TREE_TOKEN(tmp_left_paren),
                            TP_TREE_NODE(tmp_expression),
                            TP_TREE_TOKEN(tmp_right_paren),
                            TP_TREE_NODE(tmp_statement)
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
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_iteration_statement_do(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_do)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: iteration-statement -> do statement while ( expression ) ;
    {
        TP_PARSE_TREE* tmp_statement = NULL;
        TP_PARSE_TREE* tmp_expression = NULL;

        if (tmp_statement = tp_parse_statement(symbol_table)){

            TP_TOKEN* tmp_while = TP_POS(symbol_table);

            if (IS_TOKEN_KEYWORD_WHILE(tmp_while)){

                ++TP_POS(symbol_table);

                TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

                if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

                    ++TP_POS(symbol_table);

                    tmp_expression = tp_parse_expression(symbol_table);

                    if (tmp_expression){

                        TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                        if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                            ++TP_POS(symbol_table);

                            TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

                            if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                                ++TP_POS(symbol_table);

                                return MAKE_PARSE_SUBTREE(
                                    symbol_table,
                                    TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1,
                                    TP_TREE_TOKEN(tmp_do),
                                    TP_TREE_NODE(tmp_statement),
                                    TP_TREE_TOKEN(tmp_while),
                                    TP_TREE_TOKEN(tmp_left_paren),
                                    TP_TREE_NODE(tmp_expression),
                                    TP_TREE_TOKEN(tmp_right_paren),
                                    TP_TREE_TOKEN(tmp_semicolon)
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
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression);
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_iteration_statement_for(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_for)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: iteration-statement -> for ( expression? ; expression? ; expression? ) statement
    // Grammer: iteration-statement -> for ( declaration expression? ; expression? ) statement
    {
        TP_PARSE_TREE* tmp_expression1 = NULL;
        TP_TOKEN* tmp_semicolon1 = NULL;
        TP_PARSE_TREE* tmp_expression2 = NULL;
        TP_TOKEN* tmp_semicolon2 = NULL;
        TP_PARSE_TREE* tmp_expression3 = NULL;
        TP_PARSE_TREE* tmp_statement = NULL;

        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_declaration = tp_parse_declaration(symbol_table);

            if (tmp_declaration){

                tmp_expression1 = tmp_declaration;
            }else{

                tmp_expression1 = tp_parse_expression(symbol_table);

                tmp_semicolon1 = TP_POS(symbol_table);

                if (IS_TOKEN_SEMICOLON(tmp_semicolon1)){

                    ++TP_POS(symbol_table);
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }

            tmp_expression2 = tp_parse_expression(symbol_table);

            tmp_semicolon2 = TP_POS(symbol_table);

            if (IS_TOKEN_SEMICOLON(tmp_semicolon2)){

                ++TP_POS(symbol_table);

                tmp_expression3 = tp_parse_expression(symbol_table);

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    ++TP_POS(symbol_table);

                    if (tmp_statement = tp_parse_statement(symbol_table)){

                        if (tmp_expression1 && tmp_expression2 && tmp_expression3){

                            // Grammer: iteration-statement -> for ( declaration expression ; expression ) statement
                            // Grammer: iteration-statement -> for ( expression ; expression ; expression ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                tmp_declaration ? TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_1
                                    : TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_2,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_NODE(tmp_expression1),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_NODE(tmp_expression2),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_NODE(tmp_expression3),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if ((NULL == tmp_expression1) && tmp_expression2 && tmp_expression3){

                            // Grammer: iteration-statement -> for ( ; expression ; expression ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_3,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_NODE(tmp_expression2),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_NODE(tmp_expression3),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if (tmp_expression1 && (NULL == tmp_expression2) && tmp_expression3){

                            // Grammer: iteration-statement -> for ( declaration ; expression ) statement
                            // Grammer: iteration-statement -> for ( expression ; ; expression ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                tmp_declaration ? TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_4
                                    : TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_5,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_NODE(tmp_expression1),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_NODE(tmp_expression3),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if (tmp_expression1 && tmp_expression2 && (NULL == tmp_expression3)){

                            // Grammer: iteration-statement -> for ( declaration expression ; ) statement
                            // Grammer: iteration-statement -> for ( expression ; expression ; ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                tmp_declaration ? TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_6
                                    : TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_7,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_NODE(tmp_expression1),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_NODE(tmp_expression2),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if ((NULL == tmp_expression1) && (NULL == tmp_expression2) && tmp_expression3){

                            // Grammer: iteration-statement -> for ( ; ; expression ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_8,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_NODE(tmp_expression3),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if ((NULL == tmp_expression1) && tmp_expression2 && (NULL == tmp_expression3)){

                            // Grammer: iteration-statement -> for ( ; expression ; ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_9,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_NODE(tmp_expression2),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if (tmp_expression1 && (NULL == tmp_expression2) && (NULL == tmp_expression3)){

                            // Grammer: iteration-statement -> for ( declaration ; ) statement
                            // Grammer: iteration-statement -> for ( expression ; ; ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                tmp_declaration ? TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_10
                                    : TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_11,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_NODE(tmp_expression1),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        if ((NULL == tmp_expression1) &&
                            (NULL == tmp_expression2) && (NULL == tmp_expression3)){

                            // Grammer: iteration-statement -> for ( ; ; ) statement
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_12,
                                TP_TREE_TOKEN(tmp_for),
                                TP_TREE_TOKEN(tmp_left_paren),
                                TP_TREE_TOKEN(tmp_semicolon1),
                                TP_TREE_TOKEN(tmp_semicolon2),
                                TP_TREE_TOKEN(tmp_right_paren),
                                TP_TREE_NODE(tmp_statement)
                            );
                        }

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip_1;
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
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_expression1);
        tp_free_parse_subtree(symbol_table, &tmp_expression2);
        tp_free_parse_subtree(symbol_table, &tmp_expression3);
        tp_free_parse_subtree(symbol_table, &tmp_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_jump_statement(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: jump-statement -> 
    //     goto identifier ; |
    //     continue ; |
    //     break ; |
    //     return expression? ;
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: jump-statement -> goto identifier ;
    {
        TP_TOKEN* tmp_goto = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_GOTO(tmp_goto)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

            if (IS_TOKEN_ID(tmp_identifier)){

                ++TP_POS(symbol_table);

                TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

                if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_1,
                        TP_TREE_TOKEN(tmp_goto),
                        TP_TREE_TOKEN(tmp_identifier),
                        TP_TREE_TOKEN(tmp_semicolon)
                    );
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
        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: jump-statement -> continue ;
    {
        TP_TOKEN* tmp_continue = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_CONTINUE(tmp_continue)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

            if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_2,
                    TP_TREE_TOKEN(tmp_continue),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }
        }
skip_2:
        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: jump-statement -> break ;
    {
        TP_TOKEN* tmp_break = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_BREAK(tmp_break)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

            if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_3,
                    TP_TREE_TOKEN(tmp_break),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_3;
            }
        }
skip_3:
        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: jump-statement -> return expression? ;
    {
        TP_PARSE_TREE* tmp_expression = NULL;

        TP_TOKEN* tmp_return = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_RETURN(tmp_return)){

            ++TP_POS(symbol_table);

            tmp_expression = tp_parse_expression(symbol_table);

            TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

            if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                ++TP_POS(symbol_table);

                if (tmp_expression){

                    // Grammer: jump-statement -> return expression ;
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_4,
                        TP_TREE_TOKEN(tmp_return),
                        TP_TREE_NODE(tmp_expression),
                        TP_TREE_TOKEN(tmp_semicolon)
                    );
                }else{

                    // Grammer: jump-statement -> return ;
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_5,
                        TP_TREE_TOKEN(tmp_return),
                        TP_TREE_TOKEN(tmp_semicolon)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_4;
            }
        }
skip_4:
        tp_free_parse_subtree(symbol_table, &tmp_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

