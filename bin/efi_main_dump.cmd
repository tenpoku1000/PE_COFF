echo "dumpbin efi_main.obj /DIRECTIVES /DISASM /HEADERS /RAWDATA /RELOCATIONS /SYMBOLS > efi_main_obj.txt"  > efi_main_obj.txt 
echo; >> efi_main_obj.txt 
dumpbin efi_main.obj /DIRECTIVES /DISASM /HEADERS /RAWDATA /RELOCATIONS /SYMBOLS >> efi_main_obj.txt
