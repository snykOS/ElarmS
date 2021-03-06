#  export_scn_epic configuration file
#
#  Exports messages from a given list of station/channel/network codes
#  Knows how to decipher TYPE_TRACEBUF, TYPE_PICK2K, TYPE_CODA2K, TYPE_ELARMS_PGM msgs
#
#Background:  
# Import/Export are the long-distance message transfer
# modules. They operate as one-on-one pairs. There are two variants of
# export: export_generic, which ships messages with specified logos, and
# export_scn which ships only trace data, but with specified SCN names.
#
# Import/Export send heartbeats to each other, as well as into their
# local earthworm systems. If the heartbeat from the distant partner is
# not received whithin the expected time period (RcvAliveInt) the link is
# terminated, and an attempt to reconnect is initiated. If things go
# seriously wrong, the heartbeat into the local earthworm system in
# stopped. The expectation is that "restartMe" has been set in the .desc
# file, and we'll be killed and restarted.
#
# All socket operations are performed with a timeout. This is noramlly
# defaulted, but can be set in this file (SocketTimeout).
#
# Export maintains a circular FIFO buffer of messages to be shipped. The
# size of this buffer (RingSize) controls the maximum latency of the
# data.
#
#
#Configuration notes:
#
# "restartMe" should be stated in our desc file.
#
# The period of our local heartbeat (HeartBeatInt) must be safely smaller
# (faster) than our advertised period in our .desc file (tsec:).
# Otherwise we'll get continually restarted for no good reason.
# Note that tsec:0 implies no heartbeats expected, and so we'll never get
# restarted.
#
# The rate at which we send heartbeats to our distant partner should be
# considerably faster than the rate at which our partner expects them.
# Otherwise, a heartbeat delayed in transmission will cause our partner to
# conclude that the link is broken, and cause them to break the link and
# reinitialize. Which will cause us to do the same.
#
# For export, the ServerIPAdr is  the address of the port to be used in
# the exporting machine.  This is to specify a network card case the
# exporting machine has several network cards.
#
# If SocketTimeout is specified, it should be at least as large as the
# expected period of heartbeats from our distant partner.
#
 MyModuleId     MOD_WP_ELARMS		# module id for this program
 RingName       WAVE_RING		# transport ring to use for input/output
 HeartBeatInterval   30			# EW internal heartbeat interval (sec)
					#   Should be >= RcvAliveInt 

 LogFile        1      # If 0, don't write logfile
                       #    1, write to logfile and stdout
                       #    2, write to module log but not stderr/stdout
#Verbose               # If uncommented, VERY LARGE logfiles will be  
                       #   generated with info about queue status of  
                       #   each msg, socket alive msgs sent & received.

# Logos of messages to export to client systems.
#------------------------------------------------
# Installation and Module can be wildcards.
# Knows how to decipher these message types:
#  w/  location code: TYPE_TRACEBUF2, TYPE_TRACE2_COMP_UA, 
#                     TYPE_PICK_SCNL, TYPE_CODA_SCNL
#  w/o location code: TYPE_TRACEBUF, TYPE_TRACE_COMP_UA,
#                     TYPE_PICK2K, TYPE_CODA2K msgs
#
#              Installation     Module           Message Type
 GetTraceFrom  INST_WILDCARD    MOD_WILDCARD     TYPE_TRACEBUF2
 
