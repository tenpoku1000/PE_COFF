
// Copyright (C) 2019 Shin'ichi Ichikawa. Released under the MIT license.

#include "PE_COFF.h"

// ----------------------------------------------------------------------------------------
// Environment

static TP_SYMBOL_TABLE global_symbol_table = {
// config section:
    .member_is_output_log_file = true,

// message section:
    .member_disp_log_file = NULL,

// PE COFF section:
    // PE COFF OBJECT/IMAGE File
    .member_pe_coff_buffer = NULL,
    .member_pe_coff_size = 0,
    .member_pe_coff_current_offset = 0,

    // PE File header
    .member_dos_header_read = NULL,
    .member_pe_header64_read = NULL,

    // COFF File Header
    .member_coff_file_header = NULL,

    // Section Table
    .member_section_table = NULL,
    .member_section_table_size = 0,
    .member_section_table_num = 0,

    // Section Data(Relocations)
    .member_coff_relocations = NULL,
    .member_coff_relocations_size = 0,

    // COFF Symbol Table
    .member_coff_symbol_table = NULL,

    // COFF String Table
    .member_string_table_offset = 0,
    .member_string_table_size = 0,
    .member_string_table = NULL,
};

static uint8_t msg_buffer[TP_MESSAGE_BUFFER_SIZE] = { 0 };

static void free_memory(TP_SYMBOL_TABLE* symbol_table);
static bool make_PE_file(TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext, uint8_t* entry_point_symbol);
static bool make_PE_file_main(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol);
static bool read_file(TP_SYMBOL_TABLE* symbol_table, char* path);
static bool make_path(TP_SYMBOL_TABLE* symbol_table, char* path, size_t path_size, char* fname, char* ext);

int main()
{
    SetLastError(NO_ERROR);
    errno_t err = _set_errno(0);

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    TP_SYMBOL_TABLE* symbol_table = &global_symbol_table;
    uint8_t* entry_point_symbol = "efi_main";

    int status = EXIT_FAILURE;

    symbol_table->member_disp_log_file = stderr;

    if (0 != setvbuf(symbol_table->member_disp_log_file, msg_buffer, _IOFBF, TP_MESSAGE_BUFFER_SIZE)){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto error_out;
    }

    if ( ! make_PE_file(
        symbol_table, TP_COFF_OBJECT_DEFAULT_FNAME, TP_COFF_OBJECT_DEFAULT_EXT, entry_point_symbol)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto error_out;
    }

    if ( ! make_PE_file(symbol_table, NULL, NULL, NULL)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto error_out;
    }

    status = EXIT_SUCCESS;

error_out:
    free_memory(symbol_table);

    return status;
}

static void free_memory(TP_SYMBOL_TABLE* symbol_table)
{
    if (symbol_table->member_pe_coff_buffer){

        SecureZeroMemory(symbol_table->member_pe_coff_buffer, symbol_table->member_pe_coff_size);
        free(symbol_table->member_pe_coff_buffer);
        symbol_table->member_pe_coff_buffer = NULL;
    }

    if (symbol_table->member_section_table){

        SecureZeroMemory(symbol_table->member_section_table, symbol_table->member_section_table_size);
        free(symbol_table->member_section_table);
        symbol_table->member_section_table = NULL;
    }

    if (symbol_table->member_coff_relocations){

        for (rsize_t i = 0; symbol_table->member_section_table_num > i; ++i){

            if (symbol_table->member_coff_relocations[i].member_relocations){

                SecureZeroMemory(
                    symbol_table->member_coff_relocations[i].member_relocations,
                    symbol_table->member_coff_relocations[i].member_size
                );
                free(symbol_table->member_coff_relocations[i].member_relocations);
                symbol_table->member_coff_relocations[i].member_relocations = NULL;
            }
        }

        SecureZeroMemory(symbol_table->member_coff_relocations, symbol_table->member_coff_relocations_size);
        free(symbol_table->member_coff_relocations);
        symbol_table->member_coff_relocations = NULL;
    }
}

static bool make_PE_file(TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext, uint8_t* entry_point_symbol)
{
    bool is_print = symbol_table->member_is_output_log_file;

    char write_path[_MAX_PATH];
    memset(write_path, 0, sizeof(write_path));

    if (fname && ext){

        char read_path[_MAX_PATH];
        memset(read_path, 0, sizeof(read_path));

        if ( ! make_path(symbol_table, read_path, sizeof(read_path), fname, ext)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! read_file(symbol_table, read_path)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        if ( ! make_path(
            symbol_table, write_path, sizeof(write_path), fname, TP_PE_COFF_TEXT_DEFAULT_EXT)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else{

        if ( ! make_path(
            symbol_table, write_path, sizeof(write_path),
            TP_PE_UEFI_DEFAULT_FNAME, TP_PE_COFF_TEXT_DEFAULT_EXT)){

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
        if ( ! tp_write_data(
            symbol_table, symbol_table->member_pe_coff_buffer, symbol_table->member_pe_coff_size,
            TP_PE_UEFI_DEFAULT_FNAME, TP_PE_UEFI_DEFAULT_EXT)){

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

    uint8_t* read_buffer = (uint8_t*)calloc(read_count, sizeof(uint8_t));

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

bool tp_write_data(TP_SYMBOL_TABLE* symbol_table, uint8_t* data, rsize_t size, char* fname, char* ext)
{
    char write_path[_MAX_PATH];
    memset(write_path, 0, sizeof(write_path));

    if ( ! make_path(symbol_table, write_path, sizeof(write_path), fname, ext)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    FILE* write_code = NULL;

    errno_t err = fopen_s(&write_code, write_path, "wb");

    if (NULL == write_code){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    size_t fwrite_bytes = fwrite(data, sizeof(uint8_t), size, write_code);

    if (size > fwrite_bytes){

        int ferror_error = ferror(write_code);

        if (ferror_error){

            clearerr(write_code);
        }

        (void)fclose(write_code);

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    int fclose_error = fclose(write_code);

    if (EOF == fclose_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    write_code = NULL;

    return true;
}

static bool make_path(TP_SYMBOL_TABLE* symbol_table, char* path, size_t path_size, char* fname, char* ext)
{
    errno_t err = 0;

    char base_dir[_MAX_PATH];
    memset(base_dir, 0, sizeof(base_dir));
    char drive[_MAX_DRIVE];
    memset(drive, 0, sizeof(drive));
    char dir[_MAX_DIR];
    memset(dir, 0, sizeof(dir));

    HMODULE handle = GetModuleHandleA(NULL);

    if (0 == handle){

        TP_GET_LAST_ERROR(symbol_table);

        goto error_out;
    }

    DWORD status = GetModuleFileNameA(handle, base_dir, sizeof(base_dir));

    if (0 == status){

        goto error_out;
    }

    err = _splitpath_s(base_dir, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto error_out;
    }

    err = _makepath_s(path, path_size, drive, dir, fname, ext);

    if (err){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto error_out;
    }

    return true;

error_out:

    return false;
}

