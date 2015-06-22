#!/bin/bash

echo 'Installing base packages for ansible'
export "DEBIAN_FRONTEND=noninteractive"
# ansible needs python.
apt-get -y install python python-dev python-simplejson sudo curl make rsync git libmysqlclient-dev apparmor-utils >/dev/null

# because basic ubuntu is too stripped down we need to add logging.
apt-get --reinstall install -y bsdutils >/dev/null

# needed to use apt-add-repository.
apt-get -y install software-properties-common python-software-properties >/dev/null

echo 'Finished installing base packages for ansible'

echo 'Installing ansible'

echo "Installing pip via easy_install."
wget https://raw.githubusercontent.com/ActiveState/ez_setup/v0.9/ez_setup.py
python ez_setup.py && rm -f ez_setup.py
easy_install pip
# Make sure setuptools are installed crrectly.
pip install setuptools --no-use-wheel --upgrade

echo "Installing required python modules."
pip install paramiko pyyaml jinja2 markupsafe MySQL-python

echo "Installing Ansible."
pip install ansible

echo 'Finished installing ansible'
