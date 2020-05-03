
// Copyright (C) 2019-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool make_PE_file_SECTION_TABLE_section_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section, rsize_t index, bool is_reloc_count_ex
);
static bool make_PE_file_SECTION_TABLE_section_data_RawData(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section
);
static bool make_PE_file_SECTION_TABLE_section_data_Relocations(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section, rsize_t index, bool is_reloc_count_ex
);

bool tp_make_PE_file_SECTION_TABLE(TP_SYMBOL_TABLE* symbol_table, FILE* write_file)
{
    bool is_print = symbol_table->member_is_output_log_file;

    // ----------------------------------------------------------------------------------------
    // Section Table
    if (NULL == symbol_table->member_coff_file_header){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (IS_PE_IMAGE_FILE(symbol_table)){

        if (symbol_table->member_section_table){

            SecureZeroMemory(symbol_table->member_section_table, symbol_table->member_section_table_size);
            free(symbol_table->member_section_table);
            symbol_table->member_section_table = NULL;
        }

        rsize_t offset =
            symbol_table->member_dos_header_read->e_lfanew +
            offsetof(TP_PE_HEADER64_READ, OptionalHeader) +
            sizeof(TP_PE_OPTIONAL_HEADER64) +
            (sizeof(TP_PE_DATA_DIRECTORY) * symbol_table->member_pe_header64_read->OptionalHeader.NumberOfRvaAndSizes);

        if ( ! tp_seek_PE_COFF_file(symbol_table, 0, (long)offset)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }else{

        if ( ! tp_seek_PE_COFF_file(symbol_table, 0, (long)sizeof(TP_COFF_FILE_HEADER))){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    rsize_t num = symbol_table->member_coff_file_header->NumberOfSections;
    symbol_table->member_section_table_num = num;
    symbol_table->member_section_table_size = num * sizeof(TP_SECTION_TABLE);

    symbol_table->member_section_table = (TP_SECTION_TABLE*)calloc(num, sizeof(TP_SECTION_TABLE));

    if (NULL == symbol_table->member_section_table){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    memcpy(
        symbol_table->member_section_table, TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table),
        symbol_table->member_section_table_size
    );

    for (rsize_t i = 0; num > i; ++i){

        uint8_t* Name = symbol_table->member_section_table[i].Name;

        if ( ! IS_PE_IMAGE_FILE(symbol_table)){

            if ('/' == Name[0]){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if ( ! ((0 == strncmp(Name, ".text", 5)) ||
//              (0 == strncmp(Name, ".tls", 4)) ||
//              (0 == strncmp(Name, ".drectve", 8)) ||
                (0 == strncmp(Name, ".bss", 4)) ||
                (0 == strncmp(Name, ".data", 5)) ||
                (0 == strncmp(Name, ".rdata", 6)))){

                sprintf_s(Name, TP_IMAGE_SIZEOF_SHORT_NAME, "%c", '0' + (int)i);

                symbol_table->member_section_table[i].SizeOfRawData = 0;
                symbol_table->member_section_table[i].PointerToRawData = 0;
                symbol_table->member_section_table[i].PointerToRelocations = 0;
                symbol_table->member_section_table[i].PointerToLinenumbers = 0;
                symbol_table->member_section_table[i].NumberOfRelocations = 0;
                symbol_table->member_section_table[i].NumberOfLinenumbers = 0;

                symbol_table->member_section_table[i].Characteristics = TP_IMAGE_SCN_LNK_REMOVE;

                continue;
            }
        }

        uint32_t Characteristics = symbol_table->member_section_table[i].Characteristics;

        if (is_print){

            fprintf(write_file,
                "TP_SECTION_TABLE(%zu):\n"
                "    uint8_t Name(%.*s);\n"
                "    uint32_t VirtualSize(%x);\n"
                "    uint32_t VirtualAddress(%x);\n"
                "    uint32_t SizeOfRawData(%x);\n"
                "    uint32_t PointerToRawData(%x);\n"
                "    uint32_t PointerToRelocations(%x);\n"
                "    uint32_t PointerToLinenumbers(%x);\n"
                "    uint16_t NumberOfRelocations(%x);\n"
                "    uint16_t NumberOfLinenumbers(%x);\n"
                "    uint32_t Characteristics(%x);\n\n",
                i + 1,
                TP_IMAGE_SIZEOF_SHORT_NAME,
                Name,
                symbol_table->member_section_table[i].VirtualSize,
                symbol_table->member_section_table[i].VirtualAddress,
                symbol_table->member_section_table[i].SizeOfRawData,
                symbol_table->member_section_table[i].PointerToRawData,
                symbol_table->member_section_table[i].PointerToRelocations,
                symbol_table->member_section_table[i].PointerToLinenumbers,
                symbol_table->member_section_table[i].NumberOfRelocations,
                symbol_table->member_section_table[i].NumberOfLinenumbers,
                Characteristics
            ); 

            // Characteristics(Section Flags)

            if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_CODE\n");
            }

            if (Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA\n");
            }

            if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA\n");
            }

            if (Characteristics & TP_IMAGE_SCN_LNK_INFO){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_INFO\n");
            }

            if (Characteristics & TP_IMAGE_SCN_LNK_REMOVE){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_REMOVE\n");
            }

            if (Characteristics & TP_IMAGE_SCN_LNK_COMDAT){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_LNK_COMDAT\n");
            }

            if (Characteristics & TP_IMAGE_SCN_GPREL){

                fprintf(write_file, "uint32_t Characteristics & TP_IMAGE_SCN_GPREL\n");
            }

            switch (Characteristics & TP_IMAGE_SCN_ALIGN_MASK){
            case TP_IMAGE_SCN_ALIGN_1BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_1BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_2BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_2BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_4BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_4BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_8BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_8BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_16BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_16BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_32BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_32BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_64BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_64BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_128BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_128BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_256BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_256BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_512BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_512BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_1024BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_1024BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_2048BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_2048BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_4096BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_4096BYTES.\n");
                break;
            case TP_IMAGE_SCN_ALIGN_8192BYTES:
                fprintf(write_file, "uint32_t Characteristics is TP_IMAGE_SCN_ALIGN_8192BYTES.\n");
                break;
            default:
                break;
            }
        }

        bool is_reloc_count_ex = false;

        if (is_print){

            if (Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

                is_reloc_count_ex = true;

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_DISCARDABLE){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_DISCARDABLE\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_NOT_CACHED){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_NOT_CACHED\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_NOT_PAGED){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_NOT_PAGED\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_SHARED){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_SHARED\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_EXECUTE){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_EXECUTE\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_READ){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_READ\n");
            }

            if (Characteristics & TP_IMAGE_SCN_MEM_WRITE){

                fprintf(write_file, "uint16_t Characteristics & TP_IMAGE_SCN_MEM_WRITE\n");
            }

            fprintf(write_file, "\n");
        }

        if (is_reloc_count_ex &&
            (TP_NRELOC_OVFL_NUM != symbol_table->member_section_table[i].NumberOfRelocations)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if ( ! make_PE_file_SECTION_TABLE_section_data(
            symbol_table, write_file, &(symbol_table->member_section_table[i]), i, is_reloc_count_ex)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    if (IS_PE_IMAGE_FILE(symbol_table)){

        TP_PE_OPTIONAL_HEADER64* optional_header = &(symbol_table->member_pe_header64_read->OptionalHeader);

        if ( ! tp_make_PE_file_PE_DATA_DIRECTORY(symbol_table, write_file, optional_header)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool make_PE_file_SECTION_TABLE_section_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section, rsize_t index, bool is_reloc_count_ex)
{
    // Section Data(RawData)
    if ( ! make_PE_file_SECTION_TABLE_section_data_RawData(symbol_table, write_file, section)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_PE_IMAGE_FILE(symbol_table)){

        if (0 == strncmp(section->Name, ".reloc", 6)){

            uint8_t* raw_data = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);
            uint32_t size = section->SizeOfRawData;

            // PE File Directory(Based relocation)
            if ( ! tp_make_PE_file_PE_BASE_RELOCATION(symbol_table, write_file, raw_data, size)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }else{

        // Section Data(Relocations)
        if ( ! make_PE_file_SECTION_TABLE_section_data_Relocations(
            symbol_table, write_file, section, index, is_reloc_count_ex)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    return true;
}

static bool make_PE_file_SECTION_TABLE_section_data_RawData(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(RawData)
    bool is_print = symbol_table->member_is_output_log_file;

    uint32_t PointerToRawData = section->PointerToRawData;

    if (0 == PointerToRawData){

        return true;
    }

    if ( ! tp_seek_PE_COFF_file(symbol_table, (long)PointerToRawData, 0)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    uint8_t* raw_data = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

    rsize_t size = section->SizeOfRawData;

    if (section->Characteristics & TP_IMAGE_SCN_CNT_CODE){

        if (IS_PE_IMAGE_FILE(symbol_table)){

            if ( ! tp_write_data(
                symbol_table, raw_data, size,
                TP_PE_CODE_DEFAULT_FNAME, TP_PE_CODE_DEFAULT_EXT)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }else{

            if ( ! tp_write_data(
                symbol_table, raw_data, size,
                TP_COFF_CODE_DEFAULT_FNAME, TP_COFF_CODE_DEFAULT_EXT)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }
        }
    }

    if (is_print){

        fprintf(write_file, "section_data_RawData:\n    "); 
    }

    if ( ! tp_make_PE_file_raw_data(symbol_table, write_file, raw_data, size)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (is_print){

        fprintf(write_file, "\n"); 
    }

    return true;
}

static bool make_PE_file_SECTION_TABLE_section_data_Relocations(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section, rsize_t index, bool is_reloc_count_ex)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(Relocations)
    bool is_print = symbol_table->member_is_output_log_file;

    if ( ! tp_seek_PE_COFF_file(symbol_table, (long)(section->PointerToRelocations), 0)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    rsize_t num = section->NumberOfRelocations;

    TP_COFF_RELOCATIONS* relocations = (TP_COFF_RELOCATIONS*)TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

    if (is_reloc_count_ex){

        num = relocations->UNION.RelocCount;

        ++num;
    }

    for (rsize_t i = (is_reloc_count_ex ? 1 : 0); num > i; ++i){

        if (is_print){

            fprintf(write_file,
                "TP_COFF_RELOCATIONS(%zu):\n"
                "    uint32_t VirtualAddress(%x);\n"
                "    uint32_t RelocCount(%x);\n"
                "    uint32_t SymbolTableIndex(%x);\n"
                "    uint16_t Type(%x);\n\n",
                i,
                relocations[i].UNION.VirtualAddress,
                relocations[i].UNION.RelocCount, // When IMAGE_SCN_LNK_NRELOC_OVFL is set.
                relocations[i].SymbolTableIndex,
                relocations[i].Type
            ); 

            // Type Indicators

            switch (relocations[i].Type){
            case TP_IMAGE_REL_AMD64_ABSOLUTE:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ABSOLUTE.\n");
                break;
            case TP_IMAGE_REL_AMD64_ADDR64:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR64.\n");
                break;
            case TP_IMAGE_REL_AMD64_ADDR32:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR32.\n");
                break;
            case TP_IMAGE_REL_AMD64_ADDR32NB:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_ADDR32NB.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32_1:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_1.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32_2:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_2.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32_3:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_3.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32_4:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_4.\n");
                break;
            case TP_IMAGE_REL_AMD64_REL32_5:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_REL32_5.\n");
                break;
            case TP_IMAGE_REL_AMD64_SECTION:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECTION.\n");
                break;
            case TP_IMAGE_REL_AMD64_SECREL:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECREL.\n");
                break;
            case TP_IMAGE_REL_AMD64_SECREL7:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SECREL7.\n");
                break;
            case TP_IMAGE_REL_AMD64_TOKEN:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_TOKEN.\n");
                break;
            case TP_IMAGE_REL_AMD64_SREL32:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SREL32.\n");
                break;
            case TP_IMAGE_REL_AMD64_PAIR:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_PAIR.\n");
                break;
            case TP_IMAGE_REL_AMD64_SSPAN32:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_SSPAN32.\n");
                break;
            case TP_IMAGE_REL_AMD64_EHANDLER:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_EHANDLER.\n");
                break;
            case TP_IMAGE_REL_AMD64_IMPORT_BR:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_IMPORT_BR.\n");
                break;
            case TP_IMAGE_REL_AMD64_IMPORT_CALL:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_IMPORT_CALL.\n");
                break;
            case TP_IMAGE_REL_AMD64_CFG_BR:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_BR.\n");
                break;
            case TP_IMAGE_REL_AMD64_CFG_BR_REX:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_BR_REX.\n");
                break;
            case TP_IMAGE_REL_AMD64_CFG_CALL:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_CFG_CALL.\n");
                break;
            case TP_IMAGE_REL_AMD64_INDIR_BR:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR.\n");
                break;
            case TP_IMAGE_REL_AMD64_INDIR_BR_REX:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_REX.\n");
                break;
            case TP_IMAGE_REL_AMD64_INDIR_CALL:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_CALL.\n");
                break;
            case TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_FIRST.\n");
                break;
            case TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST:
                fprintf(write_file, "uint16_t Type is TP_IMAGE_REL_AMD64_INDIR_BR_SWITCHTABLE_LAST.\n");
                break;
            default:
                fprintf(write_file, "uint16_t Type is OTHER.\n");
                break;
            }

            fprintf(write_file, "\n");
        }

        if (TP_IMAGE_REL_AMD64_REL32 == relocations[i].Type){

            rsize_t symbols = symbol_table->member_coff_file_header->NumberOfSymbols;

            for (rsize_t j = 0; symbols > j; ++j){

                if (relocations[i].SymbolTableIndex == j){

                    if ( ! tp_make_PE_file_COFF_SYMBOL_TABLE_content(
                        symbol_table, write_file, &(symbol_table->member_coff_symbol_table[j]), j)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }

                    break;
                }

                j += symbol_table->member_coff_symbol_table[j].NumberOfAuxSymbols;
            }
        }
    }

    return true;
}

