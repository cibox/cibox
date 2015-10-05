cibox-drush
=============

Ansible role to configure [Drush](https://github.com/drush-ops/drush).

Role Variables
--------------

* **drushrc**: The content of the global Drush configuration file
  (/etc/drush/drushrc.php) as a dict.
* **drush_install_mode**: Install mode for, one of
    * ```composer``` (default): Install Drush using the Composer gobal require
    command.
    * ```git```: Install Drush from Git repo (following the
    [INSTALL - MANUAL](https://github.com/drush-ops/drush) instructions).
    * ```pear```: Install Drush using PEAR.
    * ```none```: Do not install Drush.
* **drush_composer_bin**: Path the the composer binary used for the
  ```composer``` and ```git``` installation modes.
* **drush_composer_version**: The version to install when using the
  ```composer``` installation mode, defaults to ```6.*```.
* **drush_composer_user**: The user to run Composer global require command as
  when using the ```composer``` installation mode, defaults to ```vagrant```.
* **drush_git_repo**: The URL of the Git repository to checkout Drush from when
  using the ```git``` installation method, defaults to
  https://github.com/drush-ops/drush.git
* **drush_git_version** The version of the repository to check out when using
  the ```git``` installation method. This can be the full 40-character SHA-1
  hash of a commit, the literal string HEAD, a branch name, or a tag name.
  Defaults to '6.2.0'.
* **drush_git_dest**: Path where to checkout Drush when using the ```git```
  installation method, default to ```/opt/drush```.
* **drush_git_bin**: The path for a symlink to the drush binary that will be
  created when using the ```git``` installation method. Make sure the directory
  containing the symlink is on the system PATH.
* **drush_pear_packages**: The list of PEAR package to install when using the
  ```pear``` installation method, defaults to ```['drush/drush','Console_Table']```.
* **drush_pear_channels**: The list of PEAR channels to discover when using the
  ```pear``` installation method, defaults to
  ```['pear.drush.org']```.

Example usage
-------------

    - role: drush
      drush_install_mode: git
      drushrc:
        root: /var/www
        uri: www.example.com


Requirements
------------

* Composer installed globally, if using the ```git``` or ```composer```
  installation mode.
* Pear, if using the ```pear``` installation mode.
* Make sure Composer's global bin directory is on the system PATH if using the
  ```composer``` installation mode.

TODOs
-----

* Setup drush.complete.sh for bash completion for PEAR installation.
* Setup sites aliases.

License
-------

Apache v2

Author Information
------------------

Pierre Buyle <buyle@pheromone.ca>
