#include <cstdio>
#include <iostream>
#include "include/Configuration.h"
#include "include/RetCodes.h"

main (int argc, char *argv[]) {

int return_value;
char tag[CONFIG_MAX_STRING_LEN], value[CONFIG_MAX_STRING_LEN], line[CONFIG_MAX_STRING_LEN];
Configuration *c , *c2, *c3;

	if (argc != 2) {
		std::cout  << " You need a config file to be read in as arg1" << std::endl;
		exit(1);
	}

  	std::cout << "Processing "<< argv[1] << " using next(tag, value) " << std::endl;
	c = new Configuration(argv[1]);
	while ( (return_value = c->next(tag, value)) == TN_SUCCESS) {
		std::cout << "Got Tag="<< tag << " Value="<< value << std::endl;
	}

  	std::cout <<std::endl<< "Processing "<< argv[1] << " using next(line) " << std::endl;
	c2 = new Configuration(argv[1]);
	while ( (return_value = c2->next(line)) == TN_SUCCESS) {
		std::cout << "Got line: "<< line << std::endl;
	}

  	std::cout <<std::endl<< "Processing "<< argv[1] << " using nextFull(line) " << std::endl;
	c3 = new Configuration(argv[1]);
	while ( (return_value = c3->nextFull(line)) == TN_SUCCESS) {
		std::cout << "Got line: "<< line << std::endl;
	}
	exit(0);
}
