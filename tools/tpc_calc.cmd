
rem calc
rem -i : set use int_calc_compiler.
rem -l : set output log file.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -il tps_src.txt 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

