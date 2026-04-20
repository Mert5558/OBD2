#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstdlib>

#include "connection.h"
#include "pids.h"
#include "dtc.h"
#include "logger.h"

// Terminal colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"
#define DIM     "\033[2m"

std::atomic<char> g_key{0};
std::atomic<bool> g_stop_keys{false};

void keyListener() {
    struct termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    while (!g_stop_keys) {
        fd_set set;
        struct timeval timeout;
        
        FD_ZERO(&set);
        FD_SET(STDIN_FILENO, &set);
        
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms
        
        int rv = select(STDIN_FILENO + 1, &set, nullptr, nullptr, &timeout);
        
        if (rv > 0) {
            char ch;
            if (read(STDIN_FILENO, &ch, 1) == 1) {
                g_key.store(std::tolower(ch));
            }
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

char getKey() {
    return g_key.exchange(0);
}

void restoreTerminal() {
    g_stop_keys = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
    
    system("stty sane");
}

std::string formatValue(const std::optional<int>& value, const std::string& unit = "") {
    if (!value.has_value()) {
        return DIM "--" RESET;
    }
    
    if (unit.empty()) {
        return std::to_string(value.value());
    }
    
    return std::to_string(value.value()) + " " + unit;
}

std::string formatValue(const std::optional<double>& value, const std::string& unit = "") {
    if (!value.has_value()) {
        return DIM "--" RESET;
    }
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << value.value();
    
    if (!unit.empty()) {
        ss << " " << unit;
    }
    
    return ss.str();
}

void printDashboard(const SensorData& data, bool logging_active) {
    std::cout << BOLD CYAN "╔═══════════════════════════════════════╗" RESET << std::endl;
    std::cout << BOLD CYAN "║       OBD2 Live Data Monitor          ║" RESET << std::endl;
    std::cout << BOLD CYAN "╠═══════════════════════════════════════╣" RESET << std::endl;
    
    // RPM with color coding
    std::cout << BOLD CYAN "║" RESET << " RPM              ";
    if (data.rpm.has_value()) {
        int rpm = data.rpm.value();
        if (rpm > 4000) {
            std::cout << RED << std::setw(12) << rpm << RESET;
        } else if (rpm > 2500) {
            std::cout << YELLOW << std::setw(12) << rpm << RESET;
        } else {
            std::cout << GREEN << std::setw(12) << rpm << RESET;
        }
    } else {
        std::cout << DIM << std::setw(12) << "--" << RESET;
    }
    std::cout << " " BOLD CYAN "║" RESET << std::endl;
    
    // Speed
    std::cout << BOLD CYAN "║" RESET << " Speed            " 
              << std::setw(12) << formatValue(data.speed, "km/h")
              << " " BOLD CYAN "║" RESET << std::endl;
    
    // Coolant with color coding
    std::cout << BOLD CYAN "║" RESET << " Coolant Temp     ";
    if (data.coolant_temp.has_value()) {
        int temp = data.coolant_temp.value();
        if (temp > 100) {
            std::cout << RED << std::setw(9) << temp << " °C" << RESET;
        } else if (temp > 85) {
            std::cout << YELLOW << std::setw(9) << temp << " °C" << RESET;
        } else {
            std::cout << CYAN << std::setw(9) << temp << " °C" << RESET;
        }
    } else {
        std::cout << DIM << std::setw(12) << "--" << RESET;
    }
    std::cout << " " BOLD CYAN "║" RESET << std::endl;
    
    // Intake temp
    std::cout << BOLD CYAN "║" RESET << " Intake Temp      " 
              << std::setw(12) << formatValue(data.intake_temp, "°C")
              << " " BOLD CYAN "║" RESET << std::endl;
    
    // Fuel with color coding
    std::cout << BOLD CYAN "║" RESET << " Fuel Level       ";
    if (data.fuel_level.has_value()) {
        double fuel = data.fuel_level.value();
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << fuel << " %";
        
        if (fuel < 10) {
            std::cout << RED << std::setw(12) << ss.str() << RESET;
        } else if (fuel < 25) {
            std::cout << YELLOW << std::setw(12) << ss.str() << RESET;
        } else {
            std::cout << GREEN << std::setw(12) << ss.str() << RESET;
        }
    } else {
        std::cout << DIM << std::setw(12) << "--" << RESET;
    }
    std::cout << " " BOLD CYAN "║" RESET << std::endl;
    
    std::cout << BOLD CYAN "╠═══════════════════════════════════════╣" RESET << std::endl;
    
    // Status bar
    std::cout << BOLD CYAN "║" RESET << " Logging: ";
    if (logging_active) {
        std::cout << GREEN "ON " RESET;
    } else {
        std::cout << DIM "OFF" RESET;
    }
    std::cout << "                        " BOLD CYAN "║" RESET << std::endl;
    
    std::cout << BOLD CYAN "╠═══════════════════════════════════════╣" RESET << std::endl;
    std::cout << BOLD CYAN "║" RESET << DIM " " BOLD "Q" RESET DIM " Quit  " 
              BOLD "D" RESET DIM " DTCs  " BOLD "C" RESET DIM " Clear  " 
              BOLD "L" RESET DIM " Log" << "   " BOLD CYAN "║" RESET << std::endl;
    std::cout << BOLD CYAN "╚═══════════════════════════════════════╝" RESET << std::endl;
}

void showDTCs(OBDConnection& conn) {
    restoreTerminal();
    
    std::cout << "\n" BOLD YELLOW "Reading fault codes..." RESET << std::endl;
    auto codes = DTCReader::readDTCs(conn);
    
    if (codes.empty()) {
        std::cout << GREEN "No fault codes found." RESET << std::endl;
    } else {
        std::cout << RED "Found " << codes.size() << " fault code(s):" RESET << std::endl;
        for (const auto& code : codes) {
            std::cout << "  " BOLD RED << code << RESET << std::endl;
        }
    }
    
    std::cout << "\n" DIM "Press Enter to return..." RESET << std::endl;
    std::cin.get();
}

void confirmClear(OBDConnection& conn) {
    restoreTerminal();
    
    std::cout << "\n" BOLD RED "Clear all fault codes?" RESET " This cannot be undone." << std::endl;
    std::cout << "Type " BOLD "YES" RESET " to confirm: ";
    std::cout.flush();
    
    std::string answer;
    std::getline(std::cin, answer);
    
    if (answer == "YES") {
        bool ok = DTCReader::clearDTCs(conn);
        if (ok) {
            std::cout << GREEN "Cleared." RESET << std::endl;
        } else {
            std::cout << RED "Failed to clear." RESET << std::endl;
        }
    } else {
        std::cout << DIM "Cancelled." RESET << std::endl;
    }
    
    std::cout << "\n" DIM "Press Enter to return..." RESET << std::endl;
    std::cin.get();
}

int main() {
    std::cout << BOLD CYAN "OBD2 Monitor C++" RESET << std::endl;
    std::cout << "Connecting to adapter..." << std::endl;
    
    OBDConnection conn;
    Logger logger;
    bool logging_active = false;
    
    try {
        if (!conn.connect()) {
            std::cout << RED "Failed to connect to adapter." RESET << std::endl;
            std::cout << "Check: adapter paired, rfcomm bound, ignition ON." << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cout << RED "Connection error: " << e.what() << RESET << std::endl;
        return 1;
    }
    
    std::cout << GREEN "Connected!" RESET " Starting dashboard...\n" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    g_stop_keys = false;
    std::thread key_thread(keyListener);
    
    SensorData data;
    bool running = true;
    
    try {
        while (running) {
            data = PIDReader::readAll(conn);
            
            if (logging_active) {
                logger.log(data);
            }
            
            // Clear screen and redraw
            system("clear");
            printDashboard(data, logging_active);
            
            char key = getKey();
            
            if (key == 'q') {
                running = false;
            } else if (key == 'd') {
                g_stop_keys = true;
                key_thread.join();
                
                showDTCs(conn);
                
                g_stop_keys = false;
                key_thread = std::thread(keyListener);
            } else if (key == 'c') {
                g_stop_keys = true;
                key_thread.join();
                
                confirmClear(conn);
                
                g_stop_keys = false;
                key_thread = std::thread(keyListener);
            } else if (key == 'l') {
                if (logging_active) {
                    logger.stop();
                    logging_active = false;
                } else {
                    logger.start();
                    logging_active = true;
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& e) {
        std::cout << RED "Error: " << e.what() << RESET << std::endl;
    }
    
    g_stop_keys = true;
    if (key_thread.joinable()) {
        key_thread.join();
    }
    
    system("stty sane");
    
    if (logging_active) {
        logger.stop();
    }
    
    conn.disconnect();
    std::cout << "\n" DIM "Disconnected." RESET << std::endl;
    
    return 0;
}
