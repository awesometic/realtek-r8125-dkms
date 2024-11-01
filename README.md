# Realtek r8125 DKMS

![GitHub release (latest SemVer)](https://img.shields.io/github/v/release/awesometic/realtek-r8125-dkms?sort=semver&style=for-the-badge)

This provides Realtek r8125 driver in DKMS way so that you can keep the latest driver even after the kernel upgrade.

## Before use

This DKMS package is for Realtek RTL8125 (r8125 in module name) Ethernet, which is designed for the PCI interface.

If you are searching for the Realtek 2.5 Gbits **USB Ethernet**, which may use RTL8156 (r8152 in module name), you are in the wrong place. Please refer to another DKMS project for that Realtek driver.

- [Realtek R8152 DKMS](https://github.com/awesometic/realtek-r8152-dkms)

## Install
There are 3 ways to install this DKMS module not every works for every
distribution. This table should help you decide which way to go.

| Nr | Method                      | Fedora/RedHat based | Debian based    |
|----|-----------------------------|:-------------------:|:---------------:|
| 1  | [deb package](#deb)         |                     | v               |
| 2  | [Launchpad PPA](#launchpad) |                     | v               |
| 3  | [autorun.sh](#autorun)      | v                   | v               |
| 4  | [dkms-install.sh](#dkms)    | v (Recommended)     | v (Recommended) |

Those are not interfering with each other. So you can do all 3 methods but
absolutely you don't need to.

1. First method using the Debian (deb) package <a name="deb"></a>

   1. Download the deb package or build it from scratch

      + Download deb package
         ```bash
         curl -sSL ...
         ```

      + From scratch
         ```bash
         sudo apt install devscripts debmake debhelper build-essential dkms
         git clone --depth 1 https://github.com/awesometic/realtek-r8125-dkms.git
         cd realtek-r8125-dkms
         dpkg-buildpackage -b -rfakeroot -us -uc
         ```

   2. Install the deb package
      ```bash
      sudo dpkg -i realtek-r8125-dkms_9.011.00-1_amd64.deb
      ```

      If dependency error occurs, try to fix that with `apt` command.
      ```bash
      sudo apt install --fix-broken
      ```

2. Second method using the Launchpad PPA <a name="launchpad"></a>

   1. Add the Launchpad PPA
      ```bash
      sudo add-apt-repository ppa:awesometic/ppa
      ```

   2. Install the package using `apt` tool
      ```bash
      sudo apt install realtek-r8125-dkms
      ```

3. Third  method using the autorun.sh script <a name="autorun"></a>

   Using the `autorun.sh` script that Realtek provides on their original
   driver package. This is **not installed as a DKMS**, only efforts to the
   current kernel.

   1. Download/Clone repository
      ```bash
      git clone --depth 1 https://github.com/awesometic/realtek-r8125-dkms.git
      cd realtek-r8125-dkms
      ```
   2. Install driver by running the autorun.sh script
      ```bash
      sudo ./autorun.sh
      ```

4. Fourth method using the `dkms-install.sh` script <a name="dkms"></a>

   This script is from aircrack-ng team. You can install the DKMS module by a
   simple command.

   1. Download/Clone repository
      ```bash
      git clone --depth 1 https://github.com/awesometic/realtek-r8125-dkms.git
      cd realtek-r8125-dkms
      ```

   2. Install dependencies <a name="dependencies"></a>
      + apt (Debian/Ubuntu)
      ```
      sudo apt install dkms
      ```
      + dnf/yum (Fedora/RedHat)
      ```bash
      sudo dnf install dkms
      ```

   3. Install DKMS module by running the `dkms-install.sh` script
      ```bash
      sudo ./dkms-install.sh
      ```

# Blacklist and load r8125 module
   1. Blacklist the r8169 module <a name="initramfs"></a>
      ```bash
      sudo tee -a /etc/modprobe.d/blacklist-r8169.conf > /dev/null <<EOT
      blacklist r8169
      EOT
      ```

   2. Ensure module r8125 will be included in the initramfs
      ```bash
      sudo tee -a /etc/modules-load.d/realtek-r8125.conf > /dev/null <<EOT
      r8125
      EOT
      ```

   3. Create the initramfs

      You can list the currently included modules if you want, there should
      only be one line of output.

      ```bash
      sudo lsinitrd /boot/initramfs-$(uname --kernel-release).img | grep r81

      -rw-r--r--   1 root     root        55164 Nov 16 19:00 usr/lib/modules/$(uname --kernel-release)/kernel/drivers/net/ethernet/realtek/r8169.ko.xz
      ```

      Now recreate the initramfs to include the blacklist and the r8125 module.
      + Fedora/RedHat
        ```bash
        sudo dnf install dracut
        sudo dracut --force --kver $(uname --kernel-release)
        ```
      + Debian/Ubuntu
        ```bash
        sudo update-initramfs -u
        ```

      Now list the modules again of the newly created initramfs. You should
      see now 4 lines.

      ```bash
      sudo lsinitrd /boot/initramfs-$(uname --kernel-release).img | grep r81

      -rw-r--r--   1 root     root           49 Nov 16 19:00 etc/modprobe.d/blacklist-r8169.conf
      -rw-r--r--   1 root     root            6 Nov 16 19:00 etc/modules-load.d/realtek-r8125.conf
      -rw-r--r--   1 root     root        64916 Nov 16 19:00 usr/lib/modules/$(uname --kernel-release)/extra/r8125.ko.xz
      -rw-r--r--   1 root     root        55164 Nov 16 19:00 usr/lib/modules/$(uname --kernel-release)/kernel/drivers/net/ethernet/realtek/r8169.ko.xz
      ```

   4. Reboot the system to load the r8125 module instead of r8168 module
      ```bash
      sudo systemctl start reboot.target --job-mode=replace-irreversibly --no-block
      ```

   5. It's time to verify the module has been successfully loaded.
      ```bash
      sudo lspci -v -d ::0200 | grep 81

<<<<<<< HEAD
      2a:00.0 Ethernet controller: Realtek Semiconductor Co., Ltd. RTL8125 2.5GbE Controller (rev 05)
          Kernel driver in use: r8125
          Kernel modules: r8169, r8125
      ```
||||||| a9a0ddf
```bash
sudo tee -a /etc/modprobe.d/blacklist-r8169.conf > /dev/null <<EOT
# To use r8125 driver explicitly
blacklist r8169
EOT
```

To apply the new blacklist to your kernel, update your initramfs via

```bash
sudo update-initramfs -u
```

Finally, reboot to take effect.

> - If you need to load both r8169 and r8125, maybe removing r8125 firmware could make it work. Please enter `sudo rm -f /lib/firmware/rtl_nic/rtl8125*` to remove all the r8125 firmwares on the system. But it is just a workaround, you should have to do this every time installing the new kernel version or new Linux firmware.
> - In the case of the Debian package, I will update the scripts to make it do this during the installation.

## Debian package build

You can build yourself this after installing some dependencies including `dkms`.

```bash
sudo apt install devscripts debmake debhelper build-essential dkms dh-dkms
```

```bash
dpkg-buildpackage -b -rfakeroot -us -uc
```
=======
```bash
sudo tee -a /etc/modprobe.d/blacklist-r8169.conf > /dev/null <<EOT
# To use r8125 driver explicitly
blacklist r8169
EOT
```

To apply the new blacklist to your kernel, update your initramfs via

```bash
sudo update-initramfs -u
```

Finally, reboot to take effect.

> - If you need to load both r8169 and r8125, maybe removing r8125 firmware could make it work. Please enter `sudo rm -f /lib/firmware/rtl_nic/rtl8125*` to remove all the r8125 firmwares on the system. But it is just a workaround, you should have to do this every time installing the new kernel version or new Linux firmware.
> - In the case of the Debian package, I will update the scripts to make it do this during the installation.

## Debian package build

You can build yourself this after installing some dependencies including `dkms`.

```bash
sudo apt install devscripts debmake debhelper build-essential dkms
```

```bash
dpkg-buildpackage -b -rfakeroot -us -uc
```
>>>>>>> origin/master

## LICENSE

GPL-2 on Realtek driver and the Debian packing.

## References

- [Realtek r8125 driver release page](https://www.realtek.com/Download/List?cate_id=584)
- [ParrotSec's realtek-rtl88xxau-dkms, where got hint from](https://github.com/ParrotSec/realtek-rtl88xxau-dkms)
