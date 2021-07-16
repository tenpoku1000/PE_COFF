
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(external declarations).

bool tp_make_C_IR_external_declaration(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(External definitions: external-declaration)

    TP_C_OBJECT* external_declaration = NULL;

    if (c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        goto fail;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    // Grammer: external-declaration -> declaration
    if (TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_2 ==
        parse_tree->member_grammer){

        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        external_declaration =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == external_declaration){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        external_declaration->member_is_static_data = true;

        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child =
            (TP_PARSE_TREE*)(element[0].member_body.member_child);

        switch (parse_tree_child->member_grammer){
        // Grammer: declaration -> declaration-specifiers init-declarator-list ;
        case TP_PARSE_TREE_GRAMMER_DECLARATION_1:
            break;
        // Grammer: declaration -> declaration-specifiers ;
        case TP_PARSE_TREE_GRAMMER_DECLARATION_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        if ( ! tp_make_C_IR_declaration(
            symbol_table, parse_tree_child, grammer_context, external_declaration)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        return true;
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);
fail:
    tp_free_c_object(symbol_table, &external_declaration);
    return false;
}

