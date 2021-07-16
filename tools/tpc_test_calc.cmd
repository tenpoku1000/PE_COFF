
rem tpc_test_calc
rem -i : set use int_calc_compiler.
rem -t : set test mode. [input file] is not necessary.

call tpc_exec_name.cmd
call %TP_EXEC_NAME% -it 1> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

