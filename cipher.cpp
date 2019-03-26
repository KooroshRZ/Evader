/*#include <stdlib.h>
#include <iostream>


void setKey(char* key, int key_size) {

	static const char alphanum[] =
		"!@#$%^&*()"
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < key_size; ++i) {
		key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
		std::cout << key[i];
	}
	std::cout << "\n";

}*/