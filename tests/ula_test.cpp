
#include "memory.hpp"
#include "ula.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

int main(void) {
    Memory memory;
    ULA ula(&memory);
    for(int i=0; i<(3560+224*2);i++) {
        ula.oneTick(0);
    }
}
