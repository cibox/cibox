#!/bin/sh

#
# Sniffers Codestyle Autofix Runner
#
# This script runs Ansible codestyleautofix.yml Ansible playbook.
# Use "-n" option to prevent the playbook from updating software each time.
# Example: ./codesyleautofix.sh -n

UPDATE=1
while getopts ":n" opt; do
  case $opt in
    n)
	  UPDATE=0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

if [ $UPDATE -eq 1 ]
then
  time ansible-playbook -vvvv codestyleautofix.yml -i 'localhost,' --connection=local
else
  time ansible-playbook -vvvv codestyleautofix.yml -i 'localhost,' --connection=local --extra-vars "sniffers_update=false"
fi