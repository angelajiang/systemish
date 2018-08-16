#!/bin/bash

# Initial setup commands. These must be run with user = akalia (by default
# CloudLab runs startup scripts as geniuser). Doing so in a bash loop is hard,
# so just write down all commands.

# Create hugepages
sudo bash -c "echo 4096 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages"

# Add a symlink to NFS directories in the homedir
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/setup-cloudlab.sh /users/akalia/setup-cloudlab.sh'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/systemish /users/akalia/systemish'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/eRPC /users/akalia/eRPC'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/raft /users/akalia/raft'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/rdma_bench /users/akalia/rdma_bench'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/dpdk /users/akalia/dpdk'

# Symlink large configs
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/vim /users/akalia/.vim'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/fzf /users/akalia/.fzf'
sudo su akalia -c 'ln -s /proj/ron-PG0/akalia/bash_history /users/akalia/.bash_history'

# Install fzf
sudo su akalia -c '/users/akalia/.fzf/install --key-bindings --completion --update-rc'

# Copy small configs directly from systemish
sudo su akalia -c 'cp /proj/ron-PG0/akalia/systemish/configs/bash_profile /users/akalia/.bash_profile'
sudo su akalia -c 'cp /proj/ron-PG0/akalia/systemish/configs/vimrc /users/akalia/.vimrc'
sudo su akalia -c 'cp /proj/ron-PG0/akalia/systemish/configs/gitconfig /users/akalia/.gitconfig'
sudo su akalia -c 'cp /proj/ron-PG0/akalia/systemish/configs/gdbinit /users/akalia/.gdbinit'

# Copy ssh keys
sudo su akalia -c 'cp /proj/ron-PG0/akalia/ssh_keys/* /users/akalia/.ssh/'
sudo su akalia -c 'cat /users/akalia/.ssh/id_rsa.pub >> /users/akalia/.ssh/authorized_keys'

# To install in next image
sudo apt -y install numactl apcalc sloccount nmap
sudo /proj/ron-PG0/akalia/raft/install.sh

sudo add-apt-repository -y ppa:ahasenack/nvdimm-update
sudo apt -y install libpmem-dev
