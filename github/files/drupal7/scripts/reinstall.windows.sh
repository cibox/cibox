#!/bin/bash
 time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local --extra-vars "is_windows=true"
