#include <stdio.h>
#include <errno.h>

#include "DMMessageEncoder.h"
#include "DMMessageDecoder.h"

using namespace std;

int
main(int argc, char **argv)
{
    FILE *fp;

    if(argc < 2) {
	cerr << "Usage: " << argv[0] << " file" << endl;
	return 0;
    }
    if((fp = fopen(argv[1], "r")) == NULL) {
	cerr << "Cannot open " << argv[1] << endl;
	cerr << strerror(errno) << endl;
	return 0;
    }

    DMMessageDecoder *decoder = new DMMessageDecoder();
    DMMessageEncoder *encoder = new DMMessageEncoder();
    vector<CoreEventInfo *> v;

    int num = decoder->decodeAllLogMessages(fp, v, true);
    fclose(fp);

    cout << "Decoded " << num << " messages: " << endl;

    vector<CoreEventInfo *>::iterator it;
    for(it = v.begin(); it != v.end(); it++) {
	string message = encoder->encodeMessage(*it);
	cout << message << endl;
	delete (*it);
    }
}
