#
# qmcast2ew configuration file
#
# This code receives onesec channel packets from qmaserv2, converts them into
# Earthworm trace buf messages, and stuffs them into a wave ring.
#
#
ModuleId	MOD_QMCAST2EW	# module id for this import,
RingName	EEW_TRACE_RING	# transport ring to use for input/output,

HeartbeatInt	30		# Heartbeat interval in seconds
				# this should match the q2ew.desc heartbeat!

LogFile		1		# If 0, don't write logfile at all,

McastAddr	224.0.0.121	# the multicast address


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
#McastRingChannels	Net	Sta	McastIF Mport	Ring 		comma_sep_chan_list
McastRingChannels	BK	BDM	0.0.0.0	21002	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BKS	0.0.0.0	21003	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BL67	0.0.0.0	21094	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BL88	0.0.0.0	21092	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BRIB	0.0.0.0	21004	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BRIE	0.0.0.0	21076	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BRIM	0.0.0.0	21078	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	BRK	0.0.0.0	21006	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	CMB	0.0.0.0	21007	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	CVS	0.0.0.0	21010	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	FARB	0.0.0.0	21012	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	GASB	0.0.0.0	21064	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	HAST	0.0.0.0	21052	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	HATC	0.0.0.0	21062	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	HELL	0.0.0.0	21060	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	HOPS	0.0.0.0	21013	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	HUMO	0.0.0.0	21046	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	JCC	0.0.0.0	21042	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	JRSC	0.0.0.0	21014	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	JRSE	0.0.0.0	21056	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	JRSM	0.0.0.0	21058	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	KCC	0.0.0.0	21084	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	MCCM	0.0.0.0	21070	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	MHC	0.0.0.0	21016	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	MHDM	0.0.0.0	21074	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	MNRC	0.0.0.0	21051	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	MOD	0.0.0.0	21017	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	ORV	0.0.0.0	21018	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	PACP	0.0.0.0	21086	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	PETB	0.0.0.0	21090	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	PKD	0.0.0.0	21020	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	RAMR	0.0.0.0	21054	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	RFSB	0.0.0.0	21043	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	SAO	0.0.0.0	21026	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	SCCB	0.0.0.0	21033	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	SUTB	0.0.0.0	21068	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	VAK	0.0.0.0	21088	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	VALB	0.0.0.0	21072	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	WDC	0.0.0.0	21029	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	WENL	0.0.0.0	21030	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	YBH	0.0.0.0	21031	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
McastRingChannels	BK	YBH	0.0.0.0	21096	EEW_TRACE_RING	HHE,HHN,HHZ,HNE,HNN,HNZ
# YBH appears twice because there are 2 Q330 at YBH, each multicast on a separate port.

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

# Optional parameters:
LatencyLogPeriod	60				# Latency logging interval in seconds
LatencyDir              /home/ncss/run/logs/qmcast2ew	# Latency log dir
GapDir                  /home/ncss/run/logs/qmcast2ew	# Gap log dir

