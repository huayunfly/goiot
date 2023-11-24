@ REM 将目标IP机器时钟设置为本机时钟
net use \\192.168.2.149 "target_machine_password" /user:"target_machine_username"
net time \\192.168.2.149 /set /y