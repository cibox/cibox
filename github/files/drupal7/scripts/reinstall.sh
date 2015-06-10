#!/bin/sh
ENV_LIST="Linux Or Windows"
echo  "\033[1mAvailbale OS: ${ENV_LIST}:\033[0m"
echo  "\033[1mSelect your OS :\033[0m";
read answer
if  [ "$answer" = "Linux" -o "$answer" = "linux" -o "$answer" = "LINUX" ]; then
time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local
fi
if [ "$answer" = "Windows" -o "$answer" = "windows" -o "$answer" = "WINDOWS" ]; then
time ansible-playbook -vvvv reinstall.yml -i 'localhost,' --connection=local --extra-vars "is_windows=true"
fi
exit 1;