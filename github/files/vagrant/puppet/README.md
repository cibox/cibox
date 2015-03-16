drupal project
======

Drupal project is for SPS

For obtaining development environment locally You should install vagrant and puppet and from console ran
```sh
vagrant up && vagrant ssh
```
afterwards.
You will be logged into virtual machine.

use 
```sh
sh reinstall.sh
```
for drupal reinstallation from scratch

Virtual host for accessing ppdorg drupal installation 

```
http://drupal.192.168.56.112.xip.io
```

Adminer for mysql administration (credentials drupal:drupal and root:root)

```
http://192.168.56.112.xip.io/adminer.php
```


If ```xip.io``` not working - create row with

```hosts
192.168.56.112 drupal.192.168.56.112.xip.io
```

in ```/etc/hosts```

Linux Containers
=====

When your system enpowered with linux containers(lxc), you can speedup a lot of things by
using them and getting rid of virtualization.
For approaching lxc, please install vagrant plugin

```sh
vagrant plugin install vagrant-lxc
apt-get install redir lxc
```

When your system is enpowered by apparmor, you should enable nfs mounts for your host
machine
Do that by editing ```/etc/apparmor.d/lxc/lxc-default``` file with one line

```ruby
profile lxc-container-default flags=(attach_disconnected,mediate_deleted) {
  ...
    mount options=(rw, bind, ro),
  ...
```
and reload apparmor service
```sh
sudo /etc/init.d/apparmor reload
```


and run the box by command

```sh
VAGRANT_CI=yes vagrant up
```

Do use 
```
VAGRANT_CI=yes
```
environment variable, if you got issues with all vagrant commands.
