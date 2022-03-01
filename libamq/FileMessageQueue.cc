#include "FileMessageQueue.h"
#include "QWMessageSerializer.h"
#include "QWMessageParser.h"
#include "ContentTable.h"
#include "Crc.h"
#include "Compat.h"
#include <stdio.h>

const char* magicstr = "MSGQUEUE";

unsigned long getCRC(const char* buf,unsigned int len);
void printFP(fstream* f);
void printBuf(int* buf,int len);

FileMessageQueue::FileMessageQueue(string _filename,int max) throw(RTIOException,RTFileCorruptedError){
    valid = false;
    filename = _filename;
    qfile = NULL;
    max_size  = max;

    init();
    valid = true;
}

void FileMessageQueue::init() throw(RTIOException,RTFileCorruptedError){
    openFile();

    if(!isFileValid()){
      throw RTIOException("Queue file is corrupted!");
    }

    //if length is zero means file is new or empty - format it
    if(length==0){
	//Format it//
	bzero((char*)&header,sizeof(MQHeader));
	strcpy(header.magicstr,magicstr);
	strcpy(header.queuename,"Persistent Message Queue For Trinet Program");
	header.bookmark = sizeof(MQHeader);
	header.msgcount = 0;
	header.pending = 0;
	qfile->write((char*)&header,sizeof(MQHeader));
    }

    if(qfile->fail())
	throw RTIOException("Error in [FileMessageQueue::init()]: IO Error occured in the last IO operation");

    closeFile(header.bookmark);
}

void FileMessageQueue::reset(){
    try{
	unlink(filename.c_str());
	valid = false;
	delete qfile;
	qfile = NULL;

	init();
	valid = true;	
    }
    catch(...){

    }
}

void FileMessageQueue::put(const Message& msg) throw(RTIOException,RTOutOfMemoryException){
    //Check if file is too long//
    //If file message queue has more than N messages, delete it//
    if(size()>=max_size && pending()==0){
	unlink(filename.c_str());
	valid = false;
	delete qfile;
	qfile = NULL;

	init();
	valid = true;
    }

    //simply write message into the queuefile
    MQBlock block;
    QWMessageSerializer ms;
    
    openFile();
    qfile->seekp(0,ios::end);
    qfile->seekg(0,ios::end);
    const char* buf = ms.serialize(msg);

    bzero((void*)&block,sizeof(MQBlock));
    block.state = UNSERVED;
    int type = 0;
    msg.getType(type);
    block.msgtype = type;

    block.msglen = strlen(buf)+1;
    block.crc = 0x00000000;
    
    //Calculate CRC and write block into the file//
    unsigned int bigbuflen = sizeof(MQBlock)+block.msglen;
    char* bigbuf = (char*)malloc(bigbuflen);
    if(!bigbuf){
	throw RTOutOfMemoryException("Error in (FileMessageQueue::put) : malloc return NULL while allocating memory for temporary buffer");
    }

    bzero((void*)bigbuf,bigbuflen);
    bcopy((const void*)&block,(void*)bigbuf,sizeof(MQBlock));    
    bcopy((const void*)buf,(void*)(bigbuf+sizeof(MQBlock)),block.msglen);
    block.crc = getCRC(bigbuf,bigbuflen);

    unsigned int old_p = qfile->tellp();
    qfile->write((char*)&block,sizeof(MQBlock));

    free(bigbuf);

    if(qfile->fail()){
      free((void*)buf);
      closeFile(header.bookmark);
      throw RTIOException("Error in (FileMessageQueue::put) : I/O Error occured while writing header in queue file");
    }
    
    qfile->write(buf,block.msglen);
    free((void*)buf);
    
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in (FileMessageQueue::put) : I/O Error occured while writing message buffer in queue file");
    }  
    
    header.msgcount++;
    header.pending++;
    closeFile(header.bookmark);
}

Message* FileMessageQueue::get() throw(RTException,RTIOException,RTFileCorruptedError,RTOutOfMemoryException){
    //read next message from the file//
    MQBlock block;
    QWMessageParser parser;
    char* buf = NULL;
    
    openFile();

    qfile->seekg(header.bookmark);
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::get() : I/O Error occured while re-locating read pointer");	
    }

    if(length - qfile->tellg() == 0){
      closeFile(header.bookmark);
      return NULL;
    }

    qfile->read((char*)&block,sizeof(MQBlock));
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::get(): I/O Error occured while reading the file");
    }

    if(qfile->eof()){
      closeFile(header.bookmark);
      throw RTFileCorruptedError("Error in FileMessageQueue::get(): No data follows after message block. Queue file must be corrupted");

    }

    buf = (char*)malloc(block.msglen);
    if(!buf){
      closeFile(header.bookmark);
      throw RTOutOfMemoryException("Error in FileMessageQueue::get(): OutOfMemory");
    }

    qfile->read(buf,block.msglen);

    if(qfile->fail()){
      free(buf);
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::get(): I/O Error occured while reading the file");
    }

    //Create message from the buffer//
    ContentTable* ctable = parser.parse(buf);
    if(!ctable){
	free(buf);
	closeFile(header.bookmark);
	throw RTException("Error in FileMessageQueue::get(): XML Parser returned no data after scanning the message buffer");
    }

    Message* msg = new Message(ctable,block.msgtype);
    if(!msg){
	free(buf);
	delete ctable;
	closeFile(header.bookmark);
	throw RTOutOfMemoryException("Error in FileMessageQueue::get(): OutofMemory, Can't allocate memory for Message object");
    }
    msg->setDomainName(parser.getDomainName());
    msg->setDest(parser.getChannelName());
    msg->setSourceName(parser.getSourceName());

    free(buf);
    closeFile(header.bookmark);
    
    return msg;
}

void FileMessageQueue::remove() throw(RTIOException,RTFileCorruptedError){
    //remove the previous message from the queue//
    openFile();

    qfile->seekg(header.bookmark);
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::remove() : I/O Error occured while re-locating read pointer");	
    }
    if(length - qfile->tellg() == 0){
      closeFile(header.bookmark);
      return;
    }

    MQBlock block;
    bzero((void*)&block,sizeof(MQBlock));
    qfile->read((char*)&block,sizeof(MQBlock));
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::remove() : I/O Error occured while reading");
    }
    if(qfile->eof()){
      closeFile(header.bookmark);
      throw RTFileCorruptedError("Error in FileMessageQueue::remove() : Queue file is corrupted");
    }

    block.state = SERVED;
    block.crc = 0x00000000;
    block.crc = calculateBlockCRC(block);

    qfile->seekp(header.bookmark);
    qfile->seekg(header.bookmark);

    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::remove() : I/O Error occured while re-locating write pointer");
    }    

    qfile->write((char*)&block,sizeof(MQBlock));
    if(qfile->fail()){
      closeFile(header.bookmark);
      throw RTIOException("Error in FileMessageQueue::remove() : I/O Error occured while writing");
    }
    
    header.pending--;
    closeFile(header.bookmark + block.msglen + sizeof(MQBlock));
}

unsigned int FileMessageQueue::size() throw(RTIOException,RTFileCorruptedError){
    openFile();
    unsigned int len = header.msgcount;
    closeFile(header.bookmark);
    return len;
}

unsigned int FileMessageQueue::pending() throw(RTIOException,RTFileCorruptedError){
    openFile();
    unsigned int pending = header.pending;
    closeFile(header.bookmark);
    return pending;
}

bool FileMessageQueue::isValid(){
    return valid;
}

bool operator !(const FileMessageQueue& q){
    return !q.valid;
}



//PRIVATE//

//This function assumes that file pointer is pointing to start of message data//
bool FileMessageQueue::isBlockCorrupted(const MQBlock& block) throw(RTIOException){
    unsigned long crc = calculateBlockCRC(block);
    if(block.crc == crc){
	//cout << "Block is OK"<<endl;
	return false;
    }
    else{
	//cout <<"Block is NOT OK"<<endl;
    }
    return true;
}

//This function assumes that file pointer is pointing to start of message data//
unsigned long FileMessageQueue::calculateBlockCRC(const MQBlock& _block) throw(RTIOException){
    MQBlock block = _block;
    block.crc = 0x00000000;
    unsigned int bigbuflen = sizeof(MQBlock)+block.msglen;

    char* buf = (char*)malloc(block.msglen);
    char* bigbuf = (char*)malloc(bigbuflen);
    
    bzero(buf,block.msglen);
    bzero(bigbuf,bigbuflen);

    qfile->read(buf,block.msglen);
    if(qfile->fail()){
	free(buf);
	free(bigbuf);
	throw RTIOException("Error in [FileMessageQueue::calculateBlockCRC]: I/O Error occured while reading");
    }
    qfile->seekg(qfile->tellg() - block.msglen);
    if(qfile->fail()){
	free(buf);
	free(bigbuf);
	throw RTIOException("Error in [FileMessageQueue::calculateBlockCRC]: I/O Error occured while seeking");
    }   

    bcopy((const void*)&block,(void*)bigbuf,sizeof(MQBlock));    
    bcopy((const void*)buf,(void*)(bigbuf+sizeof(MQBlock)),block.msglen);

    //printBuf((int*)bigbuf,bigbuflen);

    unsigned long crc =  getCRC(bigbuf,bigbuflen);
    //cout<<"CRC:"<<hex<<crc<<dec<<endl;

    free(buf);
    free(bigbuf);
    return crc;
}


bool FileMessageQueue::isFileValid() throw(RTIOException){
    
    //Get file length//
    unsigned int long old_g = qfile->tellg();

    qfile->seekg (0, ios::end);
    length = qfile->tellg();
    qfile->seekg (0, ios::beg);
    
    //if length is zero means file is new or empty 
    if(length!=0){
	//	MQHeader header;
       	qfile->read((char*)&header,sizeof(MQHeader));
	if(qfile->fail()){
	    throw RTIOException("Error in [FileMessageQueue::FileMessageQueue]: Error while reading the file");
	}
	if(strcmp(header.magicstr,magicstr)!=0){
	    return false;
	}
	//Scan through the file for corrupted blocks//
	if(!qfile->eof()){
	    MQBlock block;
	    block.msglen = 0;
	    bool blockok = false;
	    do{
		blockok = false;
		qfile->seekg(qfile->tellg()+block.msglen);
		if(qfile->fail()){
		    throw RTIOException("Error in [FileMessageQueue::FileMessageQueue]: Error occured while seeking read pointer");
		}
		if(qfile->eof() || (length - qfile->tellg() == 0)){
		    return true;
		    break;
		}

		qfile->read((char*)&block,sizeof(MQBlock));
		if(qfile->eof()){
		    return false;
		}
		if(qfile->fail()){
		    throw RTIOException("Error in [FileMessageQueue::FileMessageQueue]: Error occured while reading block");
		}
		
		if(isBlockCorrupted(block)){
		    return false;
		}
				
	    }while(!qfile->eof());
	    return false;
	}
    }
  
    qfile->seekg(old_g);
    if(qfile->fail()){
	throw RTIOException("Error in [FileMessageQueue::FileMessageQueue]: Error occured while seeking read pointer");
    }

    return true;
}

void FileMessageQueue::openFile() throw(RTIOException,RTFileCorruptedError){
    if(qfile)
	return;

    //Create or Open file for read/write
    qfile = new fstream();
    qfile->open(filename.c_str(),fstream::in | fstream::out | fstream::binary);
    if(qfile->fail()){
	qfile->open(filename.c_str(),fstream::out | fstream::binary);	
	if(qfile->fail()){
	    throw RTIOException(Compat::Form("Error in [FileMessageQueue::openFile()]: Error opening file %s",filename.c_str()));
	}
    }

    //Get file length//
    qfile->seekg (0, ios::end);
    length = qfile->tellg();
    qfile->seekg (0, ios::beg);

    if(length!=0){
       	qfile->read((char*)&header,sizeof(MQHeader));
	if(qfile->fail()){
	    throw RTIOException("Error in [FileMessageQueue::openFile()]: Error while reading the file");
	}
	if(strcmp(header.magicstr,magicstr)!=0){
	    throw RTFileCorruptedError("Error in [FileMessageQueue::openFile()]: This file is not a queue file");
	}
    }
}

void FileMessageQueue::closeFile(unsigned int bookmark) throw(RTIOException){
    if(!qfile)
	return;

    header.bookmark = bookmark;
    qfile->seekp(0, ios::beg); 
    qfile->seekg(0, ios::beg);    
    if(qfile->fail()){
	throw RTIOException("Error in FileMessageQueue::closeFile(int) : I/O Error occured while re-locating write pointer");	
    }

    qfile->write((char*)&header,sizeof(MQHeader));

    if(qfile->fail()){
	throw RTIOException("Error in FileMessageQueue::closeFile(int) : I/O Error occured while writing");	
    }   

    qfile->close();
    delete qfile;
    qfile = NULL;
}

void FileMessageQueue::printFile(const char* fname){
    if(!fname)
	return;

    fstream file;
    file.open(fname,fstream::in | fstream::binary);
    if(file.fail()){
	cout << "Can't print. Error opening "<<fname<<endl;
	file.close();
    }
    if(file.eof()){
	cout << "Message queue file "<<fname<<" is empty"<<endl;
	file.close();
    }

    file.seekg (0, ios::end);
    int len = file.tellg();
    file.seekg (0, ios::beg);

    MQHeader header;
    MQBlock block;
    
    file.read((char*)&header,sizeof(MQHeader));
    cout <<"Magic String :"<<header.magicstr<<endl;
    cout <<"Queue Name   :"<<header.queuename<<endl;
    cout <<"Queue Size   :"<<header.msgcount<<endl;
    
    while(!file.eof()){
	file.read((char*)&block,sizeof(MQBlock));
	char* buf = (char*)malloc(block.msglen);
	bzero(buf,block.msglen);
	if(!buf){
	    file.close();
	    cout <<"Error occured while allocating buffer of size "<<block.msglen<<" bytes"<<endl;
	    return;
	}
	file.read(buf,block.msglen);

	cout <<"***** Block Start *****"<<endl;
	cout <<"Block State:"<<block.state<<endl;
	cout <<"Message Type:"<<block.msgtype<<endl;
	cout <<"Message Length:"<<block.msglen<<endl;
	cout <<"CRC:"<<hex<<block.crc<<dec<<endl;
	cout <<"----- Message Content Begins Right After This Line -----"<<endl;
	cout.write(buf,block.msglen);
	cout<<endl;
	cout <<"----- Message Content Ends Right Before This Line -----"<<endl;
	cout <<"**** Block End *****"<<endl;
	
	free(buf);

	if(len - file.tellg() <=0)
	  break;
    }

    file.close();
}

FileMessageQueue::~FileMessageQueue(){
}

void printBuf(int* buf,int _len){
  cout <<hex;
  int len = _len/4;
  cout<<"Buf Size ="<<dec<<_len<<endl;
  for(int i=0;i<len;i++){
      cout<<hex<<"0x"<<buf[i]<<",";
  }
  cout <<dec<<endl;
}


void printFP(fstream* f){
    if(!f)
	return;
    cout <<"Read pointer = "<<f->tellg()<<", Write pointer = "<<f->tellp()<<endl;
}

unsigned long getCRC(const char* buf,unsigned int len){	
    return crc((unsigned char*)buf,len);
}

