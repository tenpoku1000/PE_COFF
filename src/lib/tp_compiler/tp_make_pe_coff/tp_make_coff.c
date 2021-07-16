
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_COFF_DATA_ALIGN_8(size) (uint32_t)((size) + (rsize_t)(-((int64_t)(size)) & (8 - 1)))
#define TP_COFF_DATA_ALIGN_16(size) (uint32_t)((size) + (rsize_t)(-((int64_t)(size)) & (16 - 1)))

static bool append_coff_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    TP_SECTION_TABLE* section, int16_t section_number, uint32_t offset_value,
    bool is_sym_class_external, bool is_sym_dtype_function,
    TP_SECTION_KIND section_kind, uint8_t* symbol, rsize_t symbol_length, uint32_t* symbol_index
);
static bool append_coff_symbol(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    TP_SECTION_KIND section_kind, TP_COFF_SYMBOL_TABLE* coff_symbol_data, uint32_t* symbol_index
);
static bool append_coff_string_table(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* symbol, rsize_t symbol_length, uint32_t* string_offset
);
static bool append_coff_relocation(
    TP_SYMBOL_TABLE* symbol_table,
    TP_COFF_RELOCATIONS_ARRAY* coff_relocation_array, TP_COFF_RELOCATIONS* coff_relocation
);

bool tp_add_coff_section(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind, TP_SECTION_ALIGN section_align)
{
    uint32_t Characteristics = 0;

    switch (section_align){
    case TP_SECTION_ALIGN_8_BYTE:
        Characteristics = TP_IMAGE_SCN_ALIGN_8BYTES;
        break;
    case TP_SECTION_ALIGN_16_BYTE:
        Characteristics = TP_IMAGE_SCN_ALIGN_16BYTES;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_COFF_WRITE* coff = &(symbol_table->member_coff);
    TP_SECTION_TABLE* section = NULL;

    ++(coff->member_section_num);

    switch (section_kind){
    case TP_SECTION_KIND_DATA:
        if (0 == coff->member_section_number_data){

            section = &(coff->member_section_data);
            strcpy_s(section->Name, sizeof(section->Name), ".data");
            Characteristics |= (TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ | TP_IMAGE_SCN_MEM_WRITE);
            coff->member_section_number_data = coff->member_section_num;
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    case TP_SECTION_KIND_RDATA:
        if (0 == coff->member_section_number_rdata){

            section = &(coff->member_section_rdata);
            strcpy_s(section->Name, sizeof(section->Name), ".rdata");
            Characteristics |= (TP_IMAGE_SCN_CNT_INITIALIZED_DATA | TP_IMAGE_SCN_MEM_READ);
            coff->member_section_number_rdata = coff->member_section_num;
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    case TP_SECTION_KIND_TEXT:
        if (0 == coff->member_section_number_text){

            section = &(coff->member_section_text);
            strcpy_s(section->Name, sizeof(section->Name), ".text$g");
            Characteristics |= (TP_IMAGE_SCN_CNT_CODE | TP_IMAGE_SCN_MEM_EXECUTE | TP_IMAGE_SCN_MEM_READ);
            coff->member_section_number_text = coff->member_section_num;
        }else{

            TP_PUT_LOG_MSG_ICE(symbol_table);

            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    section->VirtualSize = 0;
    section->VirtualAddress = 0;
    section->SizeOfRawData = 0;
    section->PointerToRawData = 0;
    section->PointerToRelocations = 0;
    section->PointerToLinenumbers = 0;
    section->NumberOfRelocations = 0;
    section->NumberOfLinenumbers = 0;

    section->Characteristics = Characteristics;

    return true;
}

bool tp_append_coff_data_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* data, rsize_t size, TP_SECTION_ALIGN section_align)
{
    switch (section_align){
    case TP_SECTION_ALIGN_8_BYTE:
        break;
    case TP_SECTION_ALIGN_16_BYTE:
//      break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    rsize_t tmp_size = (rsize_t)(coff->member_data_pos) + size;

    if (tmp_size >= coff->member_data_size){

        rsize_t data_size =  tmp_size + coff->member_data_size_allocate_unit;
        rsize_t padding = TP_COFF_DATA_ALIGN_8(data_size);
        data_size += padding;

        uint8_t* tmp_data = (uint8_t*)TP_REALLOC(
            symbol_table, coff->member_data, data_size
        );

        if (NULL == tmp_data){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        rsize_t clear_size = (rsize_t)(coff->member_data_size_allocate_unit) + padding;
        memset(tmp_data + tmp_size, 0, clear_size);

        coff->member_data = tmp_data;
        coff->member_data_size = (uint32_t)data_size;
    }

    memcpy(coff->member_data + coff->member_data_pos, data, size);

    coff->member_data_pos += (uint32_t)size;

    return true;
}

bool tp_append_coff_rdata_section(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* rdata, rsize_t size, TP_SECTION_ALIGN section_align)
{
    switch (section_align){
    case TP_SECTION_ALIGN_16_BYTE:
        break;
    case TP_SECTION_ALIGN_8_BYTE:
//      break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    rsize_t tmp_size = (rsize_t)(coff->member_rdata_pos) + size;

    if (tmp_size >= coff->member_rdata_size){

        rsize_t rdata_size =  tmp_size + coff->member_rdata_size_allocate_unit;
        rsize_t padding = TP_COFF_DATA_ALIGN_16(rdata_size);
        rdata_size += padding;

        uint8_t* tmp_rdata = (uint8_t*)TP_REALLOC(
            symbol_table, coff->member_rdata, rdata_size
        );

        if (NULL == tmp_rdata){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        rsize_t clear_size = (rsize_t)(coff->member_rdata_size_allocate_unit) + padding;
        memset(tmp_rdata + tmp_size, 0, clear_size);

        coff->member_rdata = tmp_rdata;
        coff->member_rdata_size = (uint32_t)rdata_size;
    }

    memcpy(coff->member_rdata + coff->member_rdata_pos, rdata, size);

    coff->member_rdata_pos += (uint32_t)size;

    return true;
}

bool tp_append_coff_relocation(
    TP_SYMBOL_TABLE* symbol_table, TP_SECTION_KIND section_kind,
    uint32_t virtual_address, uint32_t symbol_table_index)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    if ((NULL == coff->member_coff_symbol) ||
        (0 == coff->member_coff_symbol_num)){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if (symbol_table_index > coff->member_coff_symbol_num){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    TP_COFF_RELOCATIONS coff_relocation = {
        .UNION.VirtualAddress = virtual_address,
        .SymbolTableIndex = symbol_table_index,
        .Type = TP_IMAGE_REL_AMD64_REL32
    };

    switch (section_kind){
    case TP_SECTION_KIND_DATA:
        if ( ! append_coff_relocation(
            symbol_table, &(coff->member_data_coff_relocations), &coff_relocation)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    case TP_SECTION_KIND_RDATA:
        if ( ! append_coff_relocation(
            symbol_table, &(coff->member_rdata_coff_relocations), &coff_relocation)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    case TP_SECTION_KIND_TEXT:
        if ( ! append_coff_relocation(
            symbol_table, &(coff->member_text_coff_relocations), &coff_relocation)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    return true;
}

bool tp_append_coff_symbol(
    TP_SYMBOL_TABLE* symbol_table, bool is_external, TP_SECTION_KIND section_kind,
    uint8_t* symbol, rsize_t symbol_length, uint32_t offset_value, uint32_t* symbol_index)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    TP_SECTION_TABLE* section = NULL;
    int16_t section_number = TP_IMAGE_SYM_UNDEFINED;

    bool is_sym_class_external = is_external;
    bool is_sym_dtype_function = false;

    switch (section_kind){
    case TP_SECTION_KIND_DATA:

        section = &(coff->member_section_data);

        if (NULL == symbol){

            symbol = ".data";
            symbol_length = 5;
        }

        if (0 == coff->member_section_number_data){

            TP_PUT_LOG_MSG_ICE(symbol_table);
 
            return false;
        }

        section_number = coff->member_section_number_data;
        break;
    case TP_SECTION_KIND_RDATA:

        section = &(coff->member_section_rdata);

        if (NULL == symbol){

            symbol = ".rdata";
            symbol_length = 6;
        }

        if (0 == coff->member_section_number_rdata){

            TP_PUT_LOG_MSG_ICE(symbol_table);
 
            return false;
        }

        section_number = coff->member_section_number_rdata;
        break;
    case TP_SECTION_KIND_TEXT:

        section = &(coff->member_section_text);

        if (symbol){

            is_sym_dtype_function = true;
        }else{

            symbol = ".text$g";
            symbol_length = 7;
        }

        if (0 == coff->member_section_number_text){

            TP_PUT_LOG_MSG_ICE(symbol_table);
 
            return false;
        }

        section_number = coff->member_section_number_text;
        break;
    default:
        TP_PUT_LOG_MSG_ICE(symbol_table);
        return false;
    }

    if (0 == symbol_length){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    if ( ! append_coff_symbol_table(
        symbol_table, coff, section, section_number, offset_value,
        is_sym_class_external, is_sym_dtype_function,
        section_kind, symbol, symbol_length, symbol_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

bool tp_set_function_offset_to_symbol(
    TP_SYMBOL_TABLE* symbol_table,
    uint32_t func_index, uint32_t text_size, uint32_t offset)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    // COFF Symbol Table
    TP_COFF_SYMBOL_TABLE* symbol = coff->member_coff_symbol;

    rsize_t num = coff->member_coff_symbol_num;

    if (0 == num){

        return true;
    }

    // SECTION TABLE
    int16_t section_number = coff->member_section_number_text;

    uint32_t func_num = 0;

    for (rsize_t i = 0; num > i; ++i){

        if (section_number != symbol[i].SectionNumber){

            continue;
        }

        // Type Representation
        // Storage Class
        uint8_t type_lsb = TP_IMAGE_SYM_TYPE_LSB(&(symbol[i]));
        uint8_t type_msb = TP_IMAGE_SYM_TYPE_MSB(&(symbol[i]));

        if ((TP_IMAGE_SYM_TYPE_NULL == type_lsb) && (TP_IMAGE_SYM_DTYPE_FUNCTION == type_msb)){

            if (text_size <= offset){

                TP_PUT_LOG_MSG_ICE(symbol_table);

                return false;
            }

            ++func_num;

            if (func_index != (func_num - 1)){

                continue;
            }

            symbol[i].Value = offset;

            return true;
        }
    }

    TP_PUT_LOG_MSG_ICE(symbol_table);

    return false;
}

static bool append_coff_symbol_table(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    TP_SECTION_TABLE* section, int16_t section_number, uint32_t offset_value,
    bool is_sym_class_external, bool is_sym_dtype_function,
    TP_SECTION_KIND section_kind, uint8_t* symbol, rsize_t symbol_length, uint32_t* symbol_index)
{
    TP_COFF_SYMBOL_TABLE coff_symbol_data = { 0 };

    if (TP_IMAGE_SIZEOF_SHORT_NAME < symbol_length){

        uint32_t string_offset = 0;

        if ( ! append_coff_string_table(symbol_table, symbol, symbol_length, &string_offset)){

            TP_PUT_LOG_MSG_TRACE(symbol_table);

            return false;
        }

        coff_symbol_data.UNION.Name.Zeroes = 0; // if 0, use LongName
        coff_symbol_data.UNION.Name.Offset = string_offset; // An offset into the string table.
    }else if (TP_IMAGE_SIZEOF_SHORT_NAME == symbol_length){

        memcpy(coff_symbol_data.UNION.ShortName, symbol, TP_IMAGE_SIZEOF_SHORT_NAME);
    }else{

        sprintf_s(coff_symbol_data.UNION.ShortName, TP_IMAGE_SIZEOF_SHORT_NAME, "%s", symbol);
    }

    // Value
    coff_symbol_data.Value = offset_value;

    // Section Number
    coff_symbol_data.SectionNumber = section_number;

    // Type Representation
    if (is_sym_dtype_function){

        coff_symbol_data.Type = ((TP_IMAGE_SYM_DTYPE_FUNCTION << 4) & 0xf0);
    }else{

        coff_symbol_data.Type = TP_IMAGE_SYM_TYPE_NULL;
    }

    // Storage Class
    if (is_sym_class_external){

        coff_symbol_data.StorageClass = TP_IMAGE_SYM_CLASS_EXTERNAL;
    }else{

        coff_symbol_data.StorageClass = TP_IMAGE_SYM_CLASS_STATIC;
    }

    coff_symbol_data.NumberOfAuxSymbols = 0;

    if ( ! append_coff_symbol(symbol_table, coff, section_kind, &coff_symbol_data, symbol_index)){

        TP_PUT_LOG_MSG_TRACE(symbol_table);

        return false;
    }

    return true;
}

static bool append_coff_symbol(
    TP_SYMBOL_TABLE* symbol_table, TP_COFF_WRITE* coff,
    TP_SECTION_KIND section_kind, TP_COFF_SYMBOL_TABLE* coff_symbol_data, uint32_t* symbol_index)
{
    if (NULL == coff->member_coff_symbol){

        coff->member_coff_symbol_num = 1;
        coff->member_coff_symbol_size =
            coff->member_coff_symbol_size_allocate_unit * sizeof(TP_COFF_SYMBOL_TABLE);

        TP_COFF_SYMBOL_TABLE* tmp_symbol = (TP_COFF_SYMBOL_TABLE*)TP_CALLOC(
            symbol_table,
            coff->member_coff_symbol_size_allocate_unit, sizeof(TP_COFF_SYMBOL_TABLE)
        );

        if (NULL == tmp_symbol){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        coff->member_coff_symbol = tmp_symbol;

        memcpy(coff->member_coff_symbol[0].UNION.ShortName, "dummy", 5);
        coff->member_coff_symbol[0].Value = 0;
        coff->member_coff_symbol[0].SectionNumber = TP_IMAGE_SYM_ABSOLUTE;
        coff->member_coff_symbol[0].Type = 0;
        coff->member_coff_symbol[0].StorageClass = TP_IMAGE_SYM_CLASS_NULL;
        coff->member_coff_symbol[0].NumberOfAuxSymbols = 0;
    }

    if (coff->member_coff_symbol_num == (coff->member_coff_symbol_size / sizeof(TP_COFF_SYMBOL_TABLE))){

        uint32_t tp_coff_symbol_size_allocate_unit =
            coff->member_coff_symbol_size_allocate_unit * sizeof(TP_COFF_SYMBOL_TABLE);

        uint32_t tp_coff_symbol_size = coff->member_coff_symbol_size + tp_coff_symbol_size_allocate_unit;

        TP_COFF_SYMBOL_TABLE* tp_coff_symbol_table = (TP_COFF_SYMBOL_TABLE*)TP_REALLOC(
            symbol_table, coff->member_coff_symbol, tp_coff_symbol_size
        );

        if (NULL == tp_coff_symbol_table){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        memset(
            ((uint8_t*)tp_coff_symbol_table) + coff->member_coff_symbol_size, 0,
            tp_coff_symbol_size_allocate_unit
        );

        coff->member_coff_symbol = tp_coff_symbol_table;
        coff->member_coff_symbol_size = tp_coff_symbol_size;
    }

    coff->member_coff_symbol[coff->member_coff_symbol_num] = *coff_symbol_data;

    if (symbol_index){

        *symbol_index = coff->member_coff_symbol_num;
    }

    ++(coff->member_coff_symbol_num);

    return true;
}

static bool append_coff_string_table(
    TP_SYMBOL_TABLE* symbol_table, uint8_t* symbol, rsize_t symbol_length, uint32_t* string_offset)
{
    TP_COFF_WRITE* coff = &(symbol_table->member_coff);

    rsize_t offset = coff->member_string_table_size;

    rsize_t size = (rsize_t)(coff->member_string_table_size) + symbol_length + 1;

    if (coff->member_string_table_size > size){

        TP_PUT_LOG_MSG_ICE(symbol_table);

        return false;
    }

    uint8_t* tmp_string_buffer = (uint8_t*)TP_REALLOC(symbol_table, coff->member_string_table, size);

    if (NULL == tmp_string_buffer){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    memcpy(tmp_string_buffer + coff->member_string_table_size, symbol, symbol_length);

    rsize_t last_index = size - 1;

    tmp_string_buffer[last_index] = '\0';

    for (rsize_t i = offset; last_index > i; ++i){

        if ('\0' == tmp_string_buffer[i]){

            tmp_string_buffer[i] = '@';
        }
    }

    coff->member_string_table = tmp_string_buffer;

    coff->member_string_table_size = (uint32_t)size;

    *string_offset = (uint32_t)offset;

    return true;
}

static bool append_coff_relocation(
    TP_SYMBOL_TABLE* symbol_table,
    TP_COFF_RELOCATIONS_ARRAY* coff_relocation_array, TP_COFF_RELOCATIONS* coff_relocation)
{
    if (NULL == coff_relocation_array->member_relocations){

        TP_COFF_RELOCATIONS first_coff_relocation = {
            .UNION.RelocCount = 0,
            .SymbolTableIndex = 0,
            .Type = TP_IMAGE_REL_AMD64_ABSOLUTE
        };

        coff_relocation_array->member_num = 1;
        coff_relocation_array->member_size = sizeof(TP_COFF_RELOCATIONS);

        TP_COFF_RELOCATIONS* tmp_reloc = (TP_COFF_RELOCATIONS*)TP_CALLOC(
            symbol_table, 1, sizeof(TP_COFF_RELOCATIONS)
        );

        if (NULL == tmp_reloc){

            TP_PRINT_CRT_ERROR(symbol_table);

            return false;
        }

        coff_relocation_array->member_relocations = tmp_reloc;

        coff_relocation_array->member_relocations[0] = first_coff_relocation;
    }

    ++(coff_relocation_array->member_num);
    coff_relocation_array->member_size += sizeof(TP_COFF_RELOCATIONS);

    TP_COFF_RELOCATIONS* tmp_reloc = (TP_COFF_RELOCATIONS*)TP_REALLOC(
        symbol_table, coff_relocation_array->member_relocations, coff_relocation_array->member_size
    );

    if (NULL == tmp_reloc){

        TP_PRINT_CRT_ERROR(symbol_table);

        return false;
    }

    coff_relocation_array->member_relocations = tmp_reloc;

    coff_relocation_array->member_relocations[coff_relocation_array->member_num - 1] = *coff_relocation;

    return true;
}

