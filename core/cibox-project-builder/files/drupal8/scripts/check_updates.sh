#!/bin/sh

time ansible-playbook -vvvv updates.yml -i 'localhost,' --connection=local
