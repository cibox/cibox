#!/bin/bash

export PYTHONUNBUFFERED=1

# Get server IP from config.yml
IP=`perl -ne '/^server_ip:\s((?:[0-9]{1,3}\.){3}[0-9]{1,3})/ && print "$1"' config.yml`


# Check if IP passed within option.
while [[ $# > 1 ]]
do
key="$1"
case $key in
    -i|--ip)
      IP="$2"
      shift # past argument
    ;;
esac
shift
done

ansible-playbook -vvvv -i $IP, jenkinsbox.yml --extra-vars "@config.yml"
