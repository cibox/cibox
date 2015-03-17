#!/bin/sh
export PYTHONUNBUFFERED=1
ansible-playbook github.yml -M ./library/ansible-xml/library/
