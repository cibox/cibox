drupal project
======

Drupal project is for SPS

For obtaining development environment locally You should install vagrant and virtualbox or lxc and from console ran
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

Tools
=====

PHP Profiler XHProf
It is installed by default, but to use it as Devel module integration use:
```sh
drush en devel -y
drush vset devel_xhprof_enabled 1
drush vset devel_xhprof_directory '/usr/share/php' && drush vset devel_xhprof_url '/xhprof_html/index.php'
ln -s /usr/share/php/xhprof_html xhprof_html
```
After `vset devel_xhprof_enabled` it could return an error about "Class 'XHProfRuns_Default' not found" - ignore it.


Linux Containers
=====

When your system enpowered with linux containers(lxc), you can speedup a lot of things by
using them and getting rid of virtualization.
For approaching lxc, please install vagrant plugin

```sh
vagrant plugin install vagrant-lxc
apt-get install redir lxc cgroup-bin
```
also you may need to apply this patch https://github.com/fgrehm/vagrant-lxc/pull/354

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
