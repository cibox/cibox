How to reinstall your site
=====

"Re-installation" here means not only using profile and installations site from scratch
but this means that you may use the reinstall.yml playbook as a tool for obtaining
dev or stage or even production environment locally.

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

- ```mysql_db```: - This variable stores name of database, been used in a project for drupal site.
 This variable can be changed only when you need to enpower your Jenkins instance with more than
 one PR/DEMO/STAGE builders. You'll need this if the project contains 2,3 or even more sites and
 you need to split sites for been used unique databases for avoiding conflicts.
 
- ```php_env_vars```: - should be used when you need pass to your server's environment variable for
 marking your environment as development. Useful for limiting large imports or background processes
 for speedup development and builds.

- ```site_url```: - variable for passing to drush commands or any other url dependant commands.

- ```pp_environment```: - This is very useful and important variable that makes everything complicated.
 There are states: default, demo, staging and production for this variable that used for splitting
 reinstall.yml to be used as conditional single playbook for ability to get different development
 environments.

- ```workflow_type```: - currently CIBOX suite enpowered with two types of possible workflows - ```profile```
 and ```sql``` based. 
 
 Profile based workflow will do reinstall drupal site from scratch using predefined
 profile from ```installation_profile_name``` variable. 
 
 Sql based workflow will prepare the site from database dump, that is obtained during process from 
 ```source_database``` variable. There is a ```make_backup``` boolean variable for setting ability to
 create backups alongside processing playbook.
 
- ```backup_user```, ```backup_pass``` and ```source_database```: - these variables should be used on Jenkins server for
pointing out to right place where sql dump archive stored by Jenkins by cron or any other background app. Keep in mind that backup should be accessible for webserver via url, injected by ```source_database```variable.
 
- ```ci_server_username```: - name of the user that executes playbooks on  CI server. In most cases should be **jenkins**.

- ```rebuild_registry```: - variable to display whether Drupal registry should be rebuilt. By default this variable is ```false```. If any module has been moved to new directory, this variable should be set to ```true```. After release to production/staging this variable should be set back to ```false```.

- ```local_backup_environment```: - variable to specify source/primary CI server environment for backups. Also set ```make_backup``` to ```true``` to create backup from this environment during reinstall.

- ```origin_site_url```: - URL of the site that provides all assets(images, files and etc.) for installations where ```stage_file_proxy_origin``` module is enabled.

Configuration per environment
=====
CIbox provides flexible configuration per environment. This means that you are able configure specific settings, drush commands and modules that should be added/executed on your environments.

- ```global_env.yml``` - global configuration that will be used widely on all environments.
- ```default_env.yml``` - cofiguration for default environment (PR builds and vagrant instances).
- ```demo_env.yml``` - cofiguration for demo environment.
- ```staging_env.yml``` - cofiguration for staging environment.
- ```production_env.tml``` - cofiguration for production environment.

### Environment configuration example

```yml
---
env:
  # Settings that will be added to settings.php before installation(works only for "SQL workflow").
  pre_settings:
    - { name: '$conf["error_level"]', status: true, value: '2' }
    - { name: '$conf["cache"]', status: true, value: '0' }
    - { name: '$conf["block_cache"]', status: true, value: '0' }
    - { name: '$conf["preprocess_css"]', status: true, value: '0' }
    - { name: '$conf["preprocess_js"]', status: true, value: '0' }

  # Modules that will be enabled after installation.
  modules:
    - { name: 'dblog', status: true }
    - { name: 'stage_file_proxy', status: true }
    - { name: 'devel', status: true }
    - { name: 'views_ui', status: true }
    - { name: 'diff', status: true }

  # Drush commands that will be executed after installation.
  drush_commands:
    # Update admin username and password.
    - { name: 'sqlq', status: true, arguments: "\"UPDATE users SET name='{{ drupal_user }}' WHERE uid=1\"" }
    - { name: 'user-password', status: true, arguments: "{{ drupal_user }} --password={{ drupal_pass }}" }

  # Settings that will be added to settings.php after installation.
  post_settings:
    - { name: '$conf["stage_file_proxy_origin"]', status: true, value: "{{ origin_site_url }}" }

```

### Configuration status

Each configuration has ```status``` attribute. This attribute displays whether configuration is enabled or disabled.

Enabled configuration:

```
- { name: '$conf["error_level"]', status: true, value: '2' }
```

Disabled configuration:

```
- { name: '$conf["error_level"]', status: false, value: '2' }
```

Switching from profile to sql flow
=====

Typically every new project, developed from scratch starts from Drupal profile creation.
This means teams should use ```workflow_type: profile``` variable state for reinstalling a site by
using profile, predefined in ```installation_profile_name``` variable.
But every project has a phase, where content managers start adding content and reinstall from profile
becomes non convenient for developers. Thats why there is ```sql``` flow predefined within reinstall.yml
playbook.
For switching to ```sql``` flow type, there are steps to be done for making it working.
- change to ```workflow_type: sql```
- create regular SQL db dumper from staging or production environment, depending of the flow complexity.
 There is a BACKUP_PROD_DB job shipped by CIBOX suite for boilerplate example how to approach db dump.
 The idea behind this to get dump periodically (daily, hourly), depending of project requirements and
 to put it to webserver folder for ability to download from any environment and to use for DEV, PR, STAGE
 or eny other environments.
- Depending of flow complexity you might want to have more than one DB dumpers - one for preparing dev
 environment by using staging database, second one for preparing staging environment by using production
 database. For approaching this you should add secondary DB DUMP job with appropriate configs and extend
 reinstall.yml for using respective dumps for different ```pp_environment``` states. 

 
Best Practices and rules.
====

1. Every developer should extend, change this document when variables above changed, renamed, appended or 
removed for some reason.
2. Every reviewer should check that point 1 was done during PR to CIBOX repo.

Please, do not add any features without adding comments to reinstall.yml and this documentation.
