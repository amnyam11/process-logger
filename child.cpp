#include "my_shmem.hpp"
#include "common.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#ifdef WIN32
#include <windows.h>
#endif



int main(int argc, char* argv[]) {

    cplib::SharedMem<my_data> shared_data(MEM_NAME);

    if (!shared_data.IsValid()) {
        std::cerr << "Failed to open shared memory." << std::endl;
        return 1;
    }
    

    std::string operation = argv[1];

    if (operation == "increment") {
        writeToLog(" Child 1 started.");

        shared_data.Lock();
        shared_data.Data()->counter += 10;
        shared_data.Data()->child1_running = false;
        shared_data.Unlock();

        writeToLog(" Child 1 exited.");
        
    } else if (operation == "multiply") {
        writeToLog(" Child 2 started.");

        shared_data.Lock();
        shared_data.Data()->counter *= 2;
        shared_data.Unlock();

        sleep_ms(2000);

        shared_data.Lock();
        shared_data.Data()->counter /= 2;
        shared_data.Data()->child2_running = false;
        shared_data.Unlock();

        writeToLog(" Child 2 exited.");
        
    } else {
        std::cerr << "Unknown operation: " << operation << std::endl;
        return 1;
    }

    return 0;
}