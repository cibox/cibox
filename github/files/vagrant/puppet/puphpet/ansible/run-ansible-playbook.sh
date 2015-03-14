#!/bin/bash

echo 'Install composer'
ansible-playbook /vagrant/puphpet/ansible/composer.yml
echo 'Finished installing composer'
