#include "my_shmem.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

const char* MEM_NAME = "counter_mem";
const char* LOG_FILE_NAME = "log_file.log";

struct my_data
{
    my_data() {
        counter = 0;
    }
    int counter;
    int pid_main_process;
};

void sleep_ms(int milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}


unsigned long long get_current_time_ms() {
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
#endif
}


int getCurrentPID(){
#ifdef WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

std::string getCurrentTime() {
    std::ostringstream oss;

#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    oss << st.wYear << "-" << st.wMonth << "-" << st.wDay << " "
        << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "."
        << st.wMilliseconds;
#else
    struct timeval tv;
    struct tm* tm;
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    oss << tm->tm_year + 1900 << "-" << tm->tm_mon + 1 << "-" << tm->tm_mday << " "
        << tm->tm_hour << ":" << tm->tm_min << ":" << tm->tm_sec << "."
        << tv.tv_usec / 1000;
#endif

    return oss.str();
}

void incrementCounter(cplib::SharedMem<my_data>& shared_data) {
    shared_data.Lock();
    shared_data.Data()->counter++;
    shared_data.Unlock();
}


void writeToLog(const std::string& message) {
    std::ofstream logFile(LOG_FILE_NAME, std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}

void logCounter(cplib::SharedMem<my_data>& shared_data) {
    shared_data.Lock();
    std::string currentTime = getCurrentTime();
    int pid = getCurrentPID();
    writeToLog(currentTime + " PID: " + std::to_string(pid) + " Counter: " + std::to_string(shared_data.Data()->counter));
    shared_data.Unlock();
}

int main() {

    cplib::SharedMem<my_data> shared_data(MEM_NAME);

    if (!shared_data.IsValid()) {
        std::cerr << "Failed to create or open shared memory." << std::endl;
        return 1;
    }


    my_data* data = shared_data.Data();

    std::string command;

    int pid = getCurrentPID();
    if (pid == data->pid_main_process)
        writeToLog(getCurrentTime() + " PID: " + std::to_string(pid) + " Start working.");

    unsigned long long lastIncrementTime = get_current_time_ms();
    unsigned long long lastLogTime = get_current_time_ms();
   
    while (true) {

        unsigned long long currentTime = get_current_time_ms();

        if (currentTime - lastIncrementTime >= 300) {
            lastIncrementTime = currentTime;

            if (pid == data->pid_main_process) {
                shared_data.Lock();
                data->counter++;
                shared_data.Unlock();
            }
        }
        if (currentTime - lastLogTime >= 1000){
            lastLogTime = currentTime;
            if (pid == data->pid_main_process) {
                logCounter(shared_data);
            }
        } 

        sleep_ms(10);

    }

    return 0;
}
