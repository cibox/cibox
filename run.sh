#!/bin/sh

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
IP=`perl -ne '/^server_ip:\s((?:[0-9]{1,3}\.){3}[0-9]{1,3})/ && print "$1"' config.yml`

ansible-playbook -vvvv -i $IP, services/jenkinsbox.yml --extra-vars "@config.yml"
