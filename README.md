# Realtek r8125 DKMS

This provides Realtek r8125 driver in DKMS way so that you can keep the latest driver even after the kernel updgrade.

## Installation

There're 3 way to install this DKMS module. Choose 1 as your tastes.

Those are not interfere each other. So you can do all 3 methods but don't need to definitely.

Installation using Debian package is recommended for the sake of getting the newer driver.

### Debian package

Download the latest Debian package from Release tab on the Github repository.

Then enter the following command.

```bash
sudo dpkg -i realtek-r8125-dkms**TBD**.deb
```

Launchpad PPA will be prepared.

### autorun.sh

Using the `autorun.sh` script that Realtek provides on their original driver package. This is **not installed as a DKMS**, only efforts to the current kernel.

Download or clone this repository and move to the extracted directory, then run the script.

```bash
sudo ./autorun.sh
```

### dkms-install.sh

This script is from aircrack-ng team. You can install the DKMS module by a simple command.

Download or clone this repository and move to the extracted directory, then run the script.

```bash
sudo ./dkms-install.sh
```

## Debian package build

You can build yourself this after installing some dependencies including `dkms`.

```bash
sudo apt install devscripts debhelper dkms
```

```bash
dpkg-buildpackage -b -rfakeroot -us -uc
```

## LICENSE

GPL-2 on Realtek driver and the debian packaing.

## References

- [Realtek r8125 driver release page](https://www.realtek.com/en/component/zoo/category/network-interface-controllers-10-100-1000m-gigabit-ethernet-pci-express-software)
- [aircrack-ng rtl8812au, where very helped to deal with DKMS](https://github.com/aircrack-ng/rtl8812au)
