#include <iostream>
#include "Dummy.h"

int main(int ArgsCount, char** Args)
{
	std::cout << MY_NAME << std::endl;
	fmt_print();

	std::cin.get();
	return 0;
}