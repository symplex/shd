#!/bin/bash
#
# Copyright 2015 National Instruments Corp.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

if ! ls | grep host > /dev/null
then
    echo "This script must be run from SHD's top-level directory."
    exit 1
fi
if [ -f fpga-src/README.md ]; then
    echo "This script requires a clean repository without fpga-src checked out!."
    exit 1
fi

FORCE_YES=0
if [ $# -eq 1 ]
then
    if [ "$1" = "-y" ]
    then
        FORCE_YES=1
    fi
fi

SHD_TOP_LEVEL=$PWD

# Get version info
VERSION=`head -1 host/cmake/debian/changelog | grep -o '[0-9.]*' | head -1`
ORIG_RELEASE=`head -1 host/cmake/debian/changelog | sed 's/.*) \(.*\);.*/\1/'`

#
# Currently supported versions can be found here:
# https://launchpad.net/ubuntu/+ppas
#
RELEASES="trusty vivid wily xenial yakkety"
PPA=ppa:ettusresearch/shd

#
# Make sure this is the intended version.
#
# This is based on the contents of debian/changelog. If convert_changelog.py was not
# run on this version, it will show the previous release.
#
echo "Will generate installer configuration files for SHD "$VERSION
if [ $FORCE_YES -ne 1 ]
then
    echo "Is this correct? (yes/no)"
    read response
    if [ "$response" != "yes" ]
    then
        exit 0
    fi
fi

# Generate the TAR file to be uploaded.
echo "Creating SHD source archive."
tar --exclude='.git*' --exclude='./debian' --exclude='*.swp' --exclude='fpga-src' --exclude='build' --exclude='./images/*.pyc' --exclude='./images/shd-*' --exclude='tags' -cJf ../shd_${VERSION}.orig.tar.xz .
if [ $? != 0 ]
then
    echo "Failed to create SHD source archive."
    exit 1
fi

# debuild expects our directory name to be ${source package}-${version}
rm -f ${SHD_TOP_LEVEL}/../shd-${VERSION}
ln -s ${SHD_TOP_LEVEL} ${SHD_TOP_LEVEL}/../shd-${VERSION}
cd ${SHD_TOP_LEVEL}/../shd-${VERSION}

#
# Generate package info for each version.
#
# This script substitutes each version string into the debian/changelog file to
# create package info for each version. We need to store the original outside the
# SHD repo, or dpkg-source will detect the change and error out.
#
cp -r host/cmake/debian .
cp host/utils/shd-smini.rules debian/shd-host.udev
find host/docs -name '*.1' > debian/shd-host.manpages
rm -f debian/postinst.in debian/postrm.in debian/preinst.in debian/prerm.in

if [ $FORCE_YES -ne 1 ]
then
    echo "Proceed to generate package info? (yes/no)"
    read response
    if [ "$response" != "yes" ]
    then
        exit 0
    fi
fi

for RELEASE in ${RELEASES}
do
    cp debian/changelog ../changelog.backup
    sed -i "s/${ORIG_RELEASE}/${RELEASE}/;s/0ubuntu1/0ubuntu1~${RELEASE}1/" debian/changelog
    debuild -S -i -sa
    if [ $? != 0 ]
    then
        echo "Failed to generate package info for" ${RELEASE}
        mv ../changelog.backup debian/changelog
        exit 1
    fi
    mv ../changelog.backup debian/changelog
done

if [ $FORCE_YES -ne 1 ]
then
    echo "Proceed to upload to launchpad? (yes/no)"
    read response
    if [ "$response" != "yes" ]
    then
        exit 0
    fi
fi

# Upload package into to Launchpad, which will automatically build packages
for RELEASE in ${RELEASES}
do
    dput ${PPA} ../shd_${VERSION}-0ubuntu1~${RELEASE}1_source.changes
    if [ $? != 0 ]
    then
        echo "Failed to upload" ${VERSION} "package info to Launchpad."
	    exit 1
    fi
done

if [ $FORCE_YES -ne 1 ]
then
    echo
    echo "Clean up build artifacts? (yes/no)"
    read response
    if [ "$response" = "yes" ]
    then
        cd ..
        rm -r ${SHD_TOP_LEVEL}/debian shd-${VERSION} shd_${VERSION}.orig.tar.xz shd*dsc shd*changes shd*debian.tar.* shd*_source.build shd*.upload
    fi
fi
