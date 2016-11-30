#!/usr/bin/env sh

# Check all strict requirements.
for file in ./requirements/*.sh
do
  if [ -e "$file" ]; then
    . ${file}
  fi
done
