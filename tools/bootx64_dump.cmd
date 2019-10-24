echo "dumpbin bootx64.efi /DIRECTIVES /DISASM /HEADERS /RAWDATA /RELOCATIONS /SYMBOLS > bootx64_efi.txt"  > bootx64_efi.txt 
echo; >> bootx64_efi.txt 
dumpbin bootx64.efi /DIRECTIVES /DISASM /HEADERS /RAWDATA /RELOCATIONS /SYMBOLS >> bootx64_efi.txt
