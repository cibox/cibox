#!/bin/sh
# You should install ansible for ability to run this script
# sudo apt-get install software-properties-common
# sudo apt-add-repository ppa:ansible/ansible
# sudo apt-get update
# sudo apt-get install ansible
# sudo apt-get install python-mysqldb
time ansible-playbook reinstall.yml
