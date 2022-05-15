
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(direct declarator).

static bool make_C_IR_direct_declarator_1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_DirectDeclaratorTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);
static bool make_C_IR_DirectDeclaratorTmp2(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type
);

bool tp_make_C_IR_direct_declarator(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    // direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2*
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: direct-declarator -> DirectDeclaratorTmp1
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
        case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1:
            if ( ! make_C_IR_direct_declarator_1(
                symbol_table,
                parse_tree_child, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        // Grammer: DirectDeclaratorTmp1 -> identifier
        case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1:
            if ( ! make_C_IR_DirectDeclaratorTmp1(
                symbol_table,
                parse_tree_child, grammer_context, c_object, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_direct_declarator_1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
        // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1:{
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_tmp1 = element[0].member_body.member_child;

        if ( ! make_C_IR_DirectDeclaratorTmp1(
            symbol_table,
            parse_tree_child_tmp1, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child_tmp2 = element[1].member_body.member_child;

        // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
        if (TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1 !=
            parse_tree_child_tmp2->member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! make_C_IR_DirectDeclaratorTmp2(
            symbol_table,
            parse_tree_child_tmp2, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_DirectDeclaratorTmp1(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
        // Grammer: DirectDeclaratorTmp1 -> identifier
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* token = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_ID(token)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        switch (grammer_context){
        case TP_GRAMMER_CONTEXT_FUNCTION_ARGS:{

            rsize_t parameter_num = type->member_function_parameter_num_attr;

            if (0 == parameter_num){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            if (TP_C_TYPE_TYPE_FUNCTION != type->member_type.member_type){

                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }

            TP_C_TYPE_FUNCTION_F_PARAM* tmp_parameter =
                type->member_type.member_body.member_type_function.member_parameter;

            for (rsize_t i = 0; parameter_num > i; ++i){

                if (i == parameter_num - 1){

                    if (NULL == tmp_parameter){

                        TP_PUT_LOG_MSG_ICE(symbol_table);
                        return false;
                    }else{

                        tmp_parameter->member_parameter_name = token;
                        break;
                    }
                }

                if (NULL == tmp_parameter){

                    TP_PUT_LOG_MSG_ICE(symbol_table);
                    return false;
                }else{

                    tmp_parameter = tmp_parameter->member_next;
                }
            }
            break;
        }
        case TP_GRAMMER_CONTEXT_FUNCTION_NAME:
//          break;
        default:
            type->member_c_namespace = TP_C_NAMESPACE_OTHER;
            type->member_token = token;

            if ( ! tp_register_defined_variable(
                symbol_table, grammer_context, token, TP_C_NAMESPACE_OTHER, type)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                return false;
            }
            break;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool make_C_IR_DirectDeclaratorTmp2(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object, TP_C_OBJECT* type)
{
    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
    case TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1:{
        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* left_parenthesis = element[0].member_body.member_tp_token;

        if ( ! IS_TOKEN_LEFT_PAREN(left_parenthesis)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_TOKEN* right_parenthesis = element[2].member_body.member_tp_token;

        if ( ! IS_TOKEN_RIGHT_PAREN(right_parenthesis)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (TP_PARSE_TREE_TYPE_NODE != element[1].member_type){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[1].member_body.member_child;

        // Grammer: parameter-type-list -> parameter-list
        if (TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2 !=
            parse_tree_child->member_grammer){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_parameter_type_list(
            symbol_table, parse_tree_child, grammer_context, c_object, type)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

