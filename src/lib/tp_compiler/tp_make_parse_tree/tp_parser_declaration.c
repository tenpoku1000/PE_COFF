
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Grammer(Declarations):
// 
// declaration -> declaration-specifiers init-declarator-list? ; |
//     static_assert-declaration
// 
// type-name -> specifier-qualifier-list abstract-declarator?
// 
// ----------------------------------------------------------------------------------------
// Grammer(declaration):
// 
// declaration-specifiers ->
//     (storage-class-specifier | type-specifier | type-qualifier |
//    function-specifier | alignment-specifier) declaration-specifiers?
// 
// storage-class-specifier -> typedef | extern | static | _Thread_local | auto | register
// 
// function-specifier -> inline | _Noreturn
// 
// alignment-specifier -> _Alignas ( type-name | constant-expression )
// 
// init-declarator-list -> init-declarator (, init-declarator)*
// 
// init-declarator -> declarator (= initializer)?
// 
// declarator -> pointer? direct-declarator
// 
// direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2*
// 
// DirectDeclaratorTmp1 -> identifier | ( declarator )
// 
// DirectDeclaratorTmp2 -> [ type-qualifier-list? assignment-expression? ] |
//     [ static type-qualifier-list? assignment-expression ] |
//     [ type-qualifier-list static assignment-expression ] |
//     [ type-qualifier-list? * ] |
//     ( parameter-type-list ) |
//     ( identifier-list? )
// 
// identifier-list -> identifier (, identifier)*
// 
// initializer -> assignment-expression | { initializer-list ,? }
// 
// initializer-list -> initializerListTmp1 (, initializerListTmp1)*
// 
// initializerListTmp1 -> designation? initializer
// 
// designation -> designator-list =
// 
// designator-list -> designator+
// 
// designator -> [ constant-expression ] | . identifier

// ----------------------------------------------------------------------------------------
// Grammer(declaration):
// 
static TP_PARSE_TREE* parse_storage_class_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_function_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_alignment_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_init_declarator_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_init_declarator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_declarator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_declarator_tmp1(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_declarator_tmp2(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_declarator_tmp2_square(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket
);
static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_static(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_TOKEN* tmp_static
);
static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_other(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket
);
static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_other_qualifier_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_PARSE_TREE* tmp_type_qualifier_list
);
static TP_PARSE_TREE* parse_direct_declarator_tmp2_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_parenthesis
);
static TP_PARSE_TREE* parse_identifier_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_initializer(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_initializer_list_tmp1(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_designation(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_designator_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_designator(TP_SYMBOL_TABLE* symbol_table);

// ----------------------------------------------------------------------------------------
// Grammer(declaration):
// 

TP_PARSE_TREE* tp_parse_declaration(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: declaration -> declaration-specifiers init-declarator-list? ; |
    //     static_assert-declaration
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: declaration -> declaration-specifiers init-declarator-list? ;
    {
        TP_PARSE_TREE* tmp_declaration_specifiers = NULL;
        TP_PARSE_TREE* tmp_init_declarator_list = NULL;

        if ( ! (tmp_declaration_specifiers = tp_parse_declaration_specifiers(symbol_table, NULL))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        tmp_init_declarator_list = parse_init_declarator_list(symbol_table);

        TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

        if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

            ++TP_POS(symbol_table);

            if (tmp_init_declarator_list){

                // Grammer: declaration -> declaration-specifiers init-declarator-list ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DECLARATION_1,
                    TP_TREE_NODE(tmp_declaration_specifiers),
                    TP_TREE_NODE(tmp_init_declarator_list),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                // Grammer: declaration -> declaration-specifiers ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DECLARATION_2,
                    TP_TREE_NODE(tmp_declaration_specifiers),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_declaration_specifiers);
        tp_free_parse_subtree(symbol_table, &tmp_init_declarator_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: declaration -> static_assert-declaration
    {
        TP_PARSE_TREE* tmp_static_assert_declaration = NULL;

        if (tmp_static_assert_declaration = tp_parse_static_assert_declaration(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATION_3,
                TP_TREE_NODE(tmp_static_assert_declaration)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_declaration_specifiers(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* prev_node)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
    //    function-specifier | alignment-specifier) declaration-specifiers?
    {
        TP_PARSE_TREE* tmp_node = NULL;
        TP_PARSE_TREE* tmp_declaration_specifiers = NULL;

        if ( ! (tmp_node = parse_storage_class_specifier(symbol_table))){

            if ( ! (tmp_node = tp_parse_type_specifier(symbol_table))){

                if ( ! (tmp_node = tp_parse_type_qualifier(symbol_table))){

                    if ( ! (tmp_node = parse_function_specifier(symbol_table))){

                        if ( ! (tmp_node = parse_alignment_specifier(symbol_table))){

                            if (prev_node){

                                return prev_node;
                            }

                            TP_PUT_LOG_MSG_TRACE(symbol_table);

                            goto skip_1;
                        }
                    }
                }
            }
        }

        tmp_declaration_specifiers = tp_parse_declaration_specifiers(symbol_table, tmp_node);

        if (tmp_declaration_specifiers == tmp_node){

            goto single;
        }

        if (tmp_declaration_specifiers){

            // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
            //    function-specifier | alignment-specifier) declaration-specifiers
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1,
                TP_TREE_NODE(tmp_node),
                TP_TREE_NODE(tmp_declaration_specifiers)
            );
        }else{

            // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
            //    function-specifier | alignment-specifier)
single:
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2,
                TP_TREE_NODE(tmp_node)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_node);
        tp_free_parse_subtree(symbol_table, &tmp_declaration_specifiers);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_storage_class_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: storage-class-specifier -> typedef | extern | static | _Thread_local | auto | register
    {
        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_TYPEDEF(tmp_token) || IS_TOKEN_KEYWORD_EXTERN(tmp_token) ||
            IS_TOKEN_KEYWORD_STATIC(tmp_token) || IS_TOKEN_KEYWORD_THREAD_LOCAL(tmp_token) ||
            IS_TOKEN_KEYWORD_AUTO(tmp_token) || IS_TOKEN_KEYWORD_REGISTER(tmp_token)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STORAGE_CLASS_SPECIFIER_1,
                TP_TREE_TOKEN(tmp_token)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_function_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: function-specifier -> inline | _Noreturn
    {
        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_INLINE(tmp_token) || IS_TOKEN_KEYWORD_NORETURN(tmp_token)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FUNCTION_SPECIFIER_1,
                TP_TREE_TOKEN(tmp_token)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_alignment_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: alignment-specifier -> _Alignas ( type-name | constant-expression )
    {
        TP_TOKEN* tmp_alignas = TP_POS(symbol_table);
        TP_PARSE_TREE* tmp_node = NULL;

        if (IS_TOKEN_KEYWORD_ALIGNAS(tmp_alignas)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_node = tp_parse_type_name(symbol_table))){

                    if ( ! (tmp_node = tp_parse_constant_expression(symbol_table))){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip_1;
                    }
                }

                TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_ALIGNMENT_SPECIFIER_1,
                        TP_TREE_TOKEN(tmp_alignas),
                        TP_TREE_TOKEN(tmp_left_parenthesis),
                        TP_TREE_NODE(tmp_node),
                        TP_TREE_TOKEN(tmp_right_parenthesis)
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
        tp_free_parse_subtree(symbol_table, &tmp_node);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_init_declarator_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: init-declarator-list -> init-declarator (, init-declarator)*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_init_declarator1 = NULL;
        TP_PARSE_TREE* tmp_init_declarator2 = NULL;

        if ( ! (tmp_init_declarator1 = parse_init_declarator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_init_declarator2 = parse_init_declarator(symbol_table)){

                    is_single = false;

                    // Grammer: init-declarator-list -> init-declarator (, init-declarator)+
                    tmp_init_declarator1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_1,
                        TP_TREE_NODE(tmp_init_declarator1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_init_declarator2)
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

            // Grammer: init-declarator-list -> init-declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_2,
                TP_TREE_NODE(tmp_init_declarator1)
            );
        }

        return tmp_init_declarator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_init_declarator1);
        tp_free_parse_subtree(symbol_table, &tmp_init_declarator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_init_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: init-declarator -> declarator (= initializer)?
    {
        TP_PARSE_TREE* tmp_declarator = NULL;
        TP_PARSE_TREE* tmp_initializer = NULL;

        if ( ! (tmp_declarator = tp_parse_declarator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_equal = TP_POS(symbol_table);

        if (IS_TOKEN_EQUAL(tmp_equal)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_initializer = parse_initializer(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            // Grammer: init-declarator -> declarator = initializer
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_1,
                TP_TREE_NODE(tmp_declarator),
                TP_TREE_TOKEN(tmp_equal),
                TP_TREE_NODE(tmp_initializer)
            );
        }else{

            // Grammer: init-declarator -> declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_2,
                TP_TREE_NODE(tmp_declarator)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_declarator);
        tp_free_parse_subtree(symbol_table, &tmp_initializer);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: declarator -> pointer? direct-declarator
    {
        TP_PARSE_TREE* tmp_pointer = NULL;
        TP_PARSE_TREE* tmp_direct_declarator = NULL;

        tmp_pointer = tp_parse_pointer(symbol_table);

        if ( ! (tmp_direct_declarator = parse_direct_declarator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        if (tmp_pointer){

            // Grammer: declarator -> pointer direct-declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATOR_1,
                TP_TREE_NODE(tmp_pointer),
                TP_TREE_NODE(tmp_direct_declarator)
            );
        }else{

            // Grammer: declarator -> direct-declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DECLARATOR_2,
                TP_TREE_NODE(tmp_direct_declarator)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_pointer);
        tp_free_parse_subtree(symbol_table, &tmp_direct_declarator);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_direct_declarator1 = NULL;
        TP_PARSE_TREE* tmp_direct_declarator2 = NULL;

        if ( ! (tmp_direct_declarator1 = parse_direct_declarator_tmp1(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_direct_declarator2 = parse_direct_declarator_tmp2(symbol_table)){

            // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
            tmp_direct_declarator1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1,
                TP_TREE_NODE(tmp_direct_declarator1),
                TP_TREE_NODE(tmp_direct_declarator2)
            );
        }

        if (is_single){

            // Grammer: direct-declarator -> DirectDeclaratorTmp1
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2,
                TP_TREE_NODE(tmp_direct_declarator1)
            );
        }

        return tmp_direct_declarator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_direct_declarator1);
        tp_free_parse_subtree(symbol_table, &tmp_direct_declarator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp1(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: DirectDeclaratorTmp1 -> identifier | ( declarator )
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectDeclaratorTmp1 -> identifier
    {
        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1,
                TP_TREE_TOKEN(tmp_identifier)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: DirectDeclaratorTmp1 -> ( declarator )
    {
        TP_PARSE_TREE* tmp_declarator = NULL;

        TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_declarator = tp_parse_declarator(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_2,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_NODE(tmp_declarator),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_declarator);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ static type-qualifier-list? assignment-expression ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [ type-qualifier-list? * ] |
    //     ( parameter-type-list ) |
    //     ( identifier-list? )
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        // Grammer: DirectDeclaratorTmp2 ->
        //     [ type-qualifier-list? assignment-expression? ] |
        //     [ static type-qualifier-list? assignment-expression ] |
        //     [ type-qualifier-list static assignment-expression ] |
        //     [ type-qualifier-list? * ]
        TP_TOKEN* tmp_left_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_SQUARE_BRACKET(tmp_left_square_bracket)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_declarator_tmp2_square
                = parse_direct_declarator_tmp2_square(
                    symbol_table, tmp_left_square_bracket
                );

            if (tmp_direct_declarator_tmp2_square){

                return tmp_direct_declarator_tmp2_square;
            }
        }

        // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list ) |
        //     ( identifier-list? )
        TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_declarator_tmp2_paren
                = parse_direct_declarator_tmp2_paren(
                    symbol_table, tmp_left_parenthesis
                );

            if (tmp_direct_declarator_tmp2_paren){

                return tmp_direct_declarator_tmp2_paren;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2_square(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket)
{
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ static type-qualifier-list? assignment-expression ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [ type-qualifier-list? * ]
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_TOKEN* tmp_static = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STATIC(tmp_static)){

            // Grammer: DirectDeclaratorTmp2 ->
            //     [ static type-qualifier-list? assignment-expression ]
            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_declarator_tmp2_square_static
                = parse_direct_declarator_tmp2_square_static(
                    symbol_table, tmp_left_square_bracket, tmp_static
                );

            if (tmp_direct_declarator_tmp2_square_static){

                return tmp_direct_declarator_tmp2_square_static;
            }
        }else{

            // Grammer: DirectDeclaratorTmp2 ->
            //     [ type-qualifier-list? assignment-expression? ] |
            //     [ type-qualifier-list static assignment-expression ] |
            //     [ type-qualifier-list? * ]
            TP_PARSE_TREE* tmp_direct_declarator_tmp2_square_other
                = parse_direct_declarator_tmp2_square_static(
                    symbol_table, tmp_left_square_bracket, tmp_static
                );

            if (tmp_direct_declarator_tmp2_square_other){

                return tmp_direct_declarator_tmp2_square_other;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_static(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_TOKEN* tmp_static)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectDeclaratorTmp2 ->
    //     [ static type-qualifier-list? assignment-expression ]
    {
        TP_PARSE_TREE* tmp_type_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        tmp_type_qualifier_list = tp_parse_type_qualifier_list(symbol_table);

        if ( ! (tmp_assignment_expression = tp_parse_assignment_expression(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

            ++TP_POS(symbol_table);

            if (tmp_type_qualifier_list){

                // Grammer: DirectDeclaratorTmp2 ->
                //     [ static type-qualifier-list assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_TOKEN(tmp_static),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                // Grammer: DirectDeclaratorTmp2 ->
                //     [ static assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_2,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_TOKEN(tmp_static),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier_list);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_other(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket)
{
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [ type-qualifier-list? * ]
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_type_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        tmp_type_qualifier_list = tp_parse_type_qualifier_list(symbol_table);

        if (tmp_type_qualifier_list){

            // Grammer: DirectDeclaratorTmp2 ->
            //     [ type-qualifier-list assignment-expression? ] |
            //     [ type-qualifier-list static assignment-expression ] |
            //     [ type-qualifier-list * ]
            TP_PARSE_TREE* tmp_direct_declarator_tmp2_square_other_qualifier_list
                = parse_direct_declarator_tmp2_square_other_qualifier_list(
                    symbol_table, tmp_left_square_bracket, tmp_type_qualifier_list
                );

            if (tmp_direct_declarator_tmp2_square_other_qualifier_list){

                return tmp_direct_declarator_tmp2_square_other_qualifier_list;
            }
        }else{

            TP_TOKEN* tmp_asterisk = TP_POS(symbol_table);

            if (IS_TOKEN_ASTERISK(tmp_asterisk)){

                // Grammer: DirectDeclaratorTmp2 -> [ * ]
                ++TP_POS(symbol_table);

                TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_1,
                        TP_TREE_TOKEN(tmp_left_square_bracket),
                        TP_TREE_TOKEN(tmp_asterisk),
                        TP_TREE_TOKEN(tmp_right_square_bracket)
                    );
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else{

                // Grammer: DirectDeclaratorTmp2 -> [ assignment-expression? ]
                tmp_assignment_expression = tp_parse_assignment_expression(symbol_table);

                TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                    ++TP_POS(symbol_table);

                    if (tmp_assignment_expression){

                        // Grammer: DirectDeclaratorTmp2 -> [ assignment-expression ]
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_2,
                            TP_TREE_TOKEN(tmp_left_square_bracket),
                            TP_TREE_NODE(tmp_assignment_expression),
                            TP_TREE_TOKEN(tmp_right_square_bracket)
                        );
                    }else{

                        // Grammer: DirectDeclaratorTmp2 -> [ ]
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_3,
                            TP_TREE_TOKEN(tmp_left_square_bracket),
                            TP_TREE_TOKEN(tmp_right_square_bracket)
                        );
                    }
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier_list);
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2_square_other_qualifier_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_PARSE_TREE* tmp_type_qualifier_list)
{
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list assignment-expression? ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [ type-qualifier-list * ]
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        TP_TOKEN* tmp_static_or_asterisk = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STATIC(tmp_static_or_asterisk)){

            // Grammer: DirectDeclaratorTmp2 ->
            //     [ type-qualifier-list static assignment-expression ]
            ++TP_POS(symbol_table);

            if ( ! (tmp_assignment_expression = tp_parse_assignment_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_TOKEN(tmp_static_or_asterisk),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }else if (IS_TOKEN_ASTERISK(tmp_static_or_asterisk)){

            // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list * ]
            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_TOKEN(tmp_static_or_asterisk),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }else{

            // Grammer: DirectDeclaratorTmp2 ->
            //     [ type-qualifier-list assignment-expression? ]

            tmp_assignment_expression = tp_parse_assignment_expression(symbol_table);

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                if (tmp_assignment_expression){

                    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list assignment-expression ]
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3,
                        TP_TREE_TOKEN(tmp_left_square_bracket),
                        TP_TREE_NODE(tmp_type_qualifier_list),
                        TP_TREE_NODE(tmp_assignment_expression),
                        TP_TREE_TOKEN(tmp_right_square_bracket)
                    );
                }else{

                    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list ]
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_4,
                        TP_TREE_TOKEN(tmp_left_square_bracket),
                        TP_TREE_NODE(tmp_type_qualifier_list),
                        TP_TREE_TOKEN(tmp_right_square_bracket)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_declarator_tmp2_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_parenthesis)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectDeclaratorTmp2 ->
    //     ( parameter-type-list ) |
    //     ( identifier-list? )
    {
        TP_PARSE_TREE* tmp_parameter_type_list = NULL;
        TP_PARSE_TREE* tmp_identifier_list = NULL;

        if (tmp_parameter_type_list = tp_parse_parameter_type_list(symbol_table)){

            ;
        }else{

            tmp_identifier_list = parse_identifier_list(symbol_table);
        }

        TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

            ++TP_POS(symbol_table);

            if (tmp_parameter_type_list){

                // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_NODE(tmp_parameter_type_list),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }else if (tmp_identifier_list){

                // Grammer: DirectDeclaratorTmp2 -> ( identifier-list )
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_2,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_NODE(tmp_identifier_list),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }else{

                // Grammer: DirectDeclaratorTmp2 -> ( )
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_3,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_parameter_type_list);
        tp_free_parse_subtree(symbol_table, &tmp_identifier_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_identifier_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: identifier-list -> identifier (, identifier)*
    {
        TP_PARSE_TREE* tmp_identifier_list1 = NULL;
        TP_PARSE_TREE* tmp_identifier_list2 = NULL;

        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            // Grammer: identifier-list -> identifier
            if ( ! (tmp_identifier_list1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_1,
                TP_TREE_TOKEN(tmp_identifier)))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            for (;;){

                TP_TOKEN* tmp_comma = TP_POS(symbol_table);

                if (IS_TOKEN_COMMA(tmp_comma)){

                    ++TP_POS(symbol_table);

                    if (tmp_identifier_list2 = parse_identifier_list(symbol_table)){

                        // Grammer: identifier-list -> identifier (, identifier)+
                        tmp_identifier_list1 = MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_2,
                            TP_TREE_NODE(tmp_identifier_list1),
                            TP_TREE_TOKEN(tmp_comma),
                            TP_TREE_NODE(tmp_identifier_list2)
                        );
                    }else{

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip_1;
                    }
                }else{

                    break;
                }
            }

            return tmp_identifier_list1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_identifier_list1);
        tp_free_parse_subtree(symbol_table, &tmp_identifier_list2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_initializer(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: initializer -> assignment-expression | { initializer-list ,? }
    {
        TP_PARSE_TREE* tmp_assignment_expression = NULL;
        TP_PARSE_TREE* tmp_initializer_list = NULL;

        if (tmp_assignment_expression = tp_parse_assignment_expression(symbol_table)){

            // Grammer: initializer -> assignment-expression
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INITIALIZER_1,
                TP_TREE_NODE(tmp_assignment_expression)
            );
        }else{

            TP_TOKEN* tmp_left_curly_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_CURLY_BRACKET(tmp_left_curly_bracket)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_initializer_list = tp_parse_initializer_list(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                bool is_comma = false;
                TP_TOKEN* tmp_comma = TP_POS(symbol_table);

                if (IS_TOKEN_COMMA(tmp_comma)){

                    ++TP_POS(symbol_table);
                    is_comma = true;
                }

                TP_TOKEN* tmp_right_curly_bracket = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_CURLY_BRACKET(tmp_right_curly_bracket)){

                    ++TP_POS(symbol_table);

                    if (is_comma){

                        // Grammer: initializer -> { initializer-list , }
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_INITIALIZER_2,
                            TP_TREE_TOKEN(tmp_left_curly_bracket),
                            TP_TREE_NODE(tmp_initializer_list),
                            TP_TREE_TOKEN(tmp_comma),
                            TP_TREE_TOKEN(tmp_right_curly_bracket)
                        );
                    }else{

                        // Grammer: initializer -> { initializer-list }
                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_INITIALIZER_3,
                            TP_TREE_TOKEN(tmp_left_curly_bracket),
                            TP_TREE_NODE(tmp_initializer_list),
                            TP_TREE_TOKEN(tmp_right_curly_bracket)
                        );
                    }
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);
        tp_free_parse_subtree(symbol_table, &tmp_initializer_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_initializer_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: initializer-list -> initializerListTmp1 (, initializerListTmp1)*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_initializer_list1 = NULL;
        TP_PARSE_TREE* tmp_initializer_list2 = NULL;

        if ( ! (tmp_initializer_list1 = parse_initializer_list_tmp1(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

               if (tmp_initializer_list2 = parse_initializer_list_tmp1(symbol_table)){

                    is_single = false;

                    // Grammer: initializer-list -> initializerListTmp1 (, initializerListTmp1)+
                    tmp_initializer_list1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_1,
                        TP_TREE_NODE(tmp_initializer_list1),
                        TP_TREE_NODE(tmp_initializer_list2)
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

            // Grammer: initializer-list -> initializerListTmp1
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_2,
                TP_TREE_NODE(tmp_initializer_list1)
            );
        }

        return tmp_initializer_list1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_initializer_list1);
        tp_free_parse_subtree(symbol_table, &tmp_initializer_list2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_initializer_list_tmp1(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: initializerListTmp1 -> designation? initializer
    {
        TP_PARSE_TREE* tmp_designation = NULL;
        TP_PARSE_TREE* tmp_initializer = NULL;

        tmp_designation = parse_designation(symbol_table);

        if ( ! (tmp_initializer = parse_initializer(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        if (tmp_designation){

            // Grammer: initializerListTmp1 -> designation initializer
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_1,
                TP_TREE_NODE(tmp_designation),
                TP_TREE_NODE(tmp_initializer)
            );
        }else{

            // Grammer: initializerListTmp1 -> initializer
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_2,
                TP_TREE_NODE(tmp_initializer)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_designation);
        tp_free_parse_subtree(symbol_table, &tmp_initializer);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_designation(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: designation -> designator-list =
    {
        TP_PARSE_TREE* tmp_designator_list = NULL;

        if ( ! (tmp_designator_list = parse_designator_list(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        TP_TOKEN* tmp_equal = TP_POS(symbol_table);

        if (IS_TOKEN_EQUAL(tmp_equal)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DESIGNATION_1,
                TP_TREE_NODE(tmp_designator_list),
                TP_TREE_TOKEN(tmp_equal)
            );
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_designator_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_designator_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: designator-list -> designator+
    {
        TP_PARSE_TREE* tmp_designator1 = NULL;
        TP_PARSE_TREE* tmp_designator2 = NULL;

        if ( ! (tmp_designator1 = parse_designator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_designator2 = parse_designator(symbol_table)){

            tmp_designator1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DESIGNATOR_LIST_1,
                TP_TREE_NODE(tmp_designator1),
                TP_TREE_NODE(tmp_designator2)
            );
        }

        return tmp_designator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_designator1);
        tp_free_parse_subtree(symbol_table, &tmp_designator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_designator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: designator -> [ constant-expression ] | . identifier
    {
        TP_PARSE_TREE* tmp_constant_expression = NULL;

        TP_TOKEN* tmp_left_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_SQUARE_BRACKET(tmp_left_square_bracket)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_constant_expression = tp_parse_constant_expression(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                // Grammer: designator -> [ constant-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DESIGNATOR_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_constant_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }

        TP_TOKEN* tmp_period = TP_POS(symbol_table);

        if (IS_TOKEN_PERIOD(tmp_period)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

            if (IS_TOKEN_ID(tmp_identifier)){

                ++TP_POS(symbol_table);

                // Grammer: designator -> . identifier
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DESIGNATOR_2,
                    TP_TREE_TOKEN(tmp_period),
                    TP_TREE_TOKEN(tmp_identifier)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_constant_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

