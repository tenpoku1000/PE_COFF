
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_MAKE_GRAMMER_STRING(grammer) (grammer), (#grammer)

typedef struct tp_grammer_string_{
    TP_PARSE_TREE_GRAMMER member_grammer;
    TP_CHAR8_T* member_string;
}TP_GRAMMER_STRING;

static TP_GRAMMER_STRING grammer_string_table[TP_PARSE_TREE_GRAMMER_NUM] = {

    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_NULL) },

// int_calc_compiler

    // Grammer:
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PROGRAM) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STATEMENT_1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STATEMENT_2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TERM_1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TERM_2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR_1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR_2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR_3) },

    // grammer_1: calc first set and follow set.
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PROGRAM_TMP1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STATEMENT) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STATEMENT_TMP1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_TMP2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TERM) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TERM_TMP1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TERM_TMP2) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR_TMP1) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FACTOR_TMP2) },

    // grammer_2, 3: calc first set and follow set.
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_E) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_E_TMP) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_T) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_T_TMP) },
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_F) },

// C compiler(Error recovery)

    // grammer_C: calc first set and follow set.
    // ToDo:

// C compiler(External definitions)

    // Grammer: translation-unit -> external-declaration external-declaration+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1) },
    // Grammer: translation-unit -> external-declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_2) },

    // Grammer: external-declaration -> function-definition
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_1) },
    // Grammer: external-declaration -> declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXTERNAL_DECLARATION_2) },

    // Grammer: function-definition -> 
    //     declaration-specifiers declarator compound-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_1) },
    // Grammer: function-definition -> 
    //     declaration-specifiers declarator declaration-list compound-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FUNCTION_DEFINITION_2) },

    // Grammer: declaration-list -> declaration declaration+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_1) },
    // Grammer: declaration-list -> declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_LIST_2) },

// C compiler(Statements)

    // Grammer: statement -> labeled-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_1) },
    // Grammer: statement -> compound-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_2) },
    // Grammer: statement -> expression-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_3) },
    // Grammer: statement -> selection-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_4) },
    // Grammer: statement -> iteration-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_5) },
    // Grammer: statement -> jump-statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_STATEMENT_6) },

    // Grammer: compound-statement -> { block-item-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_1) },
    // Grammer: compound-statement -> { }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_COMPOUND_STATEMENT_2) },

    // Grammer: labeled-statement -> identifier : statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_1) },
    // Grammer: labeled-statement -> case constant-expression : statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_2) },
    // Grammer: labeled-statement -> default : statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LABELED_STATEMENT_3) },

    // Grammer: block-item-list -> block-item block-item+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_1) },
    // Grammer: block-item-list -> block-item
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_LIST_2) },

    // Grammer: block-item -> declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_1) },
    // Grammer: block-item -> statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_BLOCK_ITEM_2) },

    // Grammer: expression-statement -> expression ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_1) },
    // Grammer: expression-statement -> ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXPRESSION_STATEMENT_2) },

    // Grammer: if ( expression ) statement else statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_1) },
    // Grammer: if ( expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_IF_2) },

    // Grammer: selection-statement -> switch ( expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SELECTION_STATEMENT_SWITCH_1) },

    // Grammer: iteration-statement -> while ( expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_WHILE_1) },

    // Grammer: iteration-statement -> do statement while ( expression ) ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_DO_1) },

    // Grammer: iteration-statement -> for ( declaration expression ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_1) },
    // Grammer: iteration-statement -> for ( expression ; expression ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_2) },
    // Grammer: iteration-statement -> for ( ; expression ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_3) },
    // Grammer: iteration-statement -> for ( declaration ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_4) },
    // Grammer: iteration-statement -> for ( expression ; ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_5) },
    // Grammer: iteration-statement -> for ( declaration expression ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_6) },
    // Grammer: iteration-statement -> for ( expression ; expression ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_7) },
    // Grammer: iteration-statement -> for ( ; ; expression ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_8) },
    // Grammer: iteration-statement -> for ( ; expression ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_9) },
    // Grammer: iteration-statement -> for ( declaration ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_10) },
    // Grammer: iteration-statement -> for ( expression ; ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_11) },
    // Grammer: iteration-statement -> for ( ; ; ) statement
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ITERATION_STATEMENT_FOR_12) },

    // Grammer: jump-statement -> goto identifier ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_1) },
    // Grammer: jump-statement -> continue ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_2) },
    // Grammer: jump-statement -> break ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_3) },
    // Grammer: jump-statement -> return expression ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_4) },
    // Grammer: jump-statement -> return ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_JUMP_STATEMENT_5) },

// C compiler(Expressions)

    // Grammer: expression -> assignment-expression (, assignment-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_EXPRESSION_1) },
    // Grammer: expression -> assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_C_EXPRESSION_2) },

    // Grammer: constant-expression -> conditional-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_CONSTANT_EXPRESSION_1) },

    // Grammer: assignment-expression -> unary-expression assignment-operator assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_1) },
    // Grammer: assignment-expression -> conditional-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ASSIGNMENT_EXPRESSION_2) },

    // Grammer: conditional-expression -> logical-OR-expression (? expression : conditional-expression)?
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_CONDITIONAL_EXPRESSION_1) },

    // Grammer: primary-expression -> identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_1) },
    // Grammer: primary-expression -> constant
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_2) },
    // Grammer: primary-expression -> string-literal
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_3) },
    // Grammer: primary-expression -> ( expression )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_4) },
    // Grammer: primary-expression -> generic-selection
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PRIMARY_EXPRESSION_5) },

    // Grammer: generic-selection -> _Generic ( assignment-expression , generic-assoc-list )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_GENERIC_SELECTION_1) },
    // Grammer: generic-assoc-list -> generic-association (, generic-association)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_1) },
    // Grammer: generic-assoc-list -> generic-association
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_GENERIC_ASSOC_LIST_2) },
    // Grammer: generic-association -> type-name : assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_1) },
    // Grammer: generic-association -> default : assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_GENERIC_ASSOCIATION_2) },

    // Grammer: postfix-expression -> primary-expression PostfixExpressionTmp1+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_1) },
    // Grammer: postfix-expression -> primary-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_2) },
    // Grammer: postfix-expression -> ( type-name ) { initializer-list , }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_3) },
    // Grammer: postfix-expression -> ( type-name ) { initializer-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_4) },

    // Grammer: PostfixExpressionTmp1 -> [ expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_1) },
    // Grammer: PostfixExpressionTmp1 -> ( argument-expression-list )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_2) },
    // Grammer: PostfixExpressionTmp1 -> ( )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_3) },
    // Grammer: PostfixExpressionTmp1 -> (. | ->) identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_4) },
    // Grammer: PostfixExpressionTmp1 -> ++ | --
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POSTFIX_EXPRESSION_TMP1_5) },

    // Grammer: argument-expression-list -> assignment-expression (, assignment-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_1) },
    // Grammer: argument-expression-list -> assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ARGUMENT_EXPRESSION_LIST_2) },

    // Grammer: unary-expression -> postfix-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_1) },
    // Grammer: unary-expression -> (++ | --) unary-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_2) },
    // Grammer: unary-expression -> unary-operator cast-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_3) },
    // Grammer: unary-expression -> sizeof unary-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_4) },
    // Grammer: unary-expression -> sizeof ( type-name )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_5) },
    // Grammer: unary-expression -> _Alignof ( type-name )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_UNARY_EXPRESSION_6) },

    // Grammer: cast-expression -> unary-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_1) },
    // Grammer: cast-expression -> ( type-name ) cast-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_CAST_EXPRESSION_2) },

    // Grammer: multiplicative-expression -> cast-expression ((* | / | %) cast-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_1) },
    // Grammer: multiplicative-expression -> cast-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_MULTIPLICATIVE_EXPRESSION_2) },

    // Grammer: additive-expression -> multiplicative-expression ((+ | -) multiplicative-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_1) },
    // Grammer: additive-expression -> multiplicative-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ADDITIVE_EXPRESSION_2) },

    // Grammer: shift-expression -> additive-expression ((<< | >>) additive-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_1) },
    // Grammer: shift-expression -> additive-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SHIFT_EXPRESSION_2) },

    // Grammer: relational-expression -> shift-expression ((< | > | <= | >=) shift-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_1) },
    // Grammer: relational-expression -> shift-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_RELATIONAL_EXPRESSION_2) },

    // Grammer: equality-expression -> relational-expression ((== | !=) relational-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_1) },
    // Grammer: equality-expression -> relational-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EQUALITY_EXPRESSION_2) },

    // Grammer: AND-expression -> equality-expression (& equality-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_1) },
    // Grammer: AND-expression -> equality-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_AND_EXPRESSION_2) },

    // Grammer: exclusive-OR-expression -> AND-expression (^ AND-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_1) },
    // Grammer: exclusive-OR-expression -> AND-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_EXCLUSIVE_OR_EXPRESSION_2) },

    // Grammer: inclusive-OR-expression -> exclusive-OR-expression (| exclusive-OR-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_1) },
    // Grammer: inclusive-OR-expression -> exclusive-OR-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INCLUSIVE_OR_EXPRESSION_2) },

    // Grammer: logical-AND-expression -> inclusive-OR-expression (&& inclusive-OR-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_1) },
    // Grammer: logical-AND-expression -> inclusive-OR-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LOGICAL_AND_EXPRESSION_2) },

    // Grammer: logical-OR-expression -> logical-AND-expression (|| logical-AND-expression)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_1) },
    // Grammer: logical-OR-expression -> logical-AND-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_LOGICAL_OR_EXPRESSION_2) },

// C compiler(Declarations: declaration)

    // Grammer: declaration -> declaration-specifiers init-declarator-list ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_1) },
    // Grammer: declaration -> declaration-specifiers ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_2) },
    // Grammer: declaration -> static_assert-declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_3) },

    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
    //    function-specifier | alignment-specifier) declaration-specifiers
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_1) },
    // Grammer: declaration-specifiers -> (storage-class-specifier | type-specifier | type-qualifier |
    //    function-specifier | alignment-specifier)
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATION_SPECIFIERS_2) },

    // Grammer: storage-class-specifier -> typedef | extern | static | _Thread_local | auto | register
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STORAGE_CLASS_SPECIFIER_1) },

    // Grammer: function-specifier -> inline | _Noreturn
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_FUNCTION_SPECIFIER_1) },

    // Grammer: alignment-specifier -> _Alignas ( type-name | constant-expression )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ALIGNMENT_SPECIFIER_1) },

    // Grammer: init-declarator-list -> init-declarator (, init-declarator)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_1) },
    // Grammer: init-declarator-list -> init-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_LIST_2) },

    // Grammer: init-declarator -> declarator = initializer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_1) },
    // Grammer: init-declarator -> declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INIT_DECLARATOR_2) },

    // Grammer: declarator -> pointer direct-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATOR_1) },
    // Grammer: declarator -> direct-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DECLARATOR_2) },

    // Grammer: direct-declarator -> DirectDeclaratorTmp1 DirectDeclaratorTmp2+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_1) },
    // Grammer: direct-declarator -> DirectDeclaratorTmp1
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_2) },

    // Grammer: DirectDeclaratorTmp1 -> identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_1) },
    // Grammer: DirectDeclaratorTmp1 -> ( declarator )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP1_2) },

    // Grammer: DirectDeclaratorTmp2 ->
    //     [ static type-qualifier-list assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_1) },
    // Grammer: DirectDeclaratorTmp2 ->
    //     [ static assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_STATIC_2) },

    // Grammer: DirectDeclaratorTmp2 -> [ * ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_1) },
    // Grammer: DirectDeclaratorTmp2 -> [ assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_2) },
    // Grammer: DirectDeclaratorTmp2 -> [ ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_3) },

    // Grammer: DirectDeclaratorTmp2 ->
    //     [ type-qualifier-list static assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1) },
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list * ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2) },
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3) },
    // Grammer: DirectDeclaratorTmp2 -> [ type-qualifier-list ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_4) },

    // Grammer: DirectDeclaratorTmp2 -> ( parameter-type-list )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_1) },
    // Grammer: DirectDeclaratorTmp2 -> ( identifier-list )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_2) },
    // Grammer: DirectDeclaratorTmp2 -> ( )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_DECLARATOR_TMP2_PAREN_3) },

    // Grammer: identifier-list -> identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_1) },
    // Grammer: identifier-list -> identifier (, identifier)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_IDENTIFIER_LIST_2) },

    // Grammer: initializer -> assignment-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_1) },
    // Grammer: initializer -> { initializer-list , }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_2) },
    // Grammer: initializer -> { initializer-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_3) },

    // Grammer: initializer-list -> initializerListTmp1 (, initializerListTmp1)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_1) },
    // Grammer: initializer-list -> initializerListTmp1
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_2) },

    // Grammer: initializerListTmp1 -> designation initializer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_1) },
    // Grammer: initializerListTmp1 -> initializer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_INITIALIZER_LIST_TMP1_2) },

    // Grammer: designation -> designator-list =
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DESIGNATION_1) },

    // Grammer: designator-list -> designator+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DESIGNATOR_LIST_1) },

    // Grammer: designator -> [ constant-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DESIGNATOR_1) },
    // Grammer: designator -> . identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DESIGNATOR_2) },

// C compiler(Declarations: type-name)

    // Grammer: type-name -> specifier-qualifier-list abstract-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_NAME_1) },
    // Grammer: type-name -> specifier-qualifier-list
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_NAME_2) },

    // Grammer: specifier-qualifier-list -> type-specifier specifier-qualifier-list
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_1) },
    // Grammer: specifier-qualifier-list -> type-specifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_2) },
    // Grammer: specifier-qualifier-list -> type-qualifier specifier-qualifier-list
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_3) },
    // Grammer: specifier-qualifier-list -> type-qualifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_SPECIFIER_QUALIFIER_LIST_4) },

    // Grammer: type-specifier -> void | char | short | int | long |
    //     float | double | signed | unsigned | _Bool | _Complex
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_1) },
    // Grammer: type-specifier -> atomic-type-specifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_2) },
    // Grammer: type-specifier -> struct-or-union-specifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_3) },
    // Grammer: type-specifier -> enum-specifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_4) },
    // Grammer: type-specifier -> typedef-name
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_SPECIFIER_5) },

    // Grammer: atomic-type-specifier -> _Atomic ( type-name )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ATOMIC_TYPE_SPECIFIER_1) },

    // Grammer: enum-specifier -> enum identifier { enumerator-list , }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_1) },
    // Grammer: enum-specifier -> enum identifier { enumerator-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_2) },
    // Grammer: enum-specifier -> enum { enumerator-list , }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_3) },
    // Grammer: enum-specifier -> enum { enumerator-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_4) },
    // Grammer: enum-specifier -> enum identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUM_SPECIFIER_5) },

    // Grammer: enumerator-list -> enumerator (, enumerator)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_1) },
    // Grammer: enumerator-list -> enumerator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUMERATOR_LIST_2) },

    // Grammer: enumerator -> identifier = constant-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUMERATOR_1) },
    // Grammer: enumerator -> identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ENUMERATOR_2) },

    // Grammer: typedef-name -> identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPEDEF_NAME_1) },

    // Grammer: struct-or-union-specifier -> struct-or-union identifier { struct-declaration-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_1) },
    // Grammer: struct-or-union-specifier -> struct-or-union { struct-declaration-list }
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_2) },
    // Grammer: struct-or-union-specifier -> struct-or-union identifier
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_SPECIFIER_3) },

    // Grammer: struct-or-union -> struct | union
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_OR_UNION_1) },

    // Grammer: struct-declaration-list -> struct-declaration+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_LIST_1) },

    // Grammer: struct-declaration -> specifier-qualifier-list struct-declarator-list ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_1) },
    // Grammer: struct-declaration -> specifier-qualifier-list ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_2) },
    // Grammer: struct-declaration -> static_assert-declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATION_3) },

    // Grammer: struct-declarator-list -> struct-declarator (, struct-declarator)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_1) },
    // Grammer: struct-declarator-list -> struct-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_LIST_2) },

    // Grammer: struct-declarator -> declarator : constant-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_1) },
    // Grammer: struct-declarator -> : constant-expression
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_2) },
    // Grammer: struct-declarator -> declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STRUCT_DECLARATOR_3) },

    // Grammer: abstract-declarator -> pointer direct-abstract-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_1) },
    // Grammer: abstract-declarator -> direct-abstract-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_2) },
    // Grammer: abstract-declarator -> pointer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_ABSTRACT_DECLARATOR_3) },

    // Grammer: pointer -> * type-qualifier-list pointer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POINTER_1) },
    // Grammer: pointer -> * type-qualifier-list
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POINTER_2) },
    // Grammer: pointer -> * pointer
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POINTER_3) },
    // Grammer: pointer -> *
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_POINTER_4) },

    // Grammer: type-qualifier-list -> type-qualifier+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_LIST_1) },

    // Grammer: type-qualifier -> const | restrict | volatile | _Atomic
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_TYPE_QUALIFIER_1) },

    // Grammer: static_assert-declaration -> _Static_assert ( constant-expression , string-literal ) ;
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_STATIC_ASSERT_DECLARATION_1) },

    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1 DirectAbstractDeclaratorTmp2+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_1) },
    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp1
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_2) },
    // Grammer: direct-abstract-declarator -> DirectAbstractDeclaratorTmp2
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_3) },

    // Grammer: DirectAbstractDeclaratorTmp1 -> ( abstract-declarator )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP1_1) },

    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ static type-qualifier-list assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_1) },
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ static assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_STATIC_2) },

    // Grammer: DirectAbstractDeclaratorTmp2 -> [*]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_1) },
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_2) },
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_3) },

    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list static assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_1) },
    // Grammer: DirectAbstractDeclaratorTmp2 ->
    //     [ type-qualifier-list assignment-expression ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_2) },
    // Grammer: DirectAbstractDeclaratorTmp2 -> [ type-qualifier-list ]
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_SQUARE_OTHER_QUALIFIER_LIST_3) },

    // Grammer: DirectAbstractDeclaratorTmp2 -> ( parameter-type-list )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_1) },
    // Grammer: DirectAbstractDeclaratorTmp2 -> ( )
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_DIRECT_ABSTRACT_DECLARATOR_TMP2_PAREN_2) },

    // Grammer: parameter-type-list -> parameter-list ParameterTypeListVararg
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_1) },
    // Grammer: parameter-type-list -> parameter-list
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_2) },

    // Grammer: parameter-list -> parameter-declaration (, parameter-declaration)+
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_1) },
    // Grammer: parameter-list -> parameter-declaration
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_LIST_2) },

    // Grammer: ParameterTypeListVararg -> , ...
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_TYPE_LIST_VARARG_1) },

    // Grammer: parameter-declaration -> declaration-specifiers declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_1) },
    // Grammer: parameter-declaration -> declaration-specifiers abstract-declarator
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_2) },
    // Grammer: parameter-declaration -> declaration-specifiers
    { TP_MAKE_GRAMMER_STRING(TP_PARSE_TREE_GRAMMER_PARAMETER_DECLARATION_3) },

//  TP_PARSE_TREE_GRAMMER_NUM
};

bool tp_print_parse_tree_grammer(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, FILE* write_file, uint8_t* indent_string)
{
    TP_PARSE_TREE_GRAMMER grammer = parse_tree->member_grammer;

    if (symbol_table->member_is_int_calc_compiler){

        if ((TP_PARSE_TREE_GRAMMER_PROGRAM <= grammer) &&
            (TP_PARSE_TREE_GRAMMER_FACTOR_3 >= grammer) &&
            (grammer_string_table[grammer].member_grammer == grammer)){

            fprintf(
                write_file, "%s%s\n",
                indent_string, grammer_string_table[grammer].member_string
            );
        }else{

            fprintf(
                write_file, "%sTP_PARSE_TREE_GRAMMER(UNKNOWN_GRAMMER: %d)\n",
                indent_string, grammer
            );
        }
    }else{

        if ((TP_PARSE_TREE_GRAMMER_TRANSLATION_UNIT_1 <= grammer) &&
            (TP_PARSE_TREE_GRAMMER_NUM > grammer) &&
            (grammer_string_table[grammer].member_grammer == grammer)){

            fprintf(
                write_file, "%s%s\n",
                indent_string, grammer_string_table[grammer].member_string
            );
        }else{

            fprintf(
                write_file, "%sTP_PARSE_TREE_GRAMMER(UNKNOWN_GRAMMER: %d)\n",
                indent_string, grammer
            );
        }
    }

    return true;
}

