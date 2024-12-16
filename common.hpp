#ifndef COMMON_HPP
#define COMMON_HPP

#include <string>

struct my_data {
    my_data() {
        counter = 0;
        pid_main_process = 0;
    }
    int counter;
    int pid_main_process;
    bool child1_running = false;
    bool child2_running = false;
};

int getCurrentPID();
std::string getCurrentTime();
void writeToLog(const std::string& message);
void sleep_ms(int milliseconds);

extern const char* MEM_NAME;
extern const char* LOG_FILE_NAME;

#endif 