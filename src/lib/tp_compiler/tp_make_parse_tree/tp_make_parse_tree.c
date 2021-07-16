
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

// Example:
// int32_t value1 = (1 + 2) * 3;
// int32_t value2 = 2 + (3 * value1);
// value1 = value2 + 100;
//
// Grammer:
// Program -> Statement+
// Statement -> Type? variable '=' Expression ';'
// Expression -> Term (('+' | '-') Term)*
// Term -> Factor (('*' | '/') Factor)*
// Factor -> '(' Expression ')' | ('+' | '-')? (variable | constant)
// Type -> int32_t
//
// Note:
//  (1) Nesting level of expression maximum 63.

#define TP_ADDITIONAL_LINE_SIZE 64

static bool dump_parse_tree_open(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool dump_parse_tree_main(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, uint8_t indent_level);
static bool dump_parse_tree_close(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_program(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_statement(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_expression(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_term(TP_SYMBOL_TABLE* symbol_table);
static TP_PARSE_TREE* parse_factor(TP_SYMBOL_TABLE* symbol_table);

bool tp_make_parse_tree(TP_SYMBOL_TABLE* symbol_table)
{
    if ((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_parse_tree_file)){

        if ( ! dump_parse_tree_open(symbol_table, symbol_table->member_parse_tree_file_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        fprintf(symbol_table->member_parse_tree_file, "=== Dump parse subtree. ===\n\n");
    }

    TP_PARSE_TREE* tmp_program = NULL;

    if (symbol_table->member_is_int_calc_compiler){

        // int_calc_compiler
        tmp_program = parse_program(symbol_table);
    }else{

        // C compiler
        tmp_program = tp_parse_translation_unit(symbol_table);
    }

    if (tmp_program && IS_END_OF_TOKEN(symbol_table->member_tp_token_position)){

        symbol_table->member_tp_parse_tree = tmp_program;

        if ((false == symbol_table->member_is_no_output_files) ||
            (symbol_table->member_is_no_output_files &&
            symbol_table->member_is_output_parse_tree_file)){

            fprintf(symbol_table->member_parse_tree_file, "=== Dump all parse tree. ===\n\n");

            uint8_t indent_level = 1;

            if ( ! dump_parse_tree_main(symbol_table, symbol_table->member_tp_parse_tree, indent_level)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        if ((false == symbol_table->member_is_no_output_files) ||
            (symbol_table->member_is_no_output_files &&
            symbol_table->member_is_output_parse_tree_file)){

            if ( ! dump_parse_tree_close(symbol_table)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }

        if (symbol_table->member_error_count){

            goto fail;
        }

        symbol_table->member_is_end_of_make_parse_tree = true;

        return true;
    }

    if ((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_parse_tree_file)){

        fprintf(symbol_table->member_parse_tree_file, "=== Syntax error. It is not dump all parse tree. ===\n\n");

        if ( ! dump_parse_tree_close(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }
    }

fail:
    symbol_table->member_tp_parse_tree = tmp_program;

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: syntax error.")
    );

    return false;
}

bool tp_set_error_reason(
    TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token, TP_CHAR8_T* error_message, TP_CHAR8_T* error_reason)
{
    symbol_table->member_error_token = token;
    symbol_table->member_error_message = error_message;
    symbol_table->member_error_reason = error_reason;
    ++(symbol_table->member_error_reason_count);

    return true;
}

bool tp_put_error_message(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* error_token = symbol_table->member_error_token;

    TP_INPUT_FILE* input_file = &(symbol_table->member_input_file);

    if (error_token->member_line > input_file->member_physical_lines_pos){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_CHAR8_T file_name[_MAX_PATH] = { 0 };

    char fname[_MAX_FNAME] = { 0 };
    char ext[_MAX_EXT] = { 0 };

    errno_t err = _splitpath_s(error_token->member_file, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }else{

        if (ext[0]){

            sprintf_s(file_name, sizeof(file_name), "%s%s", fname, ext);
        }else{

            sprintf_s(file_name, sizeof(file_name), "%s", fname);
        }
    }

    TP_CHAR8_T* physical_line = input_file->member_physical_lines[error_token->member_line];

    rsize_t physical_line_length = strlen(physical_line);

    rsize_t line_length = physical_line_length + TP_ADDITIONAL_LINE_SIZE + 1;

    TP_CHAR8_T line_size_buffer[TP_ADDITIONAL_LINE_SIZE] = { 0 };

    sprintf_s(
        line_size_buffer, sizeof(line_size_buffer), "  %zd |", error_token->member_line
    );

    rsize_t line_size_length = strlen(line_size_buffer);

    TP_CHAR8_T* line_buffer = (TP_CHAR8_T*)TP_CALLOC(symbol_table, line_length, sizeof(TP_CHAR8_T));

    if (NULL == line_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    sprintf_s(line_buffer, line_length, "%s %s", line_size_buffer, physical_line);

    rsize_t error_reason_length =
        strlen(symbol_table->member_error_reason) + line_size_length + error_token->member_column + 
        TP_ADDITIONAL_LINE_SIZE + 1;

    TP_CHAR8_T* error_reason = (TP_CHAR8_T*)TP_CALLOC(
        symbol_table, error_reason_length, sizeof(TP_CHAR8_T)
    );

    if (NULL == error_reason){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(symbol_table, &line_buffer, line_length);

        return false;
    }

    TP_CHAR8_T error_reason_fmt[TP_ADDITIONAL_LINE_SIZE] = { 0 };

    // [prev_line] line_size_buffer: "  %zd |", error_token->member_line
    sprintf_s(
        error_reason_fmt, sizeof(error_reason_fmt), "%% %zdc%% %zdc^- %%s",
        line_size_length, error_token->member_column + 1
    );

    sprintf_s(
        error_reason, error_reason_length, error_reason_fmt,
        ' ', ' ', symbol_table->member_error_reason
    );

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT(
            "%1\n"
            " --> %2\n"
            "%3\n"
            "%4"
        ),
        TP_LOG_PARAM_STRING(symbol_table->member_error_message),
        TP_LOG_PARAM_STRING(file_name),
        TP_LOG_PARAM_STRING(line_buffer),
        TP_LOG_PARAM_STRING(error_reason)
    );

    TP_FREE(symbol_table, &line_buffer, line_length);

    TP_FREE(symbol_table, &error_reason, error_reason_length);

    return true;
}

bool tp_parse_error_recovery(TP_SYMBOL_TABLE* symbol_table)
{
    symbol_table->member_is_error_recovery = true;

    if (symbol_table->member_is_error_abort){

abort_exit:

        TP_PUT_LOG_PRINT(
            symbol_table, TP_MSG_FMT("Abort of compile: error(%1), warning(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_error_count),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_warning_count)
        );

        return true; // stop error recovery.
    }

    bool status = false; // continue error recovery.

    TP_TOKEN* token = NULL;

    if (symbol_table->member_error_reason_count_prev < symbol_table->member_error_reason_count){

        symbol_table->member_error_reason_count_prev = symbol_table->member_error_reason_count;

        if ( ! tp_put_error_message(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        token = symbol_table->member_error_token;

        if (NULL == token){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto abort_exit;
        }

        for (; token && (TP_SYMBOL_NULL < token->member_symbol); ++token){

            for (rsize_t i = 0; TP_SYMBOL_NULL < symbol_table->member_first_set[i].member_symbol; ++i){

                switch (symbol_table->member_first_set[i].member_symbol){
                case TP_SYMBOL_PUNCTUATOR:
//                  break;
                case TP_SYMBOL_KEYWORD:
                    if (0 == strncmp(
                        token->member_string,
                        symbol_table->member_first_set[i].member_string, token->member_string_length)){

                        goto match;
                    }
                    break;
                case TP_SYMBOL_IDENTIFIER:
//                  break;
                case TP_SYMBOL_CONSTANT:
                    if (symbol_table->member_first_set[i].member_symbol == token->member_symbol){

                        goto match;
                    }
                    break;
                default:
                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    goto abort_exit;
                }

                for (rsize_t j = 0; TP_SYMBOL_NULL < symbol_table->member_follow_set[j].member_symbol; ++j){

                    switch (symbol_table->member_follow_set[j].member_symbol){
                    case TP_SYMBOL_EOF:
                        if (TP_SYMBOL_NULL == token->member_symbol){

                            goto match;
                        }
                        break;
                    case TP_SYMBOL_PUNCTUATOR:
//                      break;
                    case TP_SYMBOL_KEYWORD:
                        if (0 == strncmp(
                            token->member_string,
                            symbol_table->member_follow_set[j].member_string, token->member_string_length)){

                            goto match;
                        }
                        break;
                    case TP_SYMBOL_IDENTIFIER:
//                      break;
                    case TP_SYMBOL_CONSTANT:
                        if (symbol_table->member_follow_set[j].member_symbol == token->member_symbol){

                            goto match;
                        }
                        break;
                    default:
                        TP_PUT_LOG_MSG_ICE(symbol_table);

                        goto abort_exit;
                    }
                }
            }
        }

match:
        if (NULL == token){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            goto abort_exit;
        }

        TP_POS(symbol_table) = token;

        status = true; // stop error recovery.
    }

    if ( ! tp_set_error_reason(symbol_table, NULL, NULL, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        status = true; // stop error recovery.
    }

    return status;
}

TP_PARSE_TREE* tp_make_parse_subtree(
    TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE_GRAMMER grammer,
    TP_PARSE_TREE_ELEMENT* parse_tree_element, size_t parse_tree_element_num)
{
    TP_PARSE_TREE* parse_subtree = (TP_PARSE_TREE*)TP_CALLOC(symbol_table, 1, sizeof(TP_PARSE_TREE));

    if (NULL == parse_subtree){

        TP_PRINT_CRT_ERROR(symbol_table);

        return NULL;
    }

    parse_subtree->member_grammer = grammer;
    parse_subtree->member_element_num = parse_tree_element_num;
    parse_subtree->member_element = (TP_PARSE_TREE_ELEMENT*)TP_CALLOC(
        symbol_table, parse_tree_element_num + 1, sizeof(TP_PARSE_TREE_ELEMENT)
    );

    if (NULL == parse_subtree->member_element){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(symbol_table, &parse_subtree, sizeof(TP_PARSE_TREE));

        return NULL;
    }

    memcpy(
        parse_subtree->member_element, parse_tree_element,
        sizeof(TP_PARSE_TREE_ELEMENT) * parse_tree_element_num
    );
    TP_PARSE_TREE_ELEMENT null = TP_TREE_TOKEN_NULL;
    memcpy(
        parse_subtree->member_element + parse_tree_element_num, &null,
        sizeof(TP_PARSE_TREE_ELEMENT)
    );

    if (((false == symbol_table->member_is_no_output_files) ||
        (symbol_table->member_is_no_output_files &&
        symbol_table->member_is_output_parse_tree_file)) &&
        (false == symbol_table->member_is_end_of_make_parse_tree)){

        uint8_t indent_level = 1;

        if ( ! dump_parse_tree_main(symbol_table, parse_subtree, indent_level)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return NULL;
        }

        fprintf(symbol_table->member_parse_tree_file, "---------------------------\n\n");
    }

    return parse_subtree;
}

void tp_free_parse_subtree(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE** parse_subtree)
{
    if (NULL == parse_subtree){

        return;
    }

    if (*parse_subtree){

        size_t element_num = (*parse_subtree)->member_element_num;

        for (size_t i = 0; element_num > i; ++i){

            // NOTE: TP_PARSE_TREE_TYPE_TOKEN must not free memory.

            if (TP_PARSE_TREE_TYPE_NODE == (*parse_subtree)->member_element[i].member_type){

                tp_free_parse_subtree(
                    symbol_table, &((*parse_subtree)->member_element[i].member_body.member_child)
                );
            }
        }

        if ((*parse_subtree)->member_element){

            TP_FREE(symbol_table, &((*parse_subtree)->member_element),
                ((*parse_subtree)->member_element_num + 1) * sizeof(TP_PARSE_TREE_ELEMENT));
        }

        TP_FREE(symbol_table, parse_subtree, sizeof(TP_PARSE_TREE));
    }
}

static bool dump_parse_tree_open(TP_SYMBOL_TABLE* symbol_table, char* path)
{
    if ( ! tp_open_write_file(symbol_table, path, &(symbol_table->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool dump_parse_tree_main(TP_SYMBOL_TABLE* symbol_table, TP_PARSE_TREE* parse_tree, uint8_t indent_level)
{
    errno_t err = 0;

    if (NULL == parse_tree){

        return true;
    }

    FILE* write_file = symbol_table->member_parse_tree_file;

    if ((UINT8_MAX == indent_level) || (0 == indent_level)){

        fprintf(write_file, "NOTE: Bad indent level(%d).\n", indent_level);
        fprintf(write_file, "NOTE: Stop dump parse tree.\n\n");

        err = _set_errno(0);

        return true;
    }

    TP_MAKE_INDENT_STRING(indent_level);

    fprintf(write_file, "%s{\n", prev_indent_string);

    if ( ! tp_print_parse_tree_grammer(symbol_table, parse_tree, write_file, indent_string)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    TP_PARSE_TREE_ELEMENT* parse_tree_element = parse_tree->member_element;

    size_t element_num = parse_tree->member_element_num;

    for (size_t i = 0; element_num > i; ++i){

        switch (parse_tree_element[i].member_type){
        case TP_PARSE_TREE_TYPE_NULL:
            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_NULL\n", indent_string);
            break;
        case TP_PARSE_TREE_TYPE_TOKEN:

            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_TOKEN\n", indent_string);

            if ( ! tp_dump_token_main(
                symbol_table, write_file,
                parse_tree_element[i].member_body.member_tp_token, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            break;
        case TP_PARSE_TREE_TYPE_NODE:

            fprintf(write_file, "%sTP_PARSE_TREE_TYPE_NODE\n", indent_string);

            if ( ! dump_parse_tree_main(
                symbol_table, (TP_PARSE_TREE*)parse_tree_element[i].member_body.member_child, indent_level + 1)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            break;
        default:
            fprintf(write_file, "%sTP_PARSE_TREE_TYPE(UNKNOWN_TYPE: %d)\n",
                indent_string, parse_tree_element[i].member_type);
            break;
        }
    }

    fprintf(write_file, "%s}\n\n", prev_indent_string);

    err = _set_errno(0);

    return true;
}

static bool dump_parse_tree_close(TP_SYMBOL_TABLE* symbol_table)
{
    if ( ! tp_close_file(symbol_table, &(symbol_table->member_parse_tree_file))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static TP_PARSE_TREE* parse_program(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    if ( ! tp_get_grammer_1_first_set(
        symbol_table, TP_GRAMMER_1_STATEMENT_FIRST_FOLLOW_SET_INDEX, TP_PARSE_TREE_GRAMMER_STATEMENT)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return NULL;
    }

    if ( ! tp_get_grammer_1_follow_set(
        symbol_table, TP_GRAMMER_1_PROGRAM_FIRST_FOLLOW_SET_INDEX, TP_PARSE_TREE_GRAMMER_PROGRAM)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return NULL;
    }

    // Grammer: Program -> Statement+
    {
        TP_PARSE_TREE* tmp_statement_1 = NULL;
        TP_PARSE_TREE* tmp_statement_2 = NULL;

restart_1:

        if ( ! (tmp_statement_1 = parse_statement(symbol_table))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if ( ! IS_END_OF_TOKEN(symbol_table->member_tp_token_position)){

                if ( ! tp_parse_error_recovery(symbol_table)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip_1;
                }

                goto restart_1;
            }
        }

restart_2:

        while (tmp_statement_2 = parse_statement(symbol_table)){

            tmp_statement_1 = MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_PROGRAM,
                TP_TREE_NODE(tmp_statement_1),
                TP_TREE_NODE(tmp_statement_2)
            );
        }

        if ( ! IS_END_OF_TOKEN(symbol_table->member_tp_token_position)){

            if ( ! tp_parse_error_recovery(symbol_table)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto skip_2;
            }

            goto restart_2;
        }else{

            if (false == symbol_table->member_is_error_recovery){

                symbol_table->member_error_reason_count = 0;
            }
        }

skip_2:
        return tmp_statement_1;

skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_statement(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Statement -> Type? variable '=' Expression ';'
    {
        TP_TOKEN* tmp_type = NULL;
        TP_TOKEN* tmp_variable = NULL;
        TP_TOKEN* tmp_equal = NULL;
        TP_PARSE_TREE* tmp_expression = NULL;
        TP_TOKEN* tmp_semicolon = NULL;

        if (IS_TOKEN_ID(TP_POS(symbol_table)) || IS_TOKEN_KEYWORD(TP_POS(symbol_table))){

            tmp_type = TP_POS(symbol_table)++;

            if (IS_TOKEN_KEYWORD_INT(tmp_type)){

                tmp_type->member_symbol_type = TP_SYMBOL_TYPE_INT32;

                if ( ! (IS_TOKEN_ID(TP_POS(symbol_table)))){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    if ( ! tp_set_error_reason(symbol_table, TP_POS(symbol_table), "ERROR: Bad terminal symbol.", "It is not variable.")){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return NULL;
                    }

                    goto skip;
                }

                tmp_variable = TP_POS(symbol_table)++;

                tmp_variable->member_symbol_type = TP_SYMBOL_ID_INT32;

            }else{

                tmp_variable = tmp_type;

                tmp_type = NULL;

                tmp_variable->member_symbol_type = TP_SYMBOL_ID_INT32;
            }
        }else{

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if ( ! tp_set_error_reason(symbol_table, TP_POS(symbol_table), "ERROR: Bad terminal symbol.", "Neither type nor constant.")){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return NULL;
            }

            goto skip;
        }

        if ( ! (IS_TOKEN_EQUAL(TP_POS(symbol_table)))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            if ( ! tp_set_error_reason(symbol_table, TP_POS(symbol_table), "ERROR: Bad terminal symbol.", "It is not =.")){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return NULL;
            }

            goto skip;
        }

        tmp_equal = TP_POS(symbol_table)++;

        if (tmp_expression = parse_expression(symbol_table)){

            if ( ! (IS_TOKEN_SEMICOLON(TP_POS(symbol_table)))){

                static TP_TOKEN warning_semicolon_token = {
                    .member_symbol = TP_SYMBOL_PUNCTUATOR,
                    .member_symbol_kind = TP_SYMBOL_KIND_SEMICOLON,
                    .member_file = "insert semicolon.",
                    .member_string = ";",
                    .member_string_length = 1,
                };

                tmp_semicolon = &warning_semicolon_token;

                TP_PUT_LOG_WARNING(
                    symbol_table, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING("WARNING: insert semicolon.")
                );
            }else{

                tmp_semicolon = TP_POS(symbol_table)++;
            }

            if (NULL == tmp_type){

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STATEMENT_1,
                    TP_TREE_TOKEN(tmp_variable),
                    TP_TREE_TOKEN(tmp_equal),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }else{

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_STATEMENT_2,
                    TP_TREE_TOKEN(tmp_type),
                    TP_TREE_TOKEN(tmp_variable),
                    TP_TREE_TOKEN(tmp_equal),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_semicolon)
                );
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_expression(TP_SYMBOL_TABLE* symbol_table)
{
    if (NESTING_LEVEL_OF_EXPRESSION_MAXIMUM <=
        symbol_table->member_nesting_level_of_expression){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: %1 over NESTING_LEVEL_OF_EXPRESSION_MAXIMUM(%2) error."),
            TP_LOG_PARAM_INT32_VALUE(symbol_table->member_nesting_level_of_expression),
            TP_LOG_PARAM_INT32_VALUE(NESTING_LEVEL_OF_EXPRESSION_MAXIMUM)
        );

        return NULL;
    }

    ++(symbol_table->member_nesting_level_of_expression);

    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Expression -> Term (('+' | '-') Term)*
    // Example: ((1 + 2) - 3) + 4)
    {
        TP_PARSE_TREE* tmp_expression_1 = NULL;

        TP_PARSE_TREE* tmp_term_1 = NULL;
        TP_TOKEN* tmp_plus_or_minus = NULL;
        TP_PARSE_TREE* tmp_term_2 = NULL;

        if (tmp_term_1 = parse_term(symbol_table)){

            while (IS_TOKEN_PLUS(TP_POS(symbol_table)) || IS_TOKEN_MINUS(TP_POS(symbol_table))){

                tmp_plus_or_minus = TP_POS(symbol_table)++;

                if ( ! (tmp_term_2 = parse_term(symbol_table))){

                    tp_free_parse_subtree(symbol_table, &tmp_expression_1);
                    tp_free_parse_subtree(symbol_table, &tmp_term_1);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip;
                }

                if (NULL == tmp_expression_1){

                    tmp_expression_1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EXPRESSION_1,
                        TP_TREE_NODE(tmp_term_1),
                        TP_TREE_TOKEN(tmp_plus_or_minus),
                        TP_TREE_NODE(tmp_term_2)
                    );

                    if (NULL == tmp_expression_1){

                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }
                }else{

                    TP_PARSE_TREE* tmp_expression_2 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_EXPRESSION_2,
                        TP_TREE_NODE(tmp_expression_1),
                        TP_TREE_TOKEN(tmp_plus_or_minus),
                        TP_TREE_NODE(tmp_term_2)
                    );

                    if (NULL == tmp_expression_2){

                        tp_free_parse_subtree(symbol_table, &tmp_expression_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_term_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }

                    tmp_expression_1 = tmp_expression_2;
                }
            }

            if (tmp_expression_1){

                return tmp_expression_1;
            }else{

                return tmp_term_1;
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_term(TP_SYMBOL_TABLE* symbol_table)
{
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Grammer: Term -> Factor (('*' | '/') Factor)*
    // Example: ((1 * 2) / 3) * 4)
    {
        TP_PARSE_TREE* tmp_term_1 = NULL;

        TP_PARSE_TREE* tmp_factor_1 = NULL;
        TP_TOKEN* tmp_mul_or_div = NULL;
        TP_PARSE_TREE* tmp_factor_2 = NULL;

        if (tmp_factor_1 = parse_factor(symbol_table)){

            while (IS_TOKEN_MUL(TP_POS(symbol_table)) || IS_TOKEN_DIV(TP_POS(symbol_table))){

                tmp_mul_or_div = TP_POS(symbol_table)++;

                if ( ! (tmp_factor_2 = parse_factor(symbol_table))){

                    tp_free_parse_subtree(symbol_table, &tmp_factor_1);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    goto skip;
                }

                if (NULL == tmp_term_1){

                    tmp_term_1 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_TERM_1,
                        TP_TREE_NODE(tmp_factor_1),
                        TP_TREE_TOKEN(tmp_mul_or_div),
                        TP_TREE_NODE(tmp_factor_2)
                    );

                    if (NULL == tmp_term_1){

                        tp_free_parse_subtree(symbol_table, &tmp_factor_1);
                        tp_free_parse_subtree(symbol_table, &tmp_factor_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }
                }else{

                    TP_PARSE_TREE* tmp_term_2 = MAKE_PARSE_SUBTREE(
                        symbol_table,
                        TP_PARSE_TREE_GRAMMER_TERM_2,
                        TP_TREE_NODE(tmp_term_1),
                        TP_TREE_TOKEN(tmp_mul_or_div),
                        TP_TREE_NODE(tmp_factor_2)
                    );

                    if (NULL == tmp_term_2){

                        tp_free_parse_subtree(symbol_table, &tmp_term_1);
                        tp_free_parse_subtree(symbol_table, &tmp_factor_2);

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        goto skip;
                    }

                    tmp_term_1 =  tmp_term_2;
                }
            }

            if (tmp_term_1){

                return tmp_term_1;
            }else{

                return tmp_factor_1;
            }
        }
skip:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

static TP_PARSE_TREE* parse_factor(TP_SYMBOL_TABLE* symbol_table)
{
    // Grammer: Factor -> '(' Expression ')' | ('+' | '-')? (variable | constant)
    TP_TOKEN* backup_token_position = TP_POS(symbol_table);

    // Factor -> '(' Expression ')'
    {
        TP_TOKEN* tmp_left_paren = TP_POS(symbol_table);

        if (IS_TOKEN_LEFT_PAREN(tmp_left_paren)){

            ++TP_POS(symbol_table);

            TP_PARSE_TREE* tmp_expression = NULL;

            if (tmp_expression = parse_expression(symbol_table)){

                TP_TOKEN* tmp_right_paren = TP_POS(symbol_table);

                if ( ! IS_TOKEN_RIGHT_PAREN(tmp_right_paren)){

                    tp_free_parse_subtree(symbol_table, &tmp_expression);

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    if ( ! tp_set_error_reason(symbol_table, tmp_right_paren, "ERROR: Bad terminal symbol.", "It is not ).")){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return NULL;
                    }

                    goto skip_1;
                }

                ++TP_POS(symbol_table);

                return MAKE_PARSE_SUBTREE(
                    symbol_table,
                    TP_PARSE_TREE_GRAMMER_FACTOR_1,
                    TP_TREE_TOKEN(tmp_left_paren),
                    TP_TREE_NODE(tmp_expression),
                    TP_TREE_TOKEN(tmp_right_paren)
                );
            }
        }
skip_1:
        TP_POS(symbol_table) = backup_token_position;
    }

    // Factor -> ('+' | '-')? (variable | constant)
    {
        if (IS_TOKEN_PLUS(TP_POS(symbol_table)) || IS_TOKEN_MINUS(TP_POS(symbol_table))){

            TP_TOKEN* tmp_plus_or_minus = TP_POS(symbol_table)++;

            bool is_const = IS_TOKEN_TYPE_CONST_VALUE_INT32(TP_POS(symbol_table));

            if ( ! (IS_TOKEN_ID(TP_POS(symbol_table)) || is_const)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                if ( ! tp_set_error_reason(
                    symbol_table, TP_POS(symbol_table),
                    "ERROR: Bad terminal symbol.", "Neither variable nor constant.")){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return NULL;
                }

                goto skip_2;
            }

            TP_TOKEN* tmp_variable_or_constant = TP_POS(symbol_table)++;

            if ( ! is_const){

                tmp_variable_or_constant->member_symbol_type = TP_SYMBOL_ID_INT32;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FACTOR_2,
                TP_TREE_TOKEN(tmp_plus_or_minus),
                TP_TREE_TOKEN(tmp_variable_or_constant)
            );
        }else if (IS_TOKEN_ID(TP_POS(symbol_table)) || IS_TOKEN_TYPE_CONST_VALUE_INT32(TP_POS(symbol_table))){

            bool is_const = IS_TOKEN_TYPE_CONST_VALUE_INT32(TP_POS(symbol_table));

            TP_TOKEN* tmp_variable_or_constant = TP_POS(symbol_table)++;

            if ( ! is_const){

                tmp_variable_or_constant->member_symbol_type = TP_SYMBOL_ID_INT32;
            }

            return MAKE_PARSE_SUBTREE(
                symbol_table,
                TP_PARSE_TREE_GRAMMER_FACTOR_3,
                TP_TREE_TOKEN(tmp_variable_or_constant)
            );
        }

        if ( ! tp_set_error_reason(
            symbol_table, backup_token_position,
            "ERROR: Bad terminal symbol.", "Neither (, +, -, variable nor constant.")){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return NULL;
        }

skip_2:
        TP_POS(symbol_table) = backup_token_position;
    }

    return NULL;
}

