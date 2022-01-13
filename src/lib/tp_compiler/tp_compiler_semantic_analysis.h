
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_SEMANTIC_ANALYSIS_H_)
#define TP_COMPILER_SEMANTIC_ANALYSIS_H_

#include "tp_compiler_parse_tree.h"

// ----------------------------------------------------------------------------------------
// semantic analysis section:

// Common

typedef enum register_object_type_{
    NOTHING_REGISTER_OBJECT = 0,
    DEFINED_REGISTER_OBJECT,
    UNDEFINED_REGISTER_OBJECT,
}REGISTER_OBJECT_TYPE;

typedef struct tp_c_object_ TP_C_OBJECT;

typedef struct register_object_{
    REGISTER_OBJECT_TYPE member_register_object_type;
    // Use int_calc_compiler.
    uint32_t member_var_index;
    // Use C compiler.
    uint32_t member_ref_count; // If member_ref_count is zero, erase member_sama_hash_data[element].
    TP_C_OBJECT* member_stack_c_object; // NOTE: member_stack_c_object must not free memory.
}REGISTER_OBJECT;

typedef struct sama_hash_data_{
    REGISTER_OBJECT member_register_object;
    TP_CHAR8_T* member_string; // NOTE: member_string must not free memory.
}SAME_HASH_DATA;

typedef struct register_object_hash_element_ REGISTER_OBJECT_HASH_ELEMENT;

typedef struct register_object_hash_element_{
    SAME_HASH_DATA member_sama_hash_data[UINT8_MAX + 1];
    REGISTER_OBJECT_HASH_ELEMENT* member_next;
}REGISTER_OBJECT_HASH_ELEMENT;

typedef struct register_object_hash_{
    size_t member_mask;
    REGISTER_OBJECT_HASH_ELEMENT member_hash_table[UINT8_MAX + 1];
}REGISTER_OBJECT_HASH;

// int_calc_compiler

#define TP_GRAMMER_TYPE_INDEX_STATEMENT_1 0
#define TP_GRAMMER_TYPE_INDEX_STATEMENT_2 1
#define TP_GRAMMER_TYPE_INDEX_EXPRESSION_1 2
#define TP_GRAMMER_TYPE_INDEX_EXPRESSION_2 3
#define TP_GRAMMER_TYPE_INDEX_TERM_1 4
#define TP_GRAMMER_TYPE_INDEX_TERM_2 5
#define TP_GRAMMER_TYPE_INDEX_FACTOR_1 6
#define TP_GRAMMER_TYPE_INDEX_FACTOR_2 7
#define TP_GRAMMER_TYPE_INDEX_FACTOR_3 8
#define TP_GRAMMER_TYPE_INDEX_NULL 9

#define TP_PARSE_TREE_TYPE_MAX_NUM1 5
#define TP_PARSE_TREE_TYPE_MAX_NUM2 (TP_GRAMMER_TYPE_INDEX_NULL + 1)

// C compiler

#define TP_C_OBJECT_SIZE_ALLOCATE_UNIT 256

#define TP_C_OBJECT_SIZE_LOCAL_ALLOCATE_UNIT 256
#define TP_C_EXPR_SIZE_ALLOCATE_UNIT 32

#define TP_C_EXPR_STACK_EMPTY -1
#define TP_C_EXPR_STACK_SIZE_ALLOCATE_UNIT 32

#define TP_C_SCOPE_C_OBJECT_SIZE_ALLOCATE_UNIT 32
#define TP_C_SCOPE_STACK_SIZE_ALLOCATE_UNIT 32

#define TP_C_INHERIT_ATTR_TO_C_OBJECT_FROM_C_OBJECT(dst_obj, src_obj) \
    (dst_obj)->member_function_parameter_attr = (src_obj)->member_function_parameter_attr; \
    (dst_obj)->member_function_parameter_num_attr = (src_obj)->member_function_parameter_num_attr; \
    (dst_obj)->member_c_return_type_attr = (src_obj)->member_c_return_type_attr;

#define TP_C_INHERIT_ATTR_TO_COMPOUND_STATEMENT_FROM_C_OBJECT(dst_cs, src_obj) \
    (dst_cs)->member_function_parameter_attr = (src_obj)->member_function_parameter_attr; \
    (dst_cs)->member_function_parameter_num_attr = (src_obj)->member_function_parameter_num_attr; \
    (dst_cs)->member_c_return_type_attr = (src_obj)->member_c_return_type_attr;

#define TP_C_INHERIT_ATTR_TO_C_OBJECT_FROM_COMPOUND_STATEMENT(dst_obj, src_cs) \
    (dst_obj)->member_function_parameter_attr = (src_cs)->member_function_parameter_attr; \
    (dst_obj)->member_function_parameter_num_attr = (src_cs)->member_function_parameter_num_attr; \
    (dst_obj)->member_c_return_type_attr = (src_cs)->member_c_return_type_attr;

#define TP_C_INHERIT_ATTR_TO_COMPOUND_STATEMENT_FROM_COMPOUND_STATEMENT(dst_cs, src_cs) \
    (dst_cs)->member_function_parameter_attr = (src_cs)->member_function_parameter_attr; \
    (dst_cs)->member_function_parameter_num_attr = (src_cs)->member_function_parameter_num_attr; \
    (dst_cs)->member_c_return_type_attr = (src_cs)->member_c_return_type_attr;

#define TP_C_INHERIT_ATTR_TO_FUNCTION_BODY_FROM_C_OBJECT(dst_fb, src_obj) \
    (dst_fb)->member_function_parameter_attr = (src_obj)->member_function_parameter_attr; \
    (dst_fb)->member_function_parameter_num_attr = (src_obj)->member_function_parameter_num_attr; \
    (dst_fb)->member_c_return_type_attr = (src_obj)->member_c_return_type_attr;

typedef enum TP_GRAMMER_CONTEXT_{
    TP_GRAMMER_CONTEXT_NULL = 0,
    TP_GRAMMER_CONTEXT_TRANSLATION_UNIT,
    TP_GRAMMER_CONTEXT_EXTERNAL_DECLARATIONS,
    TP_GRAMMER_CONTEXT_FUNCTION_DEFINITION,
    TP_GRAMMER_CONTEXT_FUNCTION_NAME,
    TP_GRAMMER_CONTEXT_FUNCTION_ARGS,
    TP_GRAMMER_CONTEXT_FUNCTION_RETURN_TYPE,
    TP_GRAMMER_CONTEXT_FUNCTION_CALL,
    TP_GRAMMER_CONTEXT_STATEMENTS,
    TP_GRAMMER_CONTEXT_EXPRESSIONS,
    TP_GRAMMER_CONTEXT_ARITHMETIC_CONSTANT_EXPRESSION,
    TP_GRAMMER_CONTEXT_INTEGER_CONSTANT_EXPRESSION,
    TP_GRAMMER_CONTEXT_CONTROLLING_EXPRESSION,
    TP_GRAMMER_CONTEXT_L_VALUES,
    TP_GRAMMER_CONTEXT_R_VALUES,
    TP_GRAMMER_CONTEXT_DECLARATIONS,
    TP_GRAMMER_CONTEXT_INITIALIZER
}TP_GRAMMER_CONTEXT;

// C compiler: namespace

typedef enum TP_C_NAMESPACE_{
    TP_C_NAMESPACE_NONE = 0,
    TP_C_NAMESPACE_MACRO_NAME,      // preprocessor macro name.
    TP_C_NAMESPACE_STATEMENT_LABEL, // goto label.
    TP_C_NAMESPACE_TAG_NAME,        // tag name(struct, union, enum).
//  TP_C_NAMESPACE_MEMBER_NAME,
    TP_C_NAMESPACE_OTHER,           // variable, function, typedef, enum.
    TP_C_NAMESPACE_NUM
}TP_C_NAMESPACE;

typedef struct tp_c_namespace_stack_element_{
    TP_C_OBJECT** member_c_object;  // NOTE: member_c_object[element] must not free memory.
    rsize_t member_c_object_pos;
    rsize_t member_c_object_size;
}TP_C_NAMESPACE_STACK_ELEMENT;

typedef struct tp_c_namespace_stack_{
    TP_C_NAMESPACE_STACK_ELEMENT* member_stack;
    rsize_t member_stack_pos;
    rsize_t member_stack_size;
}TP_C_NAMESPACE_STACK;

typedef struct tp_c_scope_{
    TP_C_NAMESPACE_STACK member_c_namespace[TP_C_NAMESPACE_NUM];
}TP_C_SCOPE;

// C compiler: expression

typedef enum TP_C_EXPR_KIND_{
    TP_C_EXPR_KIND_IDENTIFIER_NULL,

// Call operators
    TP_C_EXPR_KIND_CALL_INDIRECT_RIP,
    TP_C_EXPR_KIND_I32_RETURN,
    TP_C_EXPR_KIND_I64_RETURN,
    TP_C_EXPR_KIND_ARGS_BEGIN,
    TP_C_EXPR_KIND_ARGS_END,

// Parametric operators
    TP_C_EXPR_KIND_DROP,

// Identifier
    TP_C_EXPR_KIND_IDENTIFIER_L_VALUES,
    TP_C_EXPR_KIND_IDENTIFIER_R_VALUES,

// Local
    TP_C_EXPR_KIND_I32_GET_LOCAL,
    TP_C_EXPR_KIND_I32_GET_LOCAL_ARG,
    TP_C_EXPR_KIND_I64_GET_LOCAL,
    TP_C_EXPR_KIND_I64_GET_LOCAL_ARG,
    TP_C_EXPR_KIND_I32_SET_LOCAL,
    TP_C_EXPR_KIND_I32_SET_LOCAL_ARG,
    TP_C_EXPR_KIND_I64_SET_LOCAL,
    TP_C_EXPR_KIND_I64_SET_LOCAL_ARG,
    TP_C_EXPR_KIND_I32_TEE_LOCAL,
    TP_C_EXPR_KIND_I32_TEE_LOCAL_ARG,
    TP_C_EXPR_KIND_I64_TEE_LOCAL,
    TP_C_EXPR_KIND_I64_TEE_LOCAL_ARG,

// Global

// Constants
    TP_C_EXPR_KIND_I32_CONST,
    TP_C_EXPR_KIND_I64_CONST,
    TP_C_EXPR_KIND_STRING_LITERAL,

// Comparison operators(i32)
    TP_C_EXPR_KIND_I32_NE,  // op1 != op2

// Comparison operators(i64)
    TP_C_EXPR_KIND_I64_NE,  // op1 != op2

// Numeric operators(i32)
    TP_C_EXPR_KIND_I32_ADD,
    TP_C_EXPR_KIND_I32_SUB,
    TP_C_EXPR_KIND_I32_MUL,
    TP_C_EXPR_KIND_I32_DIV,
    TP_C_EXPR_KIND_I32_XOR,

// Numeric operators(i64)
    TP_C_EXPR_KIND_I64_ADD,
    TP_C_EXPR_KIND_I64_SUB,
    TP_C_EXPR_KIND_I64_MUL,
    TP_C_EXPR_KIND_I64_DIV,
    TP_C_EXPR_KIND_I64_XOR
}TP_C_EXPR_KIND;

// Identifier, Local(ARG), Constants, Numeric operators

typedef struct tp_c_type_ TP_C_TYPE;

typedef struct tp_c_function_arg_{
    int32_t member_arg_index;
    TP_C_TYPE* member_c_return_type; // NOTE: member_c_return_type must not free memory.
    bool member_is_containing_function_call;
}TP_C_FUNCTION_ARG;

// Call operators(TP_C_EXPR_KIND_ARGS_BEGIN, TP_C_EXPR_KIND_ARGS_END)

typedef struct tp_c_function_args_begin_end_{
    uint32_t member_function_call_depth;
}TP_C_FUNCTION_ARGS_BEGIN_END;

// Identifier, Call operators(TP_C_EXPR_KIND_CALL_INDIRECT_RIP)

typedef struct tp_c_identifier_{
    TP_C_OBJECT* member_identifier; // NOTE: member_token must not free memory.
    TP_C_FUNCTION_ARG member_function_arg;
// Call operators(TP_C_EXPR_KIND_CALL_INDIRECT_RIP)
    uint32_t member_function_call_depth;
}TP_C_IDENTIFIER;

// Constants

typedef struct tp_c_i32_const_{
    int32_t member_value;
    TP_C_FUNCTION_ARG member_function_arg;
}TP_C_I32_CONST;

typedef struct tp_c_i64_const_{
    int64_t member_value;
    TP_C_FUNCTION_ARG member_function_arg;
}TP_C_I64_CONST;

typedef struct tp_c_string_literal_{
    TP_TOKEN* member_token; // NOTE: member_token must not free memory.
    uint32_t member_string_literal_index;
    uint32_t member_wasm_offset;
    uint32_t member_coff_offset;
    uint32_t member_symbol_index;
    TP_C_FUNCTION_ARG member_function_arg;
}TP_C_STRING_LITERAL;

typedef union tp_c_expr_body_{
// Identifier, Call operators(TP_C_EXPR_KIND_CALL_INDIRECT_RIP)
    TP_C_IDENTIFIER member_identifier;

// Call operators(TP_C_EXPR_KIND_ARGS_BEGIN, TP_C_EXPR_KIND_ARGS_END)
    TP_C_FUNCTION_ARGS_BEGIN_END member_function_args_begin_end;

// Local, Numeric operators
    TP_C_FUNCTION_ARG member_function_arg;

// Constants
    TP_C_I32_CONST member_i32_const;
    TP_C_I64_CONST member_i64_const;
    TP_C_STRING_LITERAL member_string_literal;
}TP_C_EXPR_BODY;

typedef struct tp_c_expr_{
    TP_C_EXPR_KIND member_c_expr_kind;
    TP_C_EXPR_BODY member_c_expr_body;
}TP_C_EXPR;

typedef enum TP_EXPR_TYPE_
{
    TP_EXPR_TYPE_NULL = 0,
    TP_EXPR_TYPE_I32,
    TP_EXPR_TYPE_I64
}TP_EXPR_TYPE;

// C compiler: type

typedef enum TP_C_TYPE_TYPE_{
    TP_C_TYPE_TYPE_NONE = 0,
    TP_C_TYPE_TYPE_BASIC,
    TP_C_TYPE_TYPE_STRUCT,
//  TP_C_TYPE_TYPE_UNION,
//  TP_C_TYPE_TYPE_ENUM,
    TP_C_TYPE_TYPE_FUNCTION,
    TP_C_TYPE_TYPE_FUNCTION_PARAM,
    TP_C_TYPE_TYPE_POINTER,
//  TP_C_TYPE_TYPE_ARRAY,
//  TP_C_TYPE_TYPE_TYPEDEF,
    TP_C_TYPE_TYPE_NULL_STATEMENT,
    TP_C_TYPE_TYPE_COMPOUND_STATEMENT,
//  TP_C_TYPE_TYPE_LABELED_STATEMENT_LABEL,
//  TP_C_TYPE_TYPE_LABELED_STATEMENT_CASE,
//  TP_C_TYPE_TYPE_LABELED_STATEMENT_DEFAULT,
    TP_C_TYPE_TYPE_DECLARATION_STATEMENT,
    TP_C_TYPE_TYPE_EXPRESSION_STATEMENT,
//  TP_C_TYPE_TYPE_SELECTION_STATEMENT_IF,
//  TP_C_TYPE_TYPE_SELECTION_STATEMENT_SWITCH,
//  TP_C_TYPE_TYPE_ITERATION_STATEMENT_WHILE,
    TP_C_TYPE_TYPE_ITERATION_STATEMENT_DO,
//  TP_C_TYPE_TYPE_ITERATION_STATEMENT_FOR,
//  TP_C_TYPE_TYPE_ITERATION_STATEMENT_FOR_DECL,
//  TP_C_TYPE_TYPE_JUMP_STATEMENT_GOTO,
//  TP_C_TYPE_TYPE_JUMP_STATEMENT_CONTINUE,
//  TP_C_TYPE_TYPE_JUMP_STATEMENT_BREAK,
    TP_C_TYPE_TYPE_JUMP_STATEMENT_RETURN,
}TP_C_TYPE_TYPE;

typedef enum TP_C_TYPE_SPECIFIER_{
    TP_C_TYPE_SPECIFIER_NONE = 0x0000,
    TP_C_TYPE_SPECIFIER_VOID = 0x0001,
    TP_C_TYPE_SPECIFIER_CHAR = 0x0002,
    TP_C_TYPE_SPECIFIER_SHORT = 0x0004,
    TP_C_TYPE_SPECIFIER_INT = 0x0008,
    TP_C_TYPE_SPECIFIER_LONG1 = 0x0010,
    TP_C_TYPE_SPECIFIER_LONG2 = 0x0020,
    TP_C_TYPE_SPECIFIER_MASK = 0x003F,
    TP_C_TYPE_SPECIFIER_SIGNED = 0x0040,
    TP_C_TYPE_SPECIFIER_UNSIGNED = 0x0080,
//  TP_C_TYPE_SPECIFIER_BOOL = 0x0100,
//  TP_C_TYPE_SPECIFIER_FLOAT = 0x0200,
//  TP_C_TYPE_SPECIFIER_DOUBLE = 0x0400,
//  TP_C_TYPE_SPECIFIER_COMPLEX = 0x0800,
//  TP_C_TYPE_SPECIFIER_IMAGINARY = 0x1000,
//  TP_C_TYPE_SPECIFIER_FUNCTION_VAR_ARGS = 0x2000, // ...
}TP_C_TYPE_SPECIFIER;

#define TP_MASK_C_TYPE_SPECIFIER(c_type_specifier) ((c_type_specifier) & TP_C_TYPE_SPECIFIER_MASK)

typedef enum TP_C_STORAGE_CLASS_SPECIFIER_{
    TP_C_STORAGE_CLASS_SPECIFIER_NONE = 0x00,
//  TP_C_STORAGE_CLASS_SPECIFIER_TYPEDEF = 0x01,
    TP_C_STORAGE_CLASS_SPECIFIER_EXTERN = 0x02,
    TP_C_STORAGE_CLASS_SPECIFIER_STATIC = 0x04,
//  TP_C_STORAGE_CLASS_SPECIFIER_THREAD_LOCAL = 0x08,
    TP_C_STORAGE_CLASS_SPECIFIER_AUTO = 0x0f,
    TP_C_STORAGE_CLASS_SPECIFIER_REGISTER = 0x10,
}TP_C_STORAGE_CLASS_SPECIFIER;

typedef enum TP_C_TYPE_QUALIFIER_{
    TP_C_TYPE_QUALIFIER_NONE = 0x00,
    TP_C_TYPE_QUALIFIER_CONST = 0x01,
    TP_C_TYPE_QUALIFIER_RESTRICT = 0x02,
    TP_C_TYPE_QUALIFIER_VOLATILE = 0x04,
}TP_C_TYPE_QUALIFIER;

typedef enum TP_C_FUNCTION_SPECIFIER_{
    TP_C_FUNCTION_SPECIFIER_NONE = 0x00,
    TP_C_FUNCTION_SPECIFIER_INLINE = 0x01,
    TP_C_FUNCTION_SPECIFIER_NORETURN = 0x02,
}TP_C_FUNCTION_SPECIFIER;

typedef struct tp_c_type_basic_{
    TP_C_TYPE_SPECIFIER member_type_specifier;
}TP_C_TYPE_BASIC;

typedef struct tp_c_type_struct_content_ TP_C_TYPE_STRUCT_CONTENT;

typedef struct tp_c_type_struct_content_{
    TP_C_TYPE* member_type_member;
    TP_TOKEN* member_identifier; // NOTE: member_token must not free memory.
    uint32_t member_bits;
    size_t member_offset;
    TP_C_TYPE_STRUCT_CONTENT* member_next;
}TP_C_TYPE_STRUCT_CONTENT;

typedef struct tp_c_type_struct_{
    rsize_t member_struct_member_num;
    TP_C_TYPE_STRUCT_CONTENT* member_struct_content;
    size_t member_align;
}TP_C_TYPE_STRUCT;

typedef struct tp_c_type_union_content_ TP_C_TYPE_UNION_CONTENT;

typedef struct tp_c_type_union_content_{
    TP_C_TYPE* member_type_member;
    TP_TOKEN* member_identifier; // NOTE: member_token must not free memory.
    TP_C_TYPE_UNION_CONTENT* member_next;
}TP_C_TYPE_UNION_CONTENT;

typedef struct tp_c_type_union_{
    rsize_t member_union_member_num;
    TP_C_TYPE_UNION_CONTENT* member_union_content;
    size_t member_align;
}TP_C_TYPE_UNION;

typedef struct tp_c_type_enum_content_ TP_C_TYPE_ENUM_CONTENT;

typedef struct tp_c_type_enum_content_{
    TP_TOKEN* member_identifier; // NOTE: member_token must not free memory.
    int32_t member_value;
    TP_C_TYPE_ENUM_CONTENT* member_next;
}TP_C_TYPE_ENUM_CONTENT;

typedef struct tp_c_type_enum_{
    rsize_t member_enum_num;
    TP_C_TYPE_ENUM_CONTENT* member_enum_content;
}TP_C_TYPE_ENUM;

typedef struct tp_c_type_pointer_{
    rsize_t member_pointer_level;
    TP_C_TYPE* member_type_pointer;
}TP_C_TYPE_POINTER;

typedef struct tp_c_type_array_{
    rsize_t member_array_num;
    TP_C_TYPE* member_type_array;
}TP_C_TYPE_ARRAY;

// struct TP_C_TYPE_NULL_STATEMENT is not defined.

typedef struct tp_c_function_f_param_ TP_C_TYPE_FUNCTION_F_PARAM;

typedef struct tp_c_type_compound_statement_{
    // NOTE: member_function_parameter_attr must not free memory.
    TP_C_TYPE_FUNCTION_F_PARAM* member_function_parameter_attr;
    uint32_t member_function_parameter_num_attr;
    TP_C_TYPE* member_c_return_type_attr; // NOTE: member_c_return_type_attr must not free memory.
    TP_C_OBJECT** member_statement_body;
    rsize_t member_statement_body_pos;
    rsize_t member_statement_body_size;
}TP_C_TYPE_COMPOUND_STATEMENT;

typedef struct tp_c_type_labeled_statement_label_{
    // labeled-statement -> identifier : statement
    TP_TOKEN* member_identifier; // NOTE: member_token must not free memory.
    TP_C_OBJECT* member_statement;
}TP_C_TYPE_LABELED_STATEMENT_LABEL;

typedef struct tp_c_type_labeled_statement_case_{
    // case constant-expression : statement
    TP_PARSE_TREE* member_constant_expression; // NOTE: member_constant_expression must not free memory.
    TP_C_OBJECT* member_statement;
}TP_C_TYPE_LABELED_STATEMENT_CASE;

typedef struct tp_c_type_labeled_statement_default_{
    // default : statement
    TP_C_OBJECT* member_statement;
}TP_C_TYPE_LABELED_STATEMENT_DEFAULT;

typedef struct tp_c_type_declaration_statement_{
// declaration;
    TP_C_TYPE* member_declaration;
    TP_PARSE_TREE* member_parse_tree; // NOTE: member_parse_tree must not free memory.

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_DECLARATION_STATEMENT;

typedef struct tp_c_type_expression_statement_{
    // expression-statement -> expression? ;
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_EXPRESSION_STATEMENT;

typedef struct tp_c_type_selection_statement_if_{
    // if ( expression ) statement (else statement)?
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.
    TP_C_OBJECT* member_statement;
    TP_C_OBJECT* member_statement_else;

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_SELECTION_STATEMENT_IF;

typedef struct tp_c_type_selection_statement_switch_{
    // switch ( expression ) statement
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.
    TP_C_OBJECT* member_statement;

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_SELECTION_STATEMENT_SWITCH;

typedef struct tp_c_type_iteration_statement_while_{
    // while ( expression ) statement
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.
    TP_C_OBJECT* member_statement;

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_ITERATION_STATEMENT_WHILE;

typedef struct tp_c_type_iteration_statement_do_{
    // do statement while ( expression ) ;
    TP_C_OBJECT* member_statement;
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_ITERATION_STATEMENT_DO;

typedef struct tp_c_type_iteration_statement_for_{
    // for ( expression? ; expression? ; expression? ) statement
    TP_PARSE_TREE* member_expression1; // NOTE: member_expression1 must not free memory.
    TP_PARSE_TREE* member_expression2; // NOTE: member_expression2 must not free memory.
    TP_PARSE_TREE* member_expression3; // NOTE: member_expression3 must not free memory.
    TP_C_OBJECT* member_statement;

// expression1
    TP_C_EXPR* member_c_expr1;
    rsize_t member_c_expr_pos1;
    rsize_t member_c_expr_size1;

// expression2
    TP_C_EXPR* member_c_expr2;
    rsize_t member_c_expr_pos2;
    rsize_t member_c_expr_size2;

// expression3
    TP_C_EXPR* member_c_expr3;
    rsize_t member_c_expr_pos3;
    rsize_t member_c_expr_size3;
}TP_C_TYPE_ITERATION_STATEMENT_FOR;

#define TP_C_FUNCTION_ARGS_MAXIMUM 127

typedef struct tp_c_function_f_param_{
    TP_TOKEN* member_parameter_name; // NOTE: member_token must not free memory.
    TP_C_TYPE* member_type_parameter;// NOTE: child member of member_type_parameter must not free memory.
    TP_C_TYPE_FUNCTION_F_PARAM* member_next;
}TP_C_TYPE_FUNCTION_F_PARAM;

typedef struct tp_c_type_function_{
    TP_C_TYPE* member_c_return_type;
    uint32_t member_parameter_num;
    TP_C_TYPE_FUNCTION_F_PARAM* member_parameter;
    TP_C_TYPE_COMPOUND_STATEMENT member_function_body;
    uint32_t member_function_body_offset;
    uint32_t member_function_body_size;
}TP_C_TYPE_FUNCTION;

typedef struct tp_c_type_function_formal_param_{
    TP_C_TYPE* member_type_param;
}TP_C_TYPE_FUNCTION_FORMAL_PARAM;

typedef struct tp_c_type_iteration_statement_for_decl_{
// for ( declaration expression? ; expression? ) statement
    TP_C_TYPE_DECLARATION_STATEMENT member_declaration;
    TP_PARSE_TREE* member_expression2; // NOTE: member_expression2 must not free memory.
    TP_PARSE_TREE* member_expression3; // NOTE: member_expression3 must not free memory.
    TP_C_OBJECT* member_statement;

// expression2
    TP_C_EXPR* member_c_expr2;
    rsize_t member_c_expr_pos2;
    rsize_t member_c_expr_size2;

// expression3
    TP_C_EXPR* member_c_expr3;
    rsize_t member_c_expr_pos3;
    rsize_t member_c_expr_size3;
}TP_C_TYPE_ITERATION_STATEMENT_FOR_DECL;

typedef struct tp_c_type_jump_statement_goto_{
// goto identifier ;
    TP_TOKEN* member_identifier; // NOTE: member_token must not free memory.
}TP_C_TYPE_JUMP_STATEMENT_GOTO;

// struct TP_C_TYPE_JUMP_STATEMENT_CONTINUE is not defined.
//   continue ;

// struct TP_C_TYPE_JUMP_STATEMENT_BREAK is not defined.
//   break ;

typedef struct tp_c_type_jump_statement_return_{
// return expression? ;
    TP_C_TYPE* member_c_return_type;  // NOTE: member_c_return_type must not free memory.
    TP_PARSE_TREE* member_expression; // NOTE: member_expression must not free memory.

// expression
    TP_C_EXPR* member_c_expr;
    rsize_t member_c_expr_pos;
    rsize_t member_c_expr_size;
}TP_C_TYPE_JUMP_STATEMENT_RETURN;

typedef union tp_c_type_body_{
    TP_C_TYPE_BASIC member_type_basic;
    TP_C_TYPE_STRUCT member_type_struct;
//  TP_C_TYPE_UNION member_type_union;
//  TP_C_TYPE_ENUM member_type_enum;
    TP_C_TYPE_FUNCTION member_type_function;
    TP_C_TYPE_FUNCTION_FORMAL_PARAM member_type_param;
    TP_C_TYPE_POINTER member_type_pointer;
//  TP_C_TYPE_ARRAY member_type_array;
//  TP_C_OBJECT* member_typedef_object; // NOTE: member_typedef_object must not free memory.
//  TP_C_TYPE_NULL_STATEMENT is not defined.
    TP_C_TYPE_COMPOUND_STATEMENT member_type_compound_statement;
//  TP_C_TYPE_LABELED_STATEMENT_LABEL member_type_labeled_statement_label;
//  TP_C_TYPE_LABELED_STATEMENT_CASE member_type_labeled_statement_case;
//  TP_C_TYPE_LABELED_STATEMENT_DEFAULT member_type_labeled_statement_default;
    TP_C_TYPE_DECLARATION_STATEMENT member_type_declaration_statement;
    TP_C_TYPE_EXPRESSION_STATEMENT member_type_expression_statement;
//  TP_C_TYPE_SELECTION_STATEMENT_IF member_type_selection_statement_if;
//  TP_C_TYPE_SELECTION_STATEMENT_SWITCH member_type_selection_statement_switch;
//  TP_C_TYPE_ITERATION_STATEMENT_WHILE member_type_iteration_statement_while;
    TP_C_TYPE_ITERATION_STATEMENT_DO member_type_iteration_statement_do;
//  TP_C_TYPE_ITERATION_STATEMENT_FOR member_type_iteration_statement_for;
//  TP_C_TYPE_ITERATION_STATEMENT_FOR_DECL member_type_iteration_statement_for_decl;
//  TP_C_TYPE_JUMP_STATEMENT_GOTO member_type_jump_statement_goto;
//  TP_C_TYPE_JUMP_STATEMENT_CONTINUE is not defined.
//  TP_C_TYPE_JUMP_STATEMENT_BREAK is not defined.
    TP_C_TYPE_JUMP_STATEMENT_RETURN member_type_jump_statement_return;
}TP_C_TYPE_BODY;

// C compiler: declaration

typedef enum TP_C_DECL_KIND_{
    TP_C_DECL_KIND_NONE = 0,
    TP_C_DECL_KIND_GLOBAL_FUNCTION,
    TP_C_DECL_KIND_GLOBAL_VARIABLE_I32,
    TP_C_DECL_KIND_GLOBAL_VARIABLE_I64,
    TP_C_DECL_KIND_LOCAL_VARIABLE_I32,
    TP_C_DECL_KIND_LOCAL_VARIABLE_I64,
    TP_C_DECL_KIND_FUNCTION_ARGUMENT_I32,
    TP_C_DECL_KIND_FUNCTION_ARGUMENT_I64
}TP_C_DECL_KIND;

typedef struct tp_c_global_function_{
    uint32_t member_function_index;
    int32_t member_type_return;
    uint32_t member_symbol_index;
}TP_C_GLOBAL_FUNCTION;

typedef struct tp_c_global_variable_i32_{
    uint32_t member_var_id;
    uint32_t member_init_value;
    uint32_t member_wasm_offset;
    uint32_t member_coff_offset;
    uint32_t member_symbol_index;
}TP_C_GLOBAL_VARIABLE_I32;

typedef struct tp_c_global_variable_i64_{
    uint32_t member_var_id;
    uint64_t member_init_value;
    uint32_t member_wasm_offset;
    uint32_t member_coff_offset;
    uint32_t member_symbol_index;
}TP_C_GLOBAL_VARIABLE_I64;

typedef struct tp_c_local_variable_i32_{
    uint32_t member_local_index;
    uint32_t member_wasm_offset;
}TP_C_LOCAL_VARIABLE_I32;

typedef struct tp_c_local_variable_i64_{
    uint32_t member_local_index;
    uint32_t member_wasm_offset;
}TP_C_LOCAL_VARIABLE_I64;

typedef union tp_c_decl_body_{
    TP_C_GLOBAL_FUNCTION member_global_function;
    TP_C_GLOBAL_VARIABLE_I32 member_global_variable_i32;
    TP_C_GLOBAL_VARIABLE_I64 member_global_variable_i64;
    TP_C_LOCAL_VARIABLE_I32 member_local_variable_i32;
    TP_C_LOCAL_VARIABLE_I64 member_local_variable_i64;
    uint32_t member_function_argument_index;
}TP_C_DECL_BODY;

typedef struct tp_c_decl_{
    TP_C_DECL_KIND member_c_decl_kind;
    TP_C_DECL_BODY member_c_decl_body;
}TP_C_DECL;

// C compiler: type

typedef struct tp_c_type_{
    TP_C_TYPE_TYPE member_type; // kind of type.
//  TP_C_STORAGE_CLASS_SPECIFIER member_storage_class_specifier;
//  TP_C_FUNCTION_SPECIFIER member_function_specifier;
//  TP_C_ALIGNMENT_SPECIFIER member_alignment_specifier;
//  TP_C_ATOMIC_TYPE_SPECIFIER member_atomic_type_specifier;
//  TP_C_TYPE_QUALIFIER member_type_qualifier;
    TP_C_TYPE_BODY member_body; // body of type.
    TP_C_DECL member_decl;      // additional information.
}TP_C_TYPE;

typedef struct tp_c_object_{
// name
    TP_C_NAMESPACE member_c_namespace;
    SAME_HASH_DATA* member_sama_hash_data; // NOTE: member_sama_hash_data must not free memory.
    TP_C_OBJECT* member_outside_of_stack;  // NOTE: member_outside_of_stack must not free memory.
    TP_TOKEN* member_token;                // NOTE: member_token must not free memory.

// type
    TP_C_TYPE member_type;
    bool member_is_static_data;

// attribute
    // NOTE: member_function_parameter_attr must not free memory.
    TP_C_TYPE_FUNCTION_F_PARAM* member_function_parameter_attr;
    uint32_t member_function_parameter_num_attr;
    TP_C_TYPE* member_c_return_type_attr;  // NOTE: member_c_return_type_attr must not free memory.
}TP_C_OBJECT;

#endif

