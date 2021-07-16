
// (C) Shin'ichi Ichikawa. Released under the MIT license.

#include "../tp_compiler.h"

#define TP_SIGN_BIT_OF_BYTE_IS_CLEAR(byte) (0 == ((byte) & 0x40))
#define TP_SIGN_BIT_OF_BYTE_IS_SET(byte) (0 != ((byte) & 0x40))

uint32_t tp_encode_si64leb128(uint8_t* buffer, size_t offset, int64_t value)
{
    uint32_t size = 0;

    do{
        uint8_t byte = value & 0x7f;

        value >>= 7;

        if (((0 == value) && TP_SIGN_BIT_OF_BYTE_IS_CLEAR(byte)) ||
            ((-1 == value) && TP_SIGN_BIT_OF_BYTE_IS_SET(byte))){

            if (buffer){

                (buffer + offset)[size] = byte;
            }

            size += sizeof(uint8_t);

            break;

        }else{

            byte |= 0x80;

            if (buffer){

                (buffer + offset)[size] = byte;
            }
        }

        size += sizeof(uint8_t);

    }while (true);

    return size;
}

uint32_t tp_encode_ui32leb128(uint8_t* buffer, size_t offset, uint32_t value)
{
    uint32_t size = 0;

    do{
        uint8_t byte = value & 0x7f;

        value >>= 7;

        if (value){

            byte |= 0x80;
        }

        if (buffer){

            (buffer + offset)[size] = byte;
        }

        size += sizeof(uint8_t);

    }while (value);

    return size;
}

int64_t tp_decode_si64leb128(uint8_t* buffer, uint32_t* size)
{
    uint8_t* p = buffer;

    int64_t value = 0;

    uint64_t byte = 0;

    int64_t shift = 0;

    do{
        byte = *p++;

        value |= ((byte & 0x7f) << shift);

        shift += 7;

    }while (128 <= byte);

    if (byte & 0x40){

        uint64_t init_value = -1;

        value |= (init_value << shift);
    }

    *size = (uint32_t)(p - buffer);

    return value;
}

int32_t tp_decode_si32leb128(uint8_t* buffer, uint32_t* size)
{
    uint8_t* p = buffer;

    int32_t value = 0;

    uint8_t byte = 0;

    int32_t shift = 0;

    do{
        byte = *p++;

        value |= ((byte & 0x7f) << shift);

        shift += 7;

    }while (128 <= byte);

    if (byte & 0x40){

        uint64_t init_value = -1;

        value |= (init_value << shift);
    }

    *size = (uint32_t)(p - buffer);

    return value;
}

uint32_t tp_decode_ui32leb128(uint8_t* buffer, uint32_t* size)
{
    uint8_t* p = buffer;

    uint32_t value = 0;

    for (uint32_t shift = 0; ; shift += 7){

        uint8_t byte = *p++;

        value |= ((byte & 0x7f) << shift);

        if (128 > byte){

            break;
        }
    }

    *size = (uint32_t)(p - buffer);

    return value;
}

