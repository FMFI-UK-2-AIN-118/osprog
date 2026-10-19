#include <iostream>

char* global;

int main() {
    global = new char[47];
    std::cerr << "allocated global at " << (void*) global << std::endl;
    char* str = new char[20];
    std::cerr << "allocated str    at " << (void*) str << std::endl;
    return 0;
}
