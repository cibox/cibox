#!/bin/sh

#
# Profile Config Cleaner
#
# This script helps to maintain configuration for Drupal8. The main idea of this script is to
# avoid routine handwork of selecting appropriate config files and copying them to appropriate
# directories (profile directory for profile flow or staging directory for sql flow).
#
# How to work with the script?
#
# Prepare your environment:
# 1. Please, have a look in config_clean.yml file.
# 2. Set your installation profile variable.
# 3. Set a path to your staging directory.
# 4. Add or remove config files you want to exclude from configuration.
#
# Export configuration:
# 1. Make sure you've committed you profile config directory or you've got a backup!
# 2. Run config_clean.sh
# 3. Run git status
# 4. Check your files. Probably, you'll need to remove some redundant files. If so, please, add these files to the
#    config_clean.yml exclude variable.
#
# Use the next command to get all *.yml files in directory
# find . -type f -name "*.yml" | grep 'config/install/' | grep -o '[^/]*yml$' | awk '{print "- { name: \""$1"\", status: true }"}'
#
# We recommend you to add all available configs from core, modules and themes using the command mentioned above before starting
# to work on the project. Then just exclude config files from `config_clean.yml` which you want to use.


if [ "$1" = "--windows" ]; then
    time ansible-playbook -vvvv config_clean.yml -i 'localhost,' --connection=local --extra-vars "is_windows=true"
else
    time ansible-playbook -vvvv config_clean.yml -i 'localhost,' --connection=local
fi