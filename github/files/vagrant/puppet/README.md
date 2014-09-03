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

