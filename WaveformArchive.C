/***********************************************************

File Name :
        WaveformArchive.C

Original Author:
        Patrick Small

Description:


Creation Date:
        17 June 1999


Modification History:


Usage Notes:


**********************************************************/

// Various include files
#include <iostream>
#include <cstring>
#include "RetCodes.h"
#include "WaveformArchive.h"


WaveformArchive::WaveformArchive()
{
  strcpy(archloc, "");
  strcpy(filename, "");
  strcpy(subdir, "");
  fileid = 0;
  filesize = 0;
  offset = 0;
  length = 0;
  wtype = WAVE_UNKNOWN;
  format = WAVE_FORMAT_UNKNOWN;
  dataformat = WAVE_DATAFORMAT_UNKNOWN;
  wordorder = WAVE_WORDORDER_UNKNOWN;
  recsize = 0;
  status = WAVE_STATUS_EMPTY;
  rec_type = 'D';
  wfid = 0;
}


WaveformArchive::WaveformArchive(const WaveformArchive &w)
{
  chan = w.chan;
  reqstart = w.reqstart;
  win = w.win;
  strcpy(archloc, w.archloc);
  strcpy(filename, w.filename);
  strcpy(subdir, w.subdir);
  fileid = w.fileid;
  filesize = w.filesize;
  offset = w.offset;
  length = w.length;
  wtype = w.wtype;
  format = w.format;
  dataformat = w.dataformat;
  wordorder = w.wordorder;
  recsize = w.recsize;
  status = w.status;
  wfid = w.wfid;
  rec_type = w.rec_type;
}


WaveformArchive::~WaveformArchive()
{
}


WaveformArchive& WaveformArchive::operator=(const WaveformArchive &w)
{
  chan = w.chan;
  reqstart = w.reqstart;
  win = w.win;
  strcpy(archloc, w.archloc);
  strcpy(filename, w.filename);
  fileid = w.fileid;
  filesize = w.filesize;
  offset = w.offset;
  length = w.length;
  wtype = w.wtype;
  format = w.format;
  dataformat = w.dataformat;
  wordorder = w.wordorder;
  recsize = w.recsize;
  status = w.status;
  wfid = w.wfid;
  rec_type = w.rec_type;
  return(*this);
}

