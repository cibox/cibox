#!/bin/bash

export PYTHONUNBUFFERED=1

playbooks=(
/vagrant/provisioning/ansible/sendmail.yml \
/vagrant/provisioning/ansible/misc.yml \
/vagrant/provisioning/ansible/apache.yml \
/vagrant/provisioning/ansible/php.yml \
/vagrant/provisioning/ansible/php-xdebug.yml \
/vagrant/provisioning/ansible/mysql.yml \
/vagrant/provisioning/ansible/memcached.yml \
/vagrant/provisioning/ansible/composer.yml \
/vagrant/provisioning/ansible/drush.yml \
/vagrant/provisioning/ansible/solr.yml \
/vagrant/provisioning/ansible/sniffers.yml \
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
