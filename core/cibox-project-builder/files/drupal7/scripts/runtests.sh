#!/bin/sh
# You should install ansible for ability to run this script
export PYTHONUNBUFFERED=1
time ansible-playbook -vvvv tests.yml -i 'localhost,' --connection=local
