# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # config.vm.box = "bento/ubuntu-16.04"
  config.vm.box = "bento/ubuntu-18.04"
  # config.vm.box = "generic/ubuntu1804"
  config.vm.box_check_update = false
  config.ssh.insert_key = false

  config.vm.provider "virtualbox" do |vb|
    vb.default_nic_type = "virtio"
    vb.cpus = 4
    vb.memory = 4096
  end
end
