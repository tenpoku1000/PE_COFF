
rem 64 bits WebAssembly and UEFI app.
rem -g : set TP_IMAGE_SUBSYSTEM_EFI_APPLICATION to subsystem.
rem -l : set output log file.
rem -r : set origin wasm. [input file] is not necessary.
rem -z : set output PE/COFF file.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -glrz 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

