
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#if ! defined(TP_COMPILER_SEMANTIC_ANALYSIS_H_)
#define TP_COMPILER_SEMANTIC_ANALYSIS_H_

#include "tp_compiler_parse_tree.h"

// ----------------------------------------------------------------------------------------
// semantic analysis section:

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

typedef enum register_object_type_ {
    NOTHING_REGISTER_OBJECT = 0,
    DEFINED_REGISTER_OBJECT,
    UNDEFINED_REGISTER_OBJECT,
}REGISTER_OBJECT_TYPE;

typedef struct register_object_ {
    REGISTER_OBJECT_TYPE member_register_object_type;
    uint32_t member_var_index;
}REGISTER_OBJECT;

typedef struct sama_hash_data_{
    REGISTER_OBJECT member_register_object;
    uint8_t* member_string;
}SAME_HASH_DATA;

typedef struct register_object_hash_element_{
    SAME_HASH_DATA member_sama_hash_data[UINT8_MAX + 1];
    struct register_object_hash_element_* member_next;
}REGISTER_OBJECT_HASH_ELEMENT;

typedef struct register_object_hash_{
    size_t member_mask;
    REGISTER_OBJECT_HASH_ELEMENT member_hash_table[UINT8_MAX + 1];
}REGISTER_OBJECT_HASH;

#endif

