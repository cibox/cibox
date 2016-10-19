# -*- mode: ruby -*-
# vi: set ft=ruby :

# Builder and provisioner box definition.

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/xenial64"
  config.vm.provision "shell", path: "builder-setup.sh"
end
