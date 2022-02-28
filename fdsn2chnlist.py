#!/usr/bin/env python
#/**********************************************************************************
#*    Copyright (C) by Ran Novitsky Nof                                            *
#*                                                                                 *
#*    This file is part of TRUAA                                                   *
#*                                                                                 *
#*    webppd is free software: you can redistribute it and/or modify               *
#*    it under the terms of the GNU Lesser General Public License as published by  *
#*    the Free Software Foundation, either version 3 of the License, or            *
#*    (at your option) any later version.                                          *
#*                                                                                 *
#*    This program is distributed in the hope that it will be useful,              *
#*    but WITHOUT ANY WARRANTY; without even the implied warranty of               *
#*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                *
#*    GNU Lesser General Public License for more details.                          *
#*                                                                                 *
#*    You should have received a copy of the GNU Lesser General Public License     *
#*    along with this program.  If not, see <http://www.gnu.org/licenses/>.        *
#***********************************************************************************/

# By Ran Novitsky Nof @ GSI, 2018
# ran.nof@gmail.com

from obspy.clients.fdsn import Client as client
from obspy import UTCDateTime
import re, sys
import argparse
import logging

logging.basicConfig(format='%(asctime)s | %(levelname)s | %(message)s',datefmt='%Y-%m-%dT%H:%M:%S',level=logging.DEBUG)

# command line parser
parser = argparse.ArgumentParser(
         formatter_class=argparse.RawDescriptionHelpFormatter,
         description='''Converting FDSN station data to ElarmS chnnel list''',
         epilog='''Example for proc file parameters and format:

Commandline example:
-i ".*\..*\..*\.[BHCSE][LHNC][ENZ012]$" -u "http://199.71.138.12:8181"

Created by Ran Novitsky Nof @ GSI, 2018
(ran.nof@gmail.com)''')
parser.add_argument('-i','--stationid',help="Station ID to process in a NET.STA.LOC.CHN format. Can use regex as in Python's re module.", required=True)
parser.add_argument('-u','--url',help='FDSN server (http://[host]:[port]) or data file (file://[filename]. If not using FDSN server, dataless (-d) oprion must be used.', required=True)
parser.add_argument('-s','--starttime',default=None,help='Start time for processing. Default is to use data span. Time format is any time format acceptable by obspy UTCDateTime. example: 20170101T01:02:03.444')
parser.add_argument('-e','--endtime',default=None,help='End time for processing. Default is to use data span. Time format is any time format acceptable by obspy UTCDateTime. example: 20170101T01:02:03.444')
parser.add_argument('-l','--loglevel',default='DEBUG',choices=['DEBUG', 'INFO', 'WARNING', 'ERROR', 'CRITICAL'],help='Logging level')
save_parser = parser.add_mutually_exclusive_group(required=False)
save_parser.add_argument('-o','--save', dest='save', action='store_true', help='Save to output file (default).')
save_parser.add_argument('--no-save', dest='save', action='store_false', help="Don't save to output file.")
parser.set_defaults(save=True)
parser.add_argument('-O','--outputfile',default='channels.cfg',help='Safe list to file.')

log = logging.getLogger('fdsn2chnlist')

class FDSN2CHN(object):
    def __init__(self,stationid, url, starttime, endtime, loglevel, save, outputfile):
        self.stationid = stationid
        self.url = url
        self.starttime = UTCDateTime(starttime or UTCDateTime.now())
        self.endtime = UTCDateTime(endtime or UTCDateTime.now())
        self.loglevel = loglevel
        self.log = log
        self.log.setLevel(loglevel)
        self.log.debug('Starttime: {}'.format(self.starttime))
        self.log.debug('Endtime: {}'.format(self.endtime))
        self.save = save
        self.outputfile = outputfile
        self.data = None
        self.channels = {}
        self.lines = ["# specify the columns. Any column order is okay and extra columns are allowed. Just need the names below.",
                      "#columns: network station location channel latitude longitude elevation samplerate gain units start end"]  
        self.fdsn_connect()
        self.get_data()
        self.get_chnlist()
        if self.save:
            self.tofile()
              
    def fdsn_connect(self):
        self.log.debug('Connecting to FDSN server: {}'.format(self.url))
        self.fdsn = client(self.url)
        
    def get_data(self):
        self.log.debug('Getting data from FDSN...')
        self.data = self.fdsn.get_stations(level='channel') 
          
    def get_chnlist(self):
        self.log.debug('Creating channel list.')
        for net in self.data.networks:
            for station in net.stations:
                for channel in station.channels:
                    cid = '{}.{}.{}.{}'.format(net.code, station.code, channel.location_code, channel.code)
                    if re.match(self.stationid,cid) and channel.is_active(starttime=self.starttime, endtime=self.endtime) and channel.response.instrument_sensitivity and 'None' not in channel.response.instrument_sensitivity.input_units and channel.response.instrument_sensitivity.value > 0:
                        if not cid in self.channels or UTCDateTime(self.channels[cid].split()[10]) < channel.start_date:
                            self.channels[cid] = '{N} {S} {L} {C} {lat} {lon} {elv} {sps} {gain} DU/{units} {start} {end}'.format(\
                            N=net.code, S=station.code, L = channel.location_code or '--', \
                            C=channel.code, lat=channel.latitude, lon=channel.longitude,\
                            elv=channel.elevation, sps=channel.sample_rate,\
                            gain=channel.response.instrument_sensitivity.value,\
                            units=channel.response.instrument_sensitivity.input_units, start=channel.start_date, end=channel.end_date)
        for l in self.channels.values():
            self.lines.append(l)
        self.log.debug('Total {} channels.'.format(len(self.lines)-2))
                 
    def tofile(self):
        self.log.debug('Saving to {}'.format(self.outputfile))
        with open(self.outputfile,'w') as f:
            f.write('\n'.join(self.lines))
            
if __name__ == '__main__':
    args = parser.parse_args(sys.argv[1:]) 
    FDSN2CHN(**(args.__dict__))          
