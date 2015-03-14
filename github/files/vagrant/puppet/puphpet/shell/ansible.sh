#!/bin/bash

echo 'Installing base packages for ansible'
# ansible needs python.
apt-get -y install python-simplejson sudo curl >/dev/null

# because basic ubuntu is too stripped down we need to add logging.
apt-get --reinstall install -y bsdutils >/dev/null

# needed to use apt-add-repository.
apt-get -y install software-properties-common python-software-properties >/dev/null

echo 'Finished installing base packages for ansible'

echo 'Installing ansible'

# install ansible and python mysql extension.
apt-add-repository ppa:ansible/ansible >/dev/null
apt-get update >/dev/null
apt-get -y install ansible >/dev/null
apt-get -y install python-mysqldb >/dev/null

echo 'Finished installing ansible'
