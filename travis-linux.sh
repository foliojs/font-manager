#!/bin/bash

# make sure we install the new version of libfontconfig1-dev
sudo apt-add-repository 'deb http://archive.ubuntu.com/ubuntu trusty main restricted universe multiverse'
sudo apt-get -yqq update
sudo apt-get install -y -t trusty libfontconfig1-dev

# install some fonts needed for the tests
sudo apt-get install -y fonts-droid fonts-liberation
