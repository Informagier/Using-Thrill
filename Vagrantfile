Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"

  config.vm.provider "virtualbox" do |v|
    v.name = "ThrillBox"
    v.memory = 6 * 1024
    v.cpus = 8
  end

  config.vm.provision "shell", path: "provision.sh"

end