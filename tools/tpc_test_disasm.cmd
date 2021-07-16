
rem tpc_test_disasm
rem -a : set test of disassembler of x64.
rem -l : set output log file.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -al 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

