Drupal Vagrant Dev box for CIBox support
======

#Installation
* [Vagrant](https://www.vagrantup.com/downloads.html)
* [VirtualBox](https://www.virtualbox.org/wiki/Downloads)
* Useful Vagrant's plugins
  * [Hosts Updater](https://github.com/cogitatio/vagrant-hostsupdater)
  * [vbguest](https://github.com/dotless-de/vagrant-vbguest)


#Usage

```sh
vagrant up && vagrant ssh
```

If you need to rerun provisioning

```sh
FORCE_PROVISION=yes vagrant up && vagrant ssh
```

**Drupal reinstallation from scratch**

Unix users
```sh
sh reinstall.sh
```
Windows users
```sh
sh reinstall.sh --windows
```
By default your site will be accessible by using this url. 

```
http://drupal.192.168.56.132.xip.io/
```


If ```xip.io``` not working - create row with

```hosts
192.168.56.132 drupal.192.168.56.132.xip.io
```

in ```/etc/hosts``` or just use another ServerName in apache.yml

If you have Vagrant HostUpdater plugin, your hosts file will be automatically updated.

VirtualBox additions
=====
For automatic update additions within guest, please install proper plugin

```sh
vagrant plugin install vagrant-vbguest
```


Tools
=====

* XDebug
* Drush
* Selenium 2
* Composer
* Adminer
* XHProf
* PHP Daemon
* PHP, SASS, JS sniffers/lints/hints

##Adminer
Adminer for mysql administration (credentials drupal:drupal and root:root)

```
http://192.168.56.132.xip.io/adminer.php
```

##PHP Profiler XHProf
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

When your system empowered with linux containers(lxc), you can speedup a lot of things by
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


Windows Containers
=====

Install [Cygwin](https://servercheck.in/blog/running-ansible-within-windows) according to provided steps.

Run Cygwin as Administrator user.

Use default flow to up Vagrant but run `sh reinstall.yml --windows`

##Windows troubleshooting

If you will see error liek ```...[error 26] file is busy...``` during ```sh reinstall.sh``` modify that line:

before

```yml
name: Stage File Proxy settings
sudo: yes
lineinfile: dest='sites/default/settings.php' line='$conf[\"stage_file_proxy_origin\"] = \"{{ stage_file_proxy_url }}";'
```

after:

```yml
name: Copy settings.php
sudo: yes
shell: cp sites/default/settings.php /tmp/reinstall_settings.php

name: Stage File Proxy settings
sudo: yes
lineinfile: dest='sites/default/settings.php' line='$conf[\"stage_file_proxy_origin\"] = \"{{ stage_file_proxy_url }}\";'

name: Restore settings.php
sudo: yes
shell: cp /tmp/reinstall_settings.php sites/default/settings.php
```

MacOS users
=====

Install vagrant
```
https://www.vagrantup.com/downloads.html
```

Install Virtualbox from here:
```
https://www.virtualbox.org/wiki/Downloads
```

cd to project folder

```
vagrant up && vagrant ssh
```

You will be logged into virtual machine.

Go to this path

```
/vagrant/docroot
```

and run the script

```
sh reinstall.sh
```

for drupal reinstall from scratch.

Configure Stage File Proxy to use the files from the correct source.
