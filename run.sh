#!/bin/sh

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
REGEX_IP="(?:[0-9]{1,3}\.){3}[0-9]{1,3}"
IP=`cat config.yml | grep -oP "^server_ip:\s*$REGEX_IP" | grep -oP "$REGEX_IP"`

ansible-playbook -vvvv -i $IP, jenkinsbox.yml --extra-vars "@config.yml"
