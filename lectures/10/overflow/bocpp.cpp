#include <iostream>
int main(int argc, char **argv)
{
	char buf[8];
	std::cin.width(sizeof(buf));
	std::cin >> buf;
	std::cout << buf;
	return 0;
}
