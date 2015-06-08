#!/usr/bin/env bash
export PYTHONUNBUFFERED=1
EXTRA_VARS=''
if [ -z '$1' ] && [ $1 == *"—-windows"* ]; then
  EXTRA_VARS='—extra-vars=“is-windows=true”'
fi
time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local ${EXTRA_VARS}
