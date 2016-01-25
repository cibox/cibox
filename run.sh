#!/bin/sh

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
cat config.yml | grep -e '^server_ip:\s.*$' | cut -d" " -f2

ansible-playbook -vvvv -i $IP, services/jenkinsbox.yml --extra-vars "@config.yml"