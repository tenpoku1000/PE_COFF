
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_make_PE_file_COFF_STRING_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    // ----------------------------------------------------------------------------------------
    // COFF String Table

    rsize_t offset = symbol_table->member_coff_file_header->NumberOfSymbols * sizeof(TP_COFF_SYMBOL_TABLE);

    if ( ! tp_seek_PE_COFF_file(
        symbol_table, (long)(symbol_table, symbol_table->member_coff_file_header->PointerToSymbolTable), (long)offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    memcpy(&(symbol_table->member_string_table_size), TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table), sizeof(uint32_t));

    offset += sizeof(uint32_t);

    if ( ! tp_seek_PE_COFF_file(
        symbol_table, (long)(symbol_table, symbol_table->member_coff_file_header->PointerToSymbolTable), (long)offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        if (0 == symbol_table->member_string_table_size){

            return true;
        }

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    symbol_table->member_string_table = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

    return true;
}

