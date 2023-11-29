@ REM 将本机时钟与源IP机器时钟同步
net use \\192.168.2.149 target_machine_password /user:target_machine_username
net time \\192.168.2.149 /set /y