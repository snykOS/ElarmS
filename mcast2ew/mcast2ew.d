#
# q2ew configuration file
#
# This code receives MiniSEED records from CS2MCAST, converts them into
# Earthworm trace buf messages, and stuffs them into a wave ring.
#
#
 ModuleId	MOD_MCAST2EW	# module id for this import,
 RingName	TEST_WAVE_RING	# transport ring to use for input/output,


# new optional setting for sending RAW mseed packets as TYPE_MSEED to a ring
# MSEEDring      MSEED_RING
 HeartbeatInt	30		# Heartbeat interval in seconds
				# this should match the q2ew.desc heartbeat!

 LogFile	1		# If 0, don't write logfile at all,

 McastAddr      224.1.0.200	# the multicast address





# Kalpesh added latency and gap checks, mandatory and these need GapDir and LatencyDir set
GapDir  /tmp/gap		# where to write SCNL gap log files
LatencyDir /tmp/latent		# where to write SNCL latency log files
LatencyLogPeriod 60  		# how long to wait before logging

# To turn off the above checks, which use a lot of cycles and disk writes, uncomment this line below
#NoLatencyLogging

# these Mcast directives are for each station to be received
# format is Mcast NN SSSSS  McastIF  Port
# where NN is SEED network code
# 	SSSSS is SEED station code
# 	McastIF is the Multicast Interface usually on the 10 net
#	Port the port number to listen for mcast packets for this net.sta
# DEPRECATED FORMAT
# Mcast	CI  WBS  10.1.1.6 8236

# new format:
# These McastRingChannels directives are for each station to be 
# received and put in a ring, there can be more than one of 
# these per sta format is:
#
#     McastRingChannels  NN SSSSS  McastIF McastPort Ring comma_sep_chan_list
McastRingChannels CI ARV  0.0.0.0 8314 USARRAY_RING BHE,BHN,BHZ,LDI,LDO,LHE,LHN,LHZ,VME,VMN,VMU,VMV,VMW,VMZ
McastRingChannels CI BBR  0.0.0.0 8174 USARRAY_RING BHE,BHN,BHZ,LDI,LDO,LHE,LHN,LHZ,VME,VMN,VMU,VMV,VMW,VMZ
McastRingChannels CI BC3  0.0.0.0 8079 USARRAY_RING BHE,BHN,BHZ,LDI,LDO,LHE,LHN,LHZ,VME,VMN,VMU,VMV,VMW,VMZ
McastRingChannels CI BEL  0.0.0.0 8210 USARRAY_RING BHE,BHN,BHZ,LDI,LDO,LHE,LHN,LHZ

# ChannelFile - not used yet - could be used for channel selection
#ChannelFile	/home/rtem/channels/channels_mcast2ew.cfg


# SCN2pinmap - not used  yet
# These are optional for those EW users who want to have a pin number
# instead of a SCN. The keyword here is SCN2pinmap. No duplicate checking
# occurs

# Example
#		S	C    	N	pin
#SCN2pinmap 	Q003	HHZ  	AT	1
#SCN2pinmap 	Q003	HHN  	AT	2
#SCN2pinmap 	Q003	HHE  	AT	3
