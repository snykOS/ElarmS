#!/usr/bin/bash
# Update ElViS and slink2ew.d file afer updating channels.cfg file.

awk '{print $1,$2,$5,$6}' run/bin/channels.cfg | sort -u > ElViS/stations.cfg
line=`grep Host run/params/slink2ew.d`
cp run/params/slink2ew.d.tmp run/params/slink2ew.d
awk '{print "Stream "$1"_"$2}' run/bin/channels.cfg | sort -u | grep -v "#" >> run/params/slink2ew.d
sed -i 's/\(Stream IS_MM[ABC].$\)/#\1/' run/params/slink2ew.d
sed -i 's/\(Stream GE_LHMI$\)/#\1/' run/params/slink2ew.d
sed -i "s/^SLhost.*/$line/" run/params/slink2ew.d
./ElarmSStop.bash
sleep 10
./ElarmsInit.bash