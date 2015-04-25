#!/bin/bash

export PYTHONUNBUFFERED=1

playbooks=(
/var/www/drupal/reinstall.yml \
/var/www/drupal/devops.yml \
/var/www/drupal/sniffers.yml \
/var/www/drupal/tests.yml \
)

for i in "${playbooks[@]}"
do
   echo "Install "${i}
   ansible-playbook ${i}
   if [ "$?" == "0" ]; then
    echo "Finished installing "${i}
   else
    echo "Failed installing "${i}
    exit 1
   fi
done
