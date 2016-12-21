#include "../EdsLib/Console.h"
#include "windows.h"
#include <iostream>
int main()
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	SetConsoleScreenBufferSize(hStdOut, COORD{500, 500});

	int x;
	std::cin >> x;
	return 0;
}