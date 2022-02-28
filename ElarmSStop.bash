#!/bin/bash
All=(DeciMod ElarmS2 ElarmSWP2 ActiveMQ Earthworm)
MODULS=()
if [[ " ${*} " =~ " -h " ]]; then
    echo "Stops ElarmS modules."
    echo "Usage: ElarmsStop.bash [module] [module...]"
    echo "       Valid modules:"
    echo "         ${All[@]}"
    echo "       No module will stop all."
    exit 0
fi
if [[ $# -eq 0 ]]; then
	MODULS=(${All[@]})
else
	MODULS=(${*})
fi
for M in ${MODULS[@]}; do
	if [[ ! " ${All[@]} " =~ " ${M} " ]]; then
		echo "${M} is not a valid module"
		continue
	fi
	running="$( screen -ls | grep ${M} | cut -d. -f2 | awk '{print $1}' )"
	if [ "${M}" != "${running}" ]; then
		echo "${M} is not running"
		continue
	fi
	echo "Stopping ${M}"
	case ${M} in
		Earthworm)
			screen -S Earthworm -p 0 -X stuff quit$'\n'
		;;
		*)
			screen -S ${M} -p 0 -X stuff $'\003'	
		;;
	esac
done
exit