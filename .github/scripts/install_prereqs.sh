#!/bin/bash

# The MIT License (MIT)
# Copyright (c) 2024 dātma, inc™
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

set -e 

install_openssl3() {
  echo "Building openssl..."
  OPENSSL_PREFIX=${OPENSSL_PREFIX:-/usr/local}
  OPENSSL_VERSION=3.0.11
  if [[ ! -d $OPENSSL_PREFIX/include/openssl ]]; then
    pushd /tmp
    wget $WGET_NO_CERTIFICATE https://www.openssl.org/source/openssl-$OPENSSL_VERSION.tar.gz &&
      tar -xvzf openssl-$OPENSSL_VERSION.tar.gz &&
      cd openssl-$OPENSSL_VERSION &&
      if [[ $(uname) == "darwin" ]]; then
        ./Configure darwin64-$(uname -m)-cc no-tests no-shared -fPIC --prefix=$OPENSSL_PREFIX
      else
        CFLAGS=-fPIC ./config no-tests -fPIC --prefix=$OPENSSL_PREFIX --openssldir=$OPENSSL_PREFIX
      fi 
      make -j4 && sudo make install_sw && echo "Installing OpenSSL DONE"
    rm -fr /tmp/openssl*
    popd
  fi
  export OPENSSL_ROOT_DIR=$OPENSSL_PREFIX
  if [[ $(uname) == "Linux" ]]; then
    export LD_LIBRARY_PATH=$OPENSSL_PREFIX:$LD_LIBRARY_PATH
  fi
}

install_curl() {
  CURL_PREFIX=${CURL_PREFIX:-/usr/local}
  if [[ ! -f $CURL_PREFIX/libcurl.a ]]; then
    echo "Installing CURL into $CURL_PREFIX"
    pushd /tmp
    CURL_VERSION_=$(echo $CURL_VERSION | sed -r 's/\./_/g')
    wget https://github.com/curl/curl/releases/download/curl-$CURL_VERSION_/curl-$CURL_VERSION.tar.gz &&
    tar xzf curl-$CURL_VERSION.tar.gz &&
    cd curl-$CURL_VERSION &&
    ./configure --disable-shared --with-pic -without-zstd --with-ssl=$OPENSSL_PREFIX --prefix $CURL_PREFIX &&
      make && sudo make install && echo "Installing CURL DONE"
    rm -fr /tmp/curl
    popd
  fi
  if [[ $(uname) == "Linux" ]]; then
    export LD_LIBRARY_PATH=$CURL_PREFIX:$LD_LIBRARY_PATH
  fi
}

install_prereqs_for_macos() {
  HOMEBREW_NO_AUTO_UPDATE=1
  HOMEBREW_NO_INSTALL_CLEANUP=1
  # Use the uuid from framework
  brew list ossp-uuid &> /dev/null && brew uninstall ossp-uuid
  brew list cmake &>/dev/null || brew install cmake
  brew list automake &> /dev/null || brew install automake
  brew list pkg-config &> /dev/null || brew install pkg-config
  if [[ $1 == "release" ]]; then
    install_openssl3
  else
    brew list openssl@3 &> /dev/null || brew install openssl@3
    export OPENSSL_ROOT_DIR=$(brew --prefix openssl@3)
    brew list zstd &>/dev/null || brew install zstd
    brew list catch2 &>/dev/null || brew install catch2
  fi
}

install_prereqs_for_centos7() {
  sudo yum install -y centos-release-scl &&
    sudo yum install -y devtoolset-11 &&
    sudo yum install -y -q deltarpm &&
    sudo yum update -y -q &&
    sudo yum install -y -q epel-release &&
    sudo yum install -y -q which wget git &&
    sudo yum install -y -q autoconf automake libtool unzip &&
    sudo yum install -y -q cmake3 patch &&
    sudo yum install -y -q perl perl-IPC-Cmd &&
    sudo yum install -y -q libuuid libuuid-devel &&
    sudo yum install -y -q curl libcurl-devel
  if [[ $1 == "release" ]]; then
    source /opt/rh/devtoolset-11/enable
    install_openssl3
    install_curl
  elif [[ ! -d ~/catch2-install ]]; then
    INSTALL_DIR=~/catch2-install CATCH2_VER=v$CATCH2_VER $GITHUB_WORKSPACE/.github/scripts/install_catch2.sh
  fi
}

install_prereqs_for_ubuntu() {
  sudo apt-get update -qq
  sudo apt-get -y install cmake
  sudo apt-get -y install zlib1g-dev zstd
  sudo apt-get -y install libssl-dev uuid-dev libcurl4-openssl-dev zstd
  if [[ $1 == "release" ]]; then
    install_openssl3
    install_curl
  elif [[ ! -d ~/catch2-install ]]; then
    INSTALL_DIR=~/catch2-install CATCH2_VER=v$CATCH2_VER $GITHUB_WORKSPACE/.github/scripts/install_catch2.sh
  fi
}

case $(uname) in
  Linux )
    if apt-get --version >/dev/null 2>&1; then
      export DEBIAN_FRONTEND=noninteractive
      install_prereqs_for_ubuntu $1
    else
      CENTOS_RELEASE_FILE=/etc/centos-release
      if [[ ! -f $CENTOS_RELEASE_FILE ]]; then
        CENTOS_RELEASE_FILE=/etc/redhat-release
        if [[ ! -f $CENTOS_RELEASE_FILE ]]; then
          echo "Only Ubuntu and Centos are supported"
          exit -1
        fi
      fi
      if grep -q "release 7" $CENTOS_RELEASE_FILE; then
        install_prereqs_for_centos7 $1
      else
        cat $CENTOS_RELEASE_FILE
        echo "Only Centos 7 is supported"
        exit -1
      fi
    fi
    ;;
  Darwin )
    install_prereqs_for_macos $1
    ;;
  * )
    echo "OS=`uname` not supported"
    exit 1
esac

if [[ $1 == "release" ]]; then
  cmake $GITHUB_WORKSPACE -DDISABLE_OPENMP=1
  make -j4
  cmake --build .
  sudo make install
fi