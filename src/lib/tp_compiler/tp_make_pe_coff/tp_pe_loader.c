
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

static bool dll_validation(TP_SYMBOL_TABLE* symbol_table);

typedef int (*x64_dll_func)(void);

bool tp_load_and_exec_dll(
    TP_SYMBOL_TABLE* symbol_table, char* func_name, int* return_value)
{
    HMODULE module = LoadLibraryExA(
        symbol_table->member_load_dll_file_path, NULL, 0
    );

    if (NULL == module){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    x64_dll_func func = (x64_dll_func)GetProcAddress(module, func_name);

    if (NULL == func){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    int value = func();

    TP_PUT_LOG_PRINT(
        symbol_table, TP_MSG_FMT("x64_dll_func() = %1"),
        TP_LOG_PARAM_INT32_VALUE(value)
    );

    if (return_value){

        *return_value = value;
    }

    errno_t err = _set_errno(0);

    if ( ! FreeLibrary(module)){

        TP_GET_LAST_ERROR(NULL);

        return false;
    }

    return true;
}

bool tp_load_dll(
    TP_SYMBOL_TABLE* symbol_table, uint8_t** pe_buffer, rsize_t* pe_buffer_size)
{
    if ( ! dll_validation(symbol_table)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // Section Table
    TP_PE_HEADER64_WRITE* pe_header64_write =
        (TP_PE_HEADER64_WRITE*)(symbol_table->member_pe_coff_buffer);

    rsize_t pe_header64_write_size =
        sizeof(TP_PE_HEADER64_WRITE) +
            sizeof(TP_PE_DATA_DIRECTORY) * TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM;

    TP_SECTION_TABLE* section_table  =
        (TP_SECTION_TABLE*)(symbol_table->member_pe_coff_buffer + pe_header64_write_size);

    rsize_t NumberOfSections = pe_header64_write->FileHeader.NumberOfSections;
    rsize_t section_num = 0;
    rsize_t section_data_virtual_size = 0;

    for (rsize_t i = 0; NumberOfSections > i; ++i){

        if (0 == strncmp(".data", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".rdata", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".text", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else{

            continue;
        }

        section_data_virtual_size =
            section_table[i].VirtualAddress + section_table[i].VirtualSize;

        ++section_num;
    }
    if (0 == section_num){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // PE Image
    rsize_t section_table_size = sizeof(TP_SECTION_TABLE) * section_num;

    rsize_t header_size = pe_header64_write_size + section_table_size;

    rsize_t size_of_headers = header_size + TP_PE_PADDING_SECTION_ALIGNMENT(header_size);

    rsize_t pe_image_buffer_size = size_of_headers + section_data_virtual_size;

//  pe_header64_write->OptionalHeader.SizeOfImage

    uint8_t* pe_image_buffer = (uint8_t*)VirtualAlloc(
        NULL, pe_image_buffer_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE
    );

    if (NULL == pe_image_buffer){

        TP_GET_LAST_ERROR(symbol_table);

        return false;
    }

    memset(pe_image_buffer, 0, pe_image_buffer_size);

    *pe_buffer = pe_image_buffer;
    *pe_buffer_size = pe_image_buffer_size;

    // Header
    TP_PE_OPTIONAL_HEADER64* OptionalHeader = &(pe_header64_write->OptionalHeader);
    rsize_t NumberOfRvaAndSizes = OptionalHeader->NumberOfRvaAndSizes;

    for (rsize_t i = 0; NumberOfRvaAndSizes > i; ++i){

        OptionalHeader->DataDirectory[i].VirtualAddress = 0;
        OptionalHeader->DataDirectory[i].Size = 0;
    }

    uint8_t* pe_image_header = (uint8_t*)pe_header64_write;

    if (pe_header64_write_size > pe_image_buffer_size){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    memcpy(pe_image_buffer, pe_image_header, pe_header64_write_size);

    // Section Table
    section_num = 0;
    rsize_t section_offset = 0;

    for (rsize_t i = 0; NumberOfSections > i; ++i){

        if (0 == strncmp(".data", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".rdata", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".text", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else{

            continue;
        }

        memcpy(
            pe_image_buffer + pe_header64_write_size + section_offset,
            &(section_table[section_num]), sizeof(section_table[section_num])
        );
        ++section_num;
        section_offset = sizeof(section_table[section_num]) * section_num;
    }

    // Section Data
    section_num = 0;

    for (rsize_t i = 0; NumberOfSections > i; ++i){

        bool is_text = false;

        if (0 == strncmp(".data", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".rdata", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

        }else if (0 == strncmp(".text", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            is_text = true;
        }else{

            continue;
        }

        rsize_t offset = section_table[section_num].VirtualAddress;

        if (is_text && (offset != OptionalHeader->BaseOfCode)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        uint8_t* dst = pe_image_buffer + offset;
        uint8_t* src = symbol_table->member_pe_coff_buffer + section_table[section_num].PointerToRawData;
        rsize_t size = section_table[section_num].SizeOfRawData;
        memcpy(dst, src, size);

        rsize_t padding_size = TP_PE_PADDING_SECTION_ALIGNMENT(size);

        if (is_text && padding_size){

            memset(dst + size, TP_X64_OPCODE_NOP, padding_size);
        }

        ++section_num;
    }

    return true;
}

static bool dll_validation(TP_SYMBOL_TABLE* symbol_table)
{
    // ----------------------------------------------------------------------------------------
    // PE File header
    TP_PE_HEADER64_WRITE* pe_header64_write =
        (TP_PE_HEADER64_WRITE*)(symbol_table->member_pe_coff_buffer);

    // Magic number
    if (TP_PE_DOS_HEADER_MAGIC != pe_header64_write->DosHeader.e_magic){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // PE\0\0
    if (TP_PE_HEADER64_SIGNATURE != pe_header64_write->Signature){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // COFF File Header

    // Machine Types
    if (TP_IMAGE_FILE_MACHINE_AMD64 != pe_header64_write->FileHeader.Machine){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if ((pe_header64_write->FileHeader.PointerToSymbolTable) ||
        (pe_header64_write->FileHeader.NumberOfSymbols)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // Characteristics
    uint16_t Characteristics = pe_header64_write->FileHeader.Characteristics;

    if ( ! ((TP_IMAGE_FILE_EXECUTABLE_IMAGE & Characteristics) &&
        (TP_IMAGE_FILE_LARGE_ADDRESS_AWARE & Characteristics) &&
        (TP_IMAGE_FILE_DLL & Characteristics))){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // PE File Optional header
    TP_PE_OPTIONAL_HEADER64* OptionalHeader = &(pe_header64_write->OptionalHeader);

    // PE32+
    if (TP_IMAGE_OPTIONAL_HEADER64_MAGIC != OptionalHeader->Magic){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (OptionalHeader->SizeOfUninitializedData){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (TP_PE_SECTION_ALIGNMENT != OptionalHeader->SectionAlignment){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (TP_PE_FILE_ALIGNMENT != OptionalHeader->FileAlignment){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // Subsystem
    switch (OptionalHeader->Subsystem){
    case TP_IMAGE_SUBSYSTEM_WINDOWS_GUI:
        break;
    case TP_IMAGE_SUBSYSTEM_EFI_APPLICATION:
        break;
    default:
        TP_PUT_LOG_MSG_ILE(symbol_table);
        return false;
    }

    // Data Directory
    if (TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM != OptionalHeader->NumberOfRvaAndSizes){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // ----------------------------------------------------------------------------------------
    // Section Table
    rsize_t pe_header64_write_size =
        sizeof(TP_PE_HEADER64_WRITE) +
            sizeof(TP_PE_DATA_DIRECTORY) * TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM;

    TP_SECTION_TABLE* section_table =
        (TP_SECTION_TABLE*)(symbol_table->member_pe_coff_buffer + pe_header64_write_size);

    rsize_t NumberOfSections = pe_header64_write->FileHeader.NumberOfSections;

    if (0 == NumberOfSections){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    bool is_data = false;
    bool is_rdata = false;
    bool is_text = false;
    bool is_reloc = false;
    bool is_edata = false;

    for (rsize_t i = 0; NumberOfSections > i; ++i){

        if ('/' == section_table[i].Name[0]){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if ((0 == section_table[i].VirtualSize) ||
            (0 == section_table[i].VirtualAddress) ||
            (0 == section_table[i].SizeOfRawData) ||
            (0 == section_table[i].PointerToRawData)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if (section_table[i].VirtualSize < section_table[i].SizeOfRawData){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        uint32_t offset =
            section_table[i].SizeOfRawData + section_table[i].PointerToRawData;

        if (symbol_table->member_pe_coff_size <= offset){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if ((section_table[i].PointerToRelocations) ||
            (section_table[i].PointerToLinenumbers) ||
            (section_table[i].NumberOfRelocations) ||
            (section_table[i].NumberOfLinenumbers)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        uint32_t Characteristics = section_table[i].Characteristics;

        if (0 == strncmp(".data", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            if (is_data){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_data = true;

            // .data   Initialized data (free format)
            if ( ! ((TP_IMAGE_SCN_CNT_INITIALIZED_DATA & Characteristics) &&
                (TP_IMAGE_SCN_MEM_READ & Characteristics) &&
                (TP_IMAGE_SCN_MEM_WRITE & Characteristics))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }
        }else if (0 == strncmp(".rdata", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            if (is_rdata){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_rdata = true;

            // .rdata  Read-only initialized data
            if ( ! ((TP_IMAGE_SCN_CNT_INITIALIZED_DATA & Characteristics) &&
                (TP_IMAGE_SCN_MEM_READ & Characteristics))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }
        }else if (0 == strncmp(".text", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            if (is_text){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_text = true;

            // .text   Executable code (free format)
            if ( ! ((TP_IMAGE_SCN_CNT_CODE & Characteristics) &&
                (TP_IMAGE_SCN_MEM_EXECUTE & Characteristics) &&
                (TP_IMAGE_SCN_MEM_READ & Characteristics))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }
        }else if (0 == strncmp(".reloc", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            if (is_reloc){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_reloc = true;

            // .reloc  Image relocations
            if ( ! ((TP_IMAGE_SCN_CNT_INITIALIZED_DATA & Characteristics) &&
                (TP_IMAGE_SCN_MEM_READ & Characteristics) &&
                (TP_IMAGE_SCN_MEM_DISCARDABLE & Characteristics))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            rsize_t pe_base_relocation_size = sizeof(TP_PE_BASE_RELOCATION) + sizeof(uint16_t);

            if (pe_base_relocation_size != section_table[i].SizeOfRawData){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            TP_PE_BASE_RELOCATION* pe_base_relocation =
                (TP_PE_BASE_RELOCATION*)(symbol_table->member_pe_coff_buffer +
                    section_table[i].PointerToRawData);

            if (pe_base_relocation->VirtualAddress){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if (pe_base_relocation_size != pe_base_relocation->SizeOfBlock){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            // Based relocation types
            uint16_t TypeOffset =
                (TP_IMAGE_REL_BASED_TYPE(TP_IMAGE_REL_BASED_ABSOLUTE) | TP_IMAGE_REL_BASED_OFFSET(0));

            if (TypeOffset != pe_base_relocation->TypeOffset[0]){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }
        }else if (0 == strncmp(".edata", section_table[i].Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            if (is_edata){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_edata = true;

            // .edata  Export tables
            if ( ! ((TP_IMAGE_SCN_CNT_INITIALIZED_DATA & Characteristics) &&
                (TP_IMAGE_SCN_MEM_READ & Characteristics))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }
        }else{

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }
    }

    return true;
}

