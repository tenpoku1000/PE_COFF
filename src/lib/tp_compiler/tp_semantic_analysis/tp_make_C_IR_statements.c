
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Convert parse tree to C IR(statements).

static bool make_C_IR_compound_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
static bool register_function_formal_param(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
static bool make_C_IR_block_item_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);
static bool make_C_IR_block_item(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object
);

bool tp_make_C_IR_statements(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements)

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: statement -> compound-statement
    case TP_PARSE_TREE_GRAMMER_C_STATEMENT_2:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: compound-statement -> { block-item-list }
        case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1:
            break;
        // Grammer: compound-statement -> { }
        case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_statements(
            symbol_table, parse_tree_child, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: compound-statement -> { block-item-list }
    case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1:
        if ( ! make_C_IR_compound_statement(
            symbol_table, parse_tree, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    // Grammer: compound-statement -> { }
    case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2:
        break;
    // Grammer: statement -> expression-statement
    case TP_PARSE_TREE_GRAMMER_C_STATEMENT_3:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: expression-statement -> expression ;
        case TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_1:
            break;
        // Grammer: expression-statement -> ;
        case TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_expression_statement(
            symbol_table, parse_tree_child, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: statement -> iteration-statement
    case TP_PARSE_TREE_GRAMMER_C_STATEMENT_5:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: iteration-statement -> do statement while ( expression ) ;
        case TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_iteration_statement(
            symbol_table, parse_tree_child, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    }
    // Grammer: statement -> jump-statement
    case TP_PARSE_TREE_GRAMMER_C_STATEMENT_6:{
        if (1 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: jump-statement -> return expression ;
        case TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_4:
        // Grammer: jump-statement -> return ;
        case TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_5:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_jump_statement(
            symbol_table, parse_tree_child, grammer_context, c_object)){

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

bool tp_append_c_object_to_compound_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_OBJECT* parent_c_object, TP_C_OBJECT* c_object)
{
    TP_C_TYPE_COMPOUND_STATEMENT* compound_statement = NULL;

    switch (parent_c_object->member_type.member_type){
    case TP_C_TYPE_TYPE_FUNCTION:
        compound_statement =
            &(parent_c_object->member_type.
member_body.member_type_function.member_function_body);
        break;
    case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
        compound_statement =
            &(parent_c_object->member_type.
member_body.member_type_compound_statement);
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if ((compound_statement->member_statement_body) &&
        (TP_C_NAMESPACE_OTHER == c_object->member_c_namespace)){

        rsize_t statement_body_num = compound_statement->member_statement_body_pos;

        for (rsize_t i = 0; statement_body_num >= i; ++i){

            TP_C_OBJECT* statement_body = compound_statement->member_statement_body[i];

            if (NULL == statement_body){

                continue;
            }

            if (TP_C_NAMESPACE_OTHER != statement_body->member_c_namespace){

                continue;
            }

            if (NULL == statement_body->member_token){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (NULL == c_object->member_token){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            if (0 == strcmp(
                c_object->member_token->member_string,
                statement_body->member_token->member_string)){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: Duplicate OBJECT name(%1)."),
                    TP_LOG_PARAM_STRING(c_object->member_token->member_string)
                );

                return false;
            }
        }
    }

    if ((NULL == compound_statement->member_statement_body) ||
        (compound_statement->member_statement_body_pos ==
            (compound_statement->member_statement_body_size / sizeof(TP_C_OBJECT*)))){

        rsize_t c_object_size_allocate_unit = 
            symbol_table->member_c_object_size_local_allocate_unit * sizeof(TP_C_OBJECT*);

        rsize_t c_object_size =
            compound_statement->member_statement_body_size + c_object_size_allocate_unit;

        TP_C_OBJECT** tmp_c_object = (TP_C_OBJECT**)TP_REALLOC(
            symbol_table, compound_statement->member_statement_body, c_object_size
        );

        if (NULL == tmp_c_object){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tmp_c_object) +
                compound_statement->member_statement_body_size, 0,
            c_object_size_allocate_unit
        );

        compound_statement->member_statement_body = tmp_c_object;
        compound_statement->member_statement_body_size = c_object_size;
    }

    compound_statement->member_statement_body
[compound_statement->member_statement_body_pos] = c_object;

    ++(compound_statement->member_statement_body_pos);

    return true;
}

static bool make_C_IR_compound_statement(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: compound-statement)

    TP_C_OBJECT* compound_statement = NULL;

    bool is_skip_append_c_object_to_compound_statement = true;

    if (NULL == c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    switch (grammer_context){
    case TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION:
        break;
    case TP_GRAMMER_CONTEXT_STATEMENTS:{

        compound_statement =
            (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

        if (NULL == compound_statement){

            TP_PRINT_CRT_ERROR(symbol_table);
            return false;
        }

        compound_statement->member_type.member_type =
            TP_C_TYPE_TYPE_COMPOUND_STATEMENT;

        is_skip_append_c_object_to_compound_statement = false;
        break;
    }
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: compound-statement -> { block-item-list }
    case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1:{

        bool is_push_c_namespace_stack = false;

        if (3 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[2].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
        if (( ! IS_TOKEN_LEFT_CURLY_BRACKET(element[0].member_body.member_tp_token)) ||
            ( ! IS_TOKEN_RIGHT_CURLY_BRACKET(element[2].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        TP_PARSE_TREE* parse_tree_child = element[1].member_body.member_child;

        switch (parse_tree_child->member_grammer){
        // Grammer: block-item-list -> block-item block-item+
        case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1:
            break;
        // Grammer: block-item-list -> block-item
        case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }

        if ( ! tp_push_c_namespace_stack(
            symbol_table, TP_GRAMMER_CONTEXT_STATEMENTS)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }
        is_push_c_namespace_stack = true;

        if (TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION == grammer_context){

            if ( ! register_function_formal_param(
                symbol_table, grammer_context, c_object)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
                goto fail;
            }
        }

        if (TP_GRAMMER_CONTEXT_STATEMENTS == grammer_context){

            TP_C_TYPE_COMPOUND_STATEMENT* child_compound_statement = NULL;

            switch (compound_statement->member_type.member_type){
            case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
               child_compound_statement =
                    &(compound_statement->member_type.
member_body.member_type_compound_statement);
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            TP_C_TYPE_COMPOUND_STATEMENT* parent_compound_statement = NULL;

            switch (c_object->member_type.member_type){
            case TP_C_TYPE_TYPE_FUNCTION:
               parent_compound_statement =
                    &(c_object->member_type.
member_body.member_type_function.member_function_body);
                break;
            case TP_C_TYPE_TYPE_COMPOUND_STATEMENT:
               parent_compound_statement =
                    &(c_object->member_type.
member_body.member_type_compound_statement);
                break;
            case TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO:
                c_object->member_type.member_body.
member_type_iteration_statement_do.member_statement = compound_statement;
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                goto fail;
            }

            if (parent_compound_statement){

                TP_C_INHERIT_ATTR_TO_COMPOUND_STATEMENT_FROM_COMPOUND_STATEMENT(
                   child_compound_statement, parent_compound_statement
                );

                if (!tp_append_c_object_to_compound_statement(
                    symbol_table, c_object, compound_statement)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    goto fail;
                }
            }else{

                TP_C_INHERIT_ATTR_TO_COMPOUND_STATEMENT_FROM_C_OBJECT(
                    child_compound_statement, c_object
                );
            }
            is_skip_append_c_object_to_compound_statement = true;
        }

        if ( ! make_C_IR_block_item_list(
            symbol_table, parse_tree_child, grammer_context,
            ((TP_GRAMMER_CONTEXT_STATEMENTS == grammer_context) ?
                compound_statement : c_object))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            goto fail;
        }

        if ( ! tp_pop_c_namespace_stack(
            symbol_table, TP_GRAMMER_CONTEXT_STATEMENTS)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        return true;
fail:
        if ( ! is_skip_append_c_object_to_compound_statement){

            if ( ! tp_append_c_object_to_compound_statement(
                symbol_table, c_object, compound_statement)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
            }
        }
        if (is_push_c_namespace_stack){

            if ( ! tp_pop_c_namespace_stack(
                symbol_table, TP_GRAMMER_CONTEXT_STATEMENTS)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);
            }else{

                is_push_c_namespace_stack = false;
            }
        }
        return false;
    }
    // Grammer: compound-statement -> { }
    case TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2:
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_TOKEN != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_TOKEN != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if (( ! IS_TOKEN_LEFT_CURLY_BRACKET(element[0].member_body.member_tp_token)) ||
            ( ! IS_TOKEN_RIGHT_CURLY_BRACKET(element[1].member_body.member_tp_token))){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

static bool register_function_formal_param(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    TP_C_TYPE* c_type = &(c_object->member_type);

    if (TP_C_TYPE_TYPE_FUNCTION != c_type->member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_C_TYPE_FUNCTION* type_function = &(c_type->member_body.member_type_function);

    rsize_t parameter_num = type_function->member_parameter_num;
    TP_C_TYPE_FUNCTION_F_PARAM* parameter = type_function->member_parameter;

    switch (parameter_num){
    case 0:
        return true;
    case 1:
        if (parameter){

            TP_C_TYPE* type_parameter = parameter->member_type_parameter;

            if (TP_C_TYPE_TYPE_BASIC == type_parameter->member_type){

                if (TP_C_TYPE_SPECIFIER_VOID ==
                    type_parameter->member_body.member_type_basic.member_type_specifier){

                    return true;
                }
            }
        }
        break;
    default:
        break;
    }

    TP_C_OBJECT* regist_param = NULL;
    TP_C_TYPE* type_param = NULL;

    for (rsize_t i = 0; parameter_num > i; ++i){

        if (parameter){

            TP_TOKEN* token = parameter->member_parameter_name;

            if (NULL == token){

                continue;
            }
            if ( ! IS_TOKEN_ID(token)){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            TP_C_OBJECT* tmp_regist_param =
                (TP_C_OBJECT*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_OBJECT));

            if (NULL == tmp_regist_param){

                TP_PRINT_CRT_ERROR(symbol_table);

                return false;
            }

            regist_param = tmp_regist_param;

            TP_C_TYPE* tmp_type_param =
                (TP_C_TYPE*)TP_CALLOC(symbol_table, 1, sizeof(TP_C_TYPE));

            if (NULL == tmp_type_param){

                TP_PRINT_CRT_ERROR(symbol_table);

                goto fail;
            }

            type_param = tmp_type_param;
            *type_param = *(parameter->member_type_parameter);

            regist_param->member_c_namespace = TP_C_NAMESPACE_OTHER;
            regist_param->member_token = token;
            regist_param->member_type.member_type = TP_C_TYPE_TYPE_FUNCTION_PARAM;
            regist_param->member_type.member_body.
                member_type_param.member_type_param = type_param;

            if ( ! tp_append_c_object_to_compound_statement(
                symbol_table, c_object, regist_param)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }

            if ( ! tp_register_defined_variable(
                symbol_table, grammer_context, token, TP_C_NAMESPACE_OTHER, regist_param)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

//              goto fail;
                return false;
            }

            parameter = parameter->member_next;

            regist_param = NULL;
            type_param = NULL;

            continue;
        }

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
fail:
    TP_FREE(symbol_table, &type_param, sizeof(TP_C_TYPE));
    TP_FREE(symbol_table, &regist_param, sizeof(TP_C_OBJECT));
    return false;
}

static bool make_C_IR_block_item_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: block-item-list)

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    switch (parse_tree->member_grammer){
    // Grammer: block-item-list -> block-item block-item+
    case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1:
        if (2 != element_num){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ((TP_PARSE_TREE_TYPE_NODE != element[0].member_type) ||
            (TP_PARSE_TREE_TYPE_NODE != element[1].member_type)){

            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        for (size_t i = 0; 2 > i; ++i){

            TP_PARSE_TREE* parse_tree_child = element[i].member_body.member_child;

            switch (parse_tree_child->member_grammer){
            // Grammer: block-item-list -> block-item block-item+
            case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1:
                if ( ! make_C_IR_block_item_list(
                    symbol_table, parse_tree_child, grammer_context, c_object)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    return false;
                }
                break;
            // Grammer: block-item -> declaration
            case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1:
//              break;
            // Grammer: block-item -> statement
            case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2:
                if ( ! make_C_IR_block_item(
                    symbol_table, parse_tree_child, grammer_context, c_object)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);
                    return false;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }
        break;
    // Grammer: block-item-list -> block-item
    case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_2:{
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
        // Grammer: block-item -> declaration
        case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1:
            break;
        // Grammer: block-item -> statement
        case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }

        if ( ! make_C_IR_block_item(
            symbol_table, parse_tree_child, grammer_context, c_object)){

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

static bool make_C_IR_block_item(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context,
    TP_C_OBJECT* c_object)
{
    // C compiler(Statements: block-item)

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    if (1 != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_NODE != element[0].member_type){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_PARSE_TREE* parse_tree_child = element[0].member_body.member_child;

    switch (parse_tree->member_grammer){
    // Grammer: block-item -> declaration
    case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1:
        switch (parse_tree_child->member_grammer){
        // Grammer: declaration -> declaration-specifiers init-declarator-list ;
        case TP_PARSE_TREE_GRAMMER_DECLARATION_1:
            break;
        // Grammer: declaration -> declaration-specifiers ;
        case TP_PARSE_TREE_GRAMMER_DECLARATION_2:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_declaration(
            symbol_table, parse_tree_child, grammer_context, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    // Grammer: block-item -> statement
    case TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2:
        switch (parse_tree_child->member_grammer){
        // Grammer: statement -> compound-statement
        case TP_PARSE_TREE_GRAMMER_C_STATEMENT_2:
            break;
        // Grammer: statement -> expression-statement
        case TP_PARSE_TREE_GRAMMER_C_STATEMENT_3:
            break;
        // Grammer: statement -> iteration-statement
        case TP_PARSE_TREE_GRAMMER_C_STATEMENT_5:
            break;
        // Grammer: statement -> jump-statement
        case TP_PARSE_TREE_GRAMMER_C_STATEMENT_6:
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
        if ( ! tp_make_C_IR_statements(
            symbol_table, parse_tree_child, TP_GRAMMER_CONTEXT_STATEMENTS, c_object)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

