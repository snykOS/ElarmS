/*****************************************************************************

    Copyright ©2017. 
    The Regents of the University of California (Regents). 
    Authors: Berkeley Seismological Laboratory. All Rights Reserved. 
    Permission to use, copy, modify, and distribute this software
    and its documentation for educational, research, and not-for-profit
    purposes, without fee and without a signed licensing agreement, is hereby
    granted, provided that the above copyright notice, this paragraph and the
    following four paragraphs appear in all copies, modifications, and
    distributions. Contact The Office of Technology Licensing, UC Berkeley,
    2150 Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201,
    otl@berkeley.edu, http://ipira.berkeley.edu/industry-info for commercial
    licensing opportunities.

    Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

    Attribution Rights. You must retain, in the Source Code of any Derivative
    Works that You create, all copyright, patent, or trademark notices from
    the Source Code of the Original Work, as well as any notices of licensing
    and any descriptive text identified therein as an "Attribution Notice."
    You must cause the Source Code for any Derivative Works that You create to
    carry a prominent Attribution Notice reasonably calculated to inform
    recipients that You have modified the Original Work.

    IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
    ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
    REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
    HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
    MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*****************************************************************************/
#ifndef __wordorder_h
#define __wordorder_h
//#include <sys/byteorder.h>
#include <netdb.h>

template <class T>
inline void wordorder_slave (T & n){
    if (sizeof (T) == 4 ) {
        uint32_t *p = reinterpret_cast<uint32_t*>(&n);  
        *p = ntohl( *p ) ;
        n = *(reinterpret_cast<T*> (p));        
    }    
    else if (sizeof (T) == 2 ) {
        uint16_t *p = reinterpret_cast<uint16_t*>(&n);  
        *p = ntohs( *p ) ;
        n = *(reinterpret_cast<T*> (p));
    }
    else if (ntohs(1)!=1) { //generic case
        T t;
        char *src = reinterpret_cast<char *>(&n);
        char *dst = reinterpret_cast<char *>(&t)+sizeof(T);
        do { *(--dst) = *(src++); } while(dst != reinterpret_cast<char *>(&t));
        n = t;  
    }
}

template <class T>
inline void wordorder_master (T & n){
    if (sizeof (T) == 4 ) {
        uint32_t *p = reinterpret_cast<uint32_t*>(&n);  
        *p = htonl( *p ) ;
        n = *(reinterpret_cast<T*> (p));        
    }    
    else if (sizeof (T) == 2 ) {
        uint16_t *p = reinterpret_cast<uint16_t*>(&n);  
        *p = htons( *p ) ;
        n = *(reinterpret_cast<T*> (p));
    }
    else if (htons(1)!=1) {  //generic case 
        T t;
        char *src = reinterpret_cast<char *>(&n);
        char *dst = reinterpret_cast<char *>(&t)+sizeof(T);
        do { *(--dst) = *(src++); } while(dst != reinterpret_cast<char *>(&t));
        n = t;  
    }
}

template <class T>
inline size_t serialize(T & n, char*& dst ){
    size_t i=0;
    char *src = reinterpret_cast<char *>(&n);
    for (; i < sizeof(T); ++i ){ *(dst++) = *(src++);} 
    return i;
}

template <class T>
inline size_t deserialize(T & n, char*& src ){
    size_t i=0;
    char *dst = reinterpret_cast<char *>(&n);
    for (; i < sizeof(T); ++i ){ *(dst++) = *(src++);} 
    return i;
}

#endif  
