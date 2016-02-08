#!/bin/bash

export PYTHONUNBUFFERED=1

playbooks=(
/var/www/docroot/reinstall.yml \
/var/www/docroot/devops.yml \
/var/www/docroot/sniffers.yml \
/var/www/docroot/tests.yml \
)

for i in "${playbooks[@]}"
do
   echo "Install "${i}
   ansible-playbook ${i}  -i 'localhost,' --connection=local
   if [ "$?" == "0" ]; then
    echo "Finished installing "${i}
   else
    echo "Failed installing "${i}
    exit 1
   fi
done
