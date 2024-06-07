#!/bin/bash

echo "==============================="
echo "Welcome to (Un)install SVCamKit"
echo "==============================="
echo 
echo "----------------------------------------"
echo "Usage:"
echo "using 'sudo $0 -i' to install it"
echo "using 'sudo $0 -u' to remove it"
echo "----------------------------------------"
echo

if [ $# -ne 1 ]
then
	echo "parameter mismatched"
	exit 1
fi

function check()
{
	if [ $? == 0 ]
	then
		:
	else
		echo "... not ok"
		exit 1
	fi
}

action=nothing
while getopts iu opt
do
	case $opt in
		i)
		action=install
		shift
		;;
		u)
		action=uninstall
		shift
		;;
	esac
done

if [ $action == nothing ]
then 
	echo "parameter not correct"
	exit 1
else 
	if [ $(whoami) != root ]
	then 
	echo "need root privilege"
	exit 1
	fi
fi

if [ "$action" == install ]
then 
	echo "Installing ..."
	if [ -d /etc/udev/rules.d -a -f SVCamKit/others/60-svcamkit-u3v.rules ]
	then
		cp SVCamKit/others/60-svcamkit-u3v.rules /etc/udev/rules.d
		check
	fi

	if [ -d /etc/systemd/system -a -f SVCamKit/others/svcamkit_u3v_startup.service ]
	then
		cp SVCamKit/others/svcamkit_u3v_startup.service /etc/systemd/system
		check
	fi
	if [ -d /etc/systemd -a -f SVCamKit/others/svcamkit_u3v_startup.sh ]
	then
		cp SVCamKit/others/svcamkit_u3v_startup.sh /etc/systemd
		check
	fi

	if [ -f /etc/systemd/svcamkit_u3v_startup.sh ]
	then 
		chmod +x /etc/systemd/svcamkit_u3v_startup.sh
		check
		systemctl enable svcamkit_u3v_startup.service
		check
	fi

	if [ -d /etc/sysctl.d ]
	then 
		cp SVCamKit/others/60-svcamkit-gev.conf /etc/sysctl.d
		check
	fi

	mkdir -p /opt/SVS
	check
	if [ -d /opt/SVS ]
	then 
		cp -r SVCamKit /opt/SVS
		check
	fi
	
	if [ -d /etc/ld.so.conf.d ]
	then
		cp SVCamKit/others/svcamkit.conf /etc/ld.so.conf.d
		check
		ldconfig
		check
	fi

	if [ -f /etc/environment ]
	then 
		cp /etc/environment /etc/environment_`date +"%Y%m%d_%H%M%S"`
		check
		echo 'SVS_GENICAM_CACHE="/opt/SVS/SVCamKit/SDK/Linux64_x64/cache/"' >> /etc/environment
		echo 'SVS_SDK_BIN_64="/opt/SVS/SVCamKit/SDK/Linux64_x64/"' >> /etc/environment
		echo 'GENICAM_GENTL64_PATH="/opt/SVS/SVCamKit/SDK/Linux64_x64/cti"' >> /etc/environment
		check
	fi

	echo "SVCamKit is successfully installed!"
	echo "Please reboot the pc!"
else 
	if [ "$action" == uninstall ]
	then
		echo "Uninstalling ..."
		if [ -f /etc/udev/rules.d/60-svcamkit-u3v.rules ]
		then
			rm /etc/udev/rules.d/60-svcamkit-u3v.rules
			check
		fi

		if [ -f /etc/systemd/system/svcamkit_u3v_startup.service ]
		then
			systemctl disable svcamkit_u3v_startup.service 
			check
			rm /etc/systemd/system/svcamkit_u3v_startup.service
			check
		fi

		if [ -f /etc/systemd/svcamkit_u3v_startup.sh ]
		then
			rm /etc/systemd/svcamkit_u3v_startup.sh
			check
		fi

		if [ -f /etc/sysctl.d/60-svcamkit-gev.conf ]
		then
			rm /etc/sysctl.d/60-svcamkit-gev.conf
			check
		fi

		if [ -d /opt/SVS ]
		then
			rm -rf /opt/SVS
			check
		fi

		if [ -f /etc/ld.so.conf.d/svcamkit.conf ]
		then
			rm /etc/ld.so.conf.d/svcamkit.conf
			check
			ldconfig
			check
		fi
		
		if [ -f /etc/environment ]
		then 
			cp /etc/environment /etc/environment_`date +"%Y%m%d_%H%M%S"`
			check
		fi

		sed -i -e '/^SVS_GENICAM_CACHE/d' -e '/^SVS_SDK_BIN_64/d' -e '/^GENICAM_GENTL64_PATH/d' /etc/environment
		check
		
		if [ -d ~/.config/SVS-VISTEK\ GmbH ]
		then
			rm -rf ~/.config/SVS-VISTEK\ GmbH
			check
		fi

		if [ -d ~/Documents/SVCapture ]
		then
			rm -rf ~/Documents/SVCapture
			check
		fi
		echo "SVCamKit is successfully uninstalled!"
		echo "Please reboot the pc!"
	fi
fi

