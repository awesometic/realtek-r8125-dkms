#!/bin/bash

if [[ $EUID -ne 0 ]]; then
  echo "You must run this with superuser privileges.  Try \"sudo ./dkms-install.sh\"" 2>&1
  exit 1
else
  echo "About to run dkms install steps..."
fi

DRV_DIR="$(pwd)"
DRV_NAME=r8125
DRV_VERSION=9.015.00
KERNEL_VERSION="${KERNEL_VERSION:-$(uname -r)}"

cp -r ${DRV_DIR} /usr/src/${DRV_NAME}-${DRV_VERSION}

dkms add -m ${DRV_NAME} -v ${DRV_VERSION} -k ${KERNEL_VERSION}
dkms build -m ${DRV_NAME} -v ${DRV_VERSION} -k ${KERNEL_VERSION}
dkms install -m ${DRV_NAME} -v ${DRV_VERSION} -k ${KERNEL_VERSION}
RESULT=$?

echo "Finished running dkms install steps."

exit $RESULT
