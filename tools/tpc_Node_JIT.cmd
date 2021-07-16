
rem tpc_Node_JIT
rem -l : set output log file.

call tpc_exec_name.cmd
call tpc_src_C_name.cmd
call %TP_EXEC_NAME% -l %TP_SRC_C_NAME% 1> tp_exec_log.log 2>&1
call node tpc_node.js 1>> tp_exec_log.log 2>&1
type tp_exec_log.log

@echo off
setlocal
set /p ch="press enter key:"

