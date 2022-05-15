
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(type specifier).

/* @@@
bool tp_make_C_IR_struct_or_union_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type)
*/

static bool make_C_IR_type_specifier_basic(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type,
    TP_C_TYPE_SPECIFIER* type_specifier
);

bool tp_make_C_IR_type_specifier(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type)
{
    TP_C_TYPE_SPECIFIER* type_specifier = NULL;

    // Grammer: type-specifier -> void | char | short | int | long |
    //     signed | unsigned
    if (TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1 !=
        (parse_tree->member_grammer)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    switch (type->member_type){
    case TP_C_TYPE_TYPE_DECLARATION_STATEMENT:{

        TP_C_TYPE** type_declaration = &(type->member_body.
            member_type_declaration_statement.member_declaration);

        if (*type_declaration){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_C_TYPE* tmp_type_declaration =
            (TP_C_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_TYPE));

        if (NULL == tmp_type_declaration){

            TP_PRINT_CRT_ERROR(symbol_table);
            return false;
        }
        *type_declaration = tmp_type_declaration;
        type = *type_declaration;
        goto common;
    }
    case TP_C_TYPE_TYPE_FUNCTION:
        switch (grammer_context){
        case TP_GRAMMER_CONTEXT_FUNCTION_ARGS:
            break;
        case TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
//      break;
    default:
common:
        type->member_type = TP_C_TYPE_TYPE_BASIC;
        type_specifier = &(type->member_body.member_type_basic.member_type_specifier);
        break;
    }

    if ( ! make_C_IR_type_specifier_basic(
        symbol_table, parse_tree, grammer_context, c_object, type, type_specifier)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool make_C_IR_type_specifier_basic(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_TYPE* type,
    TP_C_TYPE_SPECIFIER* type_specifier)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    TP_TOKEN* token = NULL;

    switch (parse_tree->member_grammer){
    // Grammer: type-specifier -> void | char | short | int | long |
    //     signed | unsigned
    case TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1:
        if (0 == element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        for (size_t i = 0; element_num > i; ++i){

            if (TP_PARSE_TREE_TYPE_TOKEN != element[i].member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            token = element[i].member_body.member_tp_token;

            if (IS_TOKEN_KEYWORD_VOID(token)){

                if (1 != element_num){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_VOID;
            }else if (IS_TOKEN_KEYWORD_SIGNED(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_SIGNED) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_UNSIGNED)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_SIGNED;
            }else if (IS_TOKEN_KEYWORD_UNSIGNED(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_SIGNED) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_UNSIGNED)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_UNSIGNED;
            }else if (IS_TOKEN_KEYWORD_CHAR(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_SHORT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_INT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_CHAR;
            }else if (IS_TOKEN_KEYWORD_SHORT(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_SHORT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_SHORT;
            }else if (IS_TOKEN_KEYWORD_INT(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_INT) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR)){

                       goto fail;
                }
                (*type_specifier) |= TP_C_TYPE_SPECIFIER_INT;
            }else if (IS_TOKEN_KEYWORD_LONG(token)){

                if (((*type_specifier) & TP_C_TYPE_SPECIFIER_CHAR) ||
                    ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG2)){

                    goto fail;
                }else if ((*type_specifier) & TP_C_TYPE_SPECIFIER_LONG1){

                    (*type_specifier) ^= TP_C_TYPE_SPECIFIER_LONG1;
                    (*type_specifier) |= TP_C_TYPE_SPECIFIER_LONG2;
                }else{

                    (*type_specifier) |= TP_C_TYPE_SPECIFIER_LONG1;
                }
            }else{

                goto fail;
            }
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
fail:
    if (NULL == token){

        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }
    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: type error(%1)."),
        TP_LOG_PARAM_STRING(token->member_string)
    );
    return false;
}

