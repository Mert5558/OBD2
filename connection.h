#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <vector>

class OBDConnection {
public:
    OBDConnection(const std::string& port = "/dev/rfcomm0", int baud = 38400);
    ~OBDConnection();

    bool connect();
    void disconnect();
    std::string send(const std::string& cmd, int timeout = 5);

private:
    std::string port_;
    int baud_;
    int serial_fd_;
    
    bool initAdapter();
    std::vector<std::string> splitLines(const std::string& str);
};

#endif // CONNECTION_H
