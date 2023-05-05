# WSL 2

Step 1: Go to settings > (search) turn windows feature on / off
Step 2: If Windows Hypervisor Platform and Windows Machine Platform is unchecked, please check them, else uncheck and recheck them.
Step 3: Reboot
https://github.com/microsoft/WSL/issues/8693

Upgrade version from WSL 1 to WSL 2, as WSL2 now supports GUI
```
sudo apt update
sudo apt full-upgrade
```


```
wsl -install
wsl --set-version Ubuntu 2
```
This might take few hours !

`wsl -l -v`
NAME      STATE           VERSION
* Ubuntu    Stopped         2


___________________________________________
Start the Docker daemon:
```
sudo dockerd &
```
```
wget https://dl.influxdata.com/influxdb/releases/influxdb2-2.7.1-amd64.deb
sudo dpkg -i influxdb2-2.7.1-amd64.deb
```


On WSL Ubuntu, you can manually run the InfluxDB daemon with:
influxd &
The ampersand at the end makes the daemon run in the background, so you can still execute commands after launching it. You can also close the WSL window, and the daemon will keep running.

Note: you can accidently run multiple instances of the InfluxDB with this method; you can check that none is running with:
ps -A | grep influxd
```
influxd &
```
Go to localhost:8086

netsh interface portproxy add v4tov4 listenport=8086 listenaddress=0.0.0.0 connectport=8086 connectaddress=172.31.112.228
netsh interface portproxy show all

New-NetFirewallRule -DisplayName "WSL2 Port Bridge" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 8086

Tuto: https://jwstanly.com/blog/article/Port+Forwarding+WSL+2+to+Your+LAN/

# Grafana

```
sudo apt-get install -y adduser libfontconfig1
wget https://dl.grafana.com/enterprise/release/grafana-enterprise_9.5.1_amd64.deb
sudo dpkg -i grafana-enterprise_9.5.1_amd64.deb
```
```
sudo grafana-server -homepath /usr/share/grafana &
```
The default username and password for Grafana is admin / admin . The first time you login, you will be asked to reset the default admin password.

```shell
netsh interface portproxy add v4tov4 listenport=3000 listenaddress=0.0.0.0 connectport=3000 connectaddress=172.31.112.228
New-NetFirewallRule -DisplayName "WSL2 Port Bridge" -Direction Inbound -Action Allow -Protocol TCP -LocalPort 3000
```