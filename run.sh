#!/bin/sh
export PYTHONUNBUFFERED=1
ansible-playbook -vvvv jenkinsbox.yml -M ./library/ansible-xml/library/
