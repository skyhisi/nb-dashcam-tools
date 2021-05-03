#!/bin/bash
set -ex
SRC=$(</mnt/filename.txt)
VER=$(tar -xf "/mnt/$SRC" --strip-components=1 --no-anchored -O version.txt | cut -c2- | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')
rpmdev-setuptree
rpmbuild -D "version $VER" -ta "/mnt/$SRC"
cp -v "$(rpm --eval "%{_rpmdir}/%{_arch}")/nb-dashcam-tools-$VER-"*.rpm /mnt
