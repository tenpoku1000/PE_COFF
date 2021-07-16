
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

typedef struct TP_SECTION_DATA_{
    uint8_t* section_data;
    rsize_t section_data_size;
    bool is_create;
    bool is_text;
}TP_SECTION_DATA;

typedef struct TP_PE_PARAM_{
// PE Image header
    rsize_t header_size;
// PE Image Optional header
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    // Export tables(PE File Data Directory)
    uint32_t export_virtual_address;
    uint32_t export_size;
    // Based relocation(PE File Data Directory)
    uint32_t base_reloc_virtual_address;
    uint32_t base_reloc_size;
// Section Table
    TP_SECTION_TABLE* section_to;
    rsize_t number_of_sections;
    // Export tables
    bool is_edata_section;
// Section Data
    uint8_t* section_data;
    rsize_t section_data_size;
    rsize_t section_data_virtual_size;
    TP_SECTION_DATA* ref_section_data;
    // Export tables
    rsize_t pe_export_size;
    TP_PE_EXPORT* pe_export;
}TP_PE_PARAM;

static bool make_PE_file_buffer_section_validation(
    TP_SYMBOL_TABLE* symbol_table, bool* is_rename_bss2data
);
static bool make_PE_file_convert_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, bool is_edata_count, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_count(
    TP_SYMBOL_TABLE* symbol_table,
    bool* is_copy_sections, rsize_t is_copy_sections_num, rsize_t* number_of_sections
);
static bool make_PE_file_buffer_section(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    rsize_t header_size, rsize_t file_size, rsize_t page_size,
    bool* is_copy_sections, rsize_t is_copy_sections_num, bool is_rename_bss2data,
    uint8_t* entry_point_symbol, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_relocation(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t* file_size, rsize_t* page_size,
    TP_SECTION_TABLE* section_to, rsize_t j, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_export(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t* file_size, rsize_t* page_size,
    TP_SECTION_TABLE* section_to, rsize_t j, TP_PE_PARAM* param
);
static bool make_PE_file_buffer_section_data(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t header_size, rsize_t file_size, rsize_t page_size, TP_PE_PARAM* param
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
static bool make_PE_file_buffer_section_relocation_data(
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
    rsize_t pe_image_buffer_total_size = 0;

    TP_PE_PARAM param = { 0 };

    bool* is_copy_sections = NULL;
    rsize_t is_copy_sections_size = 0;

    TP_SECTION_TABLE* section_table = NULL;
    rsize_t section_table_size = 0;
    rsize_t number_of_sections = 0;
    TP_SECTION_DATA* ref_section_data = NULL;

    // ----------------------------------------------------------------------------------------
    // Section Table
    bool is_rename_bss2data = false;

    if ( ! make_PE_file_buffer_section_validation(symbol_table, &is_rename_bss2data)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    rsize_t is_copy_sections_num = symbol_table->member_section_table_num;

    if (0 == is_copy_sections_num){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        goto fail;
    }

    is_copy_sections_size = is_copy_sections_num * sizeof(bool);

    is_copy_sections = (bool*)TP_CALLOC(symbol_table, is_copy_sections_num, sizeof(bool));

    if (NULL == is_copy_sections){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    // Convert symbol table(.edata section)
    bool is_edata_count = true;

    if ( ! make_PE_file_convert_symbol_table(symbol_table, is_edata_count, &param)){ 

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if ( ! make_PE_file_buffer_section_count(
        symbol_table, is_copy_sections, is_copy_sections_num, &number_of_sections)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    if (param.is_edata_section){ ++number_of_sections; }

    section_table_size = number_of_sections * sizeof(TP_SECTION_TABLE);

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
        // Export tables(PE File Data Directory)
        .export_virtual_address = 0,
        .export_size = 0,
        // Based relocation(PE File Data Directory)
        .base_reloc_virtual_address = 0,
        .base_reloc_size = 0,
    // Section Table
        .section_to = section_table,
        .number_of_sections = number_of_sections,
        // Export tables
        .is_edata_section = false,
    // Section Data
        .section_data = NULL,
        .section_data_size = 0,
        .section_data_virtual_size = 0,
        .ref_section_data = NULL,
        // Export tables
        .pe_export_size = 0,
        .pe_export = NULL
    };

    param = tmp_param;

    rsize_t ref_section_data_size = number_of_sections * sizeof(TP_SECTION_DATA);

    ref_section_data = (TP_SECTION_DATA*)TP_CALLOC(symbol_table, number_of_sections, sizeof(TP_SECTION_DATA));

    if (NULL == ref_section_data){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    param.ref_section_data = ref_section_data;

    // Convert Section
    rsize_t header_size = param.header_size + TP_PE_PADDING_FILE_ALIGNMENT(param.header_size);
    rsize_t file_size = header_size;
    rsize_t page_size = param.header_size + TP_PE_PADDING_SECTION_ALIGNMENT(param.header_size);

    if ( ! make_PE_file_buffer_section(
        symbol_table, write_file, header_size, file_size, page_size,
        is_copy_sections, is_copy_sections_num, is_rename_bss2data, entry_point_symbol, &param)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        goto fail;
    }

    // ----------------------------------------------------------------------------------------
    // PE Image
    rsize_t size_of_headers = param.header_size + TP_PE_PADDING_FILE_ALIGNMENT(param.header_size);
    rsize_t pe_image_buffer_size = size_of_headers + param.section_data_size;
    pe_image_buffer_total_size = pe_image_buffer_size;

    pe_image_buffer = (uint8_t*)TP_CALLOC(symbol_table, pe_image_buffer_total_size, sizeof(uint8_t));

    if (NULL == pe_image_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        goto fail;
    }

    // Section Data
    memcpy(pe_image_buffer + size_of_headers, param.section_data, param.section_data_size);

    // Relocation(COFF)
    if ( ! make_PE_file_buffer_section_relocation_data(
        symbol_table, write_file, 
        section_table, number_of_sections, pe_image_buffer, pe_image_buffer_size)){

        goto fail;
    }

    TP_FREE(symbol_table, &(symbol_table->member_section_table), symbol_table->member_section_table_size);
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

    switch (symbol_table->member_subsystem){
    case TP_IMAGE_SUBSYSTEM_WINDOWS_GUI:
        pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_WINDOWS_GUI;
        break;
    case TP_IMAGE_SUBSYSTEM_WINDOWS_CUI:
        pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_WINDOWS_CUI;
        break;
    case TP_IMAGE_SUBSYSTEM_EFI_APPLICATION:
        pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_EFI_APPLICATION;
        break;
    case TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
        pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER;
        break;
    case TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        pe_header64_write->OptionalHeader.Subsystem = TP_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        goto fail;
    }

    pe_header64_write->OptionalHeader.DllCharacteristics = 0;
    pe_header64_write->OptionalHeader.SizeOfStackReserve = 0x100000;
    pe_header64_write->OptionalHeader.SizeOfStackCommit = 0x1000;
    pe_header64_write->OptionalHeader.SizeOfHeapReserve = 0x100000;
    pe_header64_write->OptionalHeader.SizeOfHeapCommit = 0x1000;
    pe_header64_write->OptionalHeader.LoaderFlags = 0;
    pe_header64_write->OptionalHeader.NumberOfRvaAndSizes = TP_PE_HEADER64_WRITE_DIRECTORY_ENTRY_NUM;

    // Export tables(PE File Data Directory)
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress =
        param.export_virtual_address;
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_EXPORT].Size =
        param.export_size;

    // Based relocation(PE File Data Directory)
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress =
        param.base_reloc_virtual_address;
    pe_header64_write->OptionalHeader.DataDirectory[TP_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size =
        param.base_reloc_size;

    // ----------------------------------------------------------------------------------------
    // PE Image
    if (symbol_table->member_pe_coff_buffer){

        TP_FREE(symbol_table, &(symbol_table->member_pe_coff_buffer), symbol_table->member_pe_coff_size);
    }

    symbol_table->member_pe_coff_buffer = pe_image_buffer;
    symbol_table->member_pe_coff_size = pe_image_buffer_total_size;
    symbol_table->member_pe_coff_current_offset = 0;

    status = true;

fail:
    if (false == status){

        TP_FREE(symbol_table, &pe_image_buffer, pe_image_buffer_total_size);

        TP_FREE(symbol_table, &section_table, section_table_size);

        symbol_table->member_section_table_num = number_of_sections;
    }

    TP_FREE(symbol_table, &is_copy_sections, is_copy_sections_size);

    TP_FREE(symbol_table, &(param.section_data), param.section_data_size);

    if (ref_section_data){

        for (rsize_t i = 0; number_of_sections > i; ++i){

            if (ref_section_data[i].is_create && ref_section_data[i].section_data){

                TP_FREE(
                    symbol_table, &(ref_section_data[i].section_data),
                    ref_section_data[i].section_data_size
                );
            }
        }

        TP_FREE(symbol_table, &ref_section_data, ref_section_data_size);
    }

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

typedef struct TP_COMPARE_SYMBOL_{
    TP_COFF_SYMBOL_TABLE* coff_symbol_table; // NOTE: member_coff_symbol_table must not free memory.
    uint8_t* string_table;                   // NOTE: member_string_table must not free memory.
    uint32_t export_address;                 // RVA
    uint8_t* symbol_name;
    rsize_t symbol_name_size;
    uint16_t ordinal;
}TP_COMPARE_SYMBOL;

static uint8_t* get_symbol_name(TP_COMPARE_SYMBOL* symbol)
{
    uint8_t* symbol_name = NULL;

    uint8_t short_name[TP_IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };

    if (0 == symbol->coff_symbol_table->UNION.Name.Zeroes){

        // COFF String Table
        rsize_t offset = symbol->coff_symbol_table->UNION.Name.Offset;

        symbol_name = symbol->string_table + offset;
    }else{

        memcpy(short_name, symbol->coff_symbol_table->UNION.ShortName, TP_IMAGE_SIZEOF_SHORT_NAME);

        symbol_name = short_name;
    }

    return strdup(symbol_name);
}

static int compare_symbol_table(const void* arg1, const void* arg2)
{
    uint8_t* symbol_name1 = get_symbol_name((TP_COMPARE_SYMBOL*)arg1);
    uint8_t* symbol_name2 = get_symbol_name((TP_COMPARE_SYMBOL*)arg2);

    int v = strcmp(symbol_name1, symbol_name2);

    TP_FREE(NULL, &symbol_name1, strlen(symbol_name1));
    TP_FREE(NULL, &symbol_name2, strlen(symbol_name2));

    return v;
}

static bool make_PE_file_convert_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, bool is_edata_count, TP_PE_PARAM* param)
{
    // ----------------------------------------------------------------------------------------
    // Convert symbol table(.edata section)
    bool status = false;

    uint32_t num_eat_names = 0;
    TP_COMPARE_SYMBOL* eat_names = NULL;

    if (TP_IMAGE_SUBSYSTEM_WINDOWS_GUI != symbol_table->member_subsystem){

        return true;
    }

    // COFF Symbol Table
    rsize_t num = symbol_table->member_coff_file_header->NumberOfSymbols;

    for (rsize_t i = 0; num > i; ++i){

        TP_COFF_SYMBOL_TABLE* coff_symbol_table = symbol_table->member_coff_symbol_table + i;

        if (TP_IMAGE_SYM_CLASS_EXTERNAL != coff_symbol_table->StorageClass){

            continue;
        }

        param->is_edata_section = true;

        ++num_eat_names;

        TP_COMPARE_SYMBOL* tmp_eat_names = (TP_COMPARE_SYMBOL*)
            TP_REALLOC(symbol_table, eat_names, sizeof(TP_COMPARE_SYMBOL) * num_eat_names);

        if (NULL == tmp_eat_names){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        eat_names = tmp_eat_names;

        memset(&(eat_names[num_eat_names - 1]), 0, sizeof(TP_COMPARE_SYMBOL));
        eat_names[num_eat_names - 1].coff_symbol_table = coff_symbol_table;
        eat_names[num_eat_names - 1].string_table = symbol_table->member_string_table;

        if (0 == coff_symbol_table->UNION.Name.Zeroes){

            // COFF String Table
            uint32_t size = symbol_table->member_string_table_size;

            if (0 == size){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                goto fail;
            }

            uint32_t offset = coff_symbol_table->UNION.Name.Offset;

            if (size <= offset){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                goto fail;
            }
        }
    }

    if (is_edata_count || (0 == num_eat_names)){

        goto success;
    }

    num = num_eat_names;

    qsort(eat_names, num, sizeof(TP_COMPARE_SYMBOL), compare_symbol_table);

    rsize_t symbol_name_size = 0;
    rsize_t ordinal_size = 0;
    rsize_t export_address_size = 0;

    for (rsize_t i = 0; num > i; ++i){

        TP_COMPARE_SYMBOL* symbol = eat_names + i;

        // Symbol
        symbol->symbol_name = NULL;
        symbol->symbol_name_size = 0;

        uint8_t* symbol_name = NULL;
        uint8_t short_name[TP_IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };

        if (0 == symbol->coff_symbol_table->UNION.Name.Zeroes){

            // COFF String Table
            uint32_t size = symbol_table->member_string_table_size;

            if (0 == size){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                goto fail;
            }

            rsize_t offset = symbol->coff_symbol_table->UNION.Name.Offset;

            if (size <= offset){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                goto fail;
            }

            symbol_name = symbol->string_table + offset;
        }else{

            memcpy(short_name, symbol->coff_symbol_table->UNION.ShortName, TP_IMAGE_SIZEOF_SHORT_NAME);

            symbol_name = short_name;
        }

        symbol->symbol_name = strdup(symbol_name);
        symbol->symbol_name_size = strlen(symbol_name) + 1;
        symbol_name_size += symbol->symbol_name_size;

        // Ordinal
        symbol->ordinal = (uint16_t)i;
        ordinal_size += sizeof(symbol->ordinal);

        // Export Address
        int16_t SectionNumber = symbol->coff_symbol_table->SectionNumber;

        if ((0 >= SectionNumber) || (param->number_of_sections < SectionNumber)){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            goto fail;
        }

        symbol->export_address = 
            param->section_to[SectionNumber - 1].VirtualAddress +
                symbol->coff_symbol_table->Value;

        export_address_size += sizeof(symbol->export_address);
    }

    // DllName
    uint8_t dll_name[_MAX_PATH] = { 0 };
    rsize_t dll_name_size = 0;

    char fname[_MAX_FNAME] = { 0 };
    char ext[_MAX_EXT] = { 0 };

    errno_t err = _splitpath_s(
        symbol_table->member_pe_file_path, NULL, 0, NULL, 0, fname, _MAX_FNAME, ext, _MAX_EXT
    );

    if (err){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        goto fail;
    }else{

        if (ext[0]){

            sprintf_s(dll_name, sizeof(dll_name), "%s%s", fname, ext);

            dll_name_size = strlen(dll_name) + 1; 
        }else{

            TP_PUT_LOG_MSG_ILE(symbol_table);

            goto fail;
        }
    }

    if (param->is_edata_section){

        rsize_t pe_export_size =
            sizeof(TP_PE_EXPORT) + dll_name_size +
                export_address_size + ordinal_size +
                symbol_name_size + (sizeof(uint32_t) * num_eat_names);

        TP_PE_EXPORT* pe_export =
            (TP_PE_EXPORT*)TP_CALLOC(symbol_table, 1, pe_export_size);

        if (NULL == pe_export){

            TP_PRINT_CRT_ERROR(symbol_table);

            goto fail;
        }

        param->pe_export_size = pe_export_size;
        param->pe_export = pe_export;

        pe_export->ExportFlags = 0;
        pe_export->TimeDateStamp = 0xffffffff;
        pe_export->MajorVersion = 0;
        pe_export->MinorVersion = 0;

        // DllName
        uint8_t* write_buffer = (uint8_t*)pe_export;
        rsize_t offset = sizeof(TP_PE_EXPORT);
        memcpy(write_buffer + offset, dll_name, dll_name_size);
        pe_export->DllName = param->export_virtual_address + (uint32_t)offset;
        offset += dll_name_size;

        pe_export->OrdinalBase = 1;
        pe_export->NumberOfExportAddressTable = num_eat_names;
        pe_export->NumberOfNames = num_eat_names;

        // ExportAddressTable
        pe_export->ExportAddressTable = param->export_virtual_address + (uint32_t)offset;

        for (rsize_t i = 0; num_eat_names > i; ++i){

            memcpy(write_buffer + offset, &(eat_names[i].export_address), sizeof(eat_names[i].export_address));
            offset += sizeof(eat_names[i].export_address);
        }

        // OrdinalTable
        pe_export->OrdinalTable = param->export_virtual_address + (uint32_t)offset;

        for (rsize_t i = 0; num_eat_names > i; ++i){

            memcpy(write_buffer + offset, &(eat_names[i].ordinal), sizeof(eat_names[i].ordinal));
            offset += sizeof(eat_names[i].ordinal);
        }

        // NamePointerTable
        rsize_t name_ptr_offset = offset + symbol_name_size;
        pe_export->NamePointerTable = param->export_virtual_address + (uint32_t)name_ptr_offset;

        for (rsize_t i = 0; num_eat_names > i; ++i){

            memcpy(write_buffer + offset, eat_names[i].symbol_name, eat_names[i].symbol_name_size);

            uint32_t name_ptr = param->export_virtual_address + (uint32_t)offset;
            memcpy(write_buffer + name_ptr_offset, &name_ptr, sizeof(name_ptr));

            offset += eat_names[i].symbol_name_size;
            name_ptr_offset += sizeof(name_ptr);
        }
    }

success:
    status = true;
fail:
    for (rsize_t i = 0; num_eat_names > i; ++i){

        TP_FREE(symbol_table, &(eat_names[i].symbol_name), eat_names[i].symbol_name_size);
    }
    TP_FREE(symbol_table, &eat_names, sizeof(TP_COMPARE_SYMBOL) * num_eat_names);
    return status;
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

        is_copy_sections[i] = true;

        ++num;
    }

    ++num; // Add .reloc section.

    *number_of_sections = num;

    return true;
}

static bool make_PE_file_buffer_section(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    rsize_t header_size, rsize_t file_size, rsize_t page_size,
    bool* is_copy_sections, rsize_t is_copy_sections_num, bool is_rename_bss2data,
    uint8_t* entry_point_symbol, TP_PE_PARAM* param)
{
    // ----------------------------------------------------------------------------------------
    // Convert Section
    uint32_t size_of_code = 0;
    uint32_t size_of_initialized_data = 0;
    uint32_t size_of_uninitialized_data = 0;

    TP_SECTION_TABLE* section_to = param->section_to;
    rsize_t number_of_sections = param->number_of_sections;

    TP_SECTION_DATA* ref_section_data = param->ref_section_data;

    rsize_t j = 0;

    // Section Table
    for (rsize_t i = 0; is_copy_sections_num > i; ++i){

        if (false == is_copy_sections[i]){

            continue;
        }

        if (number_of_sections <= j){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        section_to[j] = symbol_table->member_section_table[i];

        section_to[j].PointerToLinenumbers = 0;
        section_to[j].NumberOfLinenumbers = 0;

        uint8_t* Name = section_to[j].Name;

        if ('/' == Name[0]){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

        uint32_t Characteristics = section_to[j].Characteristics;
        uint32_t size_of_raw_data = section_to[j].SizeOfRawData;

        if ((is_rename_bss2data) && (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA)){

            uint8_t* zero_fill_buffer = (uint8_t*)TP_CALLOC(
                symbol_table, section_to[j].SizeOfRawData, sizeof(uint8_t)
            );

            if (NULL == zero_fill_buffer){

                TP_PRINT_CRT_ERROR(symbol_table);

                return false;
            }

            ref_section_data[j].section_data = zero_fill_buffer;
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = true;
            ref_section_data[j].is_text = false;

            section_to[j].PointerToRawData = (uint32_t)file_size;
            file_size += size_of_raw_data;
            file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }else if (Characteristics & TP_IMAGE_SCN_CNT_UNINITIALIZED_DATA){

            ref_section_data[j].section_data = NULL;
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = false;
            ref_section_data[j].is_text = false;

            section_to[j].PointerToRawData = 0;
//          file_size += size_of_raw_data;
//          file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }else{

            if ( ! tp_seek_PE_COFF_file(symbol_table, (long)(section_to[j].PointerToRawData), 0)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
            }

            if (IS_EOF_PE_COFF(symbol_table)){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            ref_section_data[j].section_data = TP_PE_COFF_GET_CURRENT_BUFFER(symbol_table);
            ref_section_data[j].section_data_size = size_of_raw_data;
            ref_section_data[j].is_create = false;
            ref_section_data[j].is_text = false;

            section_to[j].PointerToRawData = (uint32_t)file_size;
            file_size += size_of_raw_data;
            file_size += TP_PE_PADDING_FILE_ALIGNMENT(size_of_raw_data);
        }

        section_to[j].VirtualAddress = (uint32_t)page_size;
        section_to[j].VirtualSize = size_of_raw_data;

        page_size += size_of_raw_data;
        page_size += TP_PE_PADDING_SECTION_ALIGNMENT(size_of_raw_data);

        if (Characteristics & TP_IMAGE_SCN_CNT_CODE){

            ref_section_data[j].is_text = true;

            size_of_code += size_of_raw_data;
            memcpy(Name, ".text\0\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);

            // COFF Symbol Table(entry point)
            if ( ! make_PE_file_buffer_section_symbol_entry_point(
                symbol_table, write_file, (int16_t)i, j, entry_point_symbol, param)){

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
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

                TP_PUT_LOG_MSG_TRACE(symbol_table);

                return false;
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

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        ++j;
    }

    if (number_of_sections <= j){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    // Image relocations(.reloc)
    if ( ! make_PE_file_buffer_section_relocation(
        symbol_table, &file_size, &page_size, section_to, j, param)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    ++j;

    if (number_of_sections > j){

        // Convert symbol table(.edata section)
        bool is_edata_count = false;
        param->export_virtual_address = (uint32_t)page_size;

        if ( ! make_PE_file_convert_symbol_table(symbol_table, is_edata_count, param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        // Export tables(.edata)
        if ( ! make_PE_file_buffer_section_export(
            symbol_table, &file_size, &page_size, section_to, j, param)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
    }

    // Section Data
    if ( ! make_PE_file_buffer_section_data(
        symbol_table, header_size, file_size, page_size, param)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    // PE Image Optional header
    param->size_of_code = size_of_code;
    param->size_of_initialized_data = size_of_initialized_data;
    param->size_of_uninitialized_data = size_of_uninitialized_data;

    return true;
}

static bool make_PE_file_buffer_section_relocation(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t* file_size, rsize_t* page_size,
    TP_SECTION_TABLE* section_to, rsize_t index, TP_PE_PARAM* param)
{
    // Image relocations(.reloc)
    TP_SECTION_DATA* ref_section_data = param->ref_section_data;

    rsize_t pe_base_relocation_size = sizeof(TP_PE_BASE_RELOCATION) + sizeof(uint16_t);

    TP_PE_BASE_RELOCATION* pe_base_relocation =
        (TP_PE_BASE_RELOCATION*)TP_CALLOC(symbol_table, 1, pe_base_relocation_size);

    if (NULL == pe_base_relocation){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    pe_base_relocation->VirtualAddress = 0;
    pe_base_relocation->SizeOfBlock = (uint32_t)pe_base_relocation_size;
    pe_base_relocation->TypeOffset[0] =
        TP_IMAGE_REL_BASED_TYPE(TP_IMAGE_REL_BASED_ABSOLUTE) | TP_IMAGE_REL_BASED_OFFSET(0);

    memcpy(section_to[index].Name, ".reloc\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);

    section_to[index].PointerToRawData = (uint32_t)*file_size;
    rsize_t reloc_size =
        sizeof(TP_PE_BASE_RELOCATION) + TP_PE_PADDING_FILE_ALIGNMENT(sizeof(TP_PE_BASE_RELOCATION));
    *file_size += reloc_size;

    section_to[index].SizeOfRawData = (uint32_t)pe_base_relocation_size;
    section_to[index].VirtualSize = (uint32_t)pe_base_relocation_size;
    param->base_reloc_size = (uint32_t)pe_base_relocation_size;

    section_to[index].VirtualAddress = (uint32_t)*page_size;
    param->base_reloc_virtual_address = (uint32_t)*page_size;
    *page_size += pe_base_relocation_size;
    *page_size += TP_PE_PADDING_SECTION_ALIGNMENT(pe_base_relocation_size);

    section_to[index].PointerToRelocations = 0;
    section_to[index].PointerToLinenumbers = 0;
    section_to[index].NumberOfRelocations = 0;
    section_to[index].NumberOfLinenumbers = 0;
    section_to[index].Characteristics =
        TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ | TP_IMAGE_SCN_MEM_DISCARDABLE;

    ref_section_data[index].section_data = (uint8_t*)pe_base_relocation;
    pe_base_relocation = NULL;
    ref_section_data[index].section_data_size = pe_base_relocation_size;
    ref_section_data[index].is_create = true;

    return true;
}

static bool make_PE_file_buffer_section_export(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t* file_size, rsize_t* page_size,
    TP_SECTION_TABLE* section_to, rsize_t index, TP_PE_PARAM* param)
{
    // Export tables(.edata)
    TP_SECTION_DATA* ref_section_data = param->ref_section_data;

    rsize_t pe_export_size = param->pe_export_size;

    TP_PE_EXPORT* pe_export = param->pe_export;

    if ((0 == pe_export_size) || (NULL == pe_export)){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    memcpy(section_to[index].Name, ".edata\0\0", TP_IMAGE_SIZEOF_SHORT_NAME);

    section_to[index].PointerToRawData = (uint32_t)*file_size;
    rsize_t export_size =
        sizeof(TP_PE_EXPORT) + TP_PE_PADDING_FILE_ALIGNMENT(sizeof(TP_PE_EXPORT));
    *file_size += export_size;

    section_to[index].SizeOfRawData = (uint32_t)pe_export_size;
    section_to[index].VirtualSize = (uint32_t)pe_export_size;
    param->export_size = (uint32_t)pe_export_size;

    section_to[index].VirtualAddress = (uint32_t)*page_size;
    param->export_virtual_address = (uint32_t)*page_size;
    *page_size += pe_export_size;
    *page_size += TP_PE_PADDING_SECTION_ALIGNMENT(pe_export_size);

    section_to[index].PointerToRelocations = 0;
    section_to[index].PointerToLinenumbers = 0;
    section_to[index].NumberOfRelocations = 0;
    section_to[index].NumberOfLinenumbers = 0;
    section_to[index].Characteristics =
        TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ;

    ref_section_data[index].section_data = (uint8_t*)pe_export;
    pe_export = NULL;
    ref_section_data[index].section_data_size = pe_export_size;
    ref_section_data[index].is_create = true;

    return true;
}

static bool make_PE_file_buffer_section_data(
    TP_SYMBOL_TABLE* symbol_table,
    rsize_t header_size, rsize_t file_size, rsize_t page_size, TP_PE_PARAM* param)
{
    // Section Data
    param->section_data = NULL;
    file_size -= header_size;
    param->section_data_size = file_size;
    param->section_data_virtual_size = page_size;

    rsize_t section_data_size = 0;

    rsize_t number_of_sections = param->number_of_sections;
    TP_SECTION_DATA* ref_section_data = param->ref_section_data;

    for (rsize_t i = 0; number_of_sections > i; ++i){

        if (ref_section_data[i].section_data){

            section_data_size +=
                (ref_section_data[i].section_data_size +
                    TP_PE_PADDING_FILE_ALIGNMENT(ref_section_data[i].section_data_size));
        }
    }

    if (file_size != section_data_size){

        TP_PUT_LOG_MSG_ILE(symbol_table);

        return false;
    }

    uint8_t* section_data = (uint8_t*)TP_CALLOC(symbol_table, file_size, sizeof(uint8_t));

    if (NULL == section_data){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    for (rsize_t i = 0, offset = 0; number_of_sections > i; ++i){

        if (ref_section_data[i].section_data){

            rsize_t size = ref_section_data[i].section_data_size;

            memcpy(section_data + offset, ref_section_data[i].section_data, size);

            offset += size;

            rsize_t padding_size = TP_PE_PADDING_FILE_ALIGNMENT(size);

            if (ref_section_data[i].is_text && padding_size){

                memset(section_data + offset, TP_X64_OPCODE_NOP, padding_size);
            }

            offset += padding_size;
        }
    }

    param->section_data = section_data;

    return true;
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

        uint8_t* symbol_name = NULL;

        uint8_t short_name[TP_IMAGE_SIZEOF_SHORT_NAME + 1] = { 0 };

        if (0 == symbol[i].UNION.Name.Zeroes){

            // COFF String Table
            uint32_t size = symbol_table->member_string_table_size;

            if (0 == size){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            rsize_t offset = symbol[i].UNION.Name.Offset;

            if (size <= offset){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

            symbol_name = symbol_table->member_string_table + offset;
        }else{

            memcpy(short_name, symbol[i].UNION.ShortName, TP_IMAGE_SIZEOF_SHORT_NAME);

            symbol_name = short_name;
        }

        if (0 != strcmp(symbol_name, entry_point_symbol)){

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

        if ('/' == ref_section->Name[0]){

            TP_PUT_LOG_MSG_ILE(symbol_table);

            return false;
        }

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

static bool make_PE_file_buffer_section_relocation_data(
    TP_SYMBOL_TABLE* symbol_table, FILE* write_file,
    TP_SECTION_TABLE* section_to, rsize_t number_of_sections,
    uint8_t* pe_image_buffer, rsize_t pe_image_buffer_size)
{
    // ----------------------------------------------------------------------------------------
    // Relocation(COFF)
    bool is_print = IS_PE_PRINT(symbol_table);

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
    bool is_print = IS_PE_PRINT(symbol_table);

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

    if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) &&
        ((TP_IMAGE_SYM_DTYPE_NULL == type_msb) ||
        (TP_IMAGE_SYM_DTYPE_FUNCTION == type_msb)) &&
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

            if ('/' == Name[0]){

                TP_PUT_LOG_MSG_ILE(symbol_table);

                return false;
            }

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
        int32_t rip = (int32_t)(section_to->VirtualAddress + virtual_address + sizeof(uint32_t));
        apply_from -= rip;

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

