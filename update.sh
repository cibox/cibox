#!/usr/bin/env bash

if [ "$*" == "" ]; then
    echo "No arguments provided"
    exit 1
fi
# From http://stackoverflow.com/a/14203146/3027445
for i in "$@"
do
case $i in
    -s=*|--subsystem=*)
    SUBSYSTEM="${i#*=}"
    shift # past argument=value
    ;;
    -p=*|--vm-path=*)
    VMPATH="${i#*=}"
    shift # past argument=value
    ;;
    --default)
    DEFAULT=YES
    shift # past argument with no value
    ;;
    *)
            # unknown option
    ;;
esac
done
echo "FILE EXTENSION  = ${EXTENSION}"
echo "VMPATH PATH     = ${VMPATH}"
echo "SUBSYSTEM       = ${SUBSYSTEM}"
echo "DEFAULT         = ${DEFAULT}"

if [ ${SUBSYSTEM} = 'host' ]
then
sudo ansible-playbook update.yml --tags="host"
fi
if [ ${SUBSYSTEM} = 'vm' ] && [ ${VMPATH} != ''  ]
then
sudo ansible-playbook update.yml --tags="${SUBSYSTEM}" --extra-vars="vm_path=${VMPATH}"
fi

if [[ -n $1 ]]; then
    echo "Last line of file specified as non-opt/last argument:"
    echo $1
fi