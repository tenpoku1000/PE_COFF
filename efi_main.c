
// (C) Shin'ichi Ichikawa. Released under the MIT license.

struct EFI_TABLE_HEADER{
    unsigned long long/* UINT64 */ Signature;
    unsigned/* UINT32 */ Revision;
    unsigned/* UINT32 */ HeaderSize;
    unsigned/* UINT32 */ CRC32;
    unsigned/* UINT32 */ Reserved;
};

struct EFI_RUNTIME_SERVICES{
    struct EFI_TABLE_HEADER Hdr;
    void* GetTime;
    void* SetTime;
    void* GetWakeupTime;
    void* SetWakeupTime;
    void* SetVirtualAddressMap;
    void* ConvertPointer;
    void* GetVariable;
    void* GetNextVariableName;
    void* SetVariable;
    void* GetNextHighMonotonicCount;

    unsigned long long/*EFI_STATUS*/ (*ResetSystem)(
        int/* enum EFI_RESET_TYPE */ ResetType,
        unsigned long long/* EFI_STATUS */ ResetStatus,
        unsigned long long/* UINTN */ DataSize,
        unsigned short* /* CHAR16* */ ResetData
    );
};

struct EFI_INPUT_KEY{
    unsigned short/*UINT16*/ ScanCode;
    unsigned short/*CHAR16*/ UnicodeChar;
};

struct SIMPLE_INPUT_INTERFACE{
    void* Reset;

    unsigned long long/*EFI_STATUS*/ (*ReadKeyStroke)(
        struct SIMPLE_INPUT_INTERFACE* This,
        struct EFI_INPUT_KEY* Key
    );

    void* WaitForKey;
};

struct SIMPLE_TEXT_OUTPUT_INTERFACE{
    void* Reset;

    unsigned long long/*EFI_STATUS*/ (*OutputString)(
        struct SIMPLE_TEXT_OUTPUT_INTERFACE* This,
        unsigned short* /* CHAR16* */ String
    );

    void* TestString;
    void* QueryMode;
    void* SetMode;
    void* SetAttribute;
    void* ClearScreen;
    void* SetCursorPosition;
    void* EnableCursor;
    void* Mode;
};

struct EFI_SYSTEM_TABLE{
    struct EFI_TABLE_HEADER Hdr;

    unsigned short* /* CHAR16* */ FirmwareVendor;
    unsigned /* UINT32 */ FirmwareRevision;

    void* ConsoleInHandle;
    struct SIMPLE_INPUT_INTERFACE* ConIn;

    void* ConsoleOutHandle;
    struct SIMPLE_TEXT_OUTPUT_INTERFACE* ConOut;

    void* StandardErrorHandle;
    void* StdErr;

    struct EFI_RUNTIME_SERVICES* RuntimeServices;
    void* BootServices;

    unsigned long long/* UINTN */ NumberOfTableEntries;
    void* ConfigurationTable;
};

static unsigned long long/* EFI_STATUS */ EFI_SUCCESS = 0;

static struct EFI_SYSTEM_TABLE* ST = 0;
static struct EFI_RUNTIME_SERVICES* RT = 0;
static struct SIMPLE_INPUT_INTERFACE* IN = 0;
static struct SIMPLE_TEXT_OUTPUT_INTERFACE* OUT = 0;

unsigned long long/* EFI_STATUS */ efi_main(void* /* EFI_HANDLE */ image_handle, struct EFI_SYSTEM_TABLE* system_table)
{
    ST = system_table;
    RT = ST->RuntimeServices;
    IN = ST->ConIn;
    OUT = ST->ConOut;

    OUT->OutputString(OUT, L"When you press any key, the system will reboot.\n");

    unsigned long long/* EFI_STATUS */ local_status = EFI_SUCCESS;

    do{
        struct EFI_INPUT_KEY key;

        local_status = IN->ReadKeyStroke(IN, &key);

    }while (EFI_SUCCESS != local_status);

    RT->ResetSystem(0/* EfiResetCold */, EFI_SUCCESS, 0, 0/* NULL */);

    return EFI_SUCCESS;
}

