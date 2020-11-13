#!/bin/bash

echo 'Installing base packages for ansible'
export DEBIAN_FRONTEND=noninteractive
export PYTHONWARNINGS=ignore
export PYTHONUNBUFFERED=1
export ANSIBLE_FORCE_COLOR=true
# ansible needs python version 2.7.9 to avoid SNIMissingWarning.
# see http://urllib3.readthedocs.io/en/latest/security.html#snimissingwarning
apt-get -y --force-yes install python
apt-get -y --force-yes install python-dev python-simplejson sudo curl make rsync git libmysqlclient-dev apparmor-utils >/dev/null

# because basic ubuntu is too stripped down we need to add logging.
apt-get --reinstall install -y --force-yes bsdutils >/dev/null

# needed to use apt-add-repository.
apt-get -y --force-yes install software-properties-common python-software-properties >/dev/null

echo 'Finished installing base packages for ansible'

echo 'Installing ansible'

echo "Installing pip via easy_install."
sudo apt-get -y --force-yes install unzip python3-pip python-dev build-essential libffi-dev libssl-dev

sudo PYTHONWARNINGS=ignore pip3 install --upgrade pip
sudo PYTHONWARNINGS=ignore pip3 install --upgrade virtualenv
sudo PYTHONWARNINGS=ignore pip3 install --upgrade pip
sudo PYTHONWARNINGS=ignore pip3 install --upgrade cffi
sudo PYTHONWARNINGS=ignore pip3 install --upgrade cryptography
sudo PYTHONWARNINGS=ignore pip3 install --upgrade Jinja2
# Make sure setuptools are installed correctly.
sudo apt install -y python3-testresources
pip3 install setuptools --upgrade

echo "Installing required python modules."
pip3 install paramiko pyyaml jinja2 markupsafe MySQL-python

echo "Installing Ansible."
pip3 install ansible

echo 'Finished installing ansible'
