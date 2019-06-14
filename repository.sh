#!/bin/sh

#. ./requirements.sh

export PYTHONUNBUFFERED=1
ansible-playbook -i 127.0.0.1, services/repository.yml
