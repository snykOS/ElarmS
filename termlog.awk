#!/usr/bin/awk  -f
# By Ran Novitsky Nof, 2014 @ BSL
# ran.nof@gmail.com
# log stdout lines with a UTC time stamp.
# example usage:
# NAME=[some program name]
# [some program] | termlog.awk -v PROG=${NAME}
# this will redirect the some program stdout to a daily file in log directory.
{ 
  system("echo `date -u +\"%T.%3N|\"`\""$0"\"  >> ../log/"PROG"/"PROG"_"strftime("%Y%m%d",systime(),1)".log")
  # turn next line to enable/disable terminal output
  print $0
}

