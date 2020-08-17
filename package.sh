#!/bin/bash

PLATFORM=$1
DISTRO=$2
BUILD_TYPE=$3


if [[ "${PLATFORM}" == "pi" ]]; then
    OS="raspbian"
    ARCH="arm"
    PACKAGE_ARCH="armhf"
fi

if [ "${BUILD_TYPE}" == "docker" ]; then
    cat << EOF > /etc/resolv.conf
options rotate
options timeout:1
nameserver 8.8.8.8
nameserver 8.8.4.4
EOF
fi

apt -y install libasio-dev libboost-system-dev libboost-program-options-dev

PACKAGE_NAME=openhd-router

TMPDIR=/tmp/${PACKAGE_NAME}-installdir

rm -rf ${TMPDIR}/*

mkdir -p ${TMPDIR} || exit 1

make clean || exit 1

make install DESTDIR=${TMPDIR} || exit 1

VERSION=$(git describe)

rm ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb > /dev/null 2>&1

fpm -a ${PACKAGE_ARCH} -s dir -t deb -n ${PACKAGE_NAME} -v ${VERSION//v} -C ${TMPDIR} \
  -p ${PACKAGE_NAME}_VERSION_ARCH.deb \
  -d "libasio-dev >= 1.10" \
  -d "libboost-system-dev >= 1.62.0" \
  -d "libboost-program-options-dev >= 1.62.0"  || exit 1

#
# Only push to cloudsmith for tags. If you don't want something to be pushed to the repo, 
# don't create a tag. You can build packages and test them locally without tagging.
#
git describe --exact-match HEAD > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
    echo "Pushing package to OpenHD repository"
    cloudsmith push deb openhd/openhd-2-0/${OS}/${DISTRO} ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb
else
    echo "Pushing package to OpenHD testing repository"
    cloudsmith push deb openhd/openhd-2-0-testing/${OS}/${DISTRO} ${PACKAGE_NAME}_${VERSION//v}_${PACKAGE_ARCH}.deb
fi
