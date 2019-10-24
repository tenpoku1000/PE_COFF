if not exist .\efi\boot (mkdir .\efi\boot && copy bootx64.efi .\efi\boot) else (copy bootx64.efi .\efi\boot)

