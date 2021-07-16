
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Grammer(External definitions):
//
// translation-unit -> external-declaration+
//
// external-declaration -> function-definition | declaration
//
// function-definition -> 
//     declaration-specifiers declarator declaration-list? compound-statement
//
// declaration-list -> declaration+

static TP_PARSE_TREE* parse_external_declaration(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_function_definition(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_declaration_list(TP_SYMBOL_TABLE* symbol_table);

TP_PARSE_TREE* tp_parse_translation_unit(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: translation-unit -> external-declaration+
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_external_declaration1 = NULL;
        TP_PARSE_TREE* tmp_external_declaration2 = NULL;

        if ( ! (tmp_external_declaration1 = parse_external_declaration(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_external_declaration2 = parse_external_declaration(symbol_table)){

            is_single = false;

            // Grammer: translation-unit -> external-declaration external-declaration+
            tmp_external_declaration1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1,
                TP_TREE_NODE(tmp_external_declaration1),
                TP_TREE_NODE(tmp_external_declaration2)
            );
        }

        if (is_single){

            // Grammer: translation-unit -> external-declaration
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_2,
                TP_TREE_NODE(tmp_external_declaration1)
            );
        }

        return tmp_external_declaration1;
skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_external_declaration(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: external-declaration -> function-definition | declaration
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: external-declaration -> function-definition
    {
        TP_PARSE_TREE* tmp_function_definition = NULL;

        if (tmp_function_definition = parse_function_definition(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_1,
                TP_TREE_NODE(tmp_function_definition)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: external-declaration -> declaration
    {
        TP_PARSE_TREE* tmp_declaration = NULL;

        if (tmp_declaration = tp_parse_declaration(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_2,
                TP_TREE_NODE(tmp_declaration)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_function_definition(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: function-definition -> 
    //     declaration-specifiers declarator declaration-list? compound-statement
    {
        TP_PARSE_TREE* tmp_declaration_specifiers = NULL;
        TP_PARSE_TREE* tmp_declarator = NULL;
        TP_PARSE_TREE* tmp_declaration_list = NULL;
        TP_PARSE_TREE* tmp_compound_statement = NULL;

        if ( ! (tmp_declaration_specifiers = tp_parse_declaration_specifiers(symbol_table, NULL))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        if ( ! (tmp_declarator = tp_parse_declarator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        if ( ! (tmp_declaration_list = parse_declaration_list(symbol_table))){

            if ( ! (tmp_compound_statement = tp_parse_compound_statement(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            // Grammer: function-definition -> 
            //     declaration-specifiers declarator compound-statement
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_1,
                TP_TREE_NODE(tmp_declaration_specifiers),
                TP_TREE_NODE(tmp_declarator),
                TP_TREE_NODE(tmp_compound_statement)
            );
        }

        if ( ! (tmp_compound_statement = tp_parse_compound_statement(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        // Grammer: function-definition -> 
        //     declaration-specifiers declarator declaration-list compound-statement
        return MAKE_PARSE_SUBTREE(
            symbol_table,
            TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_2,
            TP_TREE_NODE(tmp_declaration_specifiers),
            TP_TREE_NODE(tmp_declarator),
            TP_TREE_NODE(tmp_declaration_list),
            TP_TREE_NODE(tmp_compound_statement)
        );
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_declaration_specifiers);
        tp_free_parse_subtree(symbol_table, &tmp_declarator);
        tp_free_parse_subtree(symbol_table, &tmp_declaration_list);
        tp_free_parse_subtree(symbol_table, &tmp_compound_statement);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_declaration_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: declaration-list -> declaration+
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_declaration1 = NULL;
        TP_PARSE_TREE* tmp_declaration2 = NULL;

        if ( ! (tmp_declaration1 = tp_parse_declaration(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_declaration2 = tp_parse_declaration(symbol_table)){

            is_single = false;

            // Grammer: declaration-list -> declaration declaration+
            tmp_declaration1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_1,
                TP_TREE_NODE(tmp_declaration1),
                TP_TREE_NODE(tmp_declaration2)
            );
        }

        if (is_single){

            // Grammer: declaration-list -> declaration
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_2,
                TP_TREE_NODE(tmp_declaration1)
            );
        }

        return tmp_declaration1;
skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

