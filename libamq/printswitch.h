#include <iostream>

enum debug_print_level{DBUG,ERROR,INFO};

/*Print switches. Turn it on if you want to print messages for that level*/

#define __DEBUG 1
#define __ERROR 1
#define __INFO 1
#define __OTHER 0

#define _print(_level,_str){         \
                                     \
 debug_print_level level = _level;   \
  switch(level){                     \
                                     \
  case DBUG: if(__DEBUG)            \
    cout<<"[DEBUG] "<<_str<<endl;    \
  break;                             \
                                     \
  case ERROR: if(__ERROR)            \
    cout<<"[ERROR] "<<_str<<endl;    \
  break;                             \
                                     \
  case INFO: if(__INFO)              \
    cout<<"[INFO] "<<_str<<endl;     \
  break;                             \
                                     \
  default: if(__OTHER)               \
    cout<<"[OTHER] "<<_str<<endl;    \
  break;                             \
  };                                 \
}
