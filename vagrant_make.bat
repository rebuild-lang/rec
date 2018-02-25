@echo off
set PATH=C:\Program Files\Oracle\VirtualBox;C:\tools\msys64\usr\bin;C:\HashiCorp\Vagrant\bin;%PATH%
set ANSICON=1
set SSH_AUTH_SOCK=/tmp/.ssh-pageant2-%USERNAME%
:: . ssh-pageant -q -r -a /tmp/.ssh-pageant2-%USERNAME%

vagrant ssh -c "cd /vagrant && make %*"
