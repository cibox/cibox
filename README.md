CIBox (Continuous Integration Box)
=====

[RoadMap for CIBOX](https://github.com/propeoplemd/cibox/wiki/RoadMap)

Current repo powered with https://github.com/willthames/ansible-lint checker for making playbooks closer to standard.
Feel free to check your code before creating PRs for increasing speed of code review.

Wiki https://github.com/propeoplemd/cibox/wiki

Introduction video http://youtu.be/EJZcqTcycf0

This repo consists basically from two playbooks
- CI server installation/provisioning jenkinsbox.yml
- github.yml repo builder with drupal, vagrant, puppet, drupal pp installation profile, scripts for reinstalling and sniffing with sniffers

You have to use *64bit* Ubuntu 12.04 LTS or 14.04 LTS systems for CI server

Possible variations
=====

Currently jenkinsbox.yml playbook powered with tags, so you can run only part of it.

Install SOLR only
=====

```sh
ansible-playbook jenkinsbox.yml --tags "ansible-jetty-solr"
```

Install php stack.
=====

```sh
ansible-playbook jenkinsbox.yml --tags "php-stack"
```

Other tags self-explained
=====

run them as
```sh
ansible-playbook jenkinsbox.yml --tags "TAGNAME"
```

- ansible-jenkins
- ansible-composer
- ansible-php-pear
- ansible-sniffers
- apache
- mysql
- cibox-mysql-config
- cibox-swap
- cibox-ssl-config


OpenVZ support
=====

If your system build on OpenVZ stack and swap is disabled you may bypass
it with using fakeswap.sh file within files/fakeswap directory

for use this script run on remote system console
```sh
chmod a+x fakeswap.sh
```
and
```sh
sh ./fakeswap.sh 4086
```
for adding 4086MB swap size
