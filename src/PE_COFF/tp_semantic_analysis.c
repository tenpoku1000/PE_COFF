
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "tp_compiler.h"

// Functions:
//  (1) Register to SYMBOL Table of defined objects.
//  (2) Register to SYMBOL Table of undefined objects.
//  (3) Variable define check.
//  (4) Variable reference check.

static bool search_parse_tree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool variable_reference_check(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool variable_reference_check_grammer_statement_1(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool variable_reference_check_grammer_statement_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool variable_reference_check_grammer_factor_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool variable_reference_check_grammer_factor_3(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree);
static bool register_defined_variable(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool register_undefined_variable(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool search_object_hash_element(
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t mask,
    uint8_t* search_string, REGISTER_OBJECT* register_object
);
static bool register_object(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, REGISTER_OBJECT* register_object
);
static uint8_t calc_hash(uint8_t* string);
static bool register_object_hash_element(
    TP_SYMBOL_TABLE* symbol_table, REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t mask, SAME_HASH_DATA* hash_data
);
static bool dump_object_hash(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool dump_object_hash_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element, uint8_t indent_level
);

bool tp_semantic_analysis(TP_SYMBOL_TABLE* symbol_table)
{
    if ( ! search_parse_tree(symbol_table, symbol_table->member_tp_parse_tree)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (symbol_table->member_is_output_log_file){

        if ( ! dump_object_hash(symbol_table, symbol_table->member_object_hash_file_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

bool tp_search_object(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, REGISTER_OBJECT* register_object)
{
    uint8_t hash = calc_hash(token->member_string);

    size_t mask = symbol_table->member_object_hash.member_mask;

    REGISTER_OBJECT_HASH_ELEMENT* hash_table = symbol_table->member_object_hash.member_hash_table;

    return search_object_hash_element(
        &(hash_table[hash & mask]), mask, token->member_string, register_object
    );
}

static bool search_parse_tree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    bool is_semantic_analysis_success = true;

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    for (size_t i = 0; element_num > i; ++i){

        if (TP_PARSE_TREE_TYPE_NULL == element[i].member_type){

            break;
        }

        if (TP_PARSE_TREE_TYPE_NODE == element[i].member_type){

            if ( ! search_parse_tree(symbol_table, (TP_PARSE_TREE*)(element[i].member_body.member_child))){

                is_semantic_analysis_success = false;
            }
        }
    }

    if ( ! variable_reference_check(symbol_table, parse_tree)){

        is_semantic_analysis_success = false;
    }

    return is_semantic_analysis_success;
}

static bool variable_reference_check(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    switch (parse_tree->member_grammer){
    case TP_PARSE_TREE_GRAMMER_STATEMENT_1:
        return variable_reference_check_grammer_statement_1(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_STATEMENT_2:
        return variable_reference_check_grammer_statement_2(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_FACTOR_2:
        return variable_reference_check_grammer_factor_2(symbol_table, parse_tree);
    case TP_PARSE_TREE_GRAMMER_FACTOR_3:
        return variable_reference_check_grammer_factor_3(symbol_table, parse_tree);
    default:
        break;
    }

    return true;
}

static bool variable_reference_check_grammer_statement_1(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Statement -> variable '=' Expression ';'

    if (symbol_table->member_grammer_statement_1_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_STATEMENT_1][0]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_1 = parse_tree->member_element[0].member_body.member_tp_token;

    if ( ! (IS_TOKEN_ID(token_1) && IS_TOKEN_TYPE_ID_INT32(token_1))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! register_undefined_variable(symbol_table, token_1)){

        return false;
    }

    symbol_table->member_last_statement = parse_tree;

    return true;
}

static bool variable_reference_check_grammer_statement_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Statement -> Type variable '=' Expression ';'

    if (symbol_table->member_grammer_statement_2_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_STATEMENT_2][1]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_2 = parse_tree->member_element[1].member_body.member_tp_token;

    if ( ! (IS_TOKEN_ID(token_2) && IS_TOKEN_TYPE_ID_INT32(token_2))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! register_defined_variable(symbol_table, token_2)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    ++(symbol_table->member_var_count);

    symbol_table->member_last_statement = parse_tree;

    return true;
}

static bool variable_reference_check_grammer_factor_2(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Factor -> ('+' | '-') (variable | constant)

    if (symbol_table->member_grammer_factor_2_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_2][1]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_2 = parse_tree->member_element[1].member_body.member_tp_token;

    if ( ! ((IS_TOKEN_ID(token_2) && IS_TOKEN_TYPE_ID_INT32(token_2)) || \
            (IS_TOKEN_CONST_VALUE(token_2) && IS_TOKEN_TYPE_CONST_VALUE_INT32(token_2)))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (IS_TOKEN_CONST_VALUE(token_2)){ return true; }

    return register_undefined_variable(symbol_table, token_2);
}

static bool variable_reference_check_grammer_factor_3(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree)
{
    // Grammer: Factor -> variable | constant

    if (symbol_table->member_grammer_factor_3_num != parse_tree->member_element_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (TP_PARSE_TREE_TYPE_TOKEN != symbol_table->member_parse_tree_type[TP_GRAMMER_TYPE_INDEX_FACTOR_3][0]){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_TOKEN* token_1 = parse_tree->member_element[0].member_body.member_tp_token;

    if ( ! ((IS_TOKEN_ID(token_1) && IS_TOKEN_TYPE_ID_INT32(token_1)) || \
            (IS_TOKEN_CONST_VALUE(token_1) && IS_TOKEN_TYPE_CONST_VALUE_INT32(token_1)))){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (IS_TOKEN_CONST_VALUE(token_1)){ return true; }

    return register_undefined_variable(symbol_table, token_1);
}

static bool register_defined_variable(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    REGISTER_OBJECT object;

    if (tp_search_object(symbol_table, token, &object)){

        switch (object.member_register_object_type){
        case DEFINED_REGISTER_OBJECT:
            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Duplicate DEFINED_REGISTER_OBJECT at %1 function."),
                TP_LOG_PARAM_STRING(__func__)
            );
            return false;
        case UNDEFINED_REGISTER_OBJECT:
            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Get UNDEFINED_REGISTER_OBJECT at %1 function."),
                TP_LOG_PARAM_STRING(__func__)
            );
            return false;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            break;
        }
    }else{

        object.member_register_object_type = DEFINED_REGISTER_OBJECT;

        if ( ! register_object(symbol_table, token, &object)){

              TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool register_undefined_variable(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    REGISTER_OBJECT object;

    if (tp_search_object(symbol_table, token, &object)){

        if (DEFINED_REGISTER_OBJECT == object.member_register_object_type){

            return true;
        }
    }else{

        object.member_register_object_type = UNDEFINED_REGISTER_OBJECT;

        if ( ! register_object(symbol_table, token, &object)){

              TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool search_object_hash_element(
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t max_size, uint8_t* search_string, REGISTER_OBJECT* object)
{
    for (size_t i = 0; max_size > i; ++i){

        uint8_t* string = hash_element->member_sama_hash_data[i].member_string;

        if (string && (0 == strcmp(string, search_string))){

            *object = hash_element->member_sama_hash_data[i].member_register_object;

            return true;
        }
    }

    REGISTER_OBJECT_HASH_ELEMENT* next = (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element->member_next);

    if (NULL == next){

        return false;
    }

    return search_object_hash_element(next, max_size, search_string, object);
}

static bool register_object(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, REGISTER_OBJECT* object)
{
    uint8_t hash = calc_hash(token->member_string);

    size_t mask = symbol_table->member_object_hash.member_mask;

    REGISTER_OBJECT_HASH_ELEMENT* hash_table = symbol_table->member_object_hash.member_hash_table;

    return register_object_hash_element(
        symbol_table,
        &(hash_table[hash & mask]), mask, 
        &(SAME_HASH_DATA){
            .member_register_object = *object,
            .member_string = token->member_string
        }
    );
}

static uint8_t calc_hash(uint8_t* string)
{
    uint8_t hash = 0;

    while (*string){

        hash ^= *string;

        ++string;
    }

    return hash;
}

static bool register_object_hash_element(
    TP_SYMBOL_TABLE* symbol_table, REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t max_size, SAME_HASH_DATA* hash_data)
{
    for (size_t i = 0; max_size > i; ++i){

        if (NOTHING_REGISTER_OBJECT ==
            hash_element->member_sama_hash_data[i].member_register_object.member_register_object_type){

            hash_element->member_sama_hash_data[i] = *hash_data;
            hash_element->member_sama_hash_data[i].member_register_object.member_var_index = symbol_table->member_var_count;

            return true;
        }else if (0 == strcmp(hash_element->member_sama_hash_data[i].member_string, hash_data->member_string)){

            return false;
        }
    }

    REGISTER_OBJECT_HASH_ELEMENT* next = (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element->member_next);

    if (NULL == next) {

        next = (REGISTER_OBJECT_HASH_ELEMENT*)calloc(sizeof(REGISTER_OBJECT_HASH_ELEMENT), 1);

        if (NULL == next){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        next->member_sama_hash_data[0] = *hash_data;
        next->member_sama_hash_data[0].member_register_object.member_var_index = symbol_table->member_var_count;

        hash_element->member_next = (struct register_object_hash_element_*)next;

        return true;
    }

    return register_object_hash_element(symbol_table, next, max_size, hash_data);
}

static bool dump_object_hash(TP_SYMBOL_TABLE* symbol_table, char* path)
{
    FILE* write_file = NULL;

    if ( ! tp_open_write_file(symbol_table, path, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint8_t indent_level = 1;

    if ( ! dump_object_hash_main(
        symbol_table, write_file,
        &(symbol_table->member_object_hash), symbol_table->member_object_hash.member_hash_table, indent_level)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

//      return false;
    }

    return true;
}

static bool dump_object_hash_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element, uint8_t indent_level)
{
    errno_t err = 0;

    if ((UINT8_MAX == indent_level) || (0 == indent_level)){

        fprintf(write_file, "NOTE: Bad indent level(%d).\n", indent_level);
        fprintf(write_file, "NOTE: Stop dump object hash.\n\n");

        err = _set_errno(0);

        return true;
    }

    TP_MAKE_INDENT_STRING(indent_level);

    size_t max_size = object_hash->member_mask;

    for (size_t i = 0; max_size > i; ++i){

        REGISTER_OBJECT_HASH_ELEMENT* next = (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element[i].member_next);

        if (next){

            if ( ! dump_object_hash_main(
                symbol_table, write_file,
                object_hash, next, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                err = _set_errno(0);

                return false;
            }
        }

        for (size_t j = 0; max_size > j; ++j) {

            SAME_HASH_DATA* same_hash_data = &(hash_element[i].member_sama_hash_data[j]);

            REGISTER_OBJECT_TYPE object_type = same_hash_data->member_register_object.member_register_object_type;

            if (NOTHING_REGISTER_OBJECT == object_type){

                continue;
            }

            fprintf(write_file, "%s{\n", prev_indent_string);

            fprintf(write_file, "%smember_hash_table[%zd].member_sama_hash_data[%zd]\n", indent_string, i, j);

            switch (object_type){
            case NOTHING_REGISTER_OBJECT:
                break;
            case DEFINED_REGISTER_OBJECT:
                fprintf(write_file, "%sDEFINED_REGISTER_OBJECT\n", indent_string);
                break;
            case UNDEFINED_REGISTER_OBJECT:
                fprintf(write_file, "%sUNDEFINED_REGISTER_OBJECT\n", indent_string);
                break;
            default:
                fprintf(write_file, "%sREGISTER_OBJECT(UNKNOWN_OBJECT: %d)\n", indent_string, object_type);
                break;
            }

            fprintf(write_file, "%smember_var_index(%d)\n",
                indent_string, same_hash_data->member_register_object.member_var_index);

            fprintf(write_file, "%smember_string(%s)\n", indent_string, same_hash_data->member_string);

            fprintf(write_file, "%s}\n\n", prev_indent_string);
        }
    }

    err = _set_errno(0);

    return true;
}

void tp_free_object_hash(
    TP_SYMBOL_TABLE* symbol_table, REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element)
{
    // NOTE: member_string must not free memory.

    size_t max_size = object_hash->member_mask;

    for (size_t i = 0; max_size > i; ++i){

        REGISTER_OBJECT_HASH_ELEMENT* next = (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element[i].member_next);

        if (next){

            tp_free_object_hash(symbol_table, object_hash, next);

            TP_FREE(symbol_table, &next, sizeof(REGISTER_OBJECT_HASH_ELEMENT));
        }
    }
}

