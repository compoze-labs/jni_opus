#!/bin/zsh

libs_dir="./.libs"
include_dir="/usr/local/include/"

function verify_include_dir() {
    if [ ! -d "$include_dir" ]; then
      mkdir -p "$include_dir"
    fi
}

function copy_lib(){
      cp "$libs_dir/$1" "$include_dir"
}

verify_include_dir
copy_lib "libopus.0.dylib"