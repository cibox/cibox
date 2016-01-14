CIBox updates
=====

CIBox is a complex system in terms of dependencies from external tools and libraries.
This document aims to help you to get updated previous versions of CIBox subsystems.

For running updates you need to execute update.sh with a root permissions

```sh
sudo update.sh
```

For updating your host machine, where you are using Vagrant, run a command

```sh
sudo update.sh -s=host
```

For updating your CIBox's Vagrant machine

```sh
vagrant up && vagrant ssh
cd /var/www/
git clone --branch master https://github.com/propeoplemd/cibox.git
cd cibox
sudo ./update.sh -s=host
cd .. && rm -rf cibox
```

For updating non provisioned or failing CIBox's Vagrant VM <- todo

```sh
git clone --branch master https://github.com/propeoplemd/cibox.git
cd cibox
sudo ./update.sh -s=vm -p=/path/to/Vagrantfile_folder
```
