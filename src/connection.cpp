#include "connection.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <chrono>
#include <sstream>
#include <algorithm>

OBDConnection::OBDConnection(const std::string& port, int baud)
    : port_(port), baud_(baud), serial_fd_(-1) {}

OBDConnection::~OBDConnection() {
    disconnect();
}

bool OBDConnection::connect() {
    // Open serial port
    serial_fd_ = open(port_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd_ == -1) {
        return false;
    }

    // Configure serial port
    struct termios options;
    tcgetattr(serial_fd_, &options);
    
    // Set baud rate
    speed_t baud_const;
    switch (baud_) {
        case 38400: baud_const = B38400; break;
        case 9600:  baud_const = B9600;  break;
        case 115200: baud_const = B115200; break;
        default:    baud_const = B38400; break;
    }
    
    cfsetispeed(&options, baud_const);
    cfsetospeed(&options, baud_const);
    
    // 8N1 mode
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
    
    // No hardware flow control
    options.c_cflag &= ~CRTSCTS;
    options.c_cflag |= CREAD | CLOCAL;
    
    // Raw input
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_oflag &= ~OPOST;
    
    tcsetattr(serial_fd_, TCSANOW, &options);
    
    return initAdapter();
}

void OBDConnection::disconnect() {
    if (serial_fd_ != -1) {
        close(serial_fd_);
        serial_fd_ = -1;
    }
}

std::string OBDConnection::send(const std::string& cmd, int timeout) {
    if (serial_fd_ == -1) {
        return "";
    }
    
    // Clear input buffer
    tcflush(serial_fd_, TCIFLUSH);
    
    // Send command
    std::string full_cmd = cmd + "\r";
    write(serial_fd_, full_cmd.c_str(), full_cmd.length());
    
    // Read response
    std::string response;
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start
        ).count();
        
        if (elapsed >= timeout) {
            break;
        }
        
        char buffer[256];
        int n = read(serial_fd_, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            response += buffer;
            
            if (response.find('>') != std::string::npos) {
                break;
            }
        }
        
        usleep(10000); // 10ms delay
    }
    
    // Clean response
    size_t pos = response.find('>');
    if (pos != std::string::npos) {
        response = response.substr(0, pos);
    }
    
    auto lines = splitLines(response);
    std::string result;
    std::string cmd_upper = cmd;
    std::transform(cmd_upper.begin(), cmd_upper.end(), cmd_upper.begin(), ::toupper);
    
    for (const auto& line : lines) {
        if (!line.empty() && line != cmd_upper) {
            if (!result.empty()) result += "\n";
            result += line;
        }
    }
    
    return result;
}

bool OBDConnection::initAdapter() {
    send("ATZ", 3);
    send("ATE0");
    send("ATL0");
    send("ATS0");
    send("ATH0");
    send("ATSP0", 3);
    
    // Try protocols in order
    const char* protocols[] = {nullptr, "ATSP6", "ATSP7", "ATSP3"};
    
    for (const char* proto : protocols) {
        if (proto) {
            send(proto, 3);
        }
        
        std::string test = send("0100", 5);
        if (test.find("4100") != std::string::npos || 
            test.find("41 00") != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

std::vector<std::string> OBDConnection::splitLines(const std::string& str) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;
    
    while (std::getline(ss, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    return lines;
}
