
// Copyright (C) 2019-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

bool tp_make_PE_file_PE_DATA_DIRECTORY(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_PE_OPTIONAL_HEADER64* optional_header)
{
    // ----------------------------------------------------------------------------------------
    // PE File Data Directory
    bool is_print = symbol_table->member_is_output_log_file;

    uint32_t number_of_RVA_and_sizes = optional_header->NumberOfRvaAndSizes;

    if (0 == number_of_RVA_and_sizes){

        return true;
    }

    rsize_t number_of_sections = symbol_table->member_coff_file_header->NumberOfSections;

    TP_PE_DATA_DIRECTORY* data_directory = optional_header->DataDirectory;

    for (uint32_t i = 0; number_of_RVA_and_sizes > i; ++i){

        if (is_print){

            // Data Directory
            fprintf(write_file, "TP_PE_DATA_DIRECTORY: ");

            switch (i){
            case TP_IMAGE_DIRECTORY_ENTRY_EXPORT:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_EXPORT\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_IMPORT:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_IMPORT\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_RESOURCE:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_RESOURCE\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_EXCEPTION:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_EXCEPTION\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_SECURITY:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_SECURITY\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_BASERELOC:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_BASERELOC\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_DEBUG:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_DEBUG\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_ARCHITECTURE\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_GLOBALPTR:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_GLOBALPTR\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_TLS:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_TLS\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_IAT:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_IAT\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR\n");
                break;
            case TP_IMAGE_DIRECTORY_ENTRY_REVERSED:
                fprintf(write_file, "TP_IMAGE_DIRECTORY_ENTRY_REVERSED\n");
                break;
            default:
                fprintf(write_file, "OTHER(%x)\n", i);
                break;
            }
        }

        uint32_t data_directory_from = data_directory[i].VirtualAddress;
        uint32_t data_directory_to = data_directory_from + data_directory[i].Size - 1;
        uint32_t data_directory_size = data_directory[i].Size;

        if (is_print){

            fprintf(
                write_file,
                "    uint32_t VirtualAddress(%x);\n"
                "    uint32_t Size(%x);\n\n",
                data_directory_from,
                data_directory_size
            );
        }

        if (0 == data_directory_from){

            continue;
        }

        for (rsize_t j = 0; number_of_sections > j; ++j){

            uint32_t virtual_address_from = symbol_table->member_section_table[j].VirtualAddress;
            uint32_t virtual_address_to = virtual_address_from + symbol_table->member_section_table[j].VirtualSize - 1;

            if ( ! ((virtual_address_from <= data_directory_from) &&
                (virtual_address_to >= data_directory_from))){

                continue;
            }

            if ( ! ((virtual_address_from <= data_directory_to) &&
                (virtual_address_to >= data_directory_to))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            uint32_t pointer_to_raw_data = symbol_table->member_section_table[j].PointerToRawData +
                data_directory_from - virtual_address_from;

            if (0 == pointer_to_raw_data){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if ( ! tp_seek_PE_COFF_file(symbol_table, (long)pointer_to_raw_data, 0)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (IS_EOF_PE_COFF(symbol_table)){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            uint8_t* Name = symbol_table->member_section_table[j].Name;

            if (is_print){

                fprintf(write_file,
                    "TP_SECTION_TABLE(%zu):\n"
                    "    uint8_t Name(%.*s);\n"
                    "    uint32_t VirtualSize(%x);\n"
                    "    uint32_t VirtualAddress(%x);\n"
                    "    uint32_t SizeOfRawData(%x);\n"
                    "    uint32_t PointerToRawData(%x);\n\n",
                    j + 1,
                    TP_IMAGE_SIZEOF_SHORT_NAME,
                    Name,
                    symbol_table->member_section_table[j].VirtualSize,
                    virtual_address_from,
                    symbol_table->member_section_table[j].SizeOfRawData,
                    symbol_table->member_section_table[j].PointerToRawData
                ); 
            }

            uint8_t* raw_data = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

            if (is_print){

                fprintf(write_file, "Data_Directory_RawData:\n    "); 
            }

            if ( ! tp_make_PE_file_raw_data(symbol_table, write_file, raw_data, data_directory_size)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (is_print){

                fprintf(write_file, "\n");
            }

            if (TP_IMAGE_DIRECTORY_ENTRY_BASERELOC == i){

                // PE File Directory(Based relocation)
                if ( ! tp_make_PE_file_PE_BASE_RELOCATION(symbol_table, write_file, raw_data, data_directory_size)){

                    TP_PUT_LOG_MSG_TRACE(symbol_table);

                    return false;
                }
            }

            break;
        }
    }

    if (is_print){

        fprintf(write_file, "\n");
    }

    return true;
}

bool tp_make_PE_file_PE_BASE_RELOCATION(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* raw_data, uint32_t data_size)
{
    // ----------------------------------------------------------------------------------------
    // PE File Directory(Based relocation)
    bool is_print = symbol_table->member_is_output_log_file;

    TP_PE_BASE_RELOCATION* base_relocation = NULL;
    uint32_t size_of_block = 0;
    uint32_t offset = 0;

    for (int64_t size = data_size; 0 < size; size -= size_of_block){

        if (sizeof(TP_PE_BASE_RELOCATION) > size){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        base_relocation = (TP_PE_BASE_RELOCATION*)(raw_data + offset);
        size_of_block = base_relocation->SizeOfBlock;
        offset += size_of_block;

        if (is_print){

            fprintf(write_file, "TP_PE_BASE_RELOCATION:\n"
                "    uint32_t VirtualAddress(%x);\n"
                "    uint32_t SizeOfBlock(%x);\n\n",
                base_relocation->VirtualAddress,
                size_of_block
            );
        }

        if (0 == size_of_block){

            break;
        }

        if (sizeof(TP_PE_BASE_RELOCATION) == size_of_block){

            continue;
        }

        if (sizeof(TP_PE_BASE_RELOCATION) > size_of_block){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if (is_print){

            uint32_t num = (size_of_block - sizeof(TP_PE_BASE_RELOCATION))
                / sizeof(base_relocation->TypeOffset[0]);

            fprintf(write_file, "TP_IMAGE_REL_BASED_TYPE_OFFSET: %x\n", num);

            for (uint32_t i = 0; num > i; ++i){

                // Based relocation types

                switch (TP_IMAGE_REL_BASED_TYPE(base_relocation->TypeOffset[i])){
                case TP_IMAGE_REL_BASED_ABSOLUTE:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_ABSOLUTE\n", i
                    );
                    break;
                case TP_IMAGE_REL_BASED_HIGH:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_HIGH\n", i
                    );
                    break;
                case TP_IMAGE_REL_BASED_LOW:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_LOW\n", i
                    );
                    break;
                case TP_IMAGE_REL_BASED_HIGHLOW:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_HIGHLOW\n", i
                    );
                    break;
                case TP_IMAGE_REL_BASED_HIGHADJ:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_HIGHADJ\n", i
                    );
                    break;
                case TP_IMAGE_REL_BASED_DIR64:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : TP_IMAGE_REL_BASED_DIR64\n", i
                    );
                    break;
                default:
                    fprintf(
                        write_file, "    uint16_t TypeOffset[%x] : OTHER Type(%x)\n",
                        i, TP_IMAGE_REL_BASED_TYPE(base_relocation->TypeOffset[i])
                    );
                    break;
                }

                fprintf(
                    write_file, "    uint16_t TypeOffset[%x] : Offset(%x)\n\n",
                    i, TP_IMAGE_REL_BASED_OFFSET(base_relocation->TypeOffset[i])
                );
            }
        }
    }

    return true;
}

