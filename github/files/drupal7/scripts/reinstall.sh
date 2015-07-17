#!/bin/sh

if [ "$1" = "--windows" ]; then
    time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local --extra-vars "is_windows=true"
else
    time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local
fi
