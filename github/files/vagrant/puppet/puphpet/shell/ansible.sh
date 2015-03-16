#!/bin/bash

echo 'Installing base packages for ansible'
# ansible needs python.
DEBIAN_FRONTEND=noninteractive apt-get -y install python-simplejson sudo curl >/dev/null

# because basic ubuntu is too stripped down we need to add logging.
DEBIAN_FRONTEND=noninteractive apt-get --reinstall install -y bsdutils >/dev/null

# needed to use apt-add-repository.
DEBIAN_FRONTEND=noninteractive apt-get -y install software-properties-common python-software-properties >/dev/null

echo 'Finished installing base packages for ansible'

echo 'Installing ansible'

# install ansible and python mysql extension.
DEBIAN_FRONTEND=noninteractive apt-add-repository ppa:ansible/ansible >/dev/null
DEBIAN_FRONTEND=noninteractive apt-get update >/dev/null
DEBIAN_FRONTEND=noninteractive apt-get -y install ansible >/dev/null
DEBIAN_FRONTEND=noninteractive apt-get -y install python-mysqldb >/dev/null

echo 'Finished installing ansible'
