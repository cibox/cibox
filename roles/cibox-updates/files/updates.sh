#!/bin/bash

export PYTHONUNBUFFERED=1

pwd

playbooks=(
updates/hook_update_106.yml \
updates/hook_update_107.yml \
)

for i in "${playbooks[@]}"
do
   echo "Install "${i}
   ansible-playbook -i 'localhost,' ${i} --connection=local
   if [ "$?" == "0" ]; then
    echo "Finished installing "${i}
   else
    echo "Failed installing "${i}
    exit 1
   fi
done