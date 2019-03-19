/*#include "cipher.h"
#include <iostream>
#include <string>

using namespace std;

//both encryption and decryption function
char XOR(char data, char key[]) {

	string xorstring = data;

	for (int i = 0; i < xorstring.size(); i++) // for loop for scrambing bits in the string 
		xorstring[i] = data[i] ^ key[i % sizeof(key) / sizeof(char)]; // scrambling/descrambling string

	return xorstring;

}


int main() {

	char key[3]= {'k', 'e', 'y'};
	char data[4096];
	cin >> data;
	//string str(data);
	string enciphered = XOR(data, key);
	cout << "enc : " << enciphered << endl;

	string deciphered = XOR(enciphered, key);
	char result[4096];
	strcpy(result, deciphered.c_str());

	cout << "dec : " << deciphered << endl;
	int a;
	cin >> a;
	return 0;

}*/