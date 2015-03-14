#!/bin/bash

echo 'Install composer'
ansible-playbook /vagrant/puphpet/ansible/composer.yml
echo 'Finished installing composer'

echo 'Install Apache Solr'
ansible-playbook /vagrant/puphpet/ansible/solr.yml
echo 'Finished installing Apache Sole'
