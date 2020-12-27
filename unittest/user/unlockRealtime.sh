#!/bin/bash
# Run this script to unlock all permissions to run the linux binaries
# and create threads

binaries=$(find $directory -type f -name "*.elf")

echo Unlocking real time permissions for binaries and bash console...

# Set up the soft realtime limit to maximum (99)
# Please note that the hard limit needs to be set to 99 too 
# for this to work (check with ulimit -Hr).
# If that has not been done yet, add
# <username>	hard     rtprio          99
# to /etc/security/limits.conf
# It is also necessary and recommended to add
# <username>    soft     rtprio         99
# as well. This can also be done in the command line
# but would need to be done for each session.
ulimit -Sr 99

for binary in ${binaries}; do
	sudo setcap 'cap_sys_nice=eip' ${binary}
	result=$?
	if [ ${result} = 0 ];then
		echo ${binary} was unlocked
	fi
done

# sudo setcap 'cap_sys_nice=eip' /bin/bash
# result=$?
# if [ ${result} = 0 ];then
# 	echo /bin/bash was unlocked
# fi

