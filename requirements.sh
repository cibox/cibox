#!/usr/bin/env sh

# Check all strict requirements.
help="Visit http://docs.cibox.tools/en/latest/Requirements for more information."
ansible=$(which ansible)

if [ -z ${ansible} ]; then
  echo "Ansible not installed. Further interaction is not possible. ${help}"
  exit 1
fi

ansible_version_required="1.9.4"
# Resulting output of "ansible --version" will be something like this:
# ansible 2.1.2.0
#   config file =
#   configured module search path = Default w/o overrides
# Use the second column from first row.
ansible_version_current=$(${ansible} --version | head -1 | awk '{print $2}')

if ! echo ${ansible_version_current} | grep ${ansible_version_required} >/dev/null; then
  echo "You have installed Ansible ${ansible_version_current}, but CIBox requires ${ansible_version_required}. ${help}"
  exit 2
fi
