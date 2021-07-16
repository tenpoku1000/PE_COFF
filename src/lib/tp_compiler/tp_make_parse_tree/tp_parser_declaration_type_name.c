
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
// Grammer(type-name):
// 
// specifier-qualifier-list -> (type-specifier | type-qualifier) specifier-qualifier-list?
// 
// type-specifier -> void | char | short | int | long |
//     float | double | signed | unsigned | _Bool | _Complex |
//     atomic-type-specifier | struct-or-union-specifier |
//     enum-specifier | typedef-name
// 
// atomic-type-specifier -> _Atomic ( type-name )
// 
// enum-specifier -> enum (identifier | identifier? { enumerator-list ,? } )
// 
// enumerator-list -> enumerator (, enumerator)*
// 
// enumerator -> identifier (= constant-expression)?
// 
// typedef-name -> identifier
// 
// struct-or-union-specifier -> struct-or-union (identifier | identifier? { struct-declaration-list } )
// 
// struct-or-union -> struct | union
// 
// struct-declaration-list -> struct-declaration+
// 
// struct-declaration -> specifier-qualifier-list struct-declarator-list? ; |
//     static_assert-declaration
// 
// struct-declarator-list -> struct-declarator (, struct-declarator)*
// 
// struct-declarator -> declarator | declarator? : constant-expression
// 
// abstract-declarator -> pointer | pointer? direct-abstract-declarator
// 
// pointer -> * type-qualifier-list? pointer?
// 
// type-qualifier-list -> type-qualifier+
// 
// type-qualifier -> const | restrict | volatile | _Atomic
// 
// static_assert-declaration -> _Static_assert ( constant-expression , string-literal ) ;
// 
// direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2* |
//     DirectAbstractDeclaratorTmp2
// 
// DirectAbstractDeclaratorTmp1 -> ( abstract-declarator )
// 
// DirectAbstractDeclaratorTmp2 ->
//     [ type-qualifier-list? assignment-expression? ] |
//     [ static type-qualifier-list? assignment-expression ] |
//     [ type-qualifier-list static assignment-expression ] |
//     [*] |
//     ( parameter-type-list? )
// 
// parameter-type-list -> parameter-list ParameterTypeListVararg?
// 
// ParameterTypeListVararg -> , ...
// 
// parameter-list -> parameter-declaration (, parameter-declaration)*
// 
// parameter-declaration -> declaration-specifiers (declarator | abstract-declarator?)
// 

// ----------------------------------------------------------------------------------------
// Grammer(type-name):
// 
static TP_PARSE_TREE* parse_specifier_qualifier_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_atomic_type_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_enum_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_enumerator_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_enumerator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_typedef_name(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_or_union_specifier(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_or_union(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_declaration_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_declaration(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_declarator_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_struct_declarator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_abstract_declarator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_abstract_declarator(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp1(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket
);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_static(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_TOKEN* tmp_static
);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_other(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket
);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_other_qualifier_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_PARSE_TREE* tmp_type_qualifier_list
);
static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_parenthesis
);
static TP_PARSE_TREE* parse_parameter_list(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_parameter_list_vararg(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_parameter_declaration(TP_SYMBOL_TABLE* symbol_table);

// ----------------------------------------------------------------------------------------
// Grammer(type-name):
// 

TP_PARSE_TREE* tp_parse_type_name(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: type-name -> specifier-qualifier-list abstract-declarator?
    {
        TP_PARSE_TREE* tmp_specifier_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_abstract_declarator = NULL;

        if ( ! (tmp_specifier_qualifier_list = parse_specifier_qualifier_list(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        tmp_abstract_declarator = parse_abstract_declarator(symbol_table);

        if (tmp_abstract_declarator){

            // Grammer: type-name -> specifier-qualifier-list abstract-declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_NAME_1,
                TP_TREE_NODE(tmp_specifier_qualifier_list),
                TP_TREE_NODE(tmp_abstract_declarator)
            );
        }else{

            // Grammer: type-name -> specifier-qualifier-list
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_NAME_2,
                TP_TREE_NODE(tmp_specifier_qualifier_list)
            );
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_specifier_qualifier_list);
        tp_free_parse_subtree(symbol_table, &tmp_abstract_declarator);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_specifier_qualifier_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: specifier-qualifier-list -> (type-specifier | type-qualifier) specifier-qualifier-list?
    {
        TP_PARSE_TREE* tmp_type_specifier = NULL;
        TP_PARSE_TREE* tmp_type_qualifier = NULL;
        TP_PARSE_TREE* tmp_specifier_qualifier_list = NULL;

        if (tmp_type_specifier = tp_parse_type_specifier(symbol_table)){

            if (tmp_specifier_qualifier_list = parse_specifier_qualifier_list(symbol_table)){

                // Grammer: specifier-qualifier-list -> type-specifier specifier-qualifier-list
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_1,
                    TP_TREE_NODE(tmp_type_specifier),
                    TP_TREE_NODE(tmp_specifier_qualifier_list)
                );
            }else{

                // Grammer: specifier-qualifier-list -> type-specifier
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_2,
                    TP_TREE_NODE(tmp_type_specifier)
                );
            }
        }else if (tmp_type_qualifier = tp_parse_type_qualifier(symbol_table)){

            if (tmp_specifier_qualifier_list = parse_specifier_qualifier_list(symbol_table)){

                // Grammer: specifier-qualifier-list -> type-qualifier specifier-qualifier-list
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_3,
                    TP_TREE_NODE(tmp_type_qualifier),
                    TP_TREE_NODE(tmp_specifier_qualifier_list)
                );
            }else{

                // Grammer: specifier-qualifier-list -> type-qualifier
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_4,
                    TP_TREE_NODE(tmp_type_qualifier)
                );
            }
        }

        tp_free_parse_subtree(symbol_table, &tmp_type_specifier);
        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier);
        tp_free_parse_subtree(symbol_table, &tmp_specifier_qualifier_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_type_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: type-specifier -> void | char | short | int | long |
    //     float | double | signed | unsigned | _Bool | _Complex |
    //     atomic-type-specifier | struct-or-union-specifier |
    //     enum-specifier | typedef-name
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_atomic_type_specifier = NULL;
        TP_PARSE_TREE* tmp_struct_or_union_specifier = NULL;
        TP_PARSE_TREE* tmp_enum_specifier = NULL;
        TP_PARSE_TREE* tmp_typedef_name = NULL;

        // Grammer: type-specifier -> void | char | short | int | long |
        //     float | double | signed | unsigned | _Bool | _Complex
        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_VOID(tmp_token) || IS_TOKEN_KEYWORD_CHAR(tmp_token) ||
            IS_TOKEN_KEYWORD_SHORT(tmp_token) || IS_TOKEN_KEYWORD_INT(tmp_token) ||
            IS_TOKEN_KEYWORD_LONG(tmp_token) || IS_TOKEN_KEYWORD_FLOAT(tmp_token) ||
            IS_TOKEN_KEYWORD_DOUBLE(tmp_token) || IS_TOKEN_KEYWORD_SIGNED(tmp_token) ||
            IS_TOKEN_KEYWORD_UNSIGNED(tmp_token) || IS_TOKEN_KEYWORD_BOOL(tmp_token) ||
            IS_TOKEN_KEYWORD_COMPLEX(tmp_token)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1,
                TP_TREE_TOKEN(tmp_token)
            );
        }else if (tmp_atomic_type_specifier = parse_atomic_type_specifier(symbol_table)){

            // Grammer: type-specifier -> atomic-type-specifier
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_2,
                TP_TREE_NODE(tmp_atomic_type_specifier)
            );
        }else if (tmp_struct_or_union_specifier = parse_struct_or_union_specifier(symbol_table)){

            // Grammer: type-specifier -> struct-or-union-specifier
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_3,
                TP_TREE_NODE(tmp_struct_or_union_specifier)
            );
        }else if (tmp_enum_specifier = parse_enum_specifier(symbol_table)){

            // Grammer: type-specifier -> enum-specifier
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_4,
                TP_TREE_NODE(tmp_enum_specifier)
            );
// ToDo:
//      }else if (tmp_typedef_name = parse_typedef_name(symbol_table)){
//
//          // Grammer: type-specifier -> typedef-name
//          return MAKE_PARSE_SUBTREE(
//              symbol_table,
//              TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_5,
//              TP_TREE_NODE(tmp_typedef_name)
//          );
        }

        tp_free_parse_subtree(symbol_table, &tmp_atomic_type_specifier);
        tp_free_parse_subtree(symbol_table, &tmp_struct_or_union_specifier);
        tp_free_parse_subtree(symbol_table, &tmp_enum_specifier);
        tp_free_parse_subtree(symbol_table, &tmp_typedef_name);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_atomic_type_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: atomic-type-specifier -> _Atomic ( type-name )
    {
        TP_PARSE_TREE* tmp_type_name = NULL;

        TP_TOKEN* tmp_atomic = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_ATOMIC(tmp_atomic)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_type_name = tp_parse_type_name(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_ATOMIC_TYPE_SPECIFIER_1,
                        TP_TREE_TOKEN(tmp_atomic),
                        TP_TREE_TOKEN(tmp_left_parenthesis),
                        TP_TREE_NODE(tmp_type_name),
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
        tp_free_parse_subtree(symbol_table, &tmp_type_name);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_enum_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: enum-specifier -> enum (identifier | identifier? { enumerator-list ,? } )
    {
        TP_PARSE_TREE* tmp_enumerator_list = NULL;

        TP_TOKEN* tmp_enum = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_ENUM(tmp_enum)){

            ++TP_POS(symbol_table);

            bool is_identifier = false;
            TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

            if (IS_TOKEN_ID(tmp_identifier)){

                ++TP_POS(symbol_table);
                is_identifier = true;
            }

            TP_TOKEN* tmp_left_curly_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_CURLY_BRACKET(tmp_left_curly_bracket)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_enumerator_list = parse_enumerator_list(symbol_table))){

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

                    if (is_identifier){

                        if (is_comma){

                            // Grammer: enum-specifier -> enum identifier { enumerator-list , }
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_1,
                                TP_TREE_TOKEN(tmp_enum),
                                TP_TREE_TOKEN(tmp_identifier),
                                TP_TREE_TOKEN(tmp_left_curly_bracket),
                                TP_TREE_NODE(tmp_enumerator_list),
                                TP_TREE_TOKEN(tmp_comma),
                                TP_TREE_TOKEN(tmp_right_curly_bracket)
                            );
                        }else{

                            // Grammer: enum-specifier -> enum identifier { enumerator-list }
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_2,
                                TP_TREE_TOKEN(tmp_enum),
                                TP_TREE_TOKEN(tmp_identifier),
                                TP_TREE_TOKEN(tmp_left_curly_bracket),
                                TP_TREE_NODE(tmp_enumerator_list),
                                TP_TREE_TOKEN(tmp_right_curly_bracket)
                            );
                        }
                    }else{

                        if (is_comma){

                            // Grammer: enum-specifier -> enum { enumerator-list , }
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_3,
                                TP_TREE_TOKEN(tmp_enum),
                                TP_TREE_TOKEN(tmp_left_curly_bracket),
                                TP_TREE_NODE(tmp_enumerator_list),
                                TP_TREE_TOKEN(tmp_comma),
                                TP_TREE_TOKEN(tmp_right_curly_bracket)
                            );
                        }else{

                            // Grammer: enum-specifier -> enum { enumerator-list }
                            return MAKE_PARSE_SUBTREE(
                                symbol_table,
                                TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_4,
                                TP_TREE_TOKEN(tmp_enum),
                                TP_TREE_TOKEN(tmp_left_curly_bracket),
                                TP_TREE_NODE(tmp_enumerator_list),
                                TP_TREE_TOKEN(tmp_right_curly_bracket)
                            );
                        }
                    }
                }else{

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }
            }else if (is_identifier){

                // Grammer: enum-specifier -> enum identifier
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_5,
                    TP_TREE_TOKEN(tmp_enum),
                    TP_TREE_TOKEN(tmp_identifier)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_enumerator_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_enumerator_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: enumerator-list -> enumerator (, enumerator)*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_enumerator1 = NULL;
        TP_PARSE_TREE* tmp_enumerator2 = NULL;

        if ( ! (tmp_enumerator1 = parse_enumerator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_enumerator2 = parse_enumerator(symbol_table)){

                    is_single = false;

                    // Grammer: enumerator-list -> enumerator (, enumerator)+
                    tmp_enumerator1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_1,
                        TP_TREE_NODE(tmp_enumerator1),
                        TP_TREE_NODE(tmp_enumerator2)
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

            // Grammer: enumerator-list -> enumerator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_2,
                TP_TREE_NODE(tmp_enumerator1)
            );
        }

        return tmp_enumerator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_enumerator1);
        tp_free_parse_subtree(symbol_table, &tmp_enumerator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_enumerator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: enumerator -> identifier (= constant-expression)?
    {
        TP_PARSE_TREE* tmp_constant_expression = NULL;

        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_equal = TP_POS(symbol_table);

            if (IS_TOKEN_EQUAL(tmp_equal)){

                ++TP_POS(symbol_table);

                if ( ! (tmp_constant_expression = tp_parse_constant_expression(symbol_table))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                // Grammer: enumerator -> identifier = constant-expression
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ENUMERATOR_1,
                    TP_TREE_TOKEN(tmp_identifier),
                    TP_TREE_TOKEN(tmp_equal),
                    TP_TREE_NODE(tmp_constant_expression)
                );
            }else{

                // Grammer: enumerator -> identifier
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ENUMERATOR_2,
                    TP_TREE_TOKEN(tmp_identifier)
                );
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_constant_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_typedef_name(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: typedef-name -> identifier
    {
        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPEDEF_NAME_1,
                TP_TREE_TOKEN(tmp_identifier)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_or_union_specifier(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-or-union-specifier -> struct-or-union (identifier | identifier? { struct-declaration-list } )
    {
        TP_PARSE_TREE* tmp_struct_or_union = NULL;
        TP_PARSE_TREE* tmp_struct_declaration_list = NULL;

        if ( ! (tmp_struct_or_union = parse_struct_or_union(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        bool is_identifier = false;
        TP_TOKEN* tmp_identifier = TP_POS(symbol_table);

        if (IS_TOKEN_ID(tmp_identifier)){

            ++TP_POS(symbol_table);
            is_identifier = true;
        }

        TP_TOKEN* tmp_left_curly_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_CURLY_BRACKET(tmp_left_curly_bracket)){

            ++TP_POS(symbol_table);

            if ( ! (tmp_struct_declaration_list = parse_struct_declaration_list(symbol_table))){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }

            TP_TOKEN* tmp_right_curly_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_CURLY_BRACKET(tmp_right_curly_bracket)){

                ++TP_POS(symbol_table);

                if (is_identifier){

                    // Grammer: struct-or-union-specifier -> struct-or-union identifier { struct-declaration-list }
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_1,
                        TP_TREE_NODE(tmp_struct_or_union),
                        TP_TREE_TOKEN(tmp_identifier),
                        TP_TREE_TOKEN(tmp_left_curly_bracket),
                        TP_TREE_NODE(tmp_struct_declaration_list),
                        TP_TREE_TOKEN(tmp_right_curly_bracket)
                    );
                }else{

                    // Grammer: struct-or-union-specifier -> struct-or-union { struct-declaration-list }
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_2,
                        TP_TREE_NODE(tmp_struct_or_union),
                        TP_TREE_TOKEN(tmp_left_curly_bracket),
                        TP_TREE_NODE(tmp_struct_declaration_list),
                        TP_TREE_TOKEN(tmp_right_curly_bracket)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }else if (is_identifier){

            // Grammer: struct-or-union-specifier -> struct-or-union identifier
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_3,
                TP_TREE_NODE(tmp_struct_or_union),
                TP_TREE_TOKEN(tmp_identifier)
            );
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_struct_or_union);
        tp_free_parse_subtree(symbol_table, &tmp_struct_declaration_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_or_union(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-or-union -> struct | union
    {
        TP_TOKEN* tmp_struct_or_union = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STRUCT(tmp_struct_or_union) ||
            IS_TOKEN_KEYWORD_UNION(tmp_struct_or_union)){

            ++TP_POS(symbol_table);

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_1,
                TP_TREE_TOKEN(tmp_struct_or_union)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_declaration_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-declaration-list -> struct-declaration+
    {
        TP_PARSE_TREE* tmp_struct_declaration1 = NULL;
        TP_PARSE_TREE* tmp_struct_declaration2 = NULL;

        if ( ! (tmp_struct_declaration1 = parse_struct_declaration(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_struct_declaration2 = parse_struct_declaration(symbol_table)){

            tmp_struct_declaration1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_LIST_1,
                TP_TREE_NODE(tmp_struct_declaration1),
                TP_TREE_NODE(tmp_struct_declaration2)
            );
        }

        return tmp_struct_declaration1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_struct_declaration1);
        tp_free_parse_subtree(symbol_table, &tmp_struct_declaration2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_declaration(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: struct-declaration -> specifier-qualifier-list struct-declarator-list? ; |
    //     static_assert-declaration
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-declaration -> specifier-qualifier-list struct-declarator-list? ;
    {
        TP_PARSE_TREE* tmp_specifier_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_struct_declarator_list = NULL;

        if ( ! (tmp_specifier_qualifier_list = parse_specifier_qualifier_list(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        tmp_struct_declarator_list = parse_struct_declarator_list(symbol_table);

        TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

        if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

            ++TP_POS(symbol_table);

            if (tmp_struct_declarator_list){

                // Grammer: struct-declaration -> specifier-qualifier-list struct-declarator-list ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_1,
                    TP_TREE_NODE(tmp_specifier_qualifier_list),
                    TP_TREE_NODE(tmp_struct_declarator_list),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                // Grammer: struct-declaration -> specifier-qualifier-list ;
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_2,
                    TP_TREE_NODE(tmp_specifier_qualifier_list),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_specifier_qualifier_list);
        tp_free_parse_subtree(symbol_table, &tmp_struct_declarator_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: struct-declaration -> static_assert-declaration
    {
        TP_PARSE_TREE* tmp_static_assert_declaration = tp_parse_static_assert_declaration(symbol_table);

        if (tmp_static_assert_declaration){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_3,
                TP_TREE_NODE(tmp_static_assert_declaration)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_declarator_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-declarator-list -> struct-declarator (, struct-declarator)*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_struct_declarator1 = NULL;
        TP_PARSE_TREE* tmp_struct_declarator2 = NULL;

        if ( ! (tmp_struct_declarator1 = parse_struct_declarator(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_struct_declarator2 = parse_struct_declarator(symbol_table)){

                    is_single = false;

                    // Grammer: struct-declarator-list -> struct-declarator (, struct-declarator)+
                    tmp_struct_declarator1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_1,
                        TP_TREE_NODE(tmp_struct_declarator1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_struct_declarator2)
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

            // Grammer: struct-declarator-list -> struct-declarator
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_2,
                TP_TREE_NODE(tmp_struct_declarator1)
            );
        }

        return tmp_struct_declarator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_struct_declarator1);
        tp_free_parse_subtree(symbol_table, &tmp_struct_declarator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_struct_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: struct-declarator -> declarator | declarator? : constant-expression
    {
        TP_PARSE_TREE* tmp_declarator = NULL;
        TP_PARSE_TREE* tmp_constant_expression = NULL;

        tmp_declarator = tp_parse_declarator(symbol_table);

        TP_TOKEN* tmp_colon = TP_POS(symbol_table);

        if (IS_TOKEN_COLON(tmp_colon)){

            ++TP_POS(symbol_table);

            if (tmp_constant_expression = tp_parse_constant_expression(symbol_table)){

                if (tmp_declarator){

                    // Grammer: struct-declarator -> declarator : constant-expression
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_1,
                        TP_TREE_NODE(tmp_declarator),
                        TP_TREE_TOKEN(tmp_colon),
                        TP_TREE_NODE(tmp_constant_expression)
                    );
                }else{

                    // Grammer: struct-declarator -> : constant-expression
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_2,
                        TP_TREE_TOKEN(tmp_colon),
                        TP_TREE_NODE(tmp_constant_expression)
                    );
                }
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }else{

            if (tmp_declarator){

                // Grammer: struct-declarator -> declarator
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_3,
                    TP_TREE_NODE(tmp_declarator)
                );
            }
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_declarator);
        tp_free_parse_subtree(symbol_table, &tmp_constant_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_abstract_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: abstract-declarator -> pointer | pointer? direct-abstract-declarator
    {
        TP_PARSE_TREE* tmp_pointer = NULL;
        TP_PARSE_TREE* tmp_direct_abstract_declarator = NULL;

        tmp_pointer = tp_parse_pointer(symbol_table);

        if (tmp_direct_abstract_declarator = parse_direct_abstract_declarator(symbol_table)){

            if (tmp_pointer){

                // Grammer: abstract-declarator -> pointer direct-abstract-declarator
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_1,
                    TP_TREE_NODE(tmp_pointer),
                    TP_TREE_NODE(tmp_direct_abstract_declarator)
                );
            }else{

                // Grammer: abstract-declarator -> direct-abstract-declarator
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_2,
                    TP_TREE_NODE(tmp_direct_abstract_declarator)
                );
            }
        }else{

            if (tmp_pointer){

                // Grammer: abstract-declarator -> pointer
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_3,
                    TP_TREE_NODE(tmp_pointer)
                );
            }
        }

        tp_free_parse_subtree(symbol_table, &tmp_pointer);
        tp_free_parse_subtree(symbol_table, &tmp_direct_abstract_declarator);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_pointer(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: pointer -> * type-qualifier-list? pointer?
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_type_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_pointer = NULL;

        TP_TOKEN* tmp_asterisk = TP_POS(symbol_table);

        if (IS_TOKEN_ASTERISK(tmp_asterisk)){

            ++TP_POS(symbol_table);

            tmp_type_qualifier_list = tp_parse_type_qualifier_list(symbol_table);

            tmp_pointer = tp_parse_pointer(symbol_table);

            if (tmp_type_qualifier_list && tmp_pointer){

                // Grammer: pointer -> * type-qualifier-list pointer
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POINTER_1,
                    TP_TREE_TOKEN(tmp_asterisk),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_NODE(tmp_pointer)
                );
            }else if (tmp_type_qualifier_list){

                // Grammer: pointer -> * type-qualifier-list
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POINTER_2,
                    TP_TREE_TOKEN(tmp_asterisk),
                    TP_TREE_NODE(tmp_type_qualifier_list)
                );
            }else if (tmp_pointer){

                // Grammer: pointer -> * pointer
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POINTER_3,
                    TP_TREE_TOKEN(tmp_asterisk),
                    TP_TREE_NODE(tmp_pointer)
                );
            }else{

                // Grammer: pointer -> *
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_POINTER_4,
                    TP_TREE_TOKEN(tmp_asterisk)
                );
            }
        }

        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier_list);
        tp_free_parse_subtree(symbol_table, &tmp_pointer);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_type_qualifier_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: type-qualifier-list -> type-qualifier+
    {
        TP_PARSE_TREE* tmp_type_qualifier1 = NULL;
        TP_PARSE_TREE* tmp_type_qualifier2 = NULL;

        if ( ! (tmp_type_qualifier1 = tp_parse_type_qualifier(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_type_qualifier2 = tp_parse_type_qualifier(symbol_table)){

            tmp_type_qualifier1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_LIST_1,
                TP_TREE_NODE(tmp_type_qualifier1),
                TP_TREE_NODE(tmp_type_qualifier2)
            );
        }

        return tmp_type_qualifier1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier1);
        tp_free_parse_subtree(symbol_table, &tmp_type_qualifier2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_type_qualifier(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: type-qualifier -> const | restrict | volatile | _Atomic
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_TOKEN* tmp_token = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_CONST(tmp_token) || IS_TOKEN_KEYWORD_RESTRICT(tmp_token) ||
            IS_TOKEN_KEYWORD_VOLATILE(tmp_token) || IS_TOKEN_KEYWORD_ATOMIC(tmp_token)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_1,
                TP_TREE_TOKEN(tmp_token)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_static_assert_declaration(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: static_assert-declaration -> _Static_assert ( constant-expression , string-literal ) ;
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_constant_expression = NULL;

        TP_TOKEN* tmp_static_assert = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STATIC_ASSERT(tmp_static_assert)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

            if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

                ++TP_POS(symbol_table);

                if (tmp_constant_expression = tp_parse_constant_expression(symbol_table)){

                    TP_TOKEN* tmp_comma = TP_POS(symbol_table);

                    if (IS_TOKEN_COMMA(tmp_comma)){

                        ++TP_POS(symbol_table);

                        TP_TOKEN* tmp_string_literal = TP_POS(symbol_table);

                        if (IS_TOKEN_STRING_LITERAL(tmp_string_literal)){

                            ++TP_POS(symbol_table);

                            TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

                            if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                                ++TP_POS(symbol_table);

                                TP_TOKEN* tmp_semicolon = TP_POS(symbol_table);

                                if (IS_TOKEN_SEMICOLON(tmp_semicolon)){

                                    ++TP_POS(symbol_table);

                                    return MAKE_PARSE_SUBTREE(
                                        symbol_table,
                                        TP_PARSE_TREE_GRAMMER_STATIC_ASSERT_DECLARATION_1,
                                        TP_TREE_TOKEN(tmp_static_assert),
                                        TP_TREE_TOKEN(tmp_left_parenthesis),
                                        TP_TREE_NODE(tmp_constant_expression),
                                        TP_TREE_TOKEN(tmp_comma),
                                        TP_TREE_TOKEN(tmp_string_literal),
                                        TP_TREE_TOKEN(tmp_right_parenthesis),
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
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_constant_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2* |
    //     DirectAbstractDeclaratorTmp2
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_direct_abstract_declarator1 = NULL;
        TP_PARSE_TREE* tmp_direct_abstract_declarator2 = NULL;

        if ( ! (tmp_direct_abstract_declarator1 = parse_direct_abstract_declarator_tmp1(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        while (tmp_direct_abstract_declarator2 = parse_direct_abstract_declarator_tmp2(symbol_table)){

            is_single = false;

            // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2+
            tmp_direct_abstract_declarator1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_1,
                TP_TREE_NODE(tmp_direct_abstract_declarator1),
                TP_TREE_NODE(tmp_direct_abstract_declarator2)
            );
        }

        if (is_single){

            // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_2,
                TP_TREE_NODE(tmp_direct_abstract_declarator1),
            );
        }

        return tmp_direct_abstract_declarator1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_direct_abstract_declarator1);
        tp_free_parse_subtree(symbol_table, &tmp_direct_abstract_declarator2);

        TP_POS(symbol_table) = backup_token_position;
    }

    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp2
    {
        TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2 = NULL;

        if (tmp_direct_abstract_declarator_tmp2 = parse_direct_abstract_declarator_tmp2(symbol_table)){

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_3,
                TP_TREE_NODE(tmp_direct_abstract_declarator_tmp2)
            );
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp1(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: DirectAbstractDeclaratorTmp1 -> ( abstract-declarator )
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_direct_abstract_declarator = NULL;

        TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

            ++TP_POS(symbol_table);

            if (tmp_direct_abstract_declarator = parse_abstract_declarator(symbol_table)){

                TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

                if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

                    ++TP_POS(symbol_table);

                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP1_1,
                        TP_TREE_TOKEN(tmp_left_parenthesis),
                        TP_TREE_NODE(tmp_direct_abstract_declarator),
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
        tp_free_parse_subtree(symbol_table, &tmp_direct_abstract_declarator);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ static type-qualifier-list? assignment-expression ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [*] |
    //     ( parameter-type-list? )
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        // Grammer: DirectAbstractDeclaratorTmp2 ->
        //     [ type-qualifier-list? assignment-expression? ] |
        //     [ static type-qualifier-list? assignment-expression ] |
        //     [ type-qualifier-list static assignment-expression ] |
        //     [*]
        TP_TOKEN* tmp_left_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_SQUARE_BRACKET(tmp_left_square_bracket)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2_square =
                parse_direct_abstract_declarator_tmp2_square(symbol_table, tmp_left_square_bracket);

            if (tmp_direct_abstract_declarator_tmp2_square){

                return tmp_direct_abstract_declarator_tmp2_square;
            }
        }

        // Grammer: DirectAbstractDeclaratorTmp2 ->
        //     ( parameter-type-list? )
        TP_TOKEN* tmp_left_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_parenthesis)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2_paren =
                parse_direct_abstract_declarator_tmp2_paren(symbol_table, tmp_left_parenthesis);

            if (tmp_direct_abstract_declarator_tmp2_paren){

                return tmp_direct_abstract_declarator_tmp2_paren;
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket)
{
    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ static type-qualifier-list? assignment-expression ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [*]
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        // Grammer: DirectAbstractDeclaratorTmp2 ->
        //     [ static type-qualifier-list? assignment-expression ]
        TP_TOKEN* tmp_static = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STATIC(tmp_static)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2_square_static
                = parse_direct_abstract_declarator_tmp2_square_static(
                    symbol_table, tmp_left_square_bracket, tmp_static
                );

            if (tmp_direct_abstract_declarator_tmp2_square_static){

                return tmp_direct_abstract_declarator_tmp2_square_static;
            }
        }

        // Grammer: DirectAbstractDeclaratorTmp2 ->
        //     [ type-qualifier-list? assignment-expression? ] |
        //     [ type-qualifier-list static assignment-expression ] |
        //     [*]
        TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2_square_other
            = parse_direct_abstract_declarator_tmp2_square_other(
                symbol_table, tmp_left_square_bracket
            );

        if (tmp_direct_abstract_declarator_tmp2_square_other){

            return tmp_direct_abstract_declarator_tmp2_square_other;
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_static(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_TOKEN* tmp_static)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectAbstractDeclaratorTmp2 ->
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

                // Grammer: DirectAbstractDeclaratorTmp2 ->
                //     [ static type-qualifier-list assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_TOKEN(tmp_static),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                // Grammer: DirectAbstractDeclaratorTmp2 -> [ static assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_2,
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

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_other(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_square_bracket)
{
    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list? assignment-expression? ] |
    //     [ type-qualifier-list static assignment-expression ] |
    //     [*]
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    {
        TP_PARSE_TREE* tmp_direct_abstract_declarator_tmp2_square_other_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_type_qualifier_list = NULL;
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        tmp_type_qualifier_list = tp_parse_type_qualifier_list(symbol_table);

        // Grammer: DirectAbstractDeclaratorTmp2 ->
        //     [ type-qualifier-list assignment-expression? ] |
        //     [ type-qualifier-list static assignment-expression ]
        if (tmp_type_qualifier_list){

            tmp_direct_abstract_declarator_tmp2_square_other_qualifier_list
                = parse_direct_abstract_declarator_tmp2_square_other_qualifier_list(
                    symbol_table,
                    tmp_left_square_bracket, tmp_type_qualifier_list
                );

            if (tmp_direct_abstract_declarator_tmp2_square_other_qualifier_list){

                return tmp_direct_abstract_declarator_tmp2_square_other_qualifier_list;
            }
        }

        // Grammer: DirectAbstractDeclaratorTmp2 -> [*]
        TP_TOKEN* tmp_asterisk = TP_POS(symbol_table);

        if (IS_TOKEN_ASTERISK(tmp_asterisk)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

            if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_TOKEN(tmp_asterisk),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }
        }

        // Grammer: DirectAbstractDeclaratorTmp2 -> [ assignment-expression? ]
        tmp_assignment_expression = tp_parse_assignment_expression(symbol_table);

        TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

            ++TP_POS(symbol_table);

            if (tmp_assignment_expression){

                // Grammer: DirectAbstractDeclaratorTmp2 -> [ assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_2,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                // Grammer: DirectAbstractDeclaratorTmp2 -> [ ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_3,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
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

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_square_other_qualifier_list(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* tmp_left_square_bracket, TP_PARSE_TREE* tmp_type_qualifier_list)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectAbstractDeclaratorTmp2 -> [ type-qualifier-list assignment-expression? ] |
    //     [ type-qualifier-list static assignment-expression ]
    {
        TP_PARSE_TREE* tmp_assignment_expression = NULL;

        bool is_static = false;
        TP_TOKEN* tmp_static = TP_POS(symbol_table);

        if (IS_TOKEN_KEYWORD_STATIC(tmp_static)){

            ++TP_POS(symbol_table);
            is_static = true;
        }

        tmp_assignment_expression = tp_parse_assignment_expression(symbol_table);

        TP_TOKEN* tmp_right_square_bracket = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_SQUARE_BRACKET(tmp_right_square_bracket)){

            ++TP_POS(symbol_table);

            if (is_static && tmp_assignment_expression){

                // Grammer: DirectAbstractDeclaratorTmp2 ->
                //     [ type-qualifier-list static assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_TOKEN(tmp_static),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else if (is_static){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_1;
            }else if (tmp_assignment_expression){

                // Grammer: DirectAbstractDeclaratorTmp2 ->
                //     [ type-qualifier-list assignment-expression ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_NODE(tmp_assignment_expression),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }else{

                // Grammer: DirectAbstractDeclaratorTmp2 -> [ type-qualifier-list ]
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3,
                    TP_TREE_TOKEN(tmp_left_square_bracket),
                    TP_TREE_NODE(tmp_type_qualifier_list),
                    TP_TREE_TOKEN(tmp_right_square_bracket)
                );
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_assignment_expression);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_direct_abstract_declarator_tmp2_paren(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* tmp_left_parenthesis)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: DirectAbstractDeclaratorTmp2 -> ( parameter-type-list? )
    {
        TP_PARSE_TREE* tmp_parameter_type_list = NULL;

        tmp_parameter_type_list = tp_parse_parameter_type_list(symbol_table);

        TP_TOKEN* tmp_right_parenthesis = TP_POS(symbol_table);

        if (IS_TOKEN_RIGHT_PAREN(tmp_right_parenthesis)){

            ++TP_POS(symbol_table);

            if (tmp_parameter_type_list){

                // Grammer: DirectAbstractDeclaratorTmp2 -> ( parameter-type-list )
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_1,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_NODE(tmp_parameter_type_list),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }else{

                // Grammer: DirectAbstractDeclaratorTmp2 -> ( )
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_2,
                    TP_TREE_TOKEN(tmp_left_parenthesis),
                    TP_TREE_TOKEN(tmp_right_parenthesis)
                );
            }
        }

        tp_free_parse_subtree(symbol_table, &tmp_parameter_type_list);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

TP_PARSE_TREE* tp_parse_parameter_type_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: parameter-type-list -> parameter-list ParameterTypeListVararg?
    {
        TP_PARSE_TREE* tmp_parameter_list = NULL;

        if (tmp_parameter_list = parse_parameter_list(symbol_table)){

            TP_PARSE_TREE* tmp_vararg = NULL;

            if (tmp_vararg = parse_parameter_list_vararg(symbol_table)){

                // Grammer: parameter-type-list -> parameter-list ParameterTypeListVararg
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_1,
                    TP_TREE_NODE(tmp_parameter_list),
                    TP_TREE_NODE(tmp_vararg)
                );
            }else{

                // Grammer: parameter-type-list -> parameter-list
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2,
                    TP_TREE_NODE(tmp_parameter_list)
                );
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_parameter_list(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)*
    {
        bool is_single = true;
        TP_PARSE_TREE* tmp_parameter_declaration1 = NULL;
        TP_PARSE_TREE* tmp_parameter_declaration2 = NULL;

        if ( ! (tmp_parameter_declaration1 = parse_parameter_declaration(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto skip_1;
        }

        for (;;){

            TP_TOKEN* tmp_comma = TP_POS(symbol_table);

            if (IS_TOKEN_COMMA(tmp_comma)){

                ++TP_POS(symbol_table);

                if (tmp_parameter_declaration2 = parse_parameter_declaration(symbol_table)){

                    is_single = false;

                    // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
                    tmp_parameter_declaration1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1,
                        TP_TREE_NODE(tmp_parameter_declaration1),
                        TP_TREE_TOKEN(tmp_comma),
                        TP_TREE_NODE(tmp_parameter_declaration2)
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

            // Grammer: parameter-list -> parameter-declaration
            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_2,
                TP_TREE_NODE(tmp_parameter_declaration1)
            );
        }

        return tmp_parameter_declaration1;
skip_1:
        tp_free_parse_subtree(symbol_table, &tmp_parameter_declaration1);
        tp_free_parse_subtree(symbol_table, &tmp_parameter_declaration2);

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_parameter_list_vararg(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: ParameterTypeListVararg -> , ...
    {
        TP_TOKEN* tmp_comma = TP_POS(symbol_table);

        if (IS_TOKEN_COMMA(tmp_comma)){

            ++TP_POS(symbol_table);

            TP_TOKEN* tmp_period1 = TP_POS(symbol_table);

            if (IS_TOKEN_PERIOD(tmp_period1)){

                ++TP_POS(symbol_table);

                TP_TOKEN* tmp_period2 = TP_POS(symbol_table);

                if (IS_TOKEN_PERIOD(tmp_period2)){

                    ++TP_POS(symbol_table);

                    TP_TOKEN* tmp_period3 = TP_POS(symbol_table);

                    if (IS_TOKEN_PERIOD(tmp_period3)){

                        ++TP_POS(symbol_table);

                        return MAKE_PARSE_SUBTREE(
                            symbol_table,
                            TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_VARARG_1,
                            TP_TREE_TOKEN(tmp_comma),
                            TP_TREE_TOKEN(tmp_period1),
                            TP_TREE_TOKEN(tmp_period2),
                            TP_TREE_TOKEN(tmp_period3));
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
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_parameter_declaration(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: parameter-declaration -> declaration-specifiers (declarator | abstract-declarator?)
    {
        TP_PARSE_TREE* tmp_declaration_specifiers = NULL;
        TP_PARSE_TREE* tmp_declarator = NULL;
        TP_PARSE_TREE* tmp_abstract_declarator = NULL;

        if (tmp_declaration_specifiers = tp_parse_declaration_specifiers(symbol_table, NULL)){

            if (tmp_declarator = tp_parse_declarator(symbol_table)){

                // Grammer: parameter-declaration -> declaration-specifiers declarator
                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1,
                    TP_TREE_NODE(tmp_declaration_specifiers),
                    TP_TREE_NODE(tmp_declarator)
                );
            }else{

                if (tmp_abstract_declarator = parse_abstract_declarator(symbol_table)){

                    // Grammer: parameter-declaration -> declaration-specifiers abstract-declarator
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_2,
                        TP_TREE_NODE(tmp_declaration_specifiers),
                        TP_TREE_NODE(tmp_abstract_declarator)
                    );
                }else{

                    // Grammer: parameter-declaration -> declaration-specifiers
                    return MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3,
                        TP_TREE_NODE(tmp_declaration_specifiers)
                    );
                }
            }
        }

        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

