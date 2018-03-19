# -*- mode: ruby -*-
# vi: set ft=ruby :
VAGRANTFILE_API_VERSION = "2"

require "yaml"

dir = File.dirname(File.expand_path(__FILE__))
configValues = YAML::load_file("#{dir}/provisioning/config.yaml")
apacheConfigValues = YAML::load_file("#{dir}/provisioning/ansible/playbooks/www.yml")
data = configValues["vagrantfile-local"]
provider = (ARGV.include?("--provider=lxc") || !data["vm"]["provider"].key?("virtualbox")) ? "lxc" : "virtualbox"


Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  config.vm.box = "#{data['vm']['provider'][provider]['box']}"
  config.vm.box_url = "#{data['vm']['provider'][provider]['box_url']}"

  if data['vm']['hostname'].to_s != ''
    config.vm.hostname = "#{data['vm']['hostname']}"
  end

  # If hostsupdater plugin is installed, add all servernames as aliases.
  if Vagrant.has_plugin?("vagrant-hostsupdater")
    config.hostsupdater.aliases = []
    for host in apacheConfigValues[0]['vars']['apache_vhosts']
      # Add all the hosts that aren't defined as Ansible vars.
      unless host['servername'].include? "{{"
        config.hostsupdater.aliases.push(host['servername'])
      end
    end
  end

  if data['vm']['network']['private_network'].to_s != ''
    config.vm.network "private_network", ip: "#{data['vm']['network']['private_network']}", lxc__bridge_name: "lxcbr0"
  end

  data['vm']['network']['forwarded_port'].each do |i, port|
    if port['guest'] != '' && port['host'] != ''
      config.vm.network :forwarded_port, guest: port['guest'].to_i, host: port['host'].to_i, auto_correct: true
    end
  end

  for folder in data['vm']['synced_folder'];
    nfs = (folder['nfs'] == "true") ? "nfs" : nil
    config.vm.synced_folder folder['source'], folder['target'],
      id: folder['id'],
      type: nfs,
      :nfs => true,
      :linux__nfs_options => ['rw','no_subtree_check','no_root_squash','async'],
      rsync__auto: "true",
      rsync__exclude: folder['excluded_paths'],
      rsync__args: ["--verbose", "--archive", "--delete", "-z", "--chmod=ugo=rwX"]
  end

  config.vm.usable_port_range = (10200..10500)

  if !data['vm']['provider']['virtualbox'].empty?
    config.vm.provider :virtualbox do |virtualbox|
      data['vm']['provider']['virtualbox']['modifyvm'].each do |key, value|
        if key == "natdnshostresolver1"
          value = value ? "on" : "off"
        end
        virtualbox.customize ["modifyvm", :id, "--#{key}", "#{value}"]
      end
    end
  end

  # Do a provision only if it manually requested ("vagrant provision" executed).
  # https://github.com/mitchellh/vagrant/issues/7043#issuecomment-197935659
  if ARGV.include?("provision") || ARGV.include?("--provision")
    config.vm.provision :shell,
      :path => "provisioning/shell/initial-setup.sh",
      :args => "/vagrant/provisioning"

    # Install ansible inside the box.
    config.vm.provision :shell, keep_color: true,
      :path => "provisioning/shell/ansible.sh"
    # Install ansible playbooks inside the box.
    config.vm.provision :shell, keep_color: true,
      :path => "provisioning/ansible/run-ansible-playbook.sh"
  end

  # Install drupal within vm for testing.
  if !ENV['VAGRANT_CI'].nil?
    config.vm.provision :shell, keep_color: true, :path => "provisioning/ansible/run-drupal-playbook.sh"
  end

  config.ssh.username = "vagrant"
  config.ssh.password = "vagrant"
  config.ssh.shell = "sh"
  config.ssh.insert_key = "false"
  config.ssh.forward_agent = true
end
