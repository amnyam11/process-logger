#include "my_shmem.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Имя разделяемой памяти и семафора
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


int getCurrentPID(){
#ifdef WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}

int main() {
    // Создаем или открываем разделяемую память для хранения counter
    cplib::SharedMem<my_data> shared_data(MEM_NAME);

    if (!shared_data.IsValid()) {
        std::cerr << "Failed to create or open shared memory." << std::endl;
        return 1;
    }

    my_data* data = shared_data.Data();
    std::string command;

    while (true) {
        std::cin >> command;
        if (command == "s") {
            // Блокируем разделяемую память для чтения counter
            shared_data.Lock();
            std::cout << "Counter: " << data->counter << std::endl;
            shared_data.Unlock();
        } else if (command == "m") {
            // Блокируем разделяемую память для изменения counter
            shared_data.Lock();
            data->counter++;
            std::cout << "Counter incremented. New value: " << data->counter << std::endl;
            shared_data.Unlock();
        } else if (command == "e") {
            break;
        } else if (command == "l") {
            int pid = getCurrentPID();
            // Блокируем разделяемую память для записи в лог-файл
            shared_data.Lock();
            if (data->pid_main_process == pid) {
                std::ofstream logFile(LOG_FILE_NAME, std::ios::app);
                if (logFile.is_open()) {
                    std::string currentTime = getCurrentTime();
                    logFile << currentTime << " PID: " << pid << " Counter: " << data->counter << std::endl;
                    logFile.close();
                } else {
                    std::cerr << "Failed to open log file." << std::endl;
                }
            } else {
                std::cout << "Only the main program can write to the log file." << std::endl;
            }
            shared_data.Unlock();
        } else {
            std::cout << "unknown param! Use 's' to show or 'm' to modify!" << std::endl;
        }
    }

    return 0;
}
