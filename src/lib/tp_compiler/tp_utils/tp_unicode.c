
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef struct unicode_data_{
    TP_CHAR32_T code_point;
    uint8_t* name;
    uint8_t* general_category;
    uint8_t* canonical_combining_class;
    uint8_t* bidi_class;
    uint8_t* decomposition_type_decomposition_mapping;
    uint8_t* numeric_type_numeric_value1;
    uint8_t* numeric_type_numeric_value2;
    uint8_t* numeric_type_numeric_value3;
    uint8_t bidi_mirrored;   // Y or N
    uint8_t* unicode_1_name; // obsolete as of 6.2.0
    uint8_t* iso_comment;    // obsolete as of 5.2.0; deprecated and stabilized as of 6.0.0
    uint8_t* simple_uppercase_mapping;
    uint8_t* simple_lowercase_mapping;
    uint8_t* simple_titlecase_mapping;
}UNICODE_DATA;

static UNICODE_DATA unicode_data[] = {
#include "../generate/unicode/unicode_data.txt"
    { 0x0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, '\0', NULL, NULL, NULL, NULL, NULL },
};

// LARGE BLOCK:
// 0x10FFFF / 0x400 = 0x43F = 1,087
// 0x10FFFF % 0x400 = 0x3FF -> 1,087 + 1 = 1,088
#define LARGE_BLOCK_NUM 1088
#define LARGE_BLOCK_SIZE 1024

// SMALL BLOCK:
// 0x3FF / 0x40 = 0xF = 15
// 0x3FF % 0x40 = 0x3F -> 15 + 1 = 16
#define SMALL_BLOCK_NUM 16
#define SMALL_BLOCK_SIZE 64

typedef struct index_large_block_{
    int64_t member_prev_large_block_popcnt;
    int64_t member_large_block_popcnt;
    uint64_t member_small_block[SMALL_BLOCK_NUM];
}INDEX_LARGE_BLOCK;

static INDEX_LARGE_BLOCK large_block[LARGE_BLOCK_NUM + 1] = {
#include "../generate/unicode/unicode_data_index.txt"
    { 0, 0, { 0 } },
};

static bool unicode_data_gen_content(
    TP_SYMBOL_TABLE* symbol_table, TP_UNICODE_DATA_GEN_KIND kind,
    FILE* read_file, FILE* write_file
);
static bool unicode_data_gen_content_write(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* line_buffer, FILE* write_file
);
static bool unicode_data_index_gen_content(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* line_buffer, INDEX_LARGE_BLOCK* large_block
);
static bool unicode_data_index_calc_write(
    TP_SYMBOL_TABLE* symbol_table,
    INDEX_LARGE_BLOCK* large_block, FILE* write_file
);
static bool test_unicode_data_index_main(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T code_point, int64_t* param_popcnt_num
);

bool tp_is_valid_utf_8(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* lines_buffer, size_t read_lines_length,
    long* utf_8_restart_pos)
{
    *utf_8_restart_pos = 0;

    for (size_t i = 0; read_lines_length > i; ++i){

        uint8_t char8 = lines_buffer[i];

        // First byte: ASCII
        if (isascii(char8)){

            // Clear NUL byte.
            if ((uint8_t)0x00 == char8){

                lines_buffer[i] = ' ';

                ++(symbol_table->member_nul_num);
            }

            continue;
        }

        rsize_t total_bytes = 0;

        bool is_lead_byte_zero = false;

        // First byte.
        if (((uint8_t)0xC0 <= char8) && ((uint8_t)0xDF >= char8)){

            total_bytes = 2;

            is_lead_byte_zero = ((uint8_t)0xC0 == char8) ? true : false;

        }else if (((uint8_t)0xE0 <= char8) && ((uint8_t)0xEF >= char8)){

            total_bytes = 3;

            is_lead_byte_zero = ((uint8_t)0xE0 == char8) ? true : false;

        }else if (((uint8_t)0xF0 <= char8) && ((uint8_t)0xF7 >= char8)){

            total_bytes = 4;

            is_lead_byte_zero = ((uint8_t)0xF0 == char8) ? true : false;

        }else{

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Invalid UTF-8(No.1), total_bytes = %1."),
                TP_LOG_PARAM_UINT64_VALUE(total_bytes)
            );

            return false;
        }

        if (read_lines_length < i + total_bytes){

            *utf_8_restart_pos = (long)total_bytes;

            return true;
        }

        // Subsequent bytes: 0x80 ～ 0xBF
        size_t subsequent_bytes = i + total_bytes;

        for (size_t j = i + 1; subsequent_bytes > j; ++j){

            uint8_t char8 = lines_buffer[j];

            if ( ! (((uint8_t)0x80 <= char8) && ((uint8_t)0xBF >= char8))){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE, TP_MSG_FMT("%1"),
                    TP_LOG_PARAM_STRING("ERROR: Invalid UTF-8(No.2), char8 = %1."),
                    TP_LOG_PARAM_UINT64_VALUE(char8)
                );

                return false;
            }
        }

        if (is_lead_byte_zero){

            for (size_t j = i + 1; subsequent_bytes > j; ++j){

                uint8_t char8 = lines_buffer[j];

                if ((subsequent_bytes - 1) == j){

                    // Is bad ascii.

                    TP_PUT_LOG_MSG(
                        symbol_table, TP_LOG_TYPE_DISP_FORCE,
                        TP_MSG_FMT("ERROR: Invalid UTF-8(No.3), char8 = %1."),
                        TP_LOG_PARAM_UINT64_VALUE(char8)
                    );

                    return false;
                }else{

                    if ((uint8_t)0x00 != (char8 & 0x3F)){

                        break;
                    }
                }
            }
        }

        i += total_bytes - 1;
    }

    return true;
}

bool tp_encode_utf_8(
    TP_SYMBOL_TABLE* symbol_table, uint32_t value, TP_CHAR8_T* string_buffer, rsize_t* total_bytes)
{
    if ((0xD800 <= value) && (0xDFFF >= value)){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: (0xD800 <= value) && (0xDFFF >= value): value = %1."),
            TP_LOG_PARAM_INT32_VALUE(value)
        );

        return false;
    }

    if (isascii(value)){

        *total_bytes = 1;
        *string_buffer = (TP_CHAR8_T)value;
    }else if ((0x80 <= value) && (0x7FF >= value)){

        *total_bytes = 2;
        string_buffer[0] = (TP_CHAR8_T)((value >> 6) | 0xC0);
        string_buffer[1] = (TP_CHAR8_T)((value & 0x3F) | 0x80);
    }else if ((0x800 <= value) && (0xFFFF >= value)){

        *total_bytes = 3;
        string_buffer[0] = (TP_CHAR8_T)((value >> 12) | 0xE0);
        string_buffer[1] = (TP_CHAR8_T)(((value >> 6) & 0x3F) | 0x80);
        string_buffer[2] = (TP_CHAR8_T)((value & 0x3F) | 0x80);
    }else if ((0x10000 <= value) && (0x10FFFF >= value)){

        *total_bytes = 4;
        string_buffer[0] = (TP_CHAR8_T)((value >> 18) | 0xF0);
        string_buffer[1] = (TP_CHAR8_T)(((value >> 12) & 0x3F) | 0x80);
        string_buffer[2] = (TP_CHAR8_T)(((value >> 6) & 0x3F) | 0x80);
        string_buffer[3] = (TP_CHAR8_T)((value & 0x3F) | 0x80);
    }else{

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: Unicode overflow value = %1."),
            TP_LOG_PARAM_INT32_VALUE(value)
        );

        return false;
    }

    return true;
}

bool tp_encode_utf_16(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_16_STRING* utf_16_string, TP_CHAR8_T* string_buffer, rsize_t string_length)
{
    TP_TOKEN_UTF_32_STRING utf_32_string = { 0 };

    if ( ! tp_decode_utf_8(symbol_table, &utf_32_string, string_buffer, string_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    TP_CHAR32_T* string32 = utf_32_string.member_string;
    rsize_t length32 = utf_32_string.member_string_length;

    TP_CHAR16_T* tmp_utf_16_string = (TP_CHAR16_T*)TP_CALLOC(
        symbol_table,
        length32 * 2 + 1 /* for UTF-16 encode. */, sizeof(TP_CHAR16_T)
    );

    if (NULL == tmp_utf_16_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        TP_FREE(symbol_table, &(utf_32_string.member_string), length32 * sizeof(TP_CHAR32_T));

        return false;
    }

    utf_16_string->member_string = tmp_utf_16_string;

    for (rsize_t i = 0; length32 > i; ++i){

        TP_CHAR32_T char32 = string32[i];

        if (0x10FFFF < char32){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: (0x10FFFF < char32): char32 = %1."),
                TP_LOG_PARAM_UINT64_VALUE(char32)
            );

            TP_FREE(symbol_table, &(utf_32_string.member_string), length32 * sizeof(TP_CHAR32_T));

            return false;
        }

        if (0x10000 > char32){

            utf_16_string->member_string[utf_16_string->member_string_length] =
                (TP_CHAR16_T)char32;

            ++(utf_16_string->member_string_length);

            continue;
        }

        char32 -= 0x10000;

        TP_CHAR16_T char16_low = (0xDC00 | (char32 & 0x3FF));

        utf_16_string->member_string[utf_16_string->member_string_length] =
            (TP_CHAR16_T)char16_low;

        ++(utf_16_string->member_string_length);

        TP_CHAR16_T char16_high = (0xD800 | ((char32 >> 10) & 0x3FF));

        utf_16_string->member_string[utf_16_string->member_string_length] =
            (TP_CHAR16_T)char16_high;

        ++(utf_16_string->member_string_length);
    }

    TP_FREE(symbol_table, &(utf_32_string.member_string), length32 * sizeof(TP_CHAR32_T));

    return true;
}

bool tp_encode_ucs_2_char(
    TP_SYMBOL_TABLE* symbol_table,
    TP_CHAR16_T* value16, TP_CHAR8_T* string_buffer, rsize_t string_length)
{
    TP_CHAR32_T value32 = 0;

    if ( ! tp_decode_utf_8_char(symbol_table, &value32, string_buffer, string_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (0x10000 > value32){

        *value16 = (TP_CHAR16_T)value32;

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: UCS-2 overflow value = %1."),
        TP_LOG_PARAM_UINT64_VALUE(value32)
    );

    return false;
}

bool tp_decode_utf_8(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_32_STRING* utf_32_string, TP_CHAR8_T* string_buffer, rsize_t string_length)
{
    TP_CHAR32_T* tmp_utf_32_string = (TP_CHAR32_T*)TP_CALLOC(
        symbol_table,
        string_length * 4 + 1 /* for UTF-32 encode. */, sizeof(TP_CHAR32_T)
    );

    if (NULL == tmp_utf_32_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    utf_32_string->member_string = tmp_utf_32_string;

    for (rsize_t i = 0; string_length > i; ++i){

        TP_CHAR8_T char8 = string_buffer[i];

        if (isascii(char8)){

            utf_32_string->member_string[utf_32_string->member_string_length] =
                (TP_CHAR32_T)char8;

            ++(utf_32_string->member_string_length);

            continue;
        }

        rsize_t total_bytes = 0;

        bool is_lead_byte_zero = false;

        TP_CHAR32_T char32 = 0;

        // First byte.
        if (((TP_CHAR8_T)0xC0 <= char8) && ((TP_CHAR8_T)0xDF >= char8)){

            total_bytes = 2;

            if (total_bytes > (string_length - i)){  TP_PUT_LOG_MSG_ICE(symbol_table);  return false; }

            is_lead_byte_zero = ((TP_CHAR8_T)0xC0 == char8) ? true : false;

            char32 = ((char8 & 0x1F) << 6);
            char32 |= (string_buffer[i + 1] & 0x3F);
        }else if (((TP_CHAR8_T)0xE0 <= char8) && ((TP_CHAR8_T)0xEF >= char8)){

            total_bytes = 3;

            if (total_bytes > (string_length - i)){  TP_PUT_LOG_MSG_ICE(symbol_table);  return false; }

            is_lead_byte_zero = ((TP_CHAR8_T)0xE0 == char8) ? true : false;

            char32 = ((char8 & 0x0F) << 12);
            char32 |= ((string_buffer[i + 1] & 0x3F) << 6);
            char32 |= (string_buffer[i + 2] & 0x3F);
        }else if (((TP_CHAR8_T)0xF0 <= char8) && ((TP_CHAR8_T)0xF7 >= char8)){

            total_bytes = 4;

            if (total_bytes > (string_length - i)){  TP_PUT_LOG_MSG_ICE(symbol_table);  return false; }

            is_lead_byte_zero = ((TP_CHAR8_T)0xF0 == char8) ? true : false;

            char32 = ((char8 & 0x07) << 18);
            char32 |= ((string_buffer[i + 1] & 0x3F) << 12);
            char32 |= ((string_buffer[i + 2] & 0x3F) << 6);
            char32 |= (string_buffer[i + 3] & 0x3F);
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        utf_32_string->member_string[utf_32_string->member_string_length] = char32;
        ++(utf_32_string->member_string_length);

        // Subsequent bytes: 0x80 ～ 0xBF
        size_t subsequent_bytes = i + total_bytes;

        for (size_t j = i + 1; subsequent_bytes > j; ++j){

            TP_CHAR8_T char8 = string_buffer[j];

            if ( ! (((TP_CHAR8_T)0x80 <= char8) && ((TP_CHAR8_T)0xBF >= char8))){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }
        }

        if (is_lead_byte_zero){

            for (size_t j = i + 1; subsequent_bytes > j; ++j){

                TP_CHAR8_T char8 = string_buffer[j];

                if ((subsequent_bytes - 1) == j){

                    // Is bad ascii.
                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }else{

                    if ((TP_CHAR8_T)0x00 != (char8 & 0x3F)){

                        break;
                    }
                }
            }
        }

        i += total_bytes - 1;
    }

    TP_CHAR32_T* string = utf_32_string->member_string;
    rsize_t length = utf_32_string->member_string_length;

    for (rsize_t i = 0; length > i; ++i){

        TP_CHAR32_T char32 = string[i];

        if (0x10FFFF < char32){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: (0x10FFFF < char32): char32 = %1."),
                TP_LOG_PARAM_UINT64_VALUE(char32)
            );

            return false;
        }
    }

    return true;
}

bool tp_decode_utf_8_char(
    TP_SYMBOL_TABLE* symbol_table,
    TP_CHAR32_T* value32, TP_CHAR8_T* string_buffer, rsize_t string_length)
{
    TP_TOKEN_UTF_32_STRING utf_32_string = { 0 };

    if ( ! tp_decode_utf_8(symbol_table, &utf_32_string, string_buffer, string_length)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (NULL == utf_32_string.member_string){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    *value32 = *(utf_32_string.member_string);

    TP_FREE(symbol_table, &(utf_32_string.member_string), utf_32_string.member_string_length * sizeof(TP_CHAR32_T));

    return true;
}

bool tp_decode_utf_16(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_16_STRING* utf_16_string, TP_TOKEN_UTF_8_STRING* utf_8_string)
{
    // for UTF-8 encode.
    rsize_t UTF_8_STRING_SIZE = utf_16_string->member_string_length * 4 + 1;

    TP_CHAR8_T* tmp_utf_8_string = (TP_CHAR8_T*)TP_CALLOC(symbol_table, UTF_8_STRING_SIZE, sizeof(TP_CHAR8_T));

    if (NULL == tmp_utf_8_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    utf_8_string->member_string = tmp_utf_8_string;
    utf_8_string->member_string_length = 0;

    rsize_t utf_16_length = utf_16_string->member_string_length;

    for (rsize_t i = 0; utf_16_length > i; ++i){

        rsize_t utf_8_length = 0;

        if ( ! tp_encode_utf_8(
            symbol_table, utf_16_string->member_string[i],
            utf_8_string->member_string + utf_8_string->member_string_length,
            &utf_8_length)){

            TP_FREE(symbol_table, &(utf_8_string->member_string), UTF_8_STRING_SIZE);
            utf_8_string->member_string_length = 0;

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        utf_8_string->member_string_length += utf_8_length;
    }

    return true;
}

bool tp_decode_ucs_2_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR16_T value16, TP_TOKEN_UTF_8_STRING* utf_8_string)
{
    if ( ! tp_decode_utf_32_char(symbol_table, value16, utf_8_string)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_decode_utf_32(
    TP_SYMBOL_TABLE* symbol_table,
    TP_TOKEN_UTF_32_STRING* utf_32_string, TP_TOKEN_UTF_8_STRING* utf_8_string)
{
    // for UTF-8 encode.
    rsize_t UTF_8_STRING_SIZE = utf_32_string->member_string_length * 4 + 1;

    TP_CHAR8_T* tmp_utf_8_string = (TP_CHAR8_T*)TP_CALLOC(symbol_table, UTF_8_STRING_SIZE, sizeof(TP_CHAR8_T));

    if (NULL == tmp_utf_8_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    utf_8_string->member_string = tmp_utf_8_string;
    utf_8_string->member_string_length = 0;

    rsize_t utf_32_length = utf_32_string->member_string_length;

    for (rsize_t i = 0; utf_32_length > i; ++i){

        rsize_t utf_8_length = 0;

        if ( ! tp_encode_utf_8(
            symbol_table, utf_32_string->member_string[i],
            utf_8_string->member_string + utf_8_string->member_string_length,
            &utf_8_length)){

            TP_FREE(symbol_table, &(utf_8_string->member_string), UTF_8_STRING_SIZE);
            utf_8_string->member_string_length = 0;

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        utf_8_string->member_string_length += utf_8_length;
    }

    return true;
}

bool tp_decode_utf_32_char(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T value32, TP_TOKEN_UTF_8_STRING* utf_8_string)
{
    const rsize_t UTF_8_STRING_SIZE = 5; // for UTF-8 encode.

    TP_CHAR8_T* tmp_utf_8_string = (TP_CHAR8_T*)TP_CALLOC(symbol_table, UTF_8_STRING_SIZE, sizeof(TP_CHAR8_T));

    if (NULL == tmp_utf_8_string){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    utf_8_string->member_string = tmp_utf_8_string;

    if ( ! tp_encode_utf_8(
        symbol_table, value32, utf_8_string->member_string, &(utf_8_string->member_string_length))){

        TP_FREE(symbol_table, &(utf_8_string->member_string), UTF_8_STRING_SIZE);
        utf_8_string->member_string_length = 0;

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_unicode_data_gen(TP_SYMBOL_TABLE* symbol_table, TP_UNICODE_DATA_GEN_KIND kind)
{
    char* kind_string = (TP_UNICODE_DATA_GEN_KIND_DATA == kind) ? "DATA" : "INDEX";

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("START %1 function(%2)."),
        TP_LOG_PARAM_STRING(__func__), TP_LOG_PARAM_STRING(kind_string)
    );
    fflush(symbol_table->member_write_log_file);

    FILE* read_file = NULL;

    if ( ! tp_open_read_file_text(
        symbol_table, symbol_table->member_read_unicode_data_path, &read_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    char* path = (TP_UNICODE_DATA_GEN_KIND_DATA == kind) ?
        symbol_table->member_unicode_data_path :
        symbol_table->member_unicode_data_index_path;

    FILE* write_file = NULL;

    if ( ! tp_open_write_file_text(symbol_table, path, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! unicode_data_gen_content(symbol_table, kind, read_file, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! tp_close_file(symbol_table, &read_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }


    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("SUCCESS %1 function(%2)."),
        TP_LOG_PARAM_STRING(__func__),
        TP_LOG_PARAM_STRING(kind_string)
    );
    fflush(symbol_table->member_write_log_file);

    return true;

fail:

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
    }

    if ( ! tp_close_file(symbol_table, &read_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);
    }

    return false;
}

static bool unicode_data_gen_content(
    TP_SYMBOL_TABLE* symbol_table, TP_UNICODE_DATA_GEN_KIND kind,
    FILE* read_file, FILE* write_file)
{
    uint8_t read_line_buffer[TP_BUFFER_SIZE] = { 0 };
    int count = TP_MAX_LINE_BYTES;

    INDEX_LARGE_BLOCK* local_large_block = NULL;

    if (TP_UNICODE_DATA_GEN_KIND_INDEX == kind){

        local_large_block = (INDEX_LARGE_BLOCK*)TP_CALLOC(
            symbol_table,
            LARGE_BLOCK_NUM, sizeof(INDEX_LARGE_BLOCK)
        );

        if (NULL == local_large_block){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }
    }

    for (;;){

        char* return_value = fgets(
            read_line_buffer, count, read_file
        );

        if (NULL == return_value){

            int ferror_error = ferror(read_file);

            if (ferror_error){

                TP_PRINT_CRT_ERROR(symbol_table);

                clearerr(read_file);

                goto fail;
            }

            break;
        }

        switch (kind){
        case TP_UNICODE_DATA_GEN_KIND_DATA:
            if ( ! unicode_data_gen_content_write(
                symbol_table, read_line_buffer, write_file)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
            break;
        case TP_UNICODE_DATA_GEN_KIND_INDEX:
            if ( ! unicode_data_index_gen_content(symbol_table, read_line_buffer, large_block)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                goto fail;
            }
            break;
        default:
            TP_PUT_LOG_MSG_ICE(symbol_table);
            goto fail;
        }
    }

    if (TP_UNICODE_DATA_GEN_KIND_INDEX == kind){

        if ( ! unicode_data_index_calc_write(symbol_table, large_block, write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            goto fail;
        }

        TP_FREE(
            symbol_table, &local_large_block,
            LARGE_BLOCK_NUM * sizeof(INDEX_LARGE_BLOCK)
        );
    }

    return true;

fail:

    TP_FREE(
        symbol_table, &local_large_block,
        LARGE_BLOCK_NUM * sizeof(INDEX_LARGE_BLOCK)
    );

    return false;
}

static bool unicode_data_gen_content_write(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* line_buffer, FILE* write_file)
{
    const int COL_MAX = 14;
    uint8_t* delimiters = ";\n";
    uint8_t* next_token = NULL;

    uint8_t* token = strtok_s(line_buffer, delimiters, &next_token);

    // 0000;<control>;Cc;0;BN;;;;;N;NULL;;;;
    // { 0x0, "<control>", "Cc", "0", "BN", NULL, NULL, NULL, NULL, 'N', "NULL", NULL, NULL, NULL, NULL },
    for (int32_t i = 0; token; ++i){

        switch (i){
        case 0:{

            char* error_first_char = NULL;

            uint32_t code_point = (uint32_t)strtoul(token, &error_first_char, 16);

            if (token == error_first_char){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
                    TP_LOG_PARAM_STRING(token)
                );

                return false;
            }

            if (ERANGE == errno){

                TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

                return false;
            }

            if (0x10FFFF < code_point){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: (0x10FFFF < code_point): code_point = %1."),
                    TP_LOG_PARAM_UINT64_VALUE(code_point)
                );

                return false;
            }

            fprintf(write_file, "    { 0x%x, ", code_point);
            break;
        }
        case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
        case 10: case 11: case 12: case 13: case 14:
            if (0 == strlen(token)){

                fprintf(write_file, "NULL, ");
            }else{

                fprintf(write_file, "\"%s\", ", token);
            }

            if (COL_MAX == i){

                fprintf(write_file, " },\n");
            }
            break;
        case 9:
            fprintf(write_file, "'%c', ", *token ? *token : ' ');
            break;
        default:
            fprintf(write_file, "\n");

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: UnicodeData.txt token overflow(string = %1)."),
                TP_LOG_PARAM_STRING(token)
            );
            return false;
        }

        token = strtok_s(NULL, delimiters, &next_token);

        if ((NULL == token) && (COL_MAX > i)){

            token = "";
        }
    }

    return true;
}

static bool unicode_data_index_gen_content(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* line_buffer, INDEX_LARGE_BLOCK* large_block)
{

    uint8_t* delimiters = ";\n";
    uint8_t* next_token = NULL;

    uint8_t* token = strtok_s(line_buffer, delimiters, &next_token);

    if (token){

        char* error_first_char = NULL;

        uint32_t code_point = (uint32_t)strtoul(token, &error_first_char, 16);

        if (token == error_first_char){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: strtoul(%1) convert failed."),
                TP_LOG_PARAM_STRING(token)
            );

            return false;
        }

        if (ERANGE == errno){

            TP_PRINT_CRT_ERROR_CONTINUE(symbol_table);

            return false;
        }

        if (0x10FFFF < code_point){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: (0x10FFFF < code_point): code_point = %1."),
                TP_LOG_PARAM_UINT64_VALUE(code_point)
            );

            return false;
        }

        uint32_t large_block_num = code_point / LARGE_BLOCK_SIZE;
        uint32_t large_block_mod = code_point % LARGE_BLOCK_SIZE;

        if (large_block_mod){

            uint32_t small_block_num = large_block_mod / SMALL_BLOCK_SIZE;
            uint32_t small_block_mod = large_block_mod % SMALL_BLOCK_SIZE;

            if (small_block_mod){

                large_block[large_block_num].member_small_block[small_block_num] |= (0x1ULL << small_block_mod);
            }else{

                large_block[large_block_num].member_small_block[small_block_num] |= 0x1;
            }
        }else{

            large_block[large_block_num].member_small_block[0] |= 0x1;
        }

        return true;
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("ERROR: Get token failed(line_buffer = %1)."),
        TP_LOG_PARAM_STRING(line_buffer)
    );

    return false;
}

static bool unicode_data_index_calc_write(
    TP_SYMBOL_TABLE* symbol_table, INDEX_LARGE_BLOCK* large_block, FILE* write_file)
{
    large_block[0].member_prev_large_block_popcnt = 0;

    for (uint32_t i = 0; LARGE_BLOCK_NUM > i; ++i){

        large_block[i].member_large_block_popcnt = 0;

        for (uint32_t j = 0; SMALL_BLOCK_NUM > j; ++j){

            int64_t r = __popcnt64(large_block[i].member_small_block[j]);
            large_block[i].member_large_block_popcnt += r;
        }
    }

    int64_t popcnt_num = 0;

    for (uint32_t i = 0; LARGE_BLOCK_NUM > i; ++i){

        popcnt_num += large_block[i].member_large_block_popcnt;

        large_block[i + 1].member_prev_large_block_popcnt = popcnt_num;
    }

    if (0 == popcnt_num){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"),
            TP_LOG_PARAM_STRING("ERROR: (0 == popcnt_num)")
        );

        return false;
    }

    for (uint32_t i = 0; LARGE_BLOCK_NUM > i; ++i){

        fprintf(
            write_file, "    { 0x%llx, 0x%llx, { ",
            large_block[i].member_prev_large_block_popcnt,
            large_block[i].member_large_block_popcnt
        );

        for (uint32_t j = 0; SMALL_BLOCK_NUM > j; ++j){

            fprintf(
                write_file, "0x%llx, ",
                large_block[i].member_small_block[j]
            );
        }

        fprintf(write_file, "}, },\n");
    }

    return true;
}

bool tp_test_unicode_data_index(TP_SYMBOL_TABLE* symbol_table)
{
    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("START %1 function."),
        TP_LOG_PARAM_STRING(__func__)
    );
    fflush(stderr);
    fflush(symbol_table->member_write_log_file);

    for (uint32_t i = 0; unicode_data[i].name; ++i){

        TP_CHAR32_T code_point = unicode_data[i].code_point;
        int64_t popcnt_num = 0;

        if ( ! test_unicode_data_index_main(symbol_table, code_point, &popcnt_num)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if (-1 == popcnt_num){

            continue;
        }

        if (code_point != unicode_data[popcnt_num].code_point){

            TP_PUT_LOG_MSG(
                symbol_table, TP_LOG_TYPE_DISP_FORCE,
                TP_MSG_FMT("ERROR: Unmatch code point(%1, %2 at %3 line)."),
                TP_LOG_PARAM_UINT64_VALUE(code_point),
                TP_LOG_PARAM_UINT64_VALUE(unicode_data[popcnt_num].code_point),
                TP_LOG_PARAM_UINT64_VALUE(popcnt_num)
            );

            return false;
        }
    }

    for (uint32_t code_point = 0; 0x10FFFF >= code_point; ++code_point){

        bool is_match = false;

        for (uint32_t i = 0; unicode_data[i].name; ++i){

            if (unicode_data[i].code_point == code_point){

                is_match = true;

                break;
            }
        }

        if (false == is_match){

            int64_t popcnt_num = 0;

            if ( ! test_unicode_data_index_main(symbol_table, code_point, &popcnt_num)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (-1 == popcnt_num){

                continue;
            }

            if (code_point == unicode_data[popcnt_num].code_point){

                TP_PUT_LOG_MSG(
                    symbol_table, TP_LOG_TYPE_DISP_FORCE,
                    TP_MSG_FMT("ERROR: Match code point(%1 at %2 line)."),
                    TP_LOG_PARAM_UINT64_VALUE(code_point),
                    TP_LOG_PARAM_UINT64_VALUE(popcnt_num)
                );

                return false;
            }
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("SUCCESS %1 function."),
        TP_LOG_PARAM_STRING(__func__)
    );
    fflush(stderr);
    fflush(symbol_table->member_write_log_file);

    return true;
}

static bool test_unicode_data_index_main(
    TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T code_point, int64_t* param_popcnt_num)
{
    bool is_exist = false;

    uint32_t large_block_num = code_point / LARGE_BLOCK_SIZE;
    uint32_t large_block_mod = code_point % LARGE_BLOCK_SIZE;

    uint32_t small_block_num = 0;
    uint32_t small_block_mod = 0;

    if (large_block_mod){

        small_block_num = large_block_mod / SMALL_BLOCK_SIZE;
        small_block_mod = large_block_mod % SMALL_BLOCK_SIZE;

        if (small_block_mod){

            is_exist = (large_block[large_block_num].member_small_block[small_block_num] & (0x1ULL << small_block_mod));
        }else{

            is_exist = (large_block[large_block_num].member_small_block[small_block_num] & 0x1);
        }
    }else{

        is_exist = (large_block[large_block_num].member_small_block[0] & 0x1);
    }

    if (false == is_exist){

        *param_popcnt_num = -1;

        return true;
    }

    INDEX_LARGE_BLOCK* local_large_block = &(large_block[large_block_num]);

    int64_t popcnt_num = local_large_block->member_prev_large_block_popcnt;

    for (uint32_t i = 0; small_block_num > i; ++i){

        popcnt_num += __popcnt64(local_large_block->member_small_block[i]);
    }

    if (small_block_mod){

        uint64_t mask = ((0x1ULL << small_block_mod) - 1);

        popcnt_num += __popcnt64(local_large_block->member_small_block[small_block_num] & mask);
    }

    uint32_t unicode_data_num = sizeof(unicode_data) / sizeof(unicode_data[0]) - 1;

    if (popcnt_num > unicode_data_num){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: (popcnt_num:%1 > unicode_data_num:%2)"),
            TP_LOG_PARAM_UINT64_VALUE(popcnt_num),
            TP_LOG_PARAM_UINT64_VALUE(unicode_data_num)
        );

        return false;
    }

    *param_popcnt_num = popcnt_num;

    return true;
}

bool tp_is_numeric_code_point(TP_SYMBOL_TABLE* symbol_table, TP_CHAR32_T code_point, bool* is_numeric)
{
    if (0x10FFFF < code_point){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: (0x10FFFF < code_point): code_point = %1."),
            TP_LOG_PARAM_UINT64_VALUE(code_point)
        );

        return false;
    }

    *is_numeric = false;

    bool is_exist = false;

    uint32_t large_block_num = code_point / LARGE_BLOCK_SIZE;
    uint32_t large_block_mod = code_point % LARGE_BLOCK_SIZE;

    uint32_t small_block_num = 0;
    uint32_t small_block_mod = 0;

    if (large_block_mod){

        small_block_num = large_block_mod / SMALL_BLOCK_SIZE;
        small_block_mod = large_block_mod % SMALL_BLOCK_SIZE;

        if (small_block_mod){

            is_exist = (large_block[large_block_num].member_small_block[small_block_num] & (0x1ULL << small_block_mod));
        }else{

            is_exist = (large_block[large_block_num].member_small_block[small_block_num] & 0x1);
        }
    }else{

        is_exist = (large_block[large_block_num].member_small_block[0] & 0x1);
    }

    if (false == is_exist){

        return true;
    }

    INDEX_LARGE_BLOCK* local_large_block = &(large_block[large_block_num]);

    int64_t popcnt_num = local_large_block->member_prev_large_block_popcnt;

    for (uint32_t i = 0; small_block_num > i; ++i){

        popcnt_num += __popcnt64(local_large_block->member_small_block[i]);
    }

    if (small_block_mod){

        uint64_t mask = ((0x1ULL << small_block_mod) - 1);

        popcnt_num += __popcnt64(local_large_block->member_small_block[small_block_num] & mask);
    }

    uint32_t unicode_data_num = sizeof(unicode_data) / sizeof(unicode_data[0]) - 1;

    if (popcnt_num > unicode_data_num){

        return true;
    }

    if (NULL == unicode_data[popcnt_num].name){

        return true;
    }

    if (code_point == unicode_data[popcnt_num].code_point){

        uint8_t* general_category = unicode_data[popcnt_num].general_category;

        if ((0 == strcmp(general_category, "Nd")) ||
            (0 == strcmp(general_category, "Nl")) ||
            (0 == strcmp(general_category, "No"))){

            *is_numeric = true;

            return true;
        }
    }

    return true;
}

