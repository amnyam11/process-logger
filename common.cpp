#include "common.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

const char* MEM_NAME = "counter_mem";
const char* LOG_FILE_NAME = "log_file.log";

int getCurrentPID() {
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

void writeToLog(const std::string& message) {
    std::ofstream logFile(LOG_FILE_NAME, std::ios::app);
    if (logFile.is_open()) {
        std::string currentTime = getCurrentTime();
        int pid = getCurrentPID();
        logFile << currentTime + " PID: " + std::to_string(pid) + message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Failed to open log file." << std::endl;
    }
}


void sleep_ms(int milliseconds) {
#ifdef WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}