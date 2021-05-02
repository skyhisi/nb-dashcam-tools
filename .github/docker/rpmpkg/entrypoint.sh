#!/bin/bash
set -ex
rpmdev-setuptree
rpmbuild -D "version $VER" -ta "/mnt/$SRC"
cp -v "$(rpm --eval "%{_rpmdir}/%{_arch}")/nb-dashcam-tools-$VER-"*.rpm /mnt
