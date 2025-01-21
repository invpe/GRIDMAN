# Autostart

The best way is to autostart with a simple script. Follow the steps below to run the worker nodes automatically.


## Linux / Raspberry PI

1. `adduser gridman`
   
2. Execute `crontab -e`

3. Add `@reboot /home/gridman/autostart.sh GRIDMAN_SERVER_IP NODE_NAME ` at the end.

4. Exit crontab editing

5. Execute `cd /home/gridman`

6. Download `autostart.sh`
 
7. Make the script executable with `chmod +x ./autostart.sh`

8. Simply `reboot` to test.

