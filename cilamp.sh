#!/bin/sh

. ./requirements.sh

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
IP=$(grep -E '^server_ip:\s.*$' config.yml | cut -d" " -f2)

if [ -z "${IP}" ]; then
    echo "Please specify IP address in config.yml"
    exit
fi

ansible-playbook -vvvv -i "${IP}", services/cilamp.yml --connection=local --extra-vars "@config.yml"
