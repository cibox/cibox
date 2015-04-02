# Ansible Role: PHP-XHProf

[![Build Status](https://travis-ci.org/geerlingguy/ansible-role-php-xhprof.svg?branch=master)](https://travis-ci.org/geerlingguy/ansible-role-php-xhprof)

Installs PHP [XHProf](http://php.net/manual/en/book.xhprof.php) on Linux servers.

## Requirements

None.

## Role Variables

Available variables are listed below, along with default values (see `defaults/main.yml`):

    TODO

## Dependencies

  - geerlingguy.php
  - geerlingguy.php-pecl

## Example Playbook

    - hosts: webservers
      roles:
        - { role: geerlingguy.php-xdebug }

## License

MIT / BSD

## Author Information

This role was created in 2014 by [Jeff Geerling](http://jeffgeerling.com/), author of [Ansible for DevOps](http://ansiblefordevops.com/).
