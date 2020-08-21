
// Copyright (C) 2019-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool get_entry_point(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t code_offset, uint8_t* entry_point_symbol, uint8_t** entry_point
);
static bool calc_code_relocations(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset
);
static bool calc_code_relocations_body(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset,
    TP_COFF_SYMBOL_TABLE* symbol,
    TP_COFF_RELOCATIONS* coff_relocations, uint32_t coff_relocations_index
);
static bool make_coff_file_HEADER(TP_SYMBOL_TABLE* symbol_table, uint8_t* coff_buffer);

bool tp_make_COFF(
    TP_SYMBOL_TABLE* symbol_table, char* fname, char* ext,
    uint8_t* x64_code_buffer, uint32_t x64_code_buffer_size,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset)
{
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
    if (coff->member_data_pos){

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

    if (fname && ext){

        if ( ! tp_write_data(symbol_table, coff_buffer, coff_file_size, fname, ext)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

bool tp_get_coff_size_and_calc_offset(
    TP_SYMBOL_TABLE* symbol_table, bool is_pe_coff,
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
        if (is_pe_coff){

            size += TP_PE_PADDING_4_BYTES_ALIGNMENT(size);
        }else{

            size += TP_PE_PADDING_SECTION_ALIGNMENT(size);
        }

        *code_offset = size;
        coff->member_section_text.PointerToRawData = (uint32_t)size; // Section Table

        rsize_t text_size = (rsize_t)(x64_code_buffer_size);
        rsize_t tmp_text_size = 0;

        if (is_pe_coff){

            tmp_text_size = text_size + TP_PE_PADDING_FILE_ALIGNMENT(text_size);
        }else{

            tmp_text_size = text_size + TP_PE_PADDING_SECTION_ALIGNMENT(text_size);
        }

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

bool tp_get_entry_point_and_calc_code_relocations(
    TP_SYMBOL_TABLE* symbol_table,
    uint8_t* coff_buffer, rsize_t coff_file_size,
    rsize_t code_offset, uint8_t* entry_point_symbol, uint8_t** entry_point)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    if ( ! get_entry_point(
        symbol_table, coff, coff_buffer, code_offset, entry_point_symbol, entry_point)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if ( ! calc_code_relocations(symbol_table, coff, coff_buffer, coff_file_size, code_offset)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool get_entry_point(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t code_offset, uint8_t* entry_point_symbol, uint8_t** entry_point)
{
    // COFF Symbol Table(entry point)
    if (TP_IMAGE_SIZEOF_SHORT_NAME < strlen(entry_point_symbol)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_COFF_SYMBOL_TABLE* symbol = coff->member_coff_symbol;

    if (NULL == symbol){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t num = coff->member_coff_symbol_num;

    for (rsize_t i = 0; num > i; ++i){

        if (coff->member_section_number_text != symbol[i].SectionNumber){

            continue;
        }

        uint8_t* ShortName = symbol[i].UNION.ShortName;

        if (0 != strncmp(ShortName, entry_point_symbol, TP_IMAGE_SIZEOF_SHORT_NAME)){

            continue;
        }

        if (coff->member_section_text.Characteristics & TP_IMAGE_SCN_CNT_CODE){

            // Type Representation
            // Storage Class
            uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(&(symbol[i]));
            uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(&(symbol[i]));

            if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) && (TP_IMAGE_SYM_DTYPE_FUNCTION == type_msb) &&
                (TP_IMAGE_SYM_CLASS_EXTERNAL == symbol[i].StorageClass)){

                if (coff->member_section_text.SizeOfRawData <= symbol[i].Value){

                    TP_PUT_LOG_MSG_ICE(symbol_table);

                    return false;
                }

                rsize_t offset = symbol[i].Value;

                *entry_point = (coff_buffer + code_offset + offset);

                return true;
            }
        }
    }

    TP_PUT_LOG_MSG(
        symbol_table, TP_LOG_TYPE_DISP_FORCE,
        TP_MSG_FMT("%1"), TP_LOG_PARAM_STRING("ERROR: Entry point is not found.")
    );

    return false;
}

static bool calc_code_relocations(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset)
{
    if (0 == code_offset){

        return true;
    }

    uint8_t* code_buffer = coff_buffer + code_offset;

    uint32_t code_size = coff->member_section_text.SizeOfRawData;

    bool is_reloc_count_ex = false;

    if (coff->member_section_text.Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

        is_reloc_count_ex = true;
    }

    TP_COFF_RELOCATIONS* coff_relocations =
        coff->member_text_coff_relocations.member_relocations;

    uint32_t coff_relocations_num = coff->member_text_coff_relocations.member_num;

    TP_COFF_SYMBOL_TABLE* coff_symbol = coff->member_coff_symbol;

    if (NULL == coff_symbol){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    rsize_t symbol_num = coff->member_coff_symbol_num;

    if (coff_relocations){

        for (uint32_t i = (is_reloc_count_ex ? 1 : 0); coff_relocations_num > i; ++i){

            // Type Indicators
            switch (coff_relocations[i].Type){
            case TP_IMAGE_REL_AMD64_ABSOLUTE:
                break;
            case TP_IMAGE_REL_AMD64_REL32:

                for (rsize_t j = 0; symbol_num > j; ++j){

                    if (coff_relocations[i].SymbolTableIndex != j){

                        continue;
                    }

                    TP_COFF_SYMBOL_TABLE* symbol = coff_symbol + j;

                    if (0 >= symbol->SectionNumber){

                        TP_PUT_LOG_MSG_ICE(symbol_table);

                        return false;
                    }

                    if ( ! calc_code_relocations_body(
                        symbol_table, coff,
                        coff_buffer, coff_file_size, code_offset, symbol, coff_relocations, i)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }

                    j += symbol_table->member_coff_symbol_table[j].NumberOfAuxSymbols;
                }
                break;
            default:
                TP_PUT_LOG_MSG_ICE(symbol_table);
                return false;
            }
        }
    }

    return true;
}

static bool calc_code_relocations_body(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    uint8_t* coff_buffer, rsize_t coff_file_size, rsize_t code_offset,
    TP_COFF_SYMBOL_TABLE* symbol,
    TP_COFF_RELOCATIONS* coff_relocations, uint32_t coff_relocations_index)
{
    uint32_t virtual_address = coff_relocations[coff_relocations_index].UNION.VirtualAddress;

    // Type Representation
    // Storage Class
    uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(symbol);
    uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(symbol);

    if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) && (TP_IMAGE_SYM_DTYPE_NULL == type_msb) &&
        ((TP_IMAGE_SYM_CLASS_STATIC == symbol->StorageClass) ||
        (TP_IMAGE_SYM_CLASS_EXTERNAL == symbol->StorageClass))){

        // Section Table
        if (coff->member_section_num < symbol->SectionNumber){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (coff->member_section_text.SizeOfRawData <= virtual_address){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        TP_SECTION_TABLE* ref_section = NULL;

        if (coff->member_section_number_data == symbol->SectionNumber){

            ref_section = &(coff->member_section_data);
        }else if (coff->member_section_number_rdata == symbol->SectionNumber){

            ref_section = &(coff->member_section_rdata);
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        if (ref_section->SizeOfRawData <= symbol->Value){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        // Relocation(COFF)
        uint32_t offset = (uint32_t)code_offset;
        rsize_t va = virtual_address;
        rsize_t coff_offset = offset + va;

        uint8_t* apply_to = coff_buffer + coff_offset;

        int32_t apply_from = (int32_t)(ref_section->PointerToRawData + symbol->Value);
        apply_from -= (int32_t)(offset + virtual_address + sizeof(uint32_t));

        if (coff_file_size <= (coff_offset + sizeof(apply_from))){

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }

        memcpy(apply_to, &apply_from, sizeof(apply_from));

        return true;
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
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

