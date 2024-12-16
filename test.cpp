#include "my_shmem.hpp"
#include "common.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <thread>

#ifdef WIN32
#include <windows.h>
#include <conio.h>
#else
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#endif 




unsigned long long getCurrentTimeMs() {
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
#endif
}

void logCounter(cplib::SharedMem<my_data>& shared_data) {
    shared_data.Lock();
    writeToLog(" Counter: " + std::to_string(shared_data.Data()->counter));
    shared_data.Unlock();
}

void incrementCounter(cplib::SharedMem<my_data>& shared_data) {
    shared_data.Lock();
    shared_data.Data()->counter++;
    shared_data.Unlock();
}


bool isInputAvailable() {
#ifdef _WIN32
    return _kbhit();
#else
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0, 0};
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    if (ret == -1) {
        return false;
    }
    return ret > 0;
#endif
}

std::string readInput() {
    std::string input;
#ifdef _WIN32
    while (true) {
        if (_kbhit()) {
            char ch = _getche();
            if (ch == '\r') {
                std::cout << std::endl;
                break;
            }
            input += ch;
        }
    }
#else
    char buffer[256];
    fgets(buffer, sizeof(buffer), stdin);
    input = buffer;
    if (!input.empty() && input.back() == '\n') {
        input.pop_back();
    }
#endif
    return input;
}


void spawnChildProcess(const std::string& operation) {
#ifdef _WIN32
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    std::string cmd = "child.exe " + operation;

    if (CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    } else {
        writeToLog("Failed to start Child process: " + operation);
    }
#else
    pid_t pid = fork();
    if (pid == 0) {
        const char* args[] = {"./child", operation.c_str(), NULL};
        execvp(args[0], const_cast<char* const*>(args));
        std::cerr << "Failed to execute child process: " << strerror(errno) << std::endl;
        exit(1);
    } else if (pid > 0) {

        writeToLog("Child process started with PID: " + std::to_string(pid));
    } else {
        writeToLog("Failed to fork child process.");
    }
#endif
}



int main() {
    cplib::SharedMem<my_data> shared_data(MEM_NAME);

    if (!shared_data.IsValid()) {
        std::cerr << "Failed to create or open shared memory." << std::endl;
        return 1;
    }

    my_data* data = shared_data.Data();


    int pid = getCurrentPID();

    shared_data.Lock();
    int mainPid = data->pid_main_process;
    shared_data.Unlock();

    if (pid == mainPid)
        writeToLog(" Start main process.");

    unsigned long long lastIncrementTime = getCurrentTimeMs();
    unsigned long long lastLogTime = getCurrentTimeMs();
    unsigned long long lastSpawnCopy = getCurrentTimeMs();

    std::cout << "Input any non-negative integer to change current counter:" << std::endl;

    while (true) {
        unsigned long long currentTime = getCurrentTimeMs();

        if (currentTime - lastIncrementTime >= 300) {
            lastIncrementTime = currentTime;
            incrementCounter(shared_data);
        }

        if (pid == mainPid) {
            if (currentTime - lastLogTime >= 1000) {
                lastLogTime = currentTime;
                logCounter(shared_data);
                }
        

            if (currentTime - lastSpawnCopy >= 3000) {
                lastSpawnCopy = currentTime;

                shared_data.Lock();
                if (!data->child1_running && !data->child2_running) {
                    data->child1_running = true;
                    data->child2_running = true;
                    shared_data.Unlock();
                
                    spawnChildProcess("increment");

                    spawnChildProcess("multiply");
                } else {
                    writeToLog("Skipping...");
                    shared_data.Unlock();
                }
            }
        }

        if (isInputAvailable()) {
            std::string input = readInput();
            if (!input.empty()) {
                try {
                    int newValue = std::stoi(input);
                    if (newValue >= 0) {
                        shared_data.Lock();
                        shared_data.Data()->counter = newValue;
                        shared_data.Unlock();
                        std::cout << "Counter set to " << newValue << std::endl;
                        std::cout << "Input any non-negative integer to change current counter:" << std::endl;
                    } else {
                        std::cerr << "Invalid input. Please enter a non-negative integer." << std::endl;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Invalid input. Please enter a non-negative integer." << std::endl;
                }
            }
        }

        sleep_ms(1);
    }

    return 0;
}