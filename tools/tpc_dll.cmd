
rem DLL
rem -f : set execute of dll.
rem -l : set output log file.
rem -z : set output PE/COFF file.

call tpc_exec_name.cmd
call tpc_src_C_name.cmd
call %TP_EXEC_NAME% -lz %TP_SRC_C_NAME% 1> tp_exec_log.log 2>&1
call %TP_EXEC_NAME% -f 1>> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

