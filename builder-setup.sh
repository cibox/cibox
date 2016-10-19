#!/bin/bash
# Create .ssh folder if it doesn't exist.
[[ -d /home/ubuntu/.ssh ]] || mkdir /home/ubuntu/.ssh
# Copy pre-generated keys.
cp /vagrant/ssh-keys/* /home/ubuntu/.ssh/
sudo apt-get -y install software-properties-common
# Uncomment the following 2 lines when using Ubuntu 14.04, to get python-2.7.9
#wget https://gist.githubusercontent.com/drjerry/06041e0d78935e9d0b9e/raw/5af1922a92e2cd9eb2fb4e620191788ea2c66b67/build-python-2.7.9.sh
#sudo sh build-python-2.7.9.sh
sudo apt-add-repository ppa:ansible/ansible-1.9
sudo apt-get -y update
sudo apt-get -y install ansible python-pip
sudo pip install jinja2
