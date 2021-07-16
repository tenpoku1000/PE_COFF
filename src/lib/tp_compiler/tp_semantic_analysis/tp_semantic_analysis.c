
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Functions:
//  (1) Register to SYMBOL Table of defined objects.
//  (2) Register to SYMBOL Table of undefined objects.
//  (3) Variable define check.
//  (4) Variable reference check.

static bool search_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, REGISTER_OBJECT* register_object
);
static bool search_parse_tree(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree,
    TP_GRAMMER_CONTEXT grammer_context
);
static bool search_object_hash_element(
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t mask,
    TP_CHAR8_T* search_string, REGISTER_OBJECT* register_object
);
static bool register_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace_kind, REGISTER_OBJECT* object
);
static bool append_c_object_to_current_namespace(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token,
    TP_C_NAMESPACE_STACK* c_namespace, REGISTER_OBJECT* object
);
static bool set_outside_of_stack_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_NAMESPACE_STACK* c_namespace, REGISTER_OBJECT* object
);
static uint8_t calc_hash(TP_CHAR8_T* string);
static bool register_object_hash_element(
    TP_SYMBOL_TABLE* symbol_table,
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t max_size,
    SAME_HASH_DATA* hash_data, REGISTER_OBJECT* object
);
static bool dump_object_hash(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool dump_object_hash_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    REGISTER_OBJECT_HASH* object_hash,
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, uint8_t indent_level
);

bool tp_semantic_analysis(TP_SYMBOL_TABLE* symbol_table)
{
    bool status = false;

    if (symbol_table->member_is_int_calc_compiler){

        // int_calc_compiler
        if ( ! search_parse_tree(
            symbol_table, symbol_table->member_tp_parse_tree, TP_GRAMMER_CONTEXT_NULL)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }
    }else{

        // C compiler
        if ( ! tp_make_C_IR(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }
    }

    status = true;
fail:
    if ((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_semantic_analysis_file)){

        if ( ! dump_object_hash(symbol_table, symbol_table->member_object_hash_file_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return status;
}

bool tp_register_defined_variable(
    TP_SYMBOL_TABLE* symbol_table, TP_GRAMMER_CONTEXT grammer_context,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, TP_C_OBJECT* c_object)
{
    REGISTER_OBJECT object = { 0 };

    if (search_object(symbol_table, token, c_namespace, &object)){

        switch (object.member_register_object_type){
        case DEFINED_REGISTER_OBJECT:
            if ((TP_GRAMMER_CONTEXT_FUNCTION_NAME != grammer_context) &&
                (TP_GRAMMER_CONTEXT_NULL != grammer_context)){

                return true;
            }
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
        object.member_ref_count = 1;
        object.member_stack_c_object = c_object;

        if ( ! register_object(symbol_table, token, c_namespace, &object)){

              TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

bool tp_register_undefined_variable(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, REGISTER_OBJECT* object)
{
    if (search_object(symbol_table, token, c_namespace, object)){

        if (DEFINED_REGISTER_OBJECT == object->member_register_object_type){

            return true;
        }
    }else{

        object->member_register_object_type = UNDEFINED_REGISTER_OBJECT;
        object->member_ref_count = 1;
        object->member_stack_c_object = NULL;

        if ( ! register_object(symbol_table, token, c_namespace, object)){

              TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

void tp_free_object_hash(
    TP_SYMBOL_TABLE* symbol_table,
    REGISTER_OBJECT_HASH* object_hash, REGISTER_OBJECT_HASH_ELEMENT* hash_element)
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

bool tp_get_var_value(
    TP_SYMBOL_TABLE* symbol_table,
    TP_PARSE_TREE* parse_tree, size_t index, uint32_t* var_value)
{
    REGISTER_OBJECT register_object = { 0 };

    TP_TOKEN* token = parse_tree->member_element[index].member_body.member_tp_token;

    if ( ! IS_TOKEN_ID(token)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (search_object(symbol_table, token, TP_C_NAMESPACE_OTHER, &register_object)){

        switch (register_object.member_register_object_type){
        case DEFINED_REGISTER_OBJECT:
            *var_value = register_object.member_var_index; // Calculated by semantic analysis.
            break;
        case UNDEFINED_REGISTER_OBJECT:
            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: use undefined symbol(%1)."),
                TP_LOG_PARAM_STRING(token->member_string)
            );
            return false;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            return false;
        }
    }else{

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
}

static bool search_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace, REGISTER_OBJECT* register_object)
{
    uint8_t hash = calc_hash(token->member_string);

    size_t mask = symbol_table->member_object_hash.member_mask;

    REGISTER_OBJECT_HASH_ELEMENT* hash_table = symbol_table->member_object_hash.member_hash_table;

    if ( ! search_object_hash_element(
        &(hash_table[hash & mask]), mask, token->member_string, register_object)){

//      TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ((symbol_table->member_is_int_calc_compiler) ||
        (UNDEFINED_REGISTER_OBJECT == register_object->member_register_object_type)){

        return true;
    }

    if (NULL == register_object->member_stack_c_object){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
}

static bool search_parse_tree(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, TP_GRAMMER_CONTEXT grammer_context)
{
    bool is_semantic_analysis_success = true;

    TP_PARSE_TREE_ELEMENT* element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    for (size_t i = 0; element_num > i; ++i){

        if (TP_PARSE_TREE_TYPE_NULL == element[i].member_type){

            break;
        }

        if (TP_PARSE_TREE_TYPE_NODE == element[i].member_type){

            TP_PARSE_TREE* parse_tree_child = (TP_PARSE_TREE*)(element[i].member_body.member_child);

            if ( ! search_parse_tree(symbol_table, parse_tree_child, grammer_context)){

                is_semantic_analysis_success = false;
            }
        }
    }

    if ( ! tp_variable_reference_check_expr(symbol_table, parse_tree, grammer_context)){

        is_semantic_analysis_success = false;
    }

    return is_semantic_analysis_success;
}

static bool search_object_hash_element(
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t max_size,
    TP_CHAR8_T* search_string, REGISTER_OBJECT* object)
{
    for (size_t i = 0; max_size > i; ++i){

        TP_CHAR8_T* string = hash_element->member_sama_hash_data[i].member_string;

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

static bool register_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN* token, TP_C_NAMESPACE c_namespace_kind, REGISTER_OBJECT* object)
{
    uint8_t hash = calc_hash(token->member_string);

    size_t mask = symbol_table->member_object_hash.member_mask;

    REGISTER_OBJECT_HASH_ELEMENT* hash_table =
        symbol_table->member_object_hash.member_hash_table;

    if ( ! register_object_hash_element(
        symbol_table,
        &(hash_table[hash & mask]), mask,
        &(SAME_HASH_DATA){
            .member_register_object = *object,
            .member_string = token->member_string
        }, object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ((symbol_table->member_is_int_calc_compiler) ||
        (UNDEFINED_REGISTER_OBJECT == object->member_register_object_type)){

        return true;
    }

    // variable, function, typedef, enum.
    TP_C_NAMESPACE_STACK* c_namespace = &(symbol_table->\
member_c_scope.member_c_namespace[c_namespace_kind]);

    if (NULL == c_namespace->member_stack){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! append_c_object_to_current_namespace(
        symbol_table, token, c_namespace, object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! set_outside_of_stack_object(symbol_table, c_namespace, object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool append_c_object_to_current_namespace(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token,
    TP_C_NAMESPACE_STACK* c_namespace, REGISTER_OBJECT* object)
{
    TP_C_NAMESPACE_STACK_ELEMENT* stack_element =
        &(c_namespace->member_stack[c_namespace->member_stack_pos - 1]);

    if (token->member_string){

        for (rsize_t i = 0; stack_element->member_c_object_pos >= i; ++i){

            if (NULL == stack_element->member_c_object){

                continue;
            }

            TP_C_OBJECT* c_object = stack_element->member_c_object[i];

            if (NULL == c_object){

                continue;
            }

            if (NULL == c_object->member_token){

                continue;
            }

            if (0 == strcmp(
                token->member_string, c_object->member_token->member_string)){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: Duplicate OBJECT name(%1)."),
                    TP_LOG_PARAM_STRING(c_object->member_token->member_string)
                );

                return false;
            }
        }
    }

    if (stack_element->member_c_object_pos == (stack_element->member_c_object_size / sizeof(TP_C_OBJECT*))){

        rsize_t c_scope_c_object_size_allocate_unit = 
            symbol_table->member_c_scope_c_object_size_allocate_unit * sizeof(TP_C_OBJECT*);

        rsize_t c_object_size = stack_element->member_c_object_size + c_scope_c_object_size_allocate_unit;

        TP_C_OBJECT** tmp_c_object = (TP_C_OBJECT**)TP_REALLOC(
            symbol_table, stack_element->member_c_object, c_object_size
        );

        if (NULL == tmp_c_object){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((TP_CHAR8_T*)tmp_c_object) + stack_element->member_c_object_size, 0,
            c_scope_c_object_size_allocate_unit
        );

        stack_element->member_c_object = tmp_c_object;
        stack_element->member_c_object_size = c_object_size;
    }

    if (stack_element->member_c_object){

        stack_element->member_c_object[stack_element->member_c_object_pos] =
            object->member_stack_c_object;

        ++(stack_element->member_c_object_pos);
    }else{

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    return true;
}

static bool set_outside_of_stack_object(
    TP_SYMBOL_TABLE* symbol_table,
    TP_C_NAMESPACE_STACK* c_namespace, REGISTER_OBJECT* object)
{
    if (1 >= c_namespace->member_stack_pos){

        return true;
    }

    TP_C_OBJECT* c_object_original = object->member_stack_c_object;

    if (NULL == c_object_original){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // Find object at outside of stack.
    rsize_t stack_pos = ((c_namespace->member_stack_pos >= 3) ? (c_namespace->member_stack_pos - 2) : 0);

    for (rsize_t i = stack_pos; 0 < i; --i){

        TP_C_NAMESPACE_STACK_ELEMENT* stack_element =
            &(c_namespace->member_stack[i]);

        for (rsize_t j = 0; stack_element->member_c_object_pos > j; ++j){

            TP_C_OBJECT* c_object = stack_element->member_c_object[j];

            if (NULL == c_object){

                continue;
            }

            if (0 == strcmp(
                c_object_original->member_token->member_string,
                c_object->member_token->member_string)){

                c_object_original->member_outside_of_stack = c_object;

                return true;
            }
        }
    }

    return true;
}

static uint8_t calc_hash(TP_CHAR8_T* string)
{
    uint8_t hash = 0;

    while (*string){

        hash ^= *string;

        ++string;
    }

    return hash;
}

static bool register_object_hash_element(
    TP_SYMBOL_TABLE* symbol_table,
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, size_t max_size,
    SAME_HASH_DATA* hash_data, REGISTER_OBJECT* object)
{
    if ((false == symbol_table->member_is_int_calc_compiler) &&
        (NULL == object->member_stack_c_object)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    for (size_t i = 0; max_size > i; ++i){

        SAME_HASH_DATA* current_element = &(hash_element->member_sama_hash_data[i]);

        if (NOTHING_REGISTER_OBJECT ==
            current_element->member_register_object.member_register_object_type){

            *current_element = *hash_data;

            if (symbol_table->member_is_int_calc_compiler){

                current_element->member_register_object.member_var_index =
                    symbol_table->member_var_count;
            }else{

                object->member_stack_c_object->member_sama_hash_data = current_element;
            }

            return true;
        }else if (0 == strcmp(current_element->member_string, hash_data->member_string)){

            if (false == symbol_table->member_is_int_calc_compiler){

                REGISTER_OBJECT* update_object = &(current_element->member_register_object);

                if (UINT32_MAX == update_object->member_ref_count){

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("ERROR: member_ref_count of hash overflow at %1 function."),
                        TP_LOG_PARAM_STRING(__func__)
                    );

                    return false;
                }

                ++(update_object->member_ref_count);

                update_object->member_stack_c_object->member_sama_hash_data = current_element;
            }

            return true;
        }
    }

    REGISTER_OBJECT_HASH_ELEMENT* next =
        (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element->member_next);

    if (NULL == next){

        next = (REGISTER_OBJECT_HASH_ELEMENT*)TP_CALLOC(
            symbol_table, 1, sizeof(REGISTER_OBJECT_HASH_ELEMENT)
        );

        if (NULL == next){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        next->member_sama_hash_data[0] = *hash_data;

        if (symbol_table->member_is_int_calc_compiler){
        
            next->member_sama_hash_data[0].member_register_object.member_var_index =
                symbol_table->member_var_count;
        }else{

            object->member_stack_c_object->member_sama_hash_data =
                &(next->member_sama_hash_data[0]);
        }

        hash_element->member_next = next;

        return true;
    }

    if ( ! register_object_hash_element(symbol_table, next, max_size, hash_data, object)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
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
        &(symbol_table->member_object_hash),
        symbol_table->member_object_hash.member_hash_table, indent_level)){

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
    REGISTER_OBJECT_HASH* object_hash,
    REGISTER_OBJECT_HASH_ELEMENT* hash_element, uint8_t indent_level)
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

        REGISTER_OBJECT_HASH_ELEMENT* next =
            (REGISTER_OBJECT_HASH_ELEMENT*)(hash_element[i].member_next);

        if (next){

            if ( ! dump_object_hash_main(
                symbol_table, write_file,
                object_hash, next, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                err = _set_errno(0);

                return false;
            }
        }

        for (size_t j = 0; max_size > j; ++j){

            SAME_HASH_DATA* same_hash_data = &(hash_element[i].member_sama_hash_data[j]);

            REGISTER_OBJECT_TYPE object_type =
                same_hash_data->member_register_object.member_register_object_type;

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

            if (symbol_table->member_is_int_calc_compiler){

                // int_calc_compiler
                fprintf(write_file, "%smember_var_index(%d)\n",
                    indent_string, same_hash_data->member_register_object.member_var_index);
            }else{

                // C compiler
                fprintf(write_file, "%smember_ref_count(%d)\n",
                    indent_string, same_hash_data->member_register_object.member_ref_count);
//              same_hash_data->member_register_object.member_stack_c_object
            }

            fprintf(write_file, "%smember_string(%s)\n", indent_string, same_hash_data->member_string);

            fprintf(write_file, "%s}\n\n", prev_indent_string);
        }
    }

    err = _set_errno(0);

    return true;
}

