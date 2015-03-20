How to reinstall your site
=====

"Re-installation" here means not only using profile and installations site from scratch
but this means that you may use the reinstall.yml playbook as a tool for obtaining
dev or stage or possible production environment locally.

Keep in mind that this script used by Jenkins server for getting Pull Request builds.
By overriding variables, that you can find within current playbook you can run it
with different conditions. These conditions can pre configure your drupal based software
- beed installed from scratch by using profile
- pre configure your drupal site by using extra provided database dump via url

So you should think about this script as universal tool, or may be better wording = application
that can do a lot of things to get your drupal based software pre configured from scratch.

Possible configurations
=====

Pretty standard way of running this playbook it to try defaults.

example
```sh
ansible-playbook reinstall.yml
```
creates drupal installation from scratch by using ```pp``` preinstalled drupal's profile
been shipped within [CIBOX](https://github.com/propeoplemd/cibox) suite.

What are defaults here?
=====

- installation_profile_name: pp
This variable talk the meaning itself - it can be overridden by
```sh
ansible-playbook reinstall.yml --extra-vars="installation_profile_name=minimal"
```
which helps you to get reinstalled drupal by using minimal drupal's standard profile.
Every variable, shipped with CIBOX suite can be overridden by the way above, just pass
them divided by whitespaces one after each other in a raw within --extra-vars command line key.

Widely used variables
=====

There are a lot of variables within CIBOX suite that can be overridden by Jenkins shell scripts
or by users due to requirements of the projects.
But here is a list of predefined variables that changed or overridden mostly every new project up.
Keep in mind we are talking about reinstall.yml playbook only.

- mysql_db: - This variable stores name of database, been used in a project for drupal site.
 This variable can be changed only when you need to enpower your Jenkins instance with more than
 one PR/DEMO/STAGE builders. You'll need this if the project contains 2,3 or even more sites and
 you need to split sites for been used unique databases for avoiding conflicts.
 
- php_env_vars: - should be used when you need pass to your server's environment variable for 
 marking your environment as development. Useful for limiting large imports or background processes
 for speedup development and builds.

- site_url: - variable for passing to drush commands or any other url dependant commands.

- pp_environment: - This is very useful and important variable that makes everything complicated.
 There are states: default, demo, staging and production for this variable that used for splitting
 reinstall.yml to be used as conditional single playbook for ability to get different development
 environments.
 * Every user and developer should think about reinstall.yml during extending it like:
 this script can change almost every environment within a project workflow, starting from dev, ending
 with production server. So if you want to change something you should add condition for which
 environment this change should be. *

- workflow_type: - currently CIBOX suite enpowered with two types of possible workflows - ```profile```
 and ```sql``` based. 
 
 Profile based workflow will do reinstall drupal site from scratch using predefined
 profile from ```installation_profile_name``` variable. 
 
 Sql based workflow will prepare the site from database dump, that is obtained during process from 
 ```source_database``` variable. There is a ```make_backup``` boolean variable for setting ability to
 create backups alongside processing playbook.
 
Best Practices and rules.
====

1. Every developer should extend, change this document when variables above changed, renamed, appended or 
removed for some reason.
2. Every reviewer should check that point 1 was done during PR to CIBOX repo.

Please, do not add any features without adding comments to reinstall.yml and this documentation.