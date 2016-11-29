#!/bin/sh

# Determine whether Jinja2 is installed or not.
info="Please install Jinja2 (v2.8 dev). Visit http://docs.cibox.tools/en/latest/Requirements for more information."
is_jinja2_installed=false
pip show Jinja2 >/dev/null 2>&1 && { is_jinja2_installed=true; }
if [ "$is_jinja2_installed" = false ]; then
  echo ${info}
  exit 1
fi

# If Jinja2 is installed but version isn't 2.8 then interrupt script execution.
is_jinja2_2_8=$(pip show Jinja2 | grep "2.8")
if [ -z "$is_jinja2_2_8" ]; then
  echo "Current Jinja2 version is not supported by CIBOX.\n"${info}
  exit 1
fi
