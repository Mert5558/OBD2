#include "logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>

Logger::Logger(const std::string& path)
    : path_(path), is_open_(false) {}

Logger::~Logger() {
    stop();
}

void Logger::start() {
    // Check if file exists
    struct stat buffer;
    bool file_exists = (stat(path_.c_str(), &buffer) == 0);
    
    file_.open(path_, std::ios::app);
    
    if (file_.is_open()) {
        is_open_ = true;
        
        // Write header if new file
        if (!file_exists) {
            file_ << "timestamp,rpm,speed,coolant_temp,intake_temp,engine_load,fuel_level,throttle\n";
            file_.flush();
        }
    }
}

void Logger::log(const SensorData& data) {
    if (!is_open_) {
        return;
    }
    
    file_ << getCurrentTimestamp() << ",";
    
    if (data.rpm.has_value()) {
        file_ << data.rpm.value();
    }
    file_ << ",";
    
    if (data.speed.has_value()) {
        file_ << data.speed.value();
    }
    file_ << ",";
    
    if (data.coolant_temp.has_value()) {
        file_ << data.coolant_temp.value();
    }
    file_ << ",";
    
    if (data.intake_temp.has_value()) {
        file_ << data.intake_temp.value();
    }
    file_ << ",";
    
    // engine_load placeholder
    file_ << ",";
    
    if (data.fuel_level.has_value()) {
        file_ << std::fixed << std::setprecision(1) << data.fuel_level.value();
    }
    file_ << ",";
    
    // throttle placeholder
    file_ << "\n";
    
    file_.flush();
}

void Logger::stop() {
    if (is_open_) {
        file_.close();
        is_open_ = false;
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
