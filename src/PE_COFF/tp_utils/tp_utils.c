
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef enum TP_LOG_FORMAT_STATUS_{
    TP_LOG_FORMAT_STATUS_TEXT_START,
    TP_LOG_FORMAT_STATUS_TEXT,
    TP_LOG_FORMAT_STATUS_PERCENT,
    TP_LOG_FORMAT_STATUS_PARAM
}TP_LOG_FORMAT_STATUS;

static bool put_log_msg_main(
    TP_SYMBOL_TABLE* symbol_table, TP_LOG_TYPE log_type, bool is_write_file, bool is_disp,
    uint8_t* format_string, TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num
);

static bool convert_string_to_value(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp,
    uint8_t* text, size_t text_start_pos, size_t text_size, size_t* param_index
);

static bool write_text_log_msg(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp,
    uint8_t* text, size_t text_start_pos, size_t text_size
);

static bool write_param_log_msg(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp, size_t param_index,
    TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num
);

#pragma optimize("", off)
void tp_free(TP_SYMBOL_TABLE* symbol_table, void** ptr, size_t size, uint8_t* file, uint8_t* func, size_t line_num)
{
    if (ptr && size){

        memset(*ptr, 0, size);
    }

    free(*ptr);
    *ptr = NULL;

    tp_print_crt_error(symbol_table, file, func, line_num);
}

void tp_free2(TP_SYMBOL_TABLE* symbol_table, void*** ptr, size_t size, uint8_t* file, uint8_t* func, size_t line_num)
{
    if (ptr && size) {

        memset(*ptr, 0, size);
    }

    free(*ptr);
    *ptr = NULL;

    tp_print_crt_error(symbol_table, file, func, line_num);
}
#pragma optimize("", on)

void tp_get_last_error(TP_SYMBOL_TABLE* symbol_table, uint8_t* file, uint8_t* func, size_t line_num)
{
    LPVOID msg_buffer = NULL;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &msg_buffer,
        0,
        NULL
    );

    if (NULL == symbol_table){

        fprintf(stderr, "%s\n", (char*)msg_buffer);
    }else{

        tp_put_log_msg(
            symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"), file, func, line_num,
            &(TP_LOG_PARAM_STRING(msg_buffer)), 1
        );
    }

    LocalFree(msg_buffer);
    msg_buffer = NULL;

    SetLastError(NO_ERROR);

    errno_t err = _set_errno(0);
}

void tp_print_crt_error(TP_SYMBOL_TABLE* symbol_table, uint8_t* file, uint8_t* func, size_t line_num)
{
    if (errno){

        char msg_buffer[TP_MESSAGE_BUFFER_SIZE];
        memset(msg_buffer, 0, sizeof(msg_buffer));

        errno_t err = strerror_s(msg_buffer, sizeof(msg_buffer), errno);

        if (NULL == symbol_table){

            fprintf(stderr, "%s\n", msg_buffer);
        }else{

            tp_put_log_msg(
                symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"), file, func, line_num,
                &(TP_LOG_PARAM_STRING(msg_buffer)), 1
            );
        }

        err = _set_errno(0);
    }
}

bool tp_put_log_msg(
    TP_SYMBOL_TABLE* symbol_table, TP_LOG_TYPE log_type,
    uint8_t* format_string, uint8_t* file, uint8_t* func, size_t line_num,
    TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num)
{
    bool is_write_file = symbol_table->member_is_output_log_file;

    bool is_disp = (
        (TP_LOG_TYPE_DISP_FORCE == log_type) ||
        ((false == symbol_table->member_log_hide_after_disp) &&
        ((TP_LOG_TYPE_DEFAULT == log_type) ||
        (TP_LOG_TYPE_HIDE_AFTER_DISP == log_type)))
    );

    if (symbol_table->member_is_no_output_messages){

        is_disp = false;
    }

    if (is_disp){

        fprintf(symbol_table->member_disp_log_file, "%s(%zd): ", file, line_num);
    }

    if (is_write_file){

        fprintf(symbol_table->member_write_log_file, "%s(%zd): ", file, line_num);
    }

    if ( ! put_log_msg_main(
        symbol_table, log_type, is_write_file, is_disp,
        format_string, log_param_element, log_param_element_num)){

        ;
    }

    if (is_disp){

        fprintf(symbol_table->member_disp_log_file, "\n");
    }

    if (is_write_file){

        fprintf(symbol_table->member_write_log_file, "\n");
    }

    if (TP_LOG_TYPE_HIDE_AFTER_DISP == log_type){

        symbol_table->member_log_hide_after_disp = true;
    }

    errno_t err = _set_errno(0);

    return true;
}

static bool put_log_msg_main(
    TP_SYMBOL_TABLE* symbol_table, TP_LOG_TYPE log_type, bool is_write_file, bool is_disp, uint8_t* format_string,
    TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num)
{
    TP_LOG_FORMAT_STATUS status = TP_LOG_FORMAT_STATUS_TEXT_START;
    size_t param_index = 0;
    size_t text_start_pos = 0;
    size_t text_end_pos = 0;

    size_t length = strlen(format_string);

    for (size_t i = 0; ; ++i){

        switch (status){
        case TP_LOG_FORMAT_STATUS_TEXT_START:

            switch (format_string[i]){
            case '\0':
                return true;
            case '%':
                status = TP_LOG_FORMAT_STATUS_PERCENT;
                break;
            default:
                status = TP_LOG_FORMAT_STATUS_TEXT;
                text_start_pos = i;
                break;
            }
            break;
        case TP_LOG_FORMAT_STATUS_TEXT:

            if (('\0' == format_string[i]) || ('%' == format_string[i])){

                status = TP_LOG_FORMAT_STATUS_PERCENT;
                text_end_pos = i - 1;

                if ( ! write_text_log_msg(
                    symbol_table, is_write_file, is_disp,
                    format_string, text_start_pos, text_end_pos - text_start_pos + 1)){

                    return false;
                }

                text_start_pos = 0;
                text_end_pos = 0;

                goto next;
            }
            break;
        case TP_LOG_FORMAT_STATUS_PERCENT:

            switch (format_string[i]){
            case '\0':

                if (is_disp){

                    fprintf(
                        symbol_table->member_disp_log_file,
                        "\nERROR: \\0 after TP_LOG_FORMAT_STATUS_PERCENT at %s(%d).\n",
                        __func__, __LINE__
                    );
                }

                if (is_write_file){

                    fprintf(
                        symbol_table->member_write_log_file,
                        "\nERROR: \\0 after TP_LOG_FORMAT_STATUS_PERCENT at %s(%d).\n",
                        __func__, __LINE__
                    );
                }

                return false;
            case '%':
                status = TP_LOG_FORMAT_STATUS_TEXT;
                break;
            default:
                status = TP_LOG_FORMAT_STATUS_PARAM;
                break;
            }

            text_start_pos = i;
            break;
        case TP_LOG_FORMAT_STATUS_PARAM:

            if (('\0' == format_string[i]) || (false == isdigit(format_string[i]))){

                status = TP_LOG_FORMAT_STATUS_TEXT_START;
                text_end_pos = i - 1;

                if ( ! convert_string_to_value(
                    symbol_table, is_write_file, is_disp,
                    format_string, text_start_pos, text_end_pos - text_start_pos + 1, &param_index)){

                    return false;
                }

                --param_index;

                text_start_pos = 0;
                text_end_pos = 0;

                if ( ! write_param_log_msg(
                    symbol_table, is_write_file, is_disp, param_index, log_param_element, log_param_element_num)){

                    return false;
                }

                goto next;
            }
            break;
        default:

            if (is_disp){

                fprintf(symbol_table->member_disp_log_file, "Bad TP_LOG_FORMAT_STATUS(%d).\n", status);
            }

            if (is_write_file){

                fprintf(symbol_table->member_write_log_file, "Bad TP_LOG_FORMAT_STATUS(%d).\n", status);
            }

            return false;
        }

        continue;

next:
        if ('\0' == format_string[i]){

            break;
        }

        if (TP_LOG_FORMAT_STATUS_TEXT_START == status){

            --i;
        }
    }

    return true;
}

static bool convert_string_to_value(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp,
    uint8_t* text, size_t text_start_pos, size_t text_size, size_t* param_index)
{
    if (text_size >= sizeof(symbol_table->member_temp_buffer)){

        if (is_disp){

            fprintf(
                symbol_table->member_disp_log_file,
                "\nERROR: text_size(%zd) >= sizeof(%zd: symbol_table->member_temp_buffer) at %s(%d).\n",
                text_size, sizeof(symbol_table->member_temp_buffer),
                __func__, __LINE__
            );
        }

        if (is_write_file){

            fprintf(
                symbol_table->member_write_log_file,
                "\nERROR: text_size(%zd) >= sizeof(%zd: symbol_table->member_temp_buffer) at %s(%d).\n",
                text_size, sizeof(symbol_table->member_temp_buffer),
                __func__, __LINE__
            );
        }

        return false;
    }

    memset(symbol_table->member_temp_buffer, 0, sizeof(symbol_table->member_temp_buffer));
    memcpy(symbol_table->member_temp_buffer, text + text_start_pos, text_size);

    char* error_first_char = NULL;

    long value = strtol(symbol_table->member_temp_buffer, &error_first_char, 0);

    if (NULL == error_first_char){

        if (is_disp){

            fprintf(
                symbol_table->member_disp_log_file,
                "\nERROR: strtol(\"%s\") convert failed at %s(%d).\n", symbol_table->member_temp_buffer,
                __func__, __LINE__
            );
        }

        if (is_write_file){

            fprintf(
                symbol_table->member_write_log_file,
                "\nERROR: strtol(\"%s\") convert failed at %s(%d).\n", symbol_table->member_temp_buffer,
                __func__, __LINE__
            );
        }

        return false;
    }

    if (ERANGE == errno){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    *param_index = (size_t)value;

    return true;
}

static bool write_text_log_msg(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp, uint8_t* text, size_t text_start_pos, size_t text_size)
{
    size_t fwrite_bytes = 0;

    if (is_disp){

        fwrite_bytes = fwrite(text + text_start_pos, sizeof(uint8_t), text_size, symbol_table->member_disp_log_file);
    }

    if (is_write_file){

        fwrite_bytes = fwrite(text + text_start_pos, sizeof(uint8_t), text_size, symbol_table->member_write_log_file);
    }

    return true;
}

static bool write_param_log_msg(
    TP_SYMBOL_TABLE* symbol_table, bool is_write_file, bool is_disp, size_t param_index,
    TP_LOG_PARAM_ELEMENT* log_param_element, size_t log_param_element_num)
{
    if (log_param_element_num <= param_index){

        if (is_disp){

            fprintf(
                symbol_table->member_disp_log_file, "\nERROR: log_param_element_num(%zd) <= param_index(%zd) at %s(%d).\n",
                log_param_element_num, param_index, __func__, __LINE__
            );
        }

        if (is_write_file){

            fprintf(
                symbol_table->member_write_log_file, "\nERROR: log_param_element_num(%zd) <= param_index(%zd) at %s(%d).\n",
                log_param_element_num, param_index, __func__, __LINE__
            );
        }

        return false;
    }

    switch (log_param_element[param_index].member_type){
    case TP_LOG_PARAM_TYPE_STRING:

        if (is_disp){

            fprintf(symbol_table->member_disp_log_file, "%s", log_param_element[param_index].member_body.member_string);
        }

        if (is_write_file){

            fprintf(symbol_table->member_write_log_file, "%s", log_param_element[param_index].member_body.member_string);
        }

        break;
    case TP_LOG_PARAM_TYPE_INT32_VALUE:

        if (is_disp){

            fprintf(symbol_table->member_disp_log_file, "%d", log_param_element[param_index].member_body.member_int32_value);
        }

        if (is_write_file){

            fprintf(symbol_table->member_write_log_file, "%d", log_param_element[param_index].member_body.member_int32_value);
        }

        break;
    case TP_LOG_PARAM_TYPE_UINT64_VALUE:

        if (is_disp){

            fprintf(symbol_table->member_disp_log_file, "%zd", log_param_element[param_index].member_body.member_uint64_value);
        }

        if (is_write_file){

            fprintf(symbol_table->member_write_log_file, "%zd", log_param_element[param_index].member_body.member_uint64_value);
        }

        break;
    default:

        if (is_disp){

            fprintf(symbol_table->member_disp_log_file, "Bad TP_LOG_PARAM_TYPE(%d).\n", log_param_element[param_index].member_type);
        }

        if (is_write_file){

            fprintf(symbol_table->member_write_log_file, "Bad TP_LOG_PARAM_TYPE(%d).\n", log_param_element[param_index].member_type);
        }

        return false;
    }

    return true;
}

