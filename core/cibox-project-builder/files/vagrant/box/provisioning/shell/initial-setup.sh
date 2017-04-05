#!/bin/bash

VAGRANT_CORE_FOLDER=$(echo "$1")
export DEBIAN_FRONTEND=noninteractive
export PYTHONWARNINGS=ignore
export PYTHONUNBUFFERED=1
export ANSIBLE_FORCE_COLOR=true
# Making a world a bit more perfect in order to continue provision due to possible locks...
sudo killall apt-get 2> /dev/null
sudo killall dpkg 2> /dev/null
sudo dpkg --configure -a || true

# CIBox ad.
cat "${VAGRANT_CORE_FOLDER}/shell/self-promotion.txt"

OS=$(/bin/bash "${VAGRANT_CORE_FOLDER}/shell/os-detect.sh" ID)
CODENAME=$(/bin/bash "${VAGRANT_CORE_FOLDER}/shell/os-detect.sh" CODENAME)

if [[ ! -d /.puphpet-stuff ]]; then
    mkdir /.puphpet-stuff

    echo "${VAGRANT_CORE_FOLDER}" > "/.puphpet-stuff/vagrant-core-folder.txt"

    echo "Created directory /.puphpet-stuff"
fi

if [[ ! -f /.puphpet-stuff/initial-setup-repo-update ]]; then
    if [ "${OS}" == 'debian' ] || [ "${OS}" == 'ubuntu' ]; then
        echo "Running initial-setup apt-get update"
        apt-get update >/dev/null
        touch /.puphpet-stuff/initial-setup-repo-update
        echo "Finished running initial-setup apt-get update"
    fi
fi

if [[ "${OS}" == 'ubuntu' && ("${CODENAME}" == 'lucid' || "${CODENAME}" == 'precise') && ! -f /.puphpet-stuff/ubuntu-required-libraries ]]; then
    echo 'Installing basic curl packages (Ubuntu only)'
    apt-get install -y libcurl3 libcurl4-gnutls-dev curl >/dev/null
    echo 'Finished installing basic curl packages (Ubuntu only)'

    touch /.puphpet-stuff/ubuntu-required-libraries
fi
