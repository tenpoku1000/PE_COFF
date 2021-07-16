
rem tpc_test_UCD
rem -e : set test UNICODE CHARACTER DATABASE.
rem -l : set output log file.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -el 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

