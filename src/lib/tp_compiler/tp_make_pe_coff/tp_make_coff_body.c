
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool get_coff_size_and_calc_offset(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t x64_code_buffer_size, rsize_t* coff_file_size, rsize_t* code_offset
);
static bool make_coff_file_HEADER(TP_SYMBOL_TABLE* symbol_table, uint8_t* coff_buffer);

bool tp_make_COFF(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size, bool is_output)
{
    bool is_print = IS_PE_PRINT(symbol_table);

    rsize_t coff_file_size = 0;
    rsize_t code_offset = 0;

    if ( ! get_coff_size_and_calc_offset(
        symbol_table, x64_code_buffer_size, &coff_file_size, &code_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    uint8_t* tmp_coff_buffer = (uint8_t*)TP_CALLOC(
        symbol_table, coff_file_size, sizeof(uint8_t)
    );

    if (NULL == tmp_coff_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    uint8_t* coff_buffer = tmp_coff_buffer;

    symbol_table->member_pe_coff_buffer = coff_buffer;
    symbol_table->member_pe_coff_size = coff_file_size;
    symbol_table->member_pe_coff_current_offset = 0;

    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    // COFF File Header
    if ( ! make_coff_file_HEADER(symbol_table, coff_buffer)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // Section Table
    if (0 >= coff->member_section_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    // .data
    if (coff->member_data_pos){

        memcpy(
            coff_buffer + coff->member_section_data_offset, &(coff->member_section_data),
            sizeof(TP_SECTION_TABLE)
        );

        memcpy(
            coff_buffer + coff->member_data_offset, coff->member_data,
            coff->member_data_pos
        );

        if (coff->member_data_coff_relocations.member_num){

            memcpy(
                coff_buffer + coff->member_data_coff_relocations_offset,
                coff->member_data_coff_relocations.member_relocations,
                coff->member_data_coff_relocations.member_num * sizeof(TP_COFF_RELOCATIONS)
            );
        }
    }

    // .rdata
    if (coff->member_rdata_pos){

        memcpy(
            coff_buffer + coff->member_section_rdata_offset, &(coff->member_section_rdata),
            sizeof(TP_SECTION_TABLE)
        );

        memcpy(
            coff_buffer + coff->member_rdata_offset, coff->member_rdata,
            coff->member_rdata_pos
        );

        if (coff->member_rdata_coff_relocations.member_num){

            memcpy(
                coff_buffer + coff->member_rdata_coff_relocations_offset,
                coff->member_rdata_coff_relocations.member_relocations,
                coff->member_rdata_coff_relocations.member_num * sizeof(TP_COFF_RELOCATIONS)
            );
        }
    }

    // .text
    if (code_offset){

        memcpy(
            coff_buffer + coff->member_section_text_offset, &(coff->member_section_text),
            sizeof(TP_SECTION_TABLE)
        );

        memcpy(coff_buffer + code_offset, x64_code_buffer, x64_code_buffer_size);

        if (coff->member_section_text.SizeOfRawData > x64_code_buffer_size){

            rsize_t padding_size = coff->member_section_text.SizeOfRawData - x64_code_buffer_size;

            if (padding_size){

                code_offset += x64_code_buffer_size;

                memset(coff_buffer + code_offset, TP_X64_OPCODE_NOP, padding_size);
            }
        }

        if (coff->member_text_coff_relocations.member_num){

            memcpy(
                coff_buffer + coff->member_text_coff_relocations_offset,
                coff->member_text_coff_relocations.member_relocations,
                coff->member_text_coff_relocations.member_num * sizeof(TP_COFF_RELOCATIONS)
            );
        }
    }

    // COFF Symbol Table
    memcpy(
        coff_buffer + coff->member_coff_symbol_offset, coff->member_coff_symbol,
        coff->member_coff_symbol_num * sizeof(TP_COFF_SYMBOL_TABLE)
    );

    // COFF String Table
    memcpy(
        coff_buffer + coff->member_string_table_offset,
        &(coff->member_string_table_size), sizeof(coff->member_string_table_size)
    );
    memcpy(
        coff_buffer + coff->member_string_table_offset + sizeof(coff->member_string_table_size),
        coff->member_string_table, coff->member_string_table_size
    );

    if (is_output){

        if (is_print){

            // ----------------------------------------------------------------------------------------
            // PE/COFF Object File
            if ( ! tp_write_file(
                symbol_table, symbol_table->member_coff_file_path,
                coff_buffer, (uint32_t)coff_file_size)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    return true;
}

static bool get_coff_size_and_calc_offset(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t x64_code_buffer_size, rsize_t* coff_file_size, rsize_t* code_offset)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    // COFF File Header
    rsize_t size = sizeof(TP_COFF_FILE_HEADER);

    if (coff->member_data_pos){

        // Section Table(.data)
        coff->member_section_data_offset = size;
        size += sizeof(TP_SECTION_TABLE);

        if (TP_NRELOC_OVFL_NUM <= coff->member_data_coff_relocations.member_num){

            coff->member_section_data.NumberOfRelocations = TP_NRELOC_OVFL_NUM;
            coff->member_section_data.Characteristics |= TP_IMAGE_SCN_LNK_NRELOC_OVFL;

            if (NULL == coff->member_data_coff_relocations.member_relocations){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            coff->member_data_coff_relocations.member_relocations[0].UNION.RelocCount =
                coff->member_data_coff_relocations.member_num;
        }else{

            coff->member_section_data.NumberOfRelocations =
                coff->member_data_coff_relocations.member_num;
        }
    }

    if (coff->member_rdata_pos){

        // Section Table(.rdata)
        coff->member_section_rdata_offset = size;
        size += sizeof(TP_SECTION_TABLE);

        if (TP_NRELOC_OVFL_NUM <= coff->member_rdata_coff_relocations.member_num){

            coff->member_section_rdata.NumberOfRelocations = TP_NRELOC_OVFL_NUM;
            coff->member_section_rdata.Characteristics |= TP_IMAGE_SCN_LNK_NRELOC_OVFL;

            if (NULL == coff->member_rdata_coff_relocations.member_relocations){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            coff->member_rdata_coff_relocations.member_relocations[0].UNION.RelocCount =
                coff->member_rdata_coff_relocations.member_num;
        }else{

            coff->member_section_rdata.NumberOfRelocations =
                coff->member_rdata_coff_relocations.member_num;
        }
    }

    if (x64_code_buffer_size){

        // Section Table(.text)
        coff->member_section_text_offset = size;
        size += sizeof(TP_SECTION_TABLE);

        if (TP_NRELOC_OVFL_NUM <= coff->member_text_coff_relocations.member_num){

            coff->member_section_text.NumberOfRelocations = TP_NRELOC_OVFL_NUM;
            coff->member_section_text.Characteristics |= TP_IMAGE_SCN_LNK_NRELOC_OVFL;

            if (NULL == coff->member_text_coff_relocations.member_relocations){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            coff->member_text_coff_relocations.member_relocations[0].UNION.RelocCount =
                coff->member_text_coff_relocations.member_num;
        }else{

            coff->member_section_text.NumberOfRelocations =
                coff->member_text_coff_relocations.member_num;
        }
    }

    if (coff->member_data_pos){

        // Section Data(.data)
        size += TP_PE_PADDING_4_BYTES_ALIGNMENT(size);
        coff->member_data_offset = size;
        coff->member_section_data.PointerToRawData = (uint32_t)size; // Section Table
        rsize_t data_size = (rsize_t)(coff->member_data_pos);
        rsize_t tmp_data_size = data_size + TP_PE_PADDING_FILE_ALIGNMENT(data_size);
        coff->member_section_data.SizeOfRawData = (uint32_t)tmp_data_size;
        size += tmp_data_size;

        // Section Data(Relocations: .data)
        if (coff->member_data_coff_relocations.member_num){

            coff->member_data_coff_relocations_offset = size;
            coff->member_section_data.PointerToRelocations = (uint32_t)size; // Section Table
            size += coff->member_data_coff_relocations.member_size;
        }else {

            coff->member_section_data.PointerToRelocations = 0; // Section Table
        }
    }

    if (coff->member_rdata_pos){

        // Section Data(.rdata)
        size += TP_PE_PADDING_4_BYTES_ALIGNMENT(size);
        coff->member_rdata_offset = size;
        coff->member_section_rdata.PointerToRawData = (uint32_t)size; // Section Table
        rsize_t rdata_size = (rsize_t)(coff->member_rdata_pos);
        rsize_t tmp_rdata_size = rdata_size + TP_PE_PADDING_FILE_ALIGNMENT(rdata_size);
        coff->member_section_rdata.SizeOfRawData = (uint32_t)tmp_rdata_size;
        size += tmp_rdata_size;

        // Section Data(Relocations: .rdata)
        if (coff->member_rdata_coff_relocations.member_num){

            coff->member_rdata_coff_relocations_offset = size;
            coff->member_section_rdata.PointerToRelocations = (uint32_t)size; // Section Table
            size += coff->member_rdata_coff_relocations.member_size;
        }else{

            coff->member_section_rdata.PointerToRelocations = 0; // Section Table
        }
    }

    if (x64_code_buffer_size){

        // Section Data(.text)
        size += TP_PE_PADDING_4_BYTES_ALIGNMENT(size);

        *code_offset = size;
        coff->member_section_text.PointerToRawData = (uint32_t)size; // Section Table

        rsize_t text_size = (rsize_t)(x64_code_buffer_size);
        rsize_t tmp_text_size = 0;

        tmp_text_size = text_size + TP_PE_PADDING_FILE_ALIGNMENT(text_size);

        coff->member_section_text.SizeOfRawData = (uint32_t)tmp_text_size;
        size += tmp_text_size;

        // Section Data(Relocations: .text)
        if (coff->member_text_coff_relocations.member_num){

            coff->member_text_coff_relocations_offset = size;
            coff->member_section_text.PointerToRelocations = (uint32_t)size; // Section Table
            size += coff->member_text_coff_relocations.member_size;
        }else{

            coff->member_section_text.PointerToRelocations = 0; // Section Table
        }
    }

    // COFF Symbol Table
    coff->member_coff_symbol_offset = size;
    size += (sizeof(TP_COFF_SYMBOL_TABLE) * (rsize_t)(coff->member_coff_symbol_num));

    // COFF String Table
    coff->member_string_table_offset = size;
    size += sizeof(coff->member_string_table_size);
    size += (rsize_t)(coff->member_string_table_size);

    *coff_file_size = size;

    return true;
}

static bool make_coff_file_HEADER(TP_SYMBOL_TABLE* symbol_table, uint8_t* coff_buffer)
{
    // COFF File Header
    TP_COFF_FILE_HEADER* file_header = &(symbol_table->member_coff.member_file_header);

    file_header->Machine = TP_IMAGE_FILE_MACHINE_AMD64;
    file_header->NumberOfSections = symbol_table->member_coff.member_section_num;
    file_header->TimeDateStamp = 0;
    file_header->PointerToSymbolTable = (uint32_t)(symbol_table->member_coff.member_coff_symbol_offset);
    file_header->NumberOfSymbols = symbol_table->member_coff.member_coff_symbol_num;
    file_header->SizeOfOptionalHeader = 0;
    file_header->Characteristics = TP_IMAGE_FILE_LARGE_ADDRESS_AWARE;

    memcpy(coff_buffer, file_header, sizeof(TP_COFF_FILE_HEADER));

    return true;
}

