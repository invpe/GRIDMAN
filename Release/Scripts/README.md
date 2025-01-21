# Autostart

The best way is to autostart with a simple script. Follow the steps below to run the worker nodes automatically.


## Linux / Raspberry PI

1. Execute `crontab -e`

2. Add `@reboot /home/pi/autostart.sh NODE_NAME GRIDMAN_SERVER_IP` at the end.

3. Exit crontab editing

4. Execute `cd /home/pi`

5. Download `autostart.sh`
 
6. Make the script executable with `chmod +x ./autostart.sh`

7. Simply `reboot` to test.

