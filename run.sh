#!/bin/sh

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
IP=`cat config.yml | grep -oP "^server_ip:\s*(?:[0-9]{1,3}\.){3}[0-9]{1,3}" | grep -oP "(?:[0-9]{1,3}\.){3}[0-9]{1,3}"`

ansible-playbook -vvvv -i $IP, jenkinsbox.yml --extra-vars "@config.yml"
