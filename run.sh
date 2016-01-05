#!/bin/sh

export PYTHONUNBUFFERED=1

# As it appears parsing YAML with bash is not so easy task. The next function
# parses simple YAML configurations files.
#
# Example YAML file (test.yml):
# item_one: value1
# item_two:
#   sub_item: value3
#
# To parse the values you should use the next syntax:
# eval $(yaml_parser test.yml "config_")
#
# Getting items:
# echo $config_item_one
#
# To get nested items:
#
# echo $config_item_two_sub_item
#
# Note: There is "shyaml" python script to read YAML files, but we will probably
# not use it in order not to have additional dependcies.
yaml_parser() {
   local prefix=$2
   local s='[[:space:]]*' w='[a-zA-Z0-9_]*' fs=$(echo @|tr @ '\034')
   sed -ne "s|^\($s\)\($w\)$s:$s\"\(.*\)\"$s\$|\1$fs\2$fs\3|p" \
        -e "s|^\($s\)\($w\)$s:$s\(.*\)$s\$|\1$fs\2$fs\3|p"  $1 |
   awk -F$fs '{
      indent = length($1)/2;
      vname[indent] = $2;
      for (i in vname) {if (i > indent) {delete vname[i]}}
      if (length($3) > 0) {
         vn=""; for (i=0; i<indent; i++) {vn=(vn)(vname[i])("_")}
         printf("%s%s%s=\"%s\"\n", "'$prefix'",vn, $2, $3);
      }
   }'
}

# Parse the config.
eval $(yaml_parser config.yml "config_")

# Here we are replacing items in various configuration files.
# The idea to use unique markers, which probably we could automate in the
# future. For now let's use: __CHANGE__variable_name__

# File "hosts".
sed -i s/__CHANGE__server_ip__/$config_server_ip/g hosts

# File "jenkinsbox.yml".
sed -i s/__CHANGE__project_name__/$config_project_name/g jenkinsbox.yml

# Finally, run the playbook.
# Also we could pass some data from config.yml here as extra vars:
# ansible-playbook -vvvv jenkinsbox.yml --extra-vars "project_name=$config_project_name"
ansible-playbook -vvvv jenkinsbox.yml
