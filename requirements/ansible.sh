#!/bin/sh

# Determine whether ansible is installed or not.
info="Please install ansible 1.9.*. Visit http://docs.cibox.tools/en/latest/Requirements for more information."
is_ansible_installed=false
ansible --version >/dev/null 2>&1 && { is_ansible_installed=true; }
if [ "$is_ansible_installed" = false ]; then
  echo ${info}
  exit 1
fi

# If ansible is installed but version isn't 1.9.* then interrupt script execution.
is_ansible_1_9=$(ansible --version | grep "1.9")
if [ -z "$is_ansible_1_9" ]; then
  echo "Current ansible version is not supported by CIBOX.\n"${info}
  exit 1
fi
