mkdir .\efi\boot

link /OUT:".\efi\boot\bootx64.efi"  ^
/SUBSYSTEM:EFI_APPLICATION  ^
/ENTRY:"efi_main"  ^
/DLL /MACHINE:X64 ^
/NODEFAULTLIB ^
/MANIFEST:NO /NXCOMPAT:NO /DYNAMICBASE:NO ^
/OPT:REF  ^
/OPT:ICF  ^
/ERRORREPORT:PROMPT /NOLOGO  ^
PE_COFF.obj
