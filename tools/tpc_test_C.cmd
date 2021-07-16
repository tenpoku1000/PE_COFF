
rem tpc_test_C
rem -t : set test mode. [input file] is not necessary.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -t 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

