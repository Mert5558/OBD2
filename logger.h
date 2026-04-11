#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include "pids.h"

class Logger {
public:
    Logger(const std::string& path = "obd_log.csv");
    ~Logger();
    
    void start();
    void log(const SensorData& data);
    void stop();

private:
    std::string path_;
    std::ofstream file_;
    bool is_open_;
    
    std::string getCurrentTimestamp();
};

#endif // LOGGER_H
