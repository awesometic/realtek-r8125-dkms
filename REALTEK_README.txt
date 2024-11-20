<Linux device driver for Realtek Ethernet controllers>

	This is the Linux device driver released for Realtek 2.5 Gigabit Ethernet controllers with PCI-Express interface.

<Requirements>

	- Kernel source tree (supported Linux kernel 2.6.x and 2.4.x)
	- For linux kernel 2.4.x, this driver supports 2.4.20 and latter.
	- Compiler/binutils for kernel compilation

<Quick install with proper kernel settings>
	Unpack the tarball :
		# tar vjxf r8125-9.aaa.bb.tar.bz2

	Change to the directory:
		# cd r8125-9.aaa.bb

	If you are running the target kernel, then you should be able to do :

		# ./autorun.sh	(as root or with sudo)

	You can check whether the driver is loaded by using following commands.

		# lsmod | grep r8125
		# ifconfig -a

	If there is a device name, ethX, shown on the monitor, the linux
	driver is loaded. Then, you can use the following command to activate
	the ethX.

		# ifconfig ethX up

		,where X=0,1,2,...

<Set the network related information>
	1. Set manually
		a. Set the IP address of your machine.

			# ifconfig ethX "the IP address of your machine"

		b. Set the IP address of DNS.

		   Insert the following configuration in /etc/resolv.conf.

			nameserver "the IP address of DNS"

		c. Set the IP address of gateway.

			# route add default gw "the IP address of gateway"

	2. Set by doing configurations in /etc/sysconfig/network-scripts
	   /ifcfg-ethX for Redhat and Fedora, or /etc/sysconfig/network
	   /ifcfg-ethX for SuSE. There are two examples to set network
	   configurations.

		a. Fixed IP address:
			DEVICE=eth0
			BOOTPROTO=static
			ONBOOT=yes
			TYPE=ethernet
			NETMASK=255.255.255.0
			IPADDR=192.168.1.1
			GATEWAY=192.168.1.254
			BROADCAST=192.168.1.255

		b. DHCP:
			DEVICE=eth0
			BOOTPROTO=dhcp
			ONBOOT=yes

<Modify the MAC address>
	There are two ways to modify the MAC address of the NIC.
	1. Use ifconfig:

		# ifconfig ethX hw ether YY:YY:YY:YY:YY:YY

	   ,where X is the device number assigned by Linux kernel, and
		  YY:YY:YY:YY:YY:YY is the MAC address assigned by the user.

	2. Use ip:

		# ip link set ethX address YY:YY:YY:YY:YY:YY

	   ,where X is the device number assigned by Linux kernel, and
		  YY:YY:YY:YY:YY:YY is the MAC address assigned by the user.

<Force Link Status>

	1. Force the link status when insert the driver.

	   If the user is in the path ~/r8125, the link status can be forced
	   to one of the 5 modes as following command.

		# insmod ./src/r8125.ko speed=SPEED_MODE duplex=DUPLEX_MODE autoneg=NWAY_OPTION

		,where
			SPEED_MODE	= 1000	for 1000Mbps
					= 100	for 100Mbps
					= 10	for 10Mbps
			DUPLEX_MODE	= 0	for half-duplex
					= 1	for full-duplex
			NWAY_OPTION	= 0	for auto-negotiation off (true force)
					= 1	for auto-negotiation on (nway force)
		For example:

			# insmod ./src/r8125.ko speed=100 duplex=0 autoneg=1

		will force PHY to operate in 100Mpbs Half-duplex(nway force).

	2. Force the link status by using ethtool.
		a. Insert the driver first.
		b. Make sure that ethtool exists in /sbin.
		c. Force the link status as the following command.

			2.5G before kernel v4.10
			# ethtool -s eth0 autoneg on advertise 0x802f

			2.5G for kernel v4.10 and later
			# ethtool -s eth0 autoneg on advertise 0x80000000002f

			# ethtool -s eth0 autoneg on advertise 0x002f (1G)
			# ethtool -s eth0 autoneg on advertise 0x000f (100M full)
			# ethtool -s eth0 autoneg on advertise 0x0003 (10M full)

<Jumbo Frame>
	Transmitting Jumbo Frames, whose packet size is bigger than 1500 bytes, please change mtu by the following command.

	# ifconfig ethX mtu MTU

	, where X=0,1,2,..., and MTU is configured by user.

	RTL8125 supports Jumbo Frame size up to 9 kBytes.

<EEE>
    Get/Set device EEE status

		Get EEE device status
		# ethtool --show-eee enp1s0

		Set EEE device status
		# ethtool --set-eee enp1s0 eee on tx-lpi on tx-timer 1546 advertise 0x0008 (100M full)
		# ethtool --set-eee enp1s0 eee on tx-lpi on tx-timer 1546 advertise 0x0020 (1G)
		# ethtool --set-eee enp1s0 eee on tx-lpi on tx-timer 1546 advertise 0x8000 (2.5G)
