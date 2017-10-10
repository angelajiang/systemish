#!/bin/bash
dir=`dirname $0`
for file in bash_profile gdbinit gitconfig vimrc vim; do
  cp -r $dir/$file ~/.$file
done
