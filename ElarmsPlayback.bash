#!/bin/bash

# /**********************************************************************************
# *    Copyright (C) by Ran Novitsky Nof                                            *
# *                                                                                 *
# *                                                                                 *
# *    This file is free software: you can redistribute it and/or modify            *
# *    it under the terms of the GNU Lesser General Public License as published by  *
# *    the Free Software Foundation, either version 3 of the License, or            *
# *    (at your option) any later version.                                          *
# *                                                                                 *
# *    This program is distributed in the hope that it will be useful,              *
# *    but WITHOUT ANY WARRANTY; without even the implied warranty of               *
# *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
# *    GNU Lesser General Public License for more details.                          *
# *                                                                                 *
# *    You should have received a copy of the GNU Lesser General Public License     *
# *    along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
# ***********************************************************************************/

# By Ran Novitsky Nof @ GSI, 2018
# ran.nof@gmail.com
#


# A code for running playback on ElarmS playback machine
# For use on playback server only.

# PARAMETERS
## seconds before and after origin time to get event time window
SECBEFORE=120
SECAFTER=300
## FDSN server
FDSNHOST=82.102.143.46
FDSNPORT=8181
# Uncomment the user and password lines if credentials are needed for fdsn server
USER=test
PASSWORD=test

# make sure we have moreutils ts installed
which ts &> /dev/null
if [ "$?" -ne "0" ]; then
	echo "No ts found, make sure moreutils is installed"
	echo "sudo apt install moreutils"
	exit 1
fi

# make sure we are in ElarmS evironment
if [[ -z $EW_INSTALL_HOME ]]; then
	echo "You need to define you ElarmS environment first:"
	echo "export EW_INSTALL_HOME=/home/${USER}/ElarmS"
	exit 1
fi

if [[ -z $EW_INSTALL_VERSION ]]; then
        echo "Adding EW install version parameter"
      	export EW_INSTALL_VERSION=earthworm-working
fi
echo "EW_INSTALL_VERSION=${EW_INSTALL_VERSION}"



# Source ElarmS parameters
. ${EW_INSTALL_HOME}/run/params/ew_linux.bash

# Make sure origin time format is acceptable or parameter is a file
: ${1?"Usage: $0 origin-time or file_name"}
echo "user input: ${1}"
if [ -e ${1} ]; then
	tnkfile=${1}
	echo "Assuming ${1} is a tankplayer file"
else
	# We need to download data first

	# make sure ms2tank is available
	# for converting mseed to Earthworm tnk files

	which ms2tank &> /dev/null
	if [ $? -ne 0 ]; then
        	echo "Make sure ms2tank is in you path."
        	echo "ms2tank is part of Earthworm"
	        echo "ms2tank can be found using:"
        	echo ""
        	echo ". ${EW_INSTALL_HOME}/run/params/ew_linux.bash"
        	echo ""
        	echo ""
	        exit 1
	fi

	# make sure msrtclice is available
	# for slicing data into 1s packets.

	which msrtslice &> /dev/null
	if [ $? -ne 0 ]; then
	        echo "Make sure msrtslice is in you path."
        	echo "msrtslice may be installed using:"
	        echo ""
	        echo "git clone https://github.com/rannof/srtpb"
                echo "COMPILE STRPB"
	        echo "export PATH=\$PATH:~/srtpb/bin/"
	        echo ""
	        echo ""
	        exit 1
	fi		

	grep -P "\d\d.\d\d.\d\d.\d\d.\d\d.\d\d.\d\d\d" <<< ${1} &> /dev/null
	if [ $? -ne 0 ]; then
		echo "Origin-time format should be: YYYY-mm-ddTHH:MM:ss.fff"
		exit 1
	fi
fi

if [ -z ${tnkfile} ]; then
	# Calculate start and end times for window.
	read Y m d H M s f <<< `echo ${1} | sed 's/\(..\).\(..\).\(..\).\(..\).\(..\).\(..\)\.\(...\)/\1 \2 \3 \4 \5 \6 \7/'`
	ot="${Y}-${m}-${d}T${H}:${M}:${s}.${f}Z"
	stime=`date -u -d "${ot} - ${SECBEFORE} seconds"  +%Y-%m-%dT%H:%M:%S.%N` &> /dev/null
	etime=`date -u -d "${ot} + ${SECAFTER} seconds"  +%Y-%m-%dT%H:%M:%S.%N` &> /dev/null
	if [ $? -ne 0 ]; then
        	echo "Origin-time format should be: YYYY-mm-ddTHH:MM:ss.fff"
        	exit 1
	fi
	echo "Getting Time window: ${stime} - ${etime}"
	# setting credentials if needed
	if [[ -z $PASSWORD || -z $USER ]] ; then
		fdsnparams="--show-progress"
                auth=''
	else
		echo "Using credentials"
                fdsnparams="--show-progress --user ${USER} --password ${PASSWORD}"
                auth='auth'
	fi
	# Download data from fdsn server
	fname="${Y}${m}${d}${H}${M}${s}.${f}"
	echo "Getting Z channels:"
	if [ ! -e ${fname}_Z.mseed ]; then
  		wget ${fdsnparams} "http://${FDSNHOST}:${FDSNPORT}/fdsnws/dataselect/1/query${auth}?starttime=${stime}&endtime=${etime}&network=*&station=*&location=*&channel=%3F%3FZ&nodata=404" -O "${fname}_Z.mseed"
		if [ $? -ne 0 ]; then
			echo "Error with fdsn server"
		        exit 1
		fi
	else
  		echo ${fname}_Z.mseed exists
	fi
  	echo "Getting N channels:"
	if [ ! -e ${fname}_N.mseed ]; then
  		wget ${fdsnparams} "http://${FDSNHOST}:${FDSNPORT}/fdsnws/dataselect/1/query${auth}?starttime=${stime}&endtime=${etime}&network=*&station=*&location=*&channel=%3F%3FN&nodata=404" -O "${fname}_N.mseed"
		if [ $? -ne 0 ]; then
                        echo "Error with fdsn server"
                        exit 1
                fi
	else
  		echo ${fname}_N.mseed exists
	fi
	echo "Getting E channels:"
	if [ ! -e ${fname}_E.mseed ]; then
  		wget ${fdsnparams} "http://${FDSNHOST}:${FDSNPORT}/fdsnws/dataselect/1/query${auth}?starttime=${stime}&endtime=${etime}&network=*&station=*&location=*&channel=%3F%3FE&nodata=404" -O "${fname}_E.mseed"
		if [ $? -ne 0 ]; then
                        echo "Error with fdsn server"
                        exit 1
                fi
	else
  		echo "${fname}_E.mseed exists"
	fi
	# sort data
	echo "Sorting file"
	if [ ! -e ${fname}_sorted.mseed ]; then
  		msrtslice -o ${fname}_sorted.mseed ${fname}_[ZNE].mseed &> msrtslice_${fname}.log
	else
  		"echo ${fname}_sorted.mseed exists"
	fi
	# Create the tank file
	echo "Creating Earthworm tank file"
	if [ ! -e ${fname}_sorted.tnk ]; then
  		ms2tank ${fname}_sorted.mseed >> ${fname}_sorted.tnk
	else
  		echo "${fname}_sorted.mseed exists"
	fi
fi
tnkfile=${tnkfile:-$PWD/${fname}_sorted.tnk}
if [ -z $fname ]; then
	fname=`echo $(basename ${tnkfile}) | sed 's/\(.*\)\..*$/\1/'`
fi
basedir=$PWD
# Make sure we are on a playback system
if [ -e ${EW_INSTALL_HOME}/run/bin/wpr.conf ] && [ -e ${EW_INSTALL_HOME}/run/bin/EWP2r ] && [ -e ${EW_INSTALL_HOME}/run/bin/EWP2r.conf ] && [ -e ${EW_INSTALL_HOME}/run/bin/E2r ] && [ -e ${EW_INSTALL_HOME}/run/bin/E2r.conf ] && [ -e ${EW_INSTALL_HOME}/run/bin/dmr ] && [ -e ${EW_INSTALL_HOME}/run/bin/dmr.conf ]; then
  echo "Starting playback environment"
else
  echo "No playback environment detected."
  echo "Make sure DMr EWP2r and E2r are configured correctly:"
  echo "cd ${EW_INSTALL_HOME}/run/bin"
  echo "cp wp.conf wpr.conf"
  echo "echo \"DoReplay          true\"   >> wpr.conf"
  echo "echo \"ReplayRealTime                false\" >> wpr.conf"
  echo "\# edit EWP2r.conf file:"
  echo "cp EWP2.conf EWP2r.conf"
  echo "sed -i 's/wp.conf/wpr.conf/' EWP2r.conf"
  echo "sed -i 's/Restart.*/Restart      true/' EWP2r.conf"
  echo "cp E2.conf E2r.conf"
  echo "\# edit E2r.conf file:"
  echo "sed -i 's/\(UseSystemTime\s*\).true\(\s*#\)/\1false\2/' E2r.conf"
  echo "cp dm.conf dmr.conf"
  echo "\# edit dmr.conf file:"
  echo "sed -i 's/DMEventTimeoutSecs.*/DMEventTimeoutSecs        60000/' dmr.conf"
  echo "sed -i 's/AlgEventTimeoutSecs.*/AlgEventTimeoutSecs        60000/' dmr.conf"
  echo "ln -s dm dmr"
  echo "ln -s E2 E2r"
  echo "ln -s EWP2 EWP2r"
  echo ""
  exit 1
fi

# make sure its a fast playback
echo "Switching to fast replay (no real time, 1 thread) on wpr.conf"
sed -i 's/.*ReplayRealTime.*/ReplayRealTime    false/' ${EW_INSTALL_HOME}/run/bin/wpr.conf
sed -i 's/.*ThreadCount.*/ThreadCount    1/' ${EW_INSTALL_HOME}/run/bin/wpr.conf

# expand tankfile full path
tnkfile="$(cd "$(dirname "$f")"; pwd)/$(basename "$tnkfile")"
# Run playback
echo "Using ${tnkfile}"

#export EW_INSTALL_HOME=/home/${USER}/ElarmS
#export EW_INSTALL_VERSION=earthworm-working

## running using detached screen
# ActiveMQ
if [[ ! `ps -ef | grep -v grep | grep $PWD | grep activemq.jar` ]]; then
  echo Starting ActiveMQ
  screen -dmS ActiveMQ bash -i -c "java -jar ${EW_INSTALL_HOME}/activemq/bin/activemq.jar start"
else
  echo "ActiveMQ already running"
fi
# DM
if [[ ! `ps -ef | grep -v grep | grep $PWD | grep "dmr dmr.conf"` ]]; then
  echo Starting DM
  screen -dmS DeciMod bash -i -c "cd ${EW_INSTALL_HOME}/run/bin;./dmr dmr.conf | ts \"%FT%H:%M:%.S|\" | tee ${basedir}/DMr_${fname}.log"
else
  echo "DM already running"
fi   
# E2
if [[ ! `ps -ef | grep -v grep | grep $PWD | grep "E2r E2r.conf"` ]]; then
  echo Starting E2r
  screen -dmS ElarmS2 bash -i -c "cd ${EW_INSTALL_HOME}/run/bin;./E2r E2r.conf -replay | ts \"%FT%H:%M:%.S|\" | tee ${basedir}/E2r_${fname}.log"
else
  echo "E2r already running"
fi    
# EWP2
if [[ ! `ps -ef | grep -v grep | grep $PWD | grep "EWP2r EWP2r.conf"` ]]; then
  echo Starting EWP2r
  screen -dmS ElarmSWP2 bash -i -c ". ${EW_INSTALL_HOME}/run/params/ew_linux.bash; cd ${EW_INSTALL_HOME}/run/bin;./EWP2r ReplayFile=${tnkfile} ChannelFile=channels.cfg params=EWP2r.conf | ts \"%FT%H:%M:%.S|\" | tee ${basedir}/EWP2r_${fname}.log"
else
  echo "EWP2r already running"
fi
echo ""
echo "Processing..."
while [[ `ps -ef | grep -v grep | grep $PWD | grep "E2r E2r.conf -replay"` ]]; do
	sleep 1
done
echo ""
IFS="
"
events=( $(grep "E:I:F:" ${basedir}/E2r_${fname}.log) )
if [ ${#events[@]} -gt 0 ]; then
  echo "Found ${#events[@]} events alerts"
  grep -m1 "^E:I:H: " ${basedir}/E2r_${fname}.log
  for ev in ${events[@]}; do
	echo ${ev}
  done
else
  echo "No alerts were found."
fi
echo ""
echo "Done."
