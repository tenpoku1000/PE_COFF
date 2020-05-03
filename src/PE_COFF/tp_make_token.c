
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "tp_compiler.h"

// Example:
// int32_t value1 = (1 + 2) * 3;
// int32_t value2 = 2 + (3 * value1);
// value1 = value2 + 100;
//
// Grammer:
// '+' = TP_SYMBOL_PLUS
// '-' = TP_SYMBOL_MINUS
// '*' = TP_SYMBOL_MUL
// '/' = TP_SYMBOL_DIV
// '(' = TP_SYMBOL_LEFT_PAREN
// ')' = TP_SYMBOL_RIGHT_PAREN
// '=' = TP_SYMBOL_EQUAL
// ';' = TP_SYMBOL_SEMICOLON
// [0-9]+ = TP_SYMBOL_CONST_VALUE
// [^0-9+-*/()=;][^+-*/()=;]* = TP_SYMBOL_ID

static const TP_TOKEN token_template = {
    .member_symbol = TP_SYMBOL_ID,
    .member_symbol_type = TP_SYMBOL_UNSPECIFIED_TYPE,
    .member_line = 0,
    .member_column = 0,
    .member_string = { 0 },
    .member_i32_value = 0
};

static bool make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length);
static bool make_token_const_value(
    TP_SYMBOL_TABLE* symbol_table, uint8_t** current_pos, uint8_t* end_pos, rsize_t* column, TP_TOKEN* token
);
static bool make_token_id(
    TP_SYMBOL_TABLE* symbol_table, int8_t** current_pos, uint8_t* end_pos, rsize_t* column, TP_TOKEN* token
);
static bool append_token(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token);
static bool dump_token(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool get_lines(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length);
static bool read_file(TP_SYMBOL_TABLE* symbol_table);
static bool normalize_buffer(TP_SYMBOL_TABLE* symbol_table, int count, size_t fread_bytes);
static bool normalize_crlf(uint8_t* read_lines_buffer, size_t fread_bytes);
static bool is_valid_utf_8(TP_SYMBOL_TABLE* symbol_table, uint8_t* lines_buffer, size_t read_lines_length);

bool tp_make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length)
{
    bool is_from_memory = (string && string_length);

    if ( ! is_from_memory){

        if ( ! tp_open_read_file(symbol_table, symbol_table->member_input_file_path, &(symbol_table->member_read_file))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }

        if (symbol_table->member_is_end_of_file){ 

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto error_proc;
        }
    }

    if ( ! make_token(symbol_table, string, string_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto error_proc;
    }

    TP_TOKEN token = token_template;
    token.member_symbol = TP_SYMBOL_NULL;

    if ( ! append_token(symbol_table, &token)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto error_proc;
    }

    if ( ! is_from_memory){

        if ( ! tp_close_file(symbol_table, &(symbol_table->member_read_file))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

//          return false;
        }
    }

    symbol_table->member_tp_token_position = symbol_table->member_tp_token;

    if (symbol_table->member_nul_num){

        TP_PUT_LOG_MSG(
            symbol_table,
            TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("NOTE: %1 number of NUL characters."),
            TP_LOG_PARAM_UINT64_VALUE(symbol_table->member_nul_num)
        );
    }

    if (symbol_table->member_is_output_log_file){

        if ( ! dump_token(symbol_table, symbol_table->member_token_file_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;

error_proc:

    if ( ! tp_close_file(symbol_table, &(symbol_table->member_read_file))){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
    }

    return false;
}

static bool make_token(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length)
{
    if ( ! get_lines(symbol_table, string, string_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (symbol_table->member_is_end_of_file && (0 == symbol_table->member_read_lines_length)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: symbol_table->member_is_end_of_file && (0 == symbol_table->member_read_lines_length)")
        );

        return false;
    }

    TP_TOKEN token = { 0 };

    uint8_t* current_pos = symbol_table->member_read_lines_current_position;
    uint8_t* end_pos = symbol_table->member_read_lines_end_position;
    rsize_t line_num = 0;
    rsize_t column = 0;

    for (;;){

        if (current_pos == end_pos){

            bool is_from_memory = (string && string_length);

            if (is_from_memory){

                if (';' == *current_pos){

                    token = token_template;
                    token.member_line = line_num;
                    token.member_column = column;
                    token.member_symbol = TP_SYMBOL_SEMICOLON;

                    if ( ! append_token(symbol_table, &token)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }
                }

                return true;
            }

            if ( ! get_lines(symbol_table, string, string_length)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (symbol_table->member_is_end_of_file && (0 == symbol_table->member_read_lines_length)){

                return true;
            }
        }

        if (isblank(*current_pos) || iscntrl(*current_pos)){

            if ('\n' == *current_pos){

                ++line_num;
                column = 0;
            }else{

                ++column;
            }

            ++current_pos;

            continue;
        }

        token = token_template;
        token.member_line = line_num;
        token.member_column = column;

        switch (*current_pos){
        case '+': token.member_symbol = TP_SYMBOL_PLUS; ++column; ++current_pos; goto next;
        case '-': token.member_symbol = TP_SYMBOL_MINUS; ++column; ++current_pos; goto next;
        case '*': token.member_symbol = TP_SYMBOL_MUL; ++column; ++current_pos; goto next;
        case '/': token.member_symbol = TP_SYMBOL_DIV; ++column; ++current_pos; goto next;
        case '(': token.member_symbol = TP_SYMBOL_LEFT_PAREN; ++column; ++current_pos; goto next;
        case ')': token.member_symbol = TP_SYMBOL_RIGHT_PAREN; ++column; ++current_pos; goto next;
        case '=': token.member_symbol = TP_SYMBOL_EQUAL; ++column; ++current_pos; goto next;
        case ';': token.member_symbol = TP_SYMBOL_SEMICOLON; ++column; ++current_pos; goto next;
        default:
            break;
        }

        if (isdigit(*current_pos)){

            // TP_SYMBOL_CONST_VALUE
            if ( ! make_token_const_value(
                symbol_table, &current_pos, end_pos, &column, &token)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            goto next;
        }

        // TP_SYMBOL_ID
        if ( ! make_token_id(
            symbol_table, &current_pos, end_pos, &column, &token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

next:
        if ( ! append_token(symbol_table, &token)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    symbol_table->member_read_lines_current_position = current_pos;
    symbol_table->member_read_lines_end_position = end_pos;

    return true;
}

static bool make_token_const_value(
    TP_SYMBOL_TABLE* symbol_table, uint8_t** current_pos, uint8_t* end_pos, rsize_t* column, TP_TOKEN* token)
{
    uint8_t* digit_pos = *current_pos;

    for (++digit_pos; digit_pos != end_pos; ){

        if ( ! isdigit(*digit_pos)){

            break;
        }

        ++digit_pos;
    }

    token->member_symbol = TP_SYMBOL_CONST_VALUE;

    rsize_t digit_length = digit_pos - (*current_pos);

    if (TP_MAX_ID_BYTES < digit_length){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: TP_MAX_ID_BYTES(%1) < digit_length(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(TP_MAX_ID_BYTES),
            TP_LOG_PARAM_UINT64_VALUE(digit_length)
        );

        return false;
    }

    memcpy(token->member_string, *current_pos, digit_length);

    memset(token->member_string + digit_length, '\0', TP_ID_SIZE - digit_length);

    *current_pos = digit_pos;

    *column += digit_length;

    return true;
}

static bool make_token_id(
    TP_SYMBOL_TABLE* symbol_table, int8_t** current_pos, uint8_t* end_pos, rsize_t* column, TP_TOKEN* token)
{
    uint8_t* id_pos = *current_pos;

    for (++id_pos; id_pos != end_pos; ){

        switch (*id_pos){
        case '+': goto out;
        case '-': goto out;
        case '*': goto out;
        case '/': goto out;
        case '(': goto out;
        case ')': goto out;
        case '=': goto out;
        case ';': goto out;
        default:
            break;
        }

        if (isblank(*id_pos) || iscntrl(*id_pos)){

            break;
        }

        ++id_pos;
    }

out:

    token->member_symbol = TP_SYMBOL_ID;

    rsize_t id_length = id_pos - (*current_pos);

    if (TP_MAX_ID_BYTES < id_length){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: TP_MAX_ID_BYTES(%1) < id_length(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(TP_MAX_ID_BYTES),
            TP_LOG_PARAM_UINT64_VALUE(id_length)
        );

        return false;
    }

    memcpy(token->member_string, *current_pos, id_length);

    memset(token->member_string + id_length, '\0', TP_ID_SIZE - id_length);

    *current_pos = id_pos;

    *column += id_length;

    return true;
}

static bool append_token(TP_SYMBOL_TABLE* symbol_table, TP_TOKEN* token)
{
    if (symbol_table->member_tp_token_pos == (symbol_table->member_tp_token_size / sizeof(TP_TOKEN))){

        rsize_t tp_token_size_allocate_unit = 
            symbol_table->member_tp_token_size_allocate_unit * sizeof(TP_TOKEN);

        rsize_t tp_token_size =  symbol_table->member_tp_token_size + tp_token_size_allocate_unit;

        TP_TOKEN* tp_token = (TP_TOKEN*)realloc(
            symbol_table->member_tp_token, tp_token_size
        );

        if (NULL == tp_token){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((uint8_t*)tp_token) + symbol_table->member_tp_token_size, 0,
            tp_token_size_allocate_unit
        );

        symbol_table->member_tp_token = tp_token;
        symbol_table->member_tp_token_size = tp_token_size;
    }

    symbol_table->member_tp_token[symbol_table->member_tp_token_pos] = *token;

    ++(symbol_table->member_tp_token_pos);

    return true;
}

static bool dump_token(TP_SYMBOL_TABLE* symbol_table, char* path)
{
    FILE* write_file = NULL;

    if ( ! tp_open_write_file(symbol_table, path, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint8_t indent_level = 1;

    for (TP_TOKEN* token = symbol_table->member_tp_token;
        TP_SYMBOL_NULL != token->member_symbol; ++token){

        if ( ! tp_dump_token_main(symbol_table, write_file, token, indent_level)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

//      return false;
    }

    return true;
}

bool tp_dump_token_main(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_TOKEN* token, uint8_t indent_level)
{
    errno_t err = 0;

    if ((UINT8_MAX == indent_level) || (0 == indent_level)){

        fprintf(write_file, "NOTE: Bad indent level(%d).\n", indent_level);
        fprintf(write_file, "NOTE: Stop dump token.\n\n");

        err = _set_errno(0);

        return true;
    }

    TP_MAKE_INDENT_STRING(indent_level);

    fprintf(write_file, "%s{\n", prev_indent_string);

    switch (token->member_symbol){
    case TP_SYMBOL_NULL:
        fprintf(write_file, "%sTP_SYMBOL_NULL\n", indent_string);
        break;
    case TP_SYMBOL_ID:
        fprintf(write_file, "%sTP_SYMBOL_ID\n", indent_string);
        break;
    case TP_SYMBOL_CONST_VALUE:
        fprintf(write_file, "%sTP_SYMBOL_CONST_VALUE\n", indent_string);
        break;
    case TP_SYMBOL_PLUS:
        fprintf(write_file, "%sTP_SYMBOL_PLUS\n", indent_string);
        break;
    case TP_SYMBOL_MINUS:
        fprintf(write_file, "%sTP_SYMBOL_MINUS\n", indent_string);
        break;
    case TP_SYMBOL_MUL:
        fprintf(write_file, "%sTP_SYMBOL_MUL\n", indent_string);
        break;
    case TP_SYMBOL_DIV:
        fprintf(write_file, "%sTP_SYMBOL_DIV\n", indent_string);
        break;
    case TP_SYMBOL_LEFT_PAREN:
        fprintf(write_file, "%sTP_SYMBOL_LEFT_PAREN\n", indent_string);
        break;
    case TP_SYMBOL_RIGHT_PAREN:
        fprintf(write_file, "%sTP_SYMBOL_RIGHT_PAREN\n", indent_string);
        break;
    case TP_SYMBOL_EQUAL:
        fprintf(write_file, "%sTP_SYMBOL_EQUAL\n", indent_string);
        break;
    case TP_SYMBOL_SEMICOLON:
        fprintf(write_file, "%sTP_SYMBOL_SEMICOLON\n", indent_string);
        break;
    default:
        fprintf(write_file, "%sTP_SYMBOL(UNKNOWN_SYMBOL: %d)\n",
            indent_string, token->member_symbol);
        break;
    }

    switch (token->member_symbol_type){
    case TP_SYMBOL_UNSPECIFIED_TYPE:
        fprintf(write_file, "%sTP_SYMBOL_UNSPECIFIED_TYPE\n", indent_string);
        break;
    case TP_SYMBOL_ID_INT32:
        fprintf(write_file, "%sTP_SYMBOL_ID_INT32\n", indent_string);
        break;
    case TP_SYMBOL_TYPE_INT32:
        fprintf(write_file, "%sTP_SYMBOL_TYPE_INT32\n", indent_string);
        break;
    case TP_SYMBOL_CONST_VALUE_INT32:
        fprintf(write_file, "%sTP_SYMBOL_CONST_VALUE_INT32\n", indent_string);
        break;
    default:
        fprintf(write_file, "%sTP_SYMBOL_TYPE(UNKNOWN_SYMBOL_TYPE: %d)\n",
            indent_string, token->member_symbol_type);
        break;
    }

    fprintf(write_file, "%s member_line(%zd)\n", indent_string, token->member_line);
    fprintf(write_file, "%s member_column(%zd)\n", indent_string, token->member_column);
    fprintf(write_file, "%s member_string(%s)\n", indent_string, token->member_string);
    fprintf(write_file, "%s member_i32_value(%d)\n", indent_string, token->member_i32_value);

    fprintf(write_file, "%s}\n\n", prev_indent_string);

    err = _set_errno(0);

    return true;
}

static bool get_lines(TP_SYMBOL_TABLE* symbol_table, uint8_t* string, rsize_t string_length)
{
    bool is_from_memory = (string && string_length);

    if (is_from_memory){

        if (false == symbol_table->member_is_end_of_file){

            symbol_table->member_is_end_of_file = true;

            if (TP_MAX_LINE_BYTES < string_length){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: TP_MAX_LINE_BYTES(%1) < string_length(%2)"),
                    TP_LOG_PARAM_UINT64_VALUE(TP_MAX_LINE_BYTES),
                    TP_LOG_PARAM_UINT64_VALUE(string_length)
                );

                return false;
            }

            memcpy(symbol_table->member_read_lines_buffer, string, string_length);

            symbol_table->member_read_lines_length = string_length;
        }
    }else{

        if ( ! read_file(symbol_table)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if (symbol_table->member_is_end_of_file && (0 == symbol_table->member_read_lines_length)){

        return true;
    }

    uint8_t* lines_buffer = symbol_table->member_read_lines_buffer;

    if (symbol_table->member_is_start_of_file){

        // Clear Byte Order Mark.
        static const uint8_t byte_order_mark[] = { 0xEF, 0xBB, 0xBF };

        if (0 == memcmp(lines_buffer, byte_order_mark, sizeof(byte_order_mark))){

            memset(lines_buffer, ' ', sizeof(byte_order_mark));
        }

        symbol_table->member_is_start_of_file = false;
    }

    symbol_table->member_read_lines_current_position = lines_buffer;
    symbol_table->member_read_lines_end_position = &(lines_buffer[symbol_table->member_read_lines_length - 1]);

    if ( ! is_valid_utf_8(symbol_table, lines_buffer, symbol_table->member_read_lines_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool read_file(TP_SYMBOL_TABLE* symbol_table)
{
    FILE* stream = symbol_table->member_read_file;

    int count = TP_MAX_LINE_BYTES;

    size_t fread_bytes = fread(
        symbol_table->member_read_lines_buffer,
        sizeof(uint8_t), count, stream
    );

    if (count > fread_bytes){

        int ferror_error = ferror(stream);

        if (ferror_error){

            TP_PRINT_CRT_ERROR(symbol_table);

            clearerr(stream);

            return false;
        }

        symbol_table->member_read_lines_length = fread_bytes;

        int feof_error = feof(stream);

        if (feof_error){

            symbol_table->member_is_end_of_file = true;

            if (0 == fread_bytes){

                return true;
            }

            return normalize_buffer(symbol_table, count, fread_bytes);
        }
    }

    symbol_table->member_read_lines_length = fread_bytes;

    return normalize_buffer(symbol_table, count, fread_bytes);
}

static bool normalize_buffer(TP_SYMBOL_TABLE* symbol_table, int count, size_t fread_bytes)
{
    if (count == fread_bytes){

        symbol_table->member_read_lines_buffer[TP_BUFFER_SIZE - 1] = '\0';
    }else{

        size_t clear_bytes = count - fread_bytes;

        memset(symbol_table->member_read_lines_buffer + fread_bytes, '\0', clear_bytes);
    }

    return normalize_crlf(symbol_table->member_read_lines_buffer, fread_bytes);
}

static bool normalize_crlf(uint8_t* read_lines_buffer, size_t fread_bytes)
{
    for (size_t i = 0; fread_bytes > i; ++i){

        uint8_t c1 = read_lines_buffer[i];

        if (fread_bytes > (i + 1)){

            uint8_t c2 = read_lines_buffer[i + 1];

            if (('\r' == c1) && ('\n' == c2)){

                read_lines_buffer[i] = '\n';
                read_lines_buffer[i + 1] = ' ';
            }

            if (('\r' == c1) && ('\n' != c2)){

                read_lines_buffer[i] = '\n';
            }
        }else{

            if ('\r' == c1){

                read_lines_buffer[i] = '\n';
            }
        }
    }

    return true;
}

static bool is_valid_utf_8(TP_SYMBOL_TABLE* symbol_table, uint8_t* lines_buffer, size_t read_lines_length)
{
    uint8_t* p = lines_buffer;

    for (size_t i = 0; read_lines_length > i; ++i){

        // First byte: ASCII
        if (((uint8_t)0x00 <= p[i]) && ((uint8_t)0x7F >= p[i])){

            // Clear NUL byte.
            if ((uint8_t)0x00 == p[i]){

                p[i] = ' ';

                ++(symbol_table->member_nul_num);
            }

            continue;
        }

        int32_t total_bytes = 0;

        bool is_lead_byte_zero = false;

        // First byte.
        if (((uint8_t)0xC0 <= p[i]) && ((uint8_t)0xDF >= p[i])){

            total_bytes = 2;

            is_lead_byte_zero = ((uint8_t)0xC0 == p[i]) ? true : false;

        }else if (((uint8_t)0xE0 <= p[i]) && ((uint8_t)0xEF >= p[i])){

            total_bytes = 3;

            is_lead_byte_zero = ((uint8_t)0xE0 == p[i]) ? true : false;

        }else if (((uint8_t)0xF0 <= p[i]) && ((uint8_t)0xF7 >= p[i])){

            total_bytes = 4;

            is_lead_byte_zero = ((uint8_t)0xF0 == p[i]) ? true : false;

        }else{

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Invalid UTF-8(No.1), total_bytes = %1."),
                TP_LOG_PARAM_INT32_VALUE(total_bytes)
            );

            return false;
        }

        // Subsequent bytes: 0x80 ～ 0xBF
        size_t subsequent_bytes = i + total_bytes;

        for (size_t j = i + 1; subsequent_bytes > j; ++j){

            if ( ! (((uint8_t)0x80 <= p[j]) && ((uint8_t)0xBF >= p[j]))){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING("ERROR: Invalid UTF-8(No.2), p[j] = %1."),
                    TP_LOG_PARAM_UINT64_VALUE(p[j])
                );

                return false;
            }
        }

        if (is_lead_byte_zero){

            for (size_t j = i + 1; subsequent_bytes > j; ++j){

                if ((subsequent_bytes - 1) == j){

                    // Is bad ascii.

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("ERROR: Invalid UTF-8(No.3), p[j] = %1."),
                        TP_LOG_PARAM_UINT64_VALUE(p[j])
                    );

                    return false;
                }else{

                    if ((uint8_t)0x00 != (p[j] & 0x3F)){

                        break;
                    }
                }
            }
        }

        i += total_bytes - 1;
    }

    return true;
}

