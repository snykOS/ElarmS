/***********************************************************

File Name :
        MessageTypes.h

Original Author:
        Patrick Small

Description:

        This header defines the Talarian message types which are 
exchanged among processes in the Real-time and Data Acquisition 
system. Each message type requires a unique identifier (as specified 
in the Message Format Description document), a string label, and a 
format string.


Creation Date:
        02 July 1998

Modification History:


Usage Notes:


**********************************************************/

#ifndef messagetypes_H
#define messagetypes_H



// Maximum length of a message type label
//
const int MTYPE_MAX_LABEL = 64;

// Maximum length of a message type format specifier
//
const int MTYPE_MAX_FORMAT = 512;


// Maximum number of message types
//
// This must be updated as new message types are added!
//
const int MTYPE_MAX_MTYPES = 25;


// Available user-defined message types for TriNet
//
const int TN_TMT_UNKNOWN          = 0;
const int TN_TMT_TRACE_BUF        = 2000;
const int TN_TMT_EVENT_LOC        = 2001;
const int TN_TMT_EVENT_CANCEL     = 2002;
const int TN_TMT_EVENT_LOC_MAG    = 2003;
const int TN_TMT_REQUEST_CARD     = 2004;
const int TN_TMT_ALARM            = 2005;
const int TN_TMT_ASSOC_LOC        = 2006;
const int TN_TMT_EVENT_AMP        = 2007;
const int TN_TMT_ASSOC_CANCEL     = 2008;
const int TN_TMT_DSS_SUMMARY      = 2009;
const int TN_TMT_RNS_SHAKING      = 2010;
const int TN_TMT_PROC_STATUS      = 2011;
const int TN_TMT_PROC_MSG         = 2012;
const int TN_TMT_SYS_CONTROL      = 2013;
const int TN_TMT_EVENT_SIG        = 2014;
const int TN_TMT_ASSOC_LOC_FILE   = 2016;
const int TN_TMT_EVENT_UPDATE_SIG = 2017;
const int TN_TMT_ACTION_SIG       = 2018;
const int TN_TMT_ALARM_CANCEL     = 2019;
const int TN_TMT_EVENT_TRIG       = 2020;
const int TN_TMT_SUBNET_TRIG      = 2021;
const int TN_TMT_DEBUG            = 2022;
const int TN_TMT_TRIGGER_SIG      = 2023;
const int TN_TMT_HEARTBEAT        = 2024;


// Available user-defined message types for SCAN
//
const int TN_SMT_AMPLITUDE        = 3000;
const int TN_SMT_TRIGGER          = 3001;
const int TN_SMT_EVENT            = 3002;


// Remapped predefined messages
//
//const int TN_TMT_PHASE_PICK = T_MT_STRING_DATA;



// Structure defining a message definition
//
struct msgdef {
    int id;
    char label[MTYPE_MAX_LABEL];
    char format[MTYPE_MAX_FORMAT];
};



// Array of message definitions to be loaded by the Connection class
// during initialization.
//
const struct msgdef msgDefs[] = { 

    // Fields: Reference Message Format Description document
    //
    // NOTES:
    //
    // TN_TMT_TRACE_BUF messages: The format of the last field may
    // be int2_array or int4_array, based upon the value of one of the str
    // fields.
    //
    // SmartSockets allows messages to contain data fields which are different
    // from the described format.
    //
    {TN_TMT_TRACE_BUF, "Trace_Buffer", 
     "int4 int4 int4 int4 int4 int4 real8 str str str str str int4_array"},

    // Fields: 
    //      Event ID (4-byte integer)
    //      Network identifier
    //      Source identifier
    //      Modification count flag
    //      Location is final flag
    //
    //      Associator assigned event ID
    //      Network ID of the associator
    //      Source ID of the associator
    //      Program ID of the associator
    //      Event time
    //      Latitude (decimal degrees)
    //      Longitude (decimal degrees)
    //      Depth
    //      Magnitude
    //      Number of phases w/ weights > 0.1
    //      Max azimuthal gap
    //      Distance to nearest station
    //      Travel time residual rms
    //      Horizontal error (km)
    //      Vertical error (km)
    //      Quality
    //      Number of picks
    //
    //      Remainder of message composed of picked channel information.
    //      Number of channels in the arrays is given by the Number of phases
    //      field above.
    // 
    //      Network ID array
    //      Station ID array
    //      Channel ID array
    //      Phase pick type
    //      Phase remark array
    //      First motion array
    //      Assigned weight array
    //      Actual weight array
    //      Arrival time array
    //      Travel time residual array
    //
    {TN_TMT_EVENT_LOC, "Event_Location", "int4 str str int4 int4 int4 str str str real8 real4 real4 real4 real4 int4 int4 real4 real4 real4 real4 char int4 str_array str_array str_array str_array str_array str_array int4_array real4_array real8_array real4_array"},
    
    // Fields: 
    //      Event ID
    //      Network
    //      Source
    //
    {TN_TMT_EVENT_CANCEL, "Event_Cancel", "int4 str str"},
    

    // Fields: 
    //      Magnitude availability flag
    //      Magnitude type
    //      Magnitude
    //      Magnitude is final flag
    //      Number of channels used in magnitude
    //
    //      Event ID (4-byte integer)
    //      Network identifier
    //      Source identifier
    //      Modification count flag
    //      Location is final flag
    //
    //      Associator assigned event ID
    //      Event time
    //      Latitude (decimal degrees)
    //      Longitude (decimal degrees)
    //      Depth
    //      Magnitude
    //      Number of phases w/ weights > 0.1
    //      Max azimuthal gap
    //      Distance to nearest station
    //      Travel time residual rms
    //      Horizontal error (km)
    //      Vertical error (km)
    //      Quality
    //      Number of picks
    //
    //      Remainder of message composed of picked channel information.
    //      Number of channels in the arrays is given by the Number of phases
    //      field above.
    // 
    //      Network ID array
    //      Station ID array
    //      Channel ID array
    //      Phase pick type
    //      Phase remark array
    //      First motion array
    //      Assigned weight array
    //      Actual weight array
    //      Arrival time array
    //      Travel time residual array
    //
    {TN_TMT_EVENT_LOC_MAG, "Event_Location_and_Magnitude", "int4 str real4 int4 int4 int4 str str int4 int4 str int4 real8 real4 real4 real4 real4 int4 int4 real4 real4 real4 real4 char int4 str_array str_array str_array str_array str_array str_array int4_array real4_array real8_array real4_array"},

    // Fields: 
    //      Event ID
    //      Network
    //      Source
    //      Number of selected channels
    //      Network ID array
    //      Station ID array
    //      Channel ID array
    //      Start time array
    //      End time array
    //
    {TN_TMT_REQUEST_CARD, "Request_Card", 
     "int4 str str int4 str_array str_array str_array real8_array real8_array"},
    
    // Fields: 
    //      Event ID
    //      Alarm type string
    //      Alarm level (obsolete)
    //      Event magnitude (obsolete)
    //
    //
    {TN_TMT_ALARM, "Alarm", "int4 str int4 real4"},
    
    
    // Fields:
    //
    //      Associator assigned event ID
    //      Network ID of the associator
    //      Source ID of the associator
    //      Program ID of the associator
    //      Event time
    //      Latitude (decimal degrees)
    //      Longitude (decimal degrees)
    //      Depth
    //      Magnitude
    //      Number of phases w/ weights > 0.1
    //      Max azimuthal gap
    //      Distance to nearest station
    //      Travel time residual rms
    //      Horizontal error (km)
    //      Vertical error (km)
    //      Quality
    //      Number of picks
    //
    //      Remainder of message composed of picked channel information.
    //      Number of channels in the arrays is given by the Number of phases
    //      field above.
    //
    //      Network ID array
    //      Station ID array
    //      Channel ID array
    //      Phase pick type
    //      Phase remark array
    //      First motion array
    //      Assigned weight array
    //      Actual weight array
    //      Arrival time array
    //      Travel time residual array
    //
    {TN_TMT_ASSOC_LOC, "Associator_Location", "int4 str str str real8 real4 real4 real4 real4 int4 int4 real4 real4 real4 real4 char int4 str_array str_array str_array str_array str_array str_array int4_array real4_array real8_array real4_array"},

    // Fields: 
    //      Event ID
    //      Network (auth in db)
    //      Source  (subsource eg. RT1)
    //      Number of amplitudes
    //      Network ID array
    //      Station ID array
    //      Channel ID array
    //      Amplitude type array
    //      Amplitude Units array
    //      Amplitude reading array
    //      Time reading taken array
    //      Channel Magnitude array
    //
    {TN_TMT_EVENT_AMP, "Event_Amplitudes", 
     "int4 str str int4 str_array str_array str_array str_array str_array real4_array real8_array real4_array"},


    // Fields: 
    //      Location ID
    //      Network ID
    //      Source ID
    //      Program ID
    //
    {TN_TMT_ASSOC_CANCEL, "Associator_Cancel", "int4 str str str"},


    // Fields: 
    //      Network ID
    //      Station ID
    //      Channel ID
    //      Time of summary
    //      Peak acceleration
    //
    {TN_TMT_DSS_SUMMARY, "DSS_Summary", "str str str real8 real4"},


    // Fields:
    //      Message Sequence Number
    //      Time Message Generated
    //      Event Number
    //      Confidence Level
    //      First Trigger Network ID
    //      First Trigger Station ID
    //      First Trigger Channel ID
    //      First Trigger Latitude
    //      First Trigger Longitude
    //      First Trigger Time of Reading
    //      First Trigger Reading
    //      Peak Acceleration is available flag
    //      Peak Acceleration Network ID
    //      Peak Acceleration Station ID
    //      Peak Acceleration Channel ID
    //      Peak Acceleration Latitude
    //      Peak Acceleration Longitude
    //      Peak Acceleration Time of Reading
    //      Peak Acceleration Reading
    //      Peak Velocity is available flag
    //      Peak Velocity Network ID
    //      Peak Velocity Station ID
    //      Peak Velocity Channel ID
    //      Peak Velocity Latitude
    //      Peak Velocity Longitude
    //      Peak Velocity Time of Reading
    //      Peak Velocity Reading
    //
    {TN_TMT_RNS_SHAKING, "RNS_Shaking_Report", 
     "int4 real8 int4 int4 str str str real4 real4 real8 real4 int4 str str str real4 real4 real8 real4 int4 str str str real4 real4 real8 real4"},

    // Fields:
    //      String identifier of process
    //      PID of process
    //      Timestamp message was generated
    //
    {TN_TMT_PROC_STATUS, "Process_Status", "str int4 real8"},

    // Fields:
    //      String identifier of process
    //      String message
    //      Timestamp that message was generated
    //
    {TN_TMT_PROC_MSG, "Process_Message", "str str real8"},

    // Fields:
    //      Command code
    //
    {TN_TMT_SYS_CONTROL, "System_Control", "int4"},

    // Fields:
    //      Event identifier
    //
    {TN_TMT_EVENT_SIG, "Event_Signal", "int4"},

    // Fields:
    //
    //      Network ID of the associator
    //      Source ID of the associator
    //      Algorithm of the associator
    //      Algorithm of the magnitude
    //      Filename of hypoinverse archive message
    //      
    //
    {TN_TMT_ASSOC_LOC_FILE, "Associator_File_Message", "str str str str str"},

    // Fields:
    //      Event identifier
    //
    {TN_TMT_EVENT_UPDATE_SIG, "Event_Update_Signal", "int4"},

    // Fields:
    //      Event identifier (Dummy field)
    //
    {TN_TMT_ACTION_SIG, "Action_Signal", "int4"},

    // Fields:
    //      Event identifier
    //      Action type string
    //
    {TN_TMT_ALARM_CANCEL, "Alarm_Cancel", "int4 str"},

    // Fields:
    //      Event detector assigned event ID
    //      Network ID of the Event detector
    //      Source ID of the Event detector
    //      Program ID of the Trigger detector
    //      Event time
    //      Latitude
    //      Longitude
    //      Network identifier (1st triggered station)
    //      Station identifier (1st triggered station)
    //      Channel identifier (1st triggered station)
    //      Peak amplitude
    //      Number of confirming stations
    //      Amplitude threshold
    //
    {TN_TMT_EVENT_TRIG, "Event_Trigger", "int4 str str str real8 real4 real4 str str str real4 int4 real4"},

    // Fields:
    //      Trigger detector assigned event ID
    //      Network ID of the Trigger detector
    //      Source ID of the Trigger detector
    //      Program ID of the Trigger detector
    //      Event time
    //      Global flag
    //      Global trigger start time
    //      Global trigger duration
    //      Number of triggered channels
    //      Network identifier array
    //      Station identifier array
    //      Channel identifier array
    //      Trigger time array
    //      Trigger start time array
    //      Trigger duration array
    //
    {TN_TMT_SUBNET_TRIG, "Subnet_Trigger", "int4 str str str real8 int4 real8 real8 int4 str_array str_array str_array real8_array real8_array real8_array"},

    // Fields:
    //      Debug message
    //
    {TN_TMT_DEBUG, "Debug", "str"},

    // Fields:
    //      Network Trigger identifier
    //
    {TN_TMT_TRIGGER_SIG, "Trigger_Signal", "int4"},

    {TN_TMT_HEARTBEAT,"Heartbeat","int4"},
    // Fields:
    //      Network
    //      Station
    //      Channel
    //      Time of peak amplitude
    //      Peak acceleration
    //      Peak velocity
    //      Peak displacement
    //
    {TN_SMT_AMPLITUDE, "SCAN_Amp", "str str str real8 real8 real8 real8"},

    // Fields:
    //      Network
    //      Station
    //      Channel
    //      Time of trigger
    //      Trigger type
    //      Trigger value
    //
    {TN_SMT_TRIGGER, "SCAN_Trigger", "str str str real8 str real8"},

    // Fields:
    //      Event ID
    //      Number of Triggers
    //      Number of Periods
    //      Latitude
    //      Longitude
    //      Depth
    //      Event Time
    //      Magnitude
    //      bevA0arh parameter
    //      bevA0arv parameter
    //      bevA0ach parameter
    //      bevA0acv parameter
    //      bevA0vrh parameter
    //      bevA0vrv parameter
    //      bevA0vch parameter
    //      bevA0vcv parameter
    //
    //      Trigger Info:
    //      station
    //      btrigPstat
    //      btrigPpdT
    //      btrigPpklza
    //      btrigPpklna
    //      btrigPpklea[0]
    //      btrigPpklea[1]
    //      btrigPpklea[2]
    //      btrigPpklea[3]
    //      btrigPpklea[4]
    //      btrigPpklzv
    //      btrigPpklnv
    //      btrigPpklev
    //
    //      Period Info:
    //      station
    //      btspkhza
    //      btspkhzv
    //      btspkhzd
    //      btspkhna
    //      btspkhnv
    //      btspkhnd
    //      btspkhea
    //      btspkhev
    //      btspkhed
    //      btspklza
    //      btspklzv
    //      btspklzd
    //      btspklna
    //      btspklnv
    //      btspklnd
    //      btspklea
    //      btspklev
    //      btspkled
    //
    {TN_SMT_EVENT, "SCAN_Event", "int4 int4 int4 real8 real8 real8 real8 real8 real8 real8 real8 real8 real8 real8 real8 real8 str_array int4 real4_array real4_array real4_array real4_array real4_array real4_array real4_array real4_array real4_array real4_array real4_array str_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array real8_array"}

};


//
// Constraints on message field sizes
//

// Maximum size of the str field in a TN_TMT_ASSOC_ARCHIVE message
//
const int MTYPE_MAX_ASSOC_ARCHIVE_SIZE = 65536;


// Maximum number of selected channels in a TN_TMT_REQUEST_CARD message
//
const int MTYPE_MAX_REQUEST_CARD_CHANNELS = 2048;


// Maximum length for an alarm type name in a TN_TMT_ALARM message
//
const int MTYPE_MAX_ALARM_TYPE_NAME_LEN = 64;


// Maximum number of phase picks in a TN_TMT_ASSOC_LOC message
//
const int MTYPE_MAX_ASSOC_LOC_PHASE_PICKS = 2048;


// Maximum number of amplitude readings in a TN_TMT_EVENT_AMP message
//
const int MTYPE_MAX_EVENT_AMP_READINGS = 32768;


// Maximum number of triggers in a TN_TMT_SUBNET_TRIG message
//
const int MTYPE_MAX_SUBNET_CHANNEL_TRIGGERS = 2048;

#endif
