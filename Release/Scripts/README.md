# Autostart

The best way is to autostart with a simple script. Follow the steps below to run the worker nodes automatically.


## Linux / Raspberry PI

1. Create user: `adduser gridman`

2. `su gridman`

3. `cd ~/`
   
4. Execute `crontab -e`

5. Add `@reboot /home/gridman/autostart.sh GRIDMAN_SERVER_IP NODE_NAME ` at the end.

6. Exit crontab editing

7. Execute `cd /home/gridman`

8. Download `autostart.sh`
 
9. Make the script executable with `chmod +x ./autostart.sh`

10. Simply `reboot` to test.

