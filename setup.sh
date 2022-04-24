#!/bin/bash
if [ "$(whoami)" != root ]; then
    echo Please run this script as root or using sudo
    exit
fi

service_name="mps-notifications.service"

# make readonly dir and copy ca certificate files there
mkdir -p /usr/local/share/multi-purpose-server

# launch service
cp -v $service_name /etc/systemd/system/$service_name
systemctl daemon-reload || echo cant reload daemon
systemctl enable --now $service_name || echo cant enable service: $service_name