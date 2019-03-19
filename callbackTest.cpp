// A simple C program to demonstrate callback 
#include <iostream> 
#include <windows.h> 

using namespace std;

void A()
{
	cout << "I am function A\n";
}

// callback function 
void B(void(*ptr)())
{
	(*ptr) (); // callback to A 
}

/*int main()
{
	void(*ptr)() = &A;

	// calling function B and passing
	// address of the function A as argument
	B(ptr);
	Sleep(5000);
	return 0;
}
*/