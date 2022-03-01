/***********************************************************

File Name :
        Message.h

Original Author:
        Patrick Small

Description:

        This header file defines the Message class - an object class 
which abstracts the use of QWServer messages. Methods 
are provided to create new messages, store scalar and array data, and
set most of the properties of the underlying message.

Creation Date:
        06 July 1998

Modification History:
	ReWritten  02 April 2004   by Kalpesh Solanki

Usage Notes:

        The user process can check to make sure that the message
was created successfully by using the "!" overloaded boolean operator.

        Most functions provide an integer return value which is
TN_SUCCESS if the operation was successful, or TN_FAILURE otherwise.

***********************************************************/


/* Note from Kalpesh Solanki
It is VERY IMPORTANT that you read this message before using this class if you like to 
avoid serious bugs and memory leaks from your program.

* All append functions will make their own copy of any data you pass in, so you are free to
release your data structures after completion of these functions.

* All next functions returns you data structures which are owned by Message object. You MUST not release it.
It is Message object's responsibility to release any information returned to you as a result of next function
calls.

 */

#ifndef message_H
#define message_H

// Various include files
#include <iostream>
#include "MessageTypes.h"
#include "ContentTable.h"

using namespace std;

// Connection class prototype
class Connection;

class Message
{
 private:
    bool iown;
    int type;
    char* dest;
    char* domainname;
    char* sourcename;
    
    ContentTable* ctable;
    ContentTable::iterator cti;
    list<void*> gclist;

    static int count;
  
    Message(const Message&);
    Message& operator=(const Message&);

 public:
    
  // Constructor
  //
  // This is a dummy constructor which does not yield a valid message.
  //
  //  Message();

  // Constructor
  //
  // This creates a message of the desired message type.
  //
  Message();
  Message(const int);
  Message(ContentTable*,const int);
  // Message(const Message&);

  // Destructor
  //
  ~Message();

  /*For all append functions argument contents will be copied.
    caller may free the allocated buffer after this function returns.*/


  // The following methods append various types of data (both scalar
  // and arrays) to the QWServer message.
  //
  int append(short int);
  // Note: This method appends the short integer array by pointer - 
  int append(short int *, int);

  int append(int);
  // Note: This method appends the integer array by pointer
  int append(int*, int);

  int append(long);
  // Note: This method appends the long integer array by pointer - 
  int append(long*, int);

  int append(long long);
  // Note: This method appends the long long array by pointer
  int append(long long*, int);

  int append(double);
  // Note: This method appends the double array by pointer
  int append(double*, int);

  int append(float);
  // Note: This method appends the double array by pointer
  int append(float*, int);

  int append(char *);
  int append(const char *);
  // Note: This method appends the string array by pointer
  int append(char **, int);
  int append(char);

  // The following methods retrieve various types of data (both scalar
  // and arrays) from the QWServer message.
  //
  // The user process must not modify or
  // deallocate these memory areas.
  //
  int next(short int&);
  int next(short int**, int&);
  int next(int&);
  int next(int**, int&);
  int next(long&);
  int next(long**, int&);
  int next(long long&);
  int next(long long**, int&);
  int next(double&);
  int next(double**, int&);
  int next(float&);
  int next(float**, int&);
  int next(char**);
  int next(char***, int&);
  int next(char&);

  // Set the subject to which this message will be published
  // Makes own copy of the string that is passed in.
  int setDest(const char *);

  // Empty the current contents (ie: data fields) of the message.
  // Content WILL NOT BE REMOVED IF the object is instanciated using Message(ContentTable,int) 
  // constructor. Caller will be responsible for clearing ContentTable structure.
  int clearData();

  // Sets the delivery mode of the current message.
  //
  // NOT CURRENTLY IMPLEMENTED.
  //
  //  int setDeliveryMode(int);

  // Sets the position of the current field pointer.
  //
  int curField(int);

  // Retrieve the subject of the message
  //
  const char* getDest() const;
  int getDest(char*);

  // Retrieve the message type
  //
  int getTypeString(char*) const;
  int getType(int&) const;
  static int getType(const char*);

  int setDomainName(const char*);
  int setSourceName(const char*);
  const char* getDomainName() const;
  const char* getSourceName() const;


  ContentTable* getContentTable() const;

  // Print all properties and data contained within the message
  // to the screen.
  //
  int print();

  // Boolean operator which returns TN_TRUE when the Message object
  // is not valid, and TN_FALSE when the object references a valid message.
  friend int operator!(const Message&);

  // Print all properties and data contained within the message
  // to the screen.
  //
  friend ostream& operator<<(ostream&,Message&);

  // The Connection class needs to view the internal state of message
  // objects.
  //
  friend class Connection;
};


#endif

