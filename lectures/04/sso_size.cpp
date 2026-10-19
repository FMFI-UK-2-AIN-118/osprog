#include <iostream>
#include <string>

int main() {
	std::cout << "sizeof(void*)       " << sizeof(void*) << "\n";
	std::cout << "sizeof(size_t)      " << sizeof(size_t) << "\n";
	std::cout << "(sizeof(void*) + sizeof(size_t) + sizeof(size_t)) "
		<< (sizeof(void*) + sizeof(size_t) + sizeof(size_t)) << "\n";
	std::cout << "sizeof(std::string) " << sizeof(std::string) << "\n";
	return 0;
}
