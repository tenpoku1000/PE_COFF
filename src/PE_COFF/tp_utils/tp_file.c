
// Copyright (C) 2018-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_open_read_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream)
{
    int fd = 0;

    errno_t _sopen_s_error = _sopen_s(&fd, path, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0);

    if (_sopen_s_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }
  
    FILE* stream = _fdopen(fd, "rb");

    if (NULL == stream){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    *file_stream = stream;

    struct stat stbuf;

    int fstat_error = fstat(fd, &stbuf);

    if (-1 == fstat_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    if (TP_MAX_FILE_BYTES < stbuf.st_size){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("ERROR: TP_MAX_FILE_BYTES(%1) < stbuf.st_size(%2)"),
            TP_LOG_PARAM_UINT64_VALUE(TP_MAX_FILE_BYTES),
            TP_LOG_PARAM_UINT64_VALUE(stbuf.st_size)
        );

        return false;
    }

    return true;
}

bool tp_open_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, FILE** file_stream)
{
    FILE* write_file = NULL;

    errno_t err = fopen_s(&write_file, path, "wb");

    if (NULL == write_file){

        TP_PRINT_CRT_ERROR(NULL);

        return false;
    }

    *file_stream = write_file;

    return true;
}

bool tp_ftell(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long* seek_position)
{
    if (NULL == file_stream){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: NULL == file_stream")
        );

        return false;
    }

    long pos = ftell(file_stream);

    if (-1 == pos){

        TP_PRINT_CRT_ERROR(symbol_table);

        clearerr(file_stream);

        return false;
    }

    *seek_position = pos;

    return true;
}

bool tp_seek(TP_SYMBOL_TABLE* symbol_table, FILE* file_stream, long seek_position, long line_bytes)
{
    if (NULL == file_stream){

        TP_PUT_LOG_MSG(
            symbol_table, TP_LOG_TYPE_DISP_FORCE,
            TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: NULL == file_stream")
        );

        return false;
    }

    long offset = seek_position + line_bytes;

    int fseek_error = fseek(file_stream, offset, SEEK_SET);

    if (-1 == fseek_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        clearerr(file_stream);

        return false;
    }

    return true;
}

bool tp_close_file(TP_SYMBOL_TABLE* symbol_table, FILE** file_stream)
{
    if (NULL == file_stream){

        return true;
    }

    if (NULL == *file_stream){

        return true;
    }

    int fclose_error = fclose(*file_stream);

    if (EOF == fclose_error){

        TP_PRINT_CRT_ERROR(symbol_table);

        clearerr(*file_stream);

        return false;
    }

    *file_stream = NULL;

    return true;
}

bool tp_write_file(TP_SYMBOL_TABLE* symbol_table, char* path, void* content, uint32_t content_size)
{
    FILE* write_file = NULL;

    if ( ! tp_open_write_file(symbol_table, path, &write_file)){

//      TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    size_t fwrite_bytes = fwrite(content, sizeof(uint8_t), content_size, write_file);

    if (content_size > fwrite_bytes){

        int ferror_error = ferror(write_file);

        if (ferror_error){

            TP_PRINT_CRT_ERROR(symbol_table);

            clearerr(write_file);
        }

        if ( ! tp_close_file(symbol_table, &write_file)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);
        }

        return false;
    }

    if ( ! tp_close_file(symbol_table, &write_file)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

//      return false;
    }

    return true;
}

