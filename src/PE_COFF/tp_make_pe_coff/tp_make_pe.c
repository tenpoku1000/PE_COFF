
// Copyright (C) 2019-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool make_PE_file_main(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol);
static bool read_file(TP_SYMBOL_TABLE* symbol_table, char* path);

bool tp_make_PE_file(
    TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext, uint8_t* entry_point_symbol)
{
    bool is_print = symbol_table->member_is_output_log_file;

    char drive[_MAX_DRIVE];
    memset(drive, 0, sizeof(drive));
    char dir[_MAX_DIR];
    memset(dir, 0, sizeof(dir));

    if ( ! tp_get_drive_dir(symbol_table, drive, dir)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    char write_path[_MAX_PATH];
    memset(write_path, 0, sizeof(write_path));

    if (fname && ext){

        char read_path[_MAX_PATH];
        memset(read_path, 0, sizeof(read_path));

        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL, fname, ext,
            read_path, sizeof(read_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! read_file(symbol_table, read_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            fname, TP_PE_COFF_TEXT_DEFAULT_EXT,
            write_path, sizeof(write_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else{

        if ( ! tp_make_path(
            symbol_table, drive, dir, NULL,
            TP_PE_UEFI_DEFAULT_FNAME, TP_PE_COFF_TEXT_DEFAULT_EXT,
            write_path, sizeof(write_path))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    FILE* write_file = NULL;

    if (is_print){

        errno_t err = fopen_s(&write_file, write_path, "w");

        if (NULL == write_file){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }
    }

    bool status = make_PE_file_main(symbol_table, write_file, entry_point_symbol);

    if (is_print){

        int fclose_error = fclose(write_file);

        if (EOF == fclose_error){

            clearerr(write_file);

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        write_file = NULL;
    }

    return status;
}

static bool make_PE_file_main(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol)
{
    // ----------------------------------------------------------------------------------------
    // PE File header
    if ( ! tp_make_PE_file_PE_HEADER64(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // COFF String Table
    if ( ! tp_make_PE_file_COFF_STRING_TABLE(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table
    if ( ! tp_make_PE_file_COFF_SYMBOL_TABLE(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // Section Table
    if ( ! tp_make_PE_file_SECTION_TABLE(symbol_table, write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ((NULL == symbol_table->member_pe_coff_buffer) || (0 == symbol_table->member_pe_coff_size)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (IS_PE_IMAGE_FILE(symbol_table)){

        // ----------------------------------------------------------------------------------------
        // PE IMAGE File(UEFI APPLICATION)
        if ( ! tp_write_file(
            symbol_table, symbol_table->member_pe_uefi_path,
            symbol_table->member_pe_coff_buffer, (uint32_t)symbol_table->member_pe_coff_size)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        return true;
    }

    // ----------------------------------------------------------------------------------------
    // Convert from COFF Object to PE Image. 
    if ( ! tp_make_PE_file_buffer(symbol_table, write_file, entry_point_symbol)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

// ----------------------------------------------------------------------------------------
// Utilities

bool tp_make_PE_file_raw_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, rsize_t size)
{
    bool is_print = symbol_table->member_is_output_log_file;

    for (rsize_t i = 0, count = 0; size > i; ++i){

        if (is_print){

            fprintf(write_file, "%02x ", raw_data[i]);
        }

        if (15 == count){

            count = 0;

            if (is_print){

                fprintf(write_file, "\n    "); 
            }
        }else{

            ++count;
        }
    }

    if (is_print){

        fprintf(write_file, "\n    "); 
    }

    for (rsize_t i = 0, count = 0; size > i; ++i){

        if (is_print){

            fprintf(write_file, "%c ", raw_data[i]); 
        }

        if (15 == count){

            count = 0;

            if (is_print){

                fprintf(write_file, "\n    "); 
            }
        }else{

            ++count;
        }
    }

    if (is_print){

        fprintf(write_file, "\n"); 
    }

    return true;
}

bool tp_seek_PE_COFF_file(TP_SYMBOL_TABLE* symbol_table, long seek_position, long line_bytes)
{
    rsize_t offset = (rsize_t)(seek_position) + (rsize_t)(line_bytes);

    symbol_table->member_pe_coff_current_offset = offset;

    return true;
}

static bool read_file(TP_SYMBOL_TABLE* symbol_table, char* path)
{
    int fd = 0;

    errno_t _sopen_s_error = _sopen_s(&fd, path, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);

    if (_sopen_s_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }
  
    FILE* read_file = _fdopen(fd, "rb");

    if (NULL == read_file){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    struct stat stbuf = { 0 };

    int fstat_error = fstat(fd, &stbuf);

    if (-1 == fstat_error){

        clearerr(read_file);
        (void)fclose(read_file);
        read_file = NULL;

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    bool is_eof = false;

    size_t read_count = stbuf.st_size;

    uint8_t* read_buffer = (uint8_t*)TP_CALLOC(symbol_table, read_count, sizeof(uint8_t));

    if (NULL == read_buffer){

        (void)fclose(read_file);
        read_file = NULL;

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    size_t fread_bytes = fread(
        read_buffer, sizeof(uint8_t), read_count, read_file
    );

    if (read_count > fread_bytes){

        int ferror_error = ferror(read_file);

        if (ferror_error){

            clearerr(read_file);
            (void)fclose(read_file);
            read_file = NULL;

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        int feof_error = feof(read_file);

        if (feof_error){

            is_eof = true;
        }

        (void)fclose(read_file);
        read_file = NULL;

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    int fclose_error = fclose(read_file);

    if (EOF == fclose_error){

        clearerr(read_file);
        read_file = NULL;

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    read_file = NULL;

    symbol_table->member_pe_coff_buffer = read_buffer;

    symbol_table->member_pe_coff_size = fread_bytes;

    return true;
}

