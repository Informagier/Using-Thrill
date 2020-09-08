Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"

  config.vm.provider "virtualbox" do |v|
    v.name = "Thriller"
    v.memory = 32 * 1024
    v.cpus = 12
  end

  config.vm.provision "shell", path: "Vagrant/provision.sh"

end