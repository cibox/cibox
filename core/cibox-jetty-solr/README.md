Ansible php-pear
====================

Ansible role to install PEAR and PEAR packages on a Debian-based system.

Example usage:

    - role: php-pear
      pear_channels:
        - pear.phing.info
      pear_packages:
        - phing/phing
        - HTTP_Request2

Role Variables
--------------

* **pear_channels**: The list of PEAR channels to discover.
* **pear_packages**: The list of PEAR packages to install.

License
-------

Apache v2

Author Information
------------------

Pierre Buyle <buyle@pheromone.ca>
