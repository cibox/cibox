#!/bin/bash

export PYTHONUNBUFFERED=1

echo 'Install apache'
ansible-playbook /vagrant/puphpet/ansible/apache.yml
echo 'Finished installing apache'

echo 'Install php'
ansible-playbook /vagrant/puphpet/ansible/php.yml
echo 'Finished installing php'

echo 'Install php xdebug'
ansible-playbook /vagrant/puphpet/ansible/php-xdebug.yml
echo 'Finished installing php xdebug'

echo 'Install mysql'
ansible-playbook /vagrant/puphpet/ansible/mysql.yml
echo 'Finished installing mysql'

echo 'Install memcached'
ansible-playbook /vagrant/puphpet/ansible/memcached.yml
echo 'Finished installing memcached'

echo 'Install composer'
ansible-playbook /vagrant/puphpet/ansible/composer.yml
echo 'Finished installing composer'

echo 'Install drush'
ansible-playbook /vagrant/puphpet/ansible/drush.yml
echo 'Finished installing drush'

echo 'Install Apache Solr'
ansible-playbook /vagrant/puphpet/ansible/solr.yml
echo 'Finished installing Apache Solr'
