
// Copyright (C) 2019-2020 Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef struct TP_PE_PARAM_{
// PE Image header
    rsize_t header_size;
// PE Image Optional header
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    uint32_t base_reloc_virtual_address;
    uint32_t base_reloc_size;
// Section Table
    TP_SECTION_TABLE* section_to;
    rsize_t number_of_sections;
// Section Data
    uint8_t* section_data;
    rsize_t section_data_size;
    rsize_t section_data_virtual_size;
}TP_PE_PARAM;

static bool make_PE_file_buffer_section_validation(
    TP_SYMBOL_TABLE* symbol_table, bool* is_rename_bss2data
);
static bool make_PE_file_buffer_section_count(
    TP_SYMBOL_TABLE* symbol_table,
    bool* is_copy_sections, rsize_t is_copy_sections_num, rsize_t* number_of_sections
);
static bool make_PE_file_buffer_section(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    bool* is_copy_sections, rsize_t is_copy_sections_num, bool is_rename_bss2data,
    uint8_t* entry_point_symbol, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_symbol_entry_point(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    int16_t section_number, rsize_t new_section_index, uint8_t* entry_point_symbol, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_symbol_comdat(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    int16_t section_number, TP_PE_PARAM* param, TP_AUX_SYMBOL_RECORD_SECTION* aux_section
);
static bool make_PE_file_buffer_section_copy_relocation_data(
    TP_SYMBOL_TABLE* symbol_table,
    bool is_reloc_count_ex, rsize_t new_section_index, TP_SECTION_TABLE* section_to
);
static bool make_PE_file_buffer_section_relocation(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section_to, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size
);
static bool make_PE_file_buffer_section_data_relocation(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section_to,
    TP_SECTION_TABLE* section, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size,
    TP_COFF_RELOCATIONS_ARRAY* coff_relocations, bool is_reloc_count_ex
);
static bool make_PE_file_buffer_section_data_relocation_symbol(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section_to,
    TP_SECTION_TABLE* section, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size,
    uint32_t virtual_address, rsize_t symbol_index
);

// ----------------------------------------------------------------------------------------
// Convert from COFF Object to PE Image. 
bool tp_make_PE_file_buffer(TP_SYMBOL_TABLE* symbol_table, FILE* write_file, uint8_t* entry_point_symbol)
{
    bool status = false;

    if (IS_PE_IMAGE_FILE(symbol_table)){

        return status;
    }

    uint8_t* pe_image_buffer = NULL;
    bool* is_copy_sections = NULL;
    TP_SECTION_TABLE* section_table = NULL;
    TP_PE_PARAM param = { 0 };
    rsize_t number_of_sections = 0;

    // ----------------------------------------------------------------------------------------
    // Section Table
    bool is_rename_bss2data = false;

    if ( ! make_PE_file_buffer_section_validation(symbol_table, &is_rename_bss2data)){

        goto fail;
    }

    rsize_t is_copy_sections_num = symbol_table->member_section_table_num;
    rsize_t is_copy_sections_size = is_copy_sections_num * sizeof(bool);

    is_copy_sections = (bool*)TP_CALLOC(symbol_table, is_copy_sections_num, sizeof(bool));

    if (NULL == is_copy_sections){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    if ( ! make_PE_file_buffer_section_count(
        symbol_table, is_copy_sections, is_copy_sections_num, &number_of_sections)){

        goto fail;
    }

    rsize_t section_table_size = number_of_sections * sizeof(TP_SECTION_TABLE);

    section_table = (TP_SECTION_TABLE*)TP_CALLOC(symbol_table, number_of_sections, sizeof(TP_SECTION_TABLE));

    if (NULL == section_table){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    symbol_table->member_coff_relocations_size = number_of_sections * sizeof(TP_COFF_RELOCATIONS_ARRAY*);

    symbol_table->member_coff_relocations =
        (TP_COFF_RELOCATIONS_ARRAY*)TP_CALLOC(symbol_table, number_of_sections, sizeof(TP_COFF_RELOCATIONS_ARRAY));

    if (NULL == symbol_table->member_coff_relocations){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    rsize_t pe_header64_write_size =
        sizeof(TP_PE_HEADER64_WRITE) + sizeof(TP_PE_DATA_DIRECTORY) * TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM;

    TP_PE_PARAM tmp_param = {
    // PE Image header
        .header_size = pe_header64_write_size + section_table_size,
    // PE Image Optional header
        .size_of_code = 0,
        .size_of_initialized_data = 0,
        .size_of_uninitialized_data = 0,
        .address_of_entry_point = 0,
        .base_of_code = 0,
        .base_reloc_virtual_address = 0,
        .base_reloc_size = 0,
    // Section Table
        .section_to = section_table,
        .number_of_sections = number_of_sections,
    // Section Data
        .section_data = NULL,
        .section_data_size = 0,
        .section_data_virtual_size = 0,
    };

    param = tmp_param;

    // Convert Section
    if ( ! make_PE_file_buffer_section(
        symbol_table, write_file,
        is_copy_sections, is_copy_sections_num, is_rename_bss2data, entry_point_symbol, &param)){

        goto fail;
    }

    // ----------------------------------------------------------------------------------------
    // PE Image
    rsize_t size_of_headers = param.header_size + TP_PE_PADDING_FILE_ALIGNMENT(param.header_size);
    rsize_t pe_image_buffer_size = size_of_headers + param.section_data_size;

    pe_image_buffer = (uint8_t*)TP_CALLOC(symbol_table, pe_image_buffer_size, sizeof(uint8_t));

    if (NULL == pe_image_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    // Section Data
    memcpy(pe_image_buffer + size_of_headers, param.section_data, param.section_data_size);

    // Relocation(COFF)
    if ( ! make_PE_file_buffer_section_relocation(
        symbol_table, write_file, 
        section_table, number_of_sections, pe_image_buffer, pe_image_buffer_size)){

        goto fail;
    }

    SecureZeroMemory(symbol_table->member_section_table, symbol_table->member_section_table_size);
    free(symbol_table->member_section_table);
    symbol_table->member_section_table = NULL;

    symbol_table->member_section_table = section_table;
    symbol_table->member_section_table_size = section_table_size;
    symbol_table->member_section_table_num = number_of_sections;

    // Section Table
    memcpy(pe_image_buffer + pe_header64_write_size, section_table, section_table_size);

    // ----------------------------------------------------------------------------------------
    // Header
    TP_PE_HEADER64_WRITE* pe_header64_write = (TP_PE_HEADER64_WRITE*)pe_image_buffer;

    // PE File DOS Header
    pe_header64_write->DosHeader.e_magic = TP_PE_DOS_HEADER_MAGIC; // MZ
    pe_header64_write->DosHeader.e_lfanew = (int32_t)offsetof(TP_PE_HEADER64_WRITE, Signature);

    // PE\0\0
    pe_header64_write->Signature = TP_PE_HEADER64_SIGNATURE;

    // COFF File Header
    pe_header64_write->FileHeader.Machine = TP_IMAGE_FILE_MACHINE_AMD64;
    pe_header64_write->FileHeader.NumberOfSections = (uint8_t)number_of_sections;
    pe_header64_write->FileHeader.TimeDateStamp = 0;
    pe_header64_write->FileHeader.PointerToSymbolTable = 0;
    pe_header64_write->FileHeader.NumberOfSymbols = 0;
    pe_header64_write->FileHeader.SizeOfOptionalHeader =
        (uint16_t)(sizeof(TP_PE_OPTIONAL_HEADER64) +
            sizeof(TP_PE_DATA_DIRECTORY) * TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM);
    pe_header64_write->FileHeader.Characteristics =
        TP_IMAGE_FILE_EXECUTABLE_IMAGE | TP_IMAGE_FILE_LARGE_ADDRESS_AWARE | TP_IMAGE_FILE_DLL;

    // PE Image Optional header
    pe_header64_write->OptionalHeader.Magic = TP_IMAGE_OPTIONAL_HEADER64_MAGIC; // PE32+
    pe_header64_write->OptionalHeader.MajorLinkerVersion = 0;
    pe_header64_write->OptionalHeader.MinorLinkerVersion = 1;
    pe_header64_write->OptionalHeader.SizeOfCode = param.size_of_code;
    pe_header64_write->OptionalHeader.SizeOfInitializedData = param.size_of_initialized_data;
    pe_header64_write->OptionalHeader.SizeOfUninitializedData = param.size_of_uninitialized_data;
    pe_header64_write->OptionalHeader.AddressOfEntryPoint = param.address_of_entry_point;
    pe_header64_write->OptionalHeader.BaseOfCode = param.base_of_code;
    pe_header64_write->OptionalHeader.ImageBase = 0x180000000;
    pe_header64_write->OptionalHeader.SectionAlignment = TP_PE_SECTION_ALIGNMENT;
    pe_header64_write->OptionalHeader.FileAlignment = TP_PE_FILE_ALIGNMENT;
    pe_header64_write->OptionalHeader.MajorOperatingSystemVersion = 0;
    pe_header64_write->OptionalHeader.MinorOperatingSystemVersion = 0;
    pe_header64_write->OptionalHeader.MajorImageVersion = 0;
    pe_header64_write->OptionalHeader.MinorImageVersion = 0;
    pe_header64_write->OptionalHeader.MajorSubsystemVersion = 1;
    pe_header64_write->OptionalHeader.MinorSubsystemVersion = 0;
    pe_header64_write->OptionalHeader.Win32VersionValue = 0;
    pe_header64_write->OptionalHeader.SizeOfImage =
        (uint32_t)(param.header_size +
            TP_PE_PADDING_SECTION_ALIGNMENT(param.header_size) + param.section_data_virtual_size);
    pe_header64_write->OptionalHeader.SizeOfHeaders = (uint32_t)size_of_headers;
    pe_header64_write->OptionalHeader.CheckSum = 0;
    pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_EFI_APPLICATION;
    pe_header64_write->OptionalHeader.DllCharacteristics = 0;
    pe_header64_write->OptionalHeader.SizeOfStackReserve = 0x100000;
    pe_header64_write->OptionalHeader.SizeOfStackCommit = 0x1000;
    pe_header64_write->OptionalHeader.SizeOfHeapReserve = 0x100000;
    pe_header64_write->OptionalHeader.SizeOfHeapCommit = 0x1000;
    pe_header64_write->OptionalHeader.LoaderFlags = 0;
    pe_header64_write->OptionalHeader.NumberOfRvaAndSizes = TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM;

    // Based relocation(PE File Data Directory)
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress =
        param.base_reloc_virtual_address;
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size =
        param.base_reloc_size;

    // ----------------------------------------------------------------------------------------
    // PE Image
    if (symbol_table->member_pe_coff_buffer){

        SecureZeroMemory(symbol_table->member_pe_coff_buffer, symbol_table->member_pe_coff_size);
        free(symbol_table->member_pe_coff_buffer);
        symbol_table->member_pe_coff_buffer = NULL;
    }

    symbol_table->member_pe_coff_buffer = pe_image_buffer;
    symbol_table->member_pe_coff_size = pe_image_buffer_size;
    symbol_table->member_pe_coff_current_offset = 0;

    status = true;

fail:
    if (false == status){

        if (pe_image_buffer){

            SecureZeroMemory(pe_image_buffer, pe_image_buffer_size);
            free(pe_image_buffer);
            pe_image_buffer = NULL;
        }

        if (section_table){

            SecureZeroMemory(section_table, section_table_size);
            free(section_table);
            section_table = NULL;
        }

        symbol_table->member_section_table_num = number_of_sections;
    }

    if (is_copy_sections){

        SecureZeroMemory(is_copy_sections, is_copy_sections_size);
        free(is_copy_sections);
        is_copy_sections = NULL;
    }

    if (param.section_data){

        SecureZeroMemory(param.section_data, param.section_data_size);
        free(param.section_data);
        param.section_data = NULL;
    };

    return status;
}

static bool make_PE_file_buffer_section_validation(TP_SYMBOL_TABLE* symbol_table, bool* is_rename_bss2data)
{
    // ----------------------------------------------------------------------------------------
    // Section Table(Validation)
    bool is_text = false;
    bool is_bss = false;
    bool is_data = false;
    bool is_rdata = false;

    rsize_t number_of_sections = symbol_table->member_section_table_num;
    
    for (rsize_t i = 0; number_of_sections > i; ++i){

        uint8_t* Name = symbol_table->member_section_table[i].Name;

        if ('/' == Name[0]){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if ( ! ((0 == strncmp(Name, ".text", 5)) ||
//          (0 == strncmp(Name, ".tls", 4)) ||
//          (0 == strncmp(Name, ".drectve", 8)) ||
            (0 == strncmp(Name, ".bss", 4)) ||
            (0 == strncmp(Name, ".data", 5)) ||
            (0 == strncmp(Name, ".rdata", 6)))){

            continue;
        }

        uint32_t Characteristics = symbol_table->member_section_table[i].Characteristics;

        if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

            if ( ! (0 == strncmp(Name, ".text", 5))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if (is_text){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_text = true;
        }else if (Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA){

            bool is_data_ = (0 == strncmp(Name, ".data", 5));
            bool is_rdata_ = (0 == strncmp(Name, ".rdata", 6));

            if ( ! (is_data_ || is_rdata_)){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if (is_data && is_data_){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }else if (is_data_){

                is_data = true;
            }

            if (is_rdata && is_rdata_){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }else if (is_rdata_){

                is_rdata = true;
            }
        }else if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

            if ( ! (0 == strncmp(Name, ".bss", 4))){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            if (is_bss){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            is_bss = true;
        }

        bool is_reloc_count_ex = false;

        if (Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

            is_reloc_count_ex = true;
        }

        if (is_reloc_count_ex &&
            (TP_NRELOC_OVFL_NUM != symbol_table->member_section_table[i].NumberOfRelocations)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if (is_reloc_count_ex &&
            (0 == symbol_table->member_section_table[i].PointerToRelocations)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }
    }

    if (is_bss && (false == is_data)){

        *is_rename_bss2data = true;
    }

    return true;
}

static bool make_PE_file_buffer_section_count(
    TP_SYMBOL_TABLE* symbol_table,
    bool* is_copy_sections, rsize_t is_copy_sections_num, rsize_t* number_of_sections)
{
    // ----------------------------------------------------------------------------------------
    // Section Table(Count)
    rsize_t num = 0;

    for (rsize_t i = 0; is_copy_sections_num > i; ++i){

        uint8_t* Name = symbol_table->member_section_table[i].Name;

        if ( ! ((0 == strncmp(Name, ".text", 5)) ||
//          (0 == strncmp(Name, ".tls", 4)) ||
//          (0 == strncmp(Name, ".drectve", 8)) ||
            (0 == strncmp(Name, ".bss", 4)) ||
            (0 == strncmp(Name, ".data", 5)) ||
            (0 == strncmp(Name, ".rdata", 6)))){

            continue;
        }

        is_copy_sections[i] = true;

        ++num;
    }

    ++num; // Add .reloc section.

    *number_of_sections = num;

    return true;
}

static bool make_PE_file_buffer_section(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    bool* is_copy_sections, rsize_t is_copy_sections_num, bool is_rename_bss2data,
    uint8_t* entry_point_symbol, TP_PE_PARAM* param)
{
    // ----------------------------------------------------------------------------------------
    // Convert Section
    bool status = false;

    typedef struct TP_SECTION_DATA_{
        uint8_t* section_data;
        rsize_t section_data_size;
        bool is_create;
    }TP_SECTION_DATA;

    TP_SECTION_DATA* ref_section_data = NULL;
    TP_PE_BASE_RELOCATION* pe_base_relocation = NULL;
    uint8_t* section_data = NULL;

    rsize_t header_size = param->header_size + TP_PE_PADDING_FILE_ALIGNMENT(param->header_size);
    rsize_t file_size = header_size;
    rsize_t page_size = param->header_size + TP_PE_PADDING_SECTION_ALIGNMENT(param->header_size);

    uint32_t size_of_code = 0;
    uint32_t size_of_initialized_data = 0;
    uint32_t size_of_uninitialized_data = 0;

    TP_SECTION_TABLE* section_to = param->section_to;
    rsize_t number_of_sections = param->number_of_sections;

    rsize_t ref_section_data_size = number_of_sections * sizeof(TP_SECTION_DATA);

    ref_section_data = (TP_SECTION_DATA*)TP_CALLOC(symbol_table, number_of_sections, sizeof(TP_SECTION_DATA));

    if (NULL == ref_section_data){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    rsize_t j = 0;

    // Section Table
    for (rsize_t i = 0; is_copy_sections_num > i; ++i){

        if (false == is_copy_sections[i]){

            continue;
        }

        if (number_of_sections <= j){

            goto fail;
        }

        section_to[j] = symbol_table->member_section_table[i];

        section_to[j].PointerToLinenumbers = 0;
        section_to[j].NumberOfLinenumbers = 0;

        uint8_t* Name = section_to[j].Name;
        uint32_t Characteristics = section_to[j].Characteristics;
        uint32_t size_of_raw_data = section_to[j].SizeOfRawData;

        if ((is_rename_bss2data) && (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA)){

            uint8_t* zero_fill_buffer = (uint8_t*)TP_CALLOC(
                symbol_table, section_to[j].SizeOfRawData, sizeof(uint8_t)
            );

            if (NULL == zero_fill_buffer){

                TP_PRINT_CRT_ERROR(symbol_table);

                goto fail;
            }

            ref_section_data[j].section_data = zero_fill_buffer;
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = true;

            section_to[j].PointerToRawData = (uint32_t)file_size;
            file_size += size_of_raw_data;
            file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }else if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

            ref_section_data[j].section_data = NULL;
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = false;

            section_to[j].PointerToRawData = 0;
//          file_size += size_of_raw_data;
//          file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }else{

            if ( ! tp_seek_PE_COFF_file(symbol_table, (long)(section_to[j].PointerToRawData), 0)){

                goto fail;
            }

            if (IS_EOF_PE_COFF(symbol_table)){

                goto fail;
            }

            ref_section_data[j].section_data = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = false;

            section_to[j].PointerToRawData = (uint32_t)file_size;
            file_size += size_of_raw_data;
            file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }

        section_to[j].VirtualAddress = (uint32_t)page_size;
        section_to[j].VirtualSize = size_of_raw_data;

        page_size += size_of_raw_data;
        page_size += TP_PE_PADDING_SECTION_ALIGNMENT(size_of_raw_data);

        if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

            size_of_code += size_of_raw_data;
            memcpy(Name, ".text\0\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);

            // COFF Symbol Table(entry point)
            if ( ! make_PE_file_buffer_section_symbol_entry_point(
                symbol_table, write_file, (int16_t)i, j, entry_point_symbol, param)){

                goto fail;
            }
        }else if (Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA){

            size_of_initialized_data += size_of_raw_data;

            if (0 == strncmp(Name, ".data", 5)){

                memcpy(Name, ".data\0\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);
            }

            if (0 == strncmp(Name, ".rdata", 6)){

                memcpy(Name, ".rdata\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);
            }
        }else if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

            if (is_rename_bss2data){

                section_to[j].Characteristics ^= TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA;
                section_to[j].Characteristics |= TP_IMAGE_SCN_CNT_INITIALIZED_DATA;
                size_of_initialized_data += size_of_raw_data;
                memcpy(Name, ".data\0\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);
            }else{

                size_of_uninitialized_data += size_of_raw_data;
                memcpy(Name, ".bss\0\0\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);
            }
        }

        if (Characteristics & TP_IMAGE_SCN_LNK_COMDAT){

            // COFF Symbol Table(Auxiliary Format 5: Section Definitions)
            TP_AUX_SYMBOL_RECORD_SECTION aux_section = { 0 };

            if ( ! make_PE_file_buffer_section_symbol_comdat(
                symbol_table, write_file, (int16_t)i, param, &aux_section)){

                goto fail;
            }

            section_to[j].Characteristics ^= TP_IMAGE_SCN_LNK_COMDAT;
        }

        bool is_reloc_count_ex = false;

        if (Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

            is_reloc_count_ex = true;
        }

        // Section Data(Relocation)
        if ( ! make_PE_file_buffer_section_copy_relocation_data(
            symbol_table, is_reloc_count_ex, j, &(section_to[j]))){

            goto fail;
        }

        ++j;
    }

    if (number_of_sections <= j){

        goto fail;
    }

    // Based relocation(.reloc)
    rsize_t pe_base_relocation_size = sizeof(TP_PE_BASE_RELOCATION) + sizeof(uint16_t);

    pe_base_relocation = (TP_PE_BASE_RELOCATION*)TP_CALLOC(symbol_table, 1, pe_base_relocation_size);

    if (NULL == pe_base_relocation){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    pe_base_relocation->VirtualAddress = 0;
    pe_base_relocation->SizeOfBlock = (uint32_t)pe_base_relocation_size;
    pe_base_relocation->TypeOffset[0] =
        TP_IMAGE_REL_BASED_TYPE(TP_IMAGE_REL_BASED_ABSOLUTE) | TP_IMAGE_REL_BASED_OFFSET(0);

    memcpy(section_to[j].Name, ".reloc\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);

    section_to[j].PointerToRawData = (uint32_t)file_size;
    rsize_t reloc_size =
        sizeof(TP_PE_BASE_RELOCATION) + TP_PE_PADDING_FILE_ALIGNMENT(sizeof(TP_PE_BASE_RELOCATION));
    file_size += reloc_size;

    section_to[j].SizeOfRawData = (uint32_t)pe_base_relocation_size;
    section_to[j].VirtualSize = (uint32_t)pe_base_relocation_size;
    param->base_reloc_size = (uint32_t)pe_base_relocation_size;

    section_to[j].VirtualAddress = (uint32_t)page_size;
    param->base_reloc_virtual_address = (uint32_t)page_size;
    page_size += pe_base_relocation_size;
    page_size += TP_PE_PADDING_SECTION_ALIGNMENT(pe_base_relocation_size);

    section_to[j].PointerToRelocations = 0;
    section_to[j].PointerToLinenumbers = 0;
    section_to[j].NumberOfRelocations = 0;
    section_to[j].NumberOfLinenumbers = 0;
    section_to[j].Characteristics = 0;

    ref_section_data[j].section_data = (uint8_t*)pe_base_relocation;
    pe_base_relocation = NULL;
    ref_section_data[j].section_data_size = pe_base_relocation_size;
    ref_section_data[j].is_create = true;

    // PE Image Optional header
    param->size_of_code = size_of_code;
    param->size_of_initialized_data = size_of_initialized_data;
    param->size_of_uninitialized_data = size_of_uninitialized_data;

    // Section Data
    param->section_data = NULL;
    file_size -= header_size;
    param->section_data_size = file_size;
    param->section_data_virtual_size = page_size;

    rsize_t section_data_size = 0;

    for (rsize_t i = 0; number_of_sections > i; ++i){

        if (ref_section_data[i].section_data){

            section_data_size +=
                (ref_section_data[i].section_data_size +
                    TP_PE_PADDING_FILE_ALIGNMENT(ref_section_data[i].section_data_size));
        }
    }

    if (file_size != section_data_size){

        goto fail;
    }

    section_data = (uint8_t*)TP_CALLOC(symbol_table, file_size, sizeof(uint8_t));

    if (NULL == section_data){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    for (rsize_t i = 0, offset = 0; number_of_sections > i; ++i){

        if (ref_section_data[i].section_data){

            memcpy(
                section_data + offset,
                ref_section_data[i].section_data,
                ref_section_data[i].section_data_size
            );

            offset +=
                (ref_section_data[i].section_data_size +
                    TP_PE_PADDING_FILE_ALIGNMENT(ref_section_data[i].section_data_size));
        }
    }

    param->section_data = section_data;

    status = true;

fail:
    if (false == status){

        if (section_data){

            SecureZeroMemory(section_data, file_size);
            free(section_data);
            section_data = NULL;
        }
    }

    if (ref_section_data){

        for (rsize_t i = 0; number_of_sections > i; ++i){

            if (ref_section_data[i].is_create && ref_section_data[i].section_data){

                SecureZeroMemory(ref_section_data[i].section_data, ref_section_data[i].section_data_size);
                free(ref_section_data[i].section_data);
                ref_section_data[i].section_data = NULL;
            }
        }

        SecureZeroMemory(ref_section_data, ref_section_data_size);
        free(ref_section_data);
        ref_section_data = NULL;
    }

    return status;
}

static bool make_PE_file_buffer_section_symbol_entry_point(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    int16_t section_number, rsize_t new_section_index, uint8_t* entry_point_symbol, TP_PE_PARAM* param)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table(entry point)
    if (NULL == entry_point_symbol){

        return true;
    }

    if (TP_IMAGE_SIZEOF_SHORT_NAME < strlen(entry_point_symbol)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if (param->number_of_sections <= new_section_index){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    ++section_number;

    TP_COFF_SYMBOL_TABLE* symbol = symbol_table->member_coff_symbol_table;

    if (NULL == symbol){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    rsize_t num = symbol_table->member_coff_file_header->NumberOfSymbols;

    for (rsize_t i = 0; num > i; ++i){

        if (section_number != symbol[i].SectionNumber){

            continue;
        }

        uint8_t* ShortName = symbol[i].UNION.ShortName;

        if (0 != strncmp(ShortName, entry_point_symbol, TP_IMAGE_SIZEOF_SHORT_NAME)){

            continue;
        }

        // Section Table
        if (symbol_table->member_section_table_num < section_number){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        TP_SECTION_TABLE* ref_section = &(symbol_table->member_section_table[section_number - 1]);

        if (ref_section->Characteristics & TP_IMAGE_SCN_CNT_CODE){

            // Type Representation
            // Storage Class
            uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(&(symbol[i]));
            uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(&(symbol[i]));

            if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) && (TP_IMAGE_SYM_DTYPE_FUNCTION == type_msb) &&
                (TP_IMAGE_SYM_CLASS_EXTERNAL == symbol[i].StorageClass)){

                // Section Table
                if (ref_section->SizeOfRawData <= symbol[i].Value){

                    TP_PUT_LOG_MSG_ILE(symbol_table);

                    return false;
                }

                param->address_of_entry_point =
                    param->section_to[new_section_index].VirtualAddress + symbol[i].Value;

                param->base_of_code =
                    param->section_to[new_section_index].VirtualAddress;

                return true;
            }
        }
    }

    TP_PUT_LOG_MSG_ILE(symbol_table);

    return false;
}

static bool make_PE_file_buffer_section_symbol_comdat(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    int16_t section_number, TP_PE_PARAM* param, TP_AUX_SYMBOL_RECORD_SECTION* aux_section)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table(Auxiliary Format 5: Section Definitions)
    ++section_number;

    TP_COFF_SYMBOL_TABLE* symbol = symbol_table->member_coff_symbol_table;

    if (NULL == symbol){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    rsize_t num = symbol_table->member_coff_file_header->NumberOfSymbols;

    for (rsize_t i = 0; num > i; ++i){

        if (section_number != symbol[i].SectionNumber){

            continue;
        }

        // Section Table
        if (symbol_table->member_section_table_num <= section_number){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        TP_SECTION_TABLE* ref_section = &(symbol_table->member_section_table[section_number - 1]);

        if (0 != strncmp(symbol[i].UNION.ShortName, ref_section->Name, TP_IMAGE_SIZEOF_SHORT_NAME)){

            continue;
        }

        if ((TP_IMAGE_SYM_CLASS_STATIC == symbol[i].StorageClass)){

            if (0 == symbol[i].NumberOfAuxSymbols){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            // COMDAT Section
            *aux_section = *(TP_AUX_SYMBOL_RECORD_SECTION*)(symbol[i].AuxiliarySymbolRecords);

            return true;
        }

        i += symbol[i].NumberOfAuxSymbols;
    }

    TP_PUT_LOG_MSG_ILE(symbol_table);

    return false;
}

static bool make_PE_file_buffer_section_copy_relocation_data(
    TP_SYMBOL_TABLE* symbol_table,
    bool is_reloc_count_ex, rsize_t new_section_index, TP_SECTION_TABLE* section_to)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(Relocation)
    if (0 == section_to->PointerToRelocations){

        return true;
    }

    if (symbol_table->member_coff_relocations_size <= new_section_index){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    if ( ! tp_seek_PE_COFF_file(symbol_table, (long)(section_to->PointerToRelocations), 0)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    if (IS_EOF_PE_COFF(symbol_table)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    uint32_t num = section_to->NumberOfRelocations;

    TP_COFF_RELOCATIONS* ref_reloc = (TP_COFF_RELOCATIONS*)TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);

    if (is_reloc_count_ex){

        num = ref_reloc->UNION.RelocCount;

        ++num;
    }

    symbol_table->member_coff_relocations[new_section_index].member_num = num;

    rsize_t size = num * sizeof(TP_COFF_RELOCATIONS);
    symbol_table->member_coff_relocations[new_section_index].member_size = size;

    TP_COFF_RELOCATIONS* relocations = (TP_COFF_RELOCATIONS*)TP_CALLOC(
        symbol_table, num, sizeof(TP_COFF_RELOCATIONS)
    );

    if (NULL == relocations){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    memcpy(relocations, ref_reloc, size);

    symbol_table->member_coff_relocations[new_section_index].member_relocations = relocations;

    return true;
}

static bool make_PE_file_buffer_section_relocation(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section_to, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size)
{
    // ----------------------------------------------------------------------------------------
    // Relocation(COFF)
    bool is_print = symbol_table->member_is_output_log_file;

    for (rsize_t i = 0; number_of_sections > i; ++i){

        uint32_t Characteristics = section_to[i].Characteristics;

        bool is_reloc_count_ex = false;

        if (Characteristics & TP_IMAGE_SCN_LNK_NRELOC_OVFL){

            is_reloc_count_ex = true;
        }

        if (0 == section_to[i].PointerToRelocations){

            section_to[i].NumberOfRelocations = 0;

            continue;
        }

        if (is_print){

            fprintf(write_file,
                "Section #%zu (Relocations):\n\n", i + 1
            ); 
        }

        TP_COFF_RELOCATIONS_ARRAY* coff_relocations = &(symbol_table->member_coff_relocations[i]);

        if ( ! make_PE_file_buffer_section_data_relocation(
            symbol_table, write_file, &(section_to[i]), section_to, number_of_sections,
            pe_image_buffer, pe_image_buffer_size, coff_relocations, is_reloc_count_ex)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        section_to[i].PointerToRelocations = 0;
        section_to[i].NumberOfRelocations = 0;
    }

    return true;
}

static bool make_PE_file_buffer_section_data_relocation(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section_to,
    TP_SECTION_TABLE* section, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size,
    TP_COFF_RELOCATIONS_ARRAY* coff_relocations, bool is_reloc_count_ex)
{
    // ----------------------------------------------------------------------------------------
    // Section Data(Relocation)
    rsize_t num = coff_relocations->member_num;

    if (is_reloc_count_ex){

        num = coff_relocations->member_relocations->UNION.RelocCount;

        ++num;
    }

    rsize_t symbols = symbol_table->member_coff_file_header->NumberOfSymbols;

    for (rsize_t i = (is_reloc_count_ex ? 1 : 0); num > i; ++i){

        // Type Indicators
        switch (coff_relocations->member_relocations[i].Type){
        case TP_IMAGE_REL_AMD64_ABSOLUTE:
            break;
        case TP_IMAGE_REL_AMD64_REL32:{

            for (rsize_t j = 0; symbols > j; ++j){

                if (coff_relocations->member_relocations[i].SymbolTableIndex == j){

                    if ( ! make_PE_file_buffer_section_data_relocation_symbol(
                        symbol_table, write_file, section_to,
                        section, number_of_sections,
                        pe_image_buffer, pe_image_buffer_size,
                        coff_relocations->member_relocations[i].UNION.VirtualAddress, j)){

                        TP_PUT_LOG_MSG_TRACE(symbol_table);

                        return false;
                    }

                    break;
                }

                j += symbol_table->member_coff_symbol_table[j].NumberOfAuxSymbols;
            }

            break;
        }
        default:
            TP_PUT_LOG_MSG_ILE(symbol_table);
            return false;
        }
    }

    return true;
}

static bool make_PE_file_buffer_section_data_relocation_symbol(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file, TP_SECTION_TABLE* section_to,
    TP_SECTION_TABLE* section, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size,
    uint32_t virtual_address, rsize_t symbol_index)
{
    // ----------------------------------------------------------------------------------------
    // COFF Symbol Table(Relocation)
    bool is_print = symbol_table->member_is_output_log_file;

   TP_COFF_SYMBOL_TABLE* symbol = symbol_table->member_coff_symbol_table;

    if (NULL == symbol){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    symbol += symbol_index;

    // Section Number Values
    if (0 >= symbol->SectionNumber){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // Type Representation
    // Storage Class
    uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(symbol);
    uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(symbol);

    if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) && (TP_IMAGE_SYM_DTYPE_NULL == type_msb) &&
        ((TP_IMAGE_SYM_CLASS_STATIC == symbol->StorageClass) ||
        (TP_IMAGE_SYM_CLASS_EXTERNAL == symbol->StorageClass))){

        if (is_print){

            fprintf(write_file,
                "TP_COFF_SYMBOL_TABLE(%zx):\n"
                "    uint8_t ShortName(%.*s);\n"
                "    uint32_t Zeroes(%x);\n"
                "    uint32_t Offset(%x);\n"
                "    uint8_t LongName(%s);\n"
                "    uint32_t Value(%x);\n"
                "    int16_t SectionNumber(%x);\n"
                "    uint16_t Type(%x);\n"
                "    uint8_t StorageClass(%x);\n"
                "    uint8_t NumberOfAuxSymbols(%x);\n\n",
                symbol_index,
                TP_IMAGE_SIZEOF_SHORT_NAME,
                symbol->UNION.ShortName,
                symbol->UNION.Name.Zeroes, // if 0, use LongName
                symbol->UNION.Name.Offset, // An offset into the string table.
                (0 == symbol->UNION.Name.Zeroes) ?
                    (symbol_table->member_string_table + symbol->UNION.Name.Offset) : "",
                symbol->Value,
                symbol->SectionNumber,
                symbol->Type,
                symbol->StorageClass,
                symbol->NumberOfAuxSymbols
            ); 
        }

        // Section Table
        if ((0 == section_to->PointerToRawData) ||
            (symbol_table->member_section_table_num < symbol->SectionNumber)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        if (section_to->SizeOfRawData <= virtual_address){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        TP_SECTION_TABLE* ref_section = &(symbol_table->member_section_table[symbol->SectionNumber - 1]);

        if (ref_section->SizeOfRawData <= symbol->Value){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        rsize_t i = 0;
        bool is_match = false;

        for (; number_of_sections > i; ++i){

            uint8_t* Name = section[i].Name;

            uint32_t Characteristics = section[i].Characteristics;

            if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

                if ((0 == strncmp(Name, ".text", 5)) &&
                    (0 == strncmp(ref_section->Name, ".text", 5))){

                    is_match = true;

                    break;
                }
            }else if (Characteristics & TP_IMAGE_SCN_CNT_INITIALIZED_DATA){

                if ((0 == strncmp(Name, ".data", 5)) &&
                    ((0 == strncmp(ref_section->Name, ".bss", 4)) ||
                    (0 == strncmp(ref_section->Name, ".data", 5)))){

                    is_match = true;

                    break;
                }

                if ((0 == strncmp(Name, ".rdata", 6)) &&
                    (0 == strncmp(ref_section->Name, ".rdata", 6))){

                    is_match = true;

                    break;
                }
            }else if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

                if ((0 == strncmp(Name, ".bss", 4)) &&
                    (0 == strncmp(ref_section->Name, ".bss", 4))){

                    is_match = true;

                    break;
                }
            }
        }

        if (false == is_match){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        // Relocation(COFF)
        uint32_t pe_image_offset = section_to->PointerToRawData + virtual_address;
        uint8_t* apply_to = pe_image_buffer + pe_image_offset;
        int32_t apply_from = (int32_t)(section[i].VirtualAddress + symbol->Value);

        if (section_to->Characteristics & TP_IMAGE_SCN_CNT_CODE){

            int32_t rip = (int32_t)(section_to->VirtualAddress + virtual_address + sizeof(uint32_t));

            apply_from -= rip;
        }

        if (pe_image_buffer_size <= (pe_image_offset + sizeof(apply_from))){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        memcpy(apply_to, &apply_from, sizeof(apply_from));

        return true;
    }

    TP_PUT_LOG_MSG_ILE(symbol_table);

    return false;
}

