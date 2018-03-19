#!/bin/bash

export PYTHONUNBUFFERED=1
export ANSIBLE_FORCE_COLOR=true

# Commented playbook stay here just in case you'll need it for updating an image or
# specific software settings.
playbooks=(
/vagrant/provisioning/ansible/playbooks/sendmail.yml \
/vagrant/provisioning/ansible/playbooks/misc.yml \
/vagrant/provisioning/ansible/playbooks/www.yml \
/vagrant/provisioning/ansible/playbooks/php.yml \
/vagrant/provisioning/ansible/playbooks/php-xdebug.yml \
/vagrant/provisioning/ansible/playbooks/mysql.yml \
/vagrant/provisioning/ansible/playbooks/memcached.yml \
/vagrant/provisioning/ansible/playbooks/composer.yml \
/vagrant/provisioning/ansible/playbooks/drush.yml \
/vagrant/provisioning/ansible/playbooks/solr.yml \
/vagrant/provisioning/ansible/playbooks/sniffers.yml \
/vagrant/provisioning/ansible/playbooks/docker.yml \
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
