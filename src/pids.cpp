#include "pids.h"
#include <algorithm>
#include <cctype>

const std::map<std::string, std::string> PIDS = {
    {"rpm",          "010C"},
    {"speed",        "010D"},
    {"coolant_temp", "0105"},
    {"intake_temp",  "010F"},
    {"fuel_level",   "012F"}
};

std::string PIDReader::clean(const std::string& resp) {
    std::string result = resp;
    result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string PIDReader::extract(const std::string& resp, const std::string& header) {
    std::string c = clean(resp);
    size_t pos = c.find(header);
    
    if (pos == std::string::npos) {
        return "";
    }
    
    return c.substr(pos + header.length());
}

std::optional<int> PIDReader::parseRPM(const std::string& resp) {
    std::string data = extract(resp, "410C");
    
    if (data.length() >= 4) {
        try {
            int A = std::stoi(data.substr(0, 2), nullptr, 16);
            int B = std::stoi(data.substr(2, 2), nullptr, 16);
            return (A * 256 + B) / 4;
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

std::optional<int> PIDReader::parseSpeed(const std::string& resp) {
    std::string data = extract(resp, "410D");
    
    if (data.length() >= 2) {
        try {
            return std::stoi(data.substr(0, 2), nullptr, 16);
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

std::optional<int> PIDReader::parseCoolantTemp(const std::string& resp) {
    std::string data = extract(resp, "4105");
    
    if (data.length() >= 2) {
        try {
            return std::stoi(data.substr(0, 2), nullptr, 16) - 40;
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

std::optional<int> PIDReader::parseIntakeTemp(const std::string& resp) {
    std::string data = extract(resp, "410F");
    
    if (data.length() >= 2) {
        try {
            return std::stoi(data.substr(0, 2), nullptr, 16) - 40;
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

std::optional<double> PIDReader::parseFuelLevel(const std::string& resp) {
    std::string data = extract(resp, "412F");
    
    if (data.length() >= 2) {
        try {
            int raw = std::stoi(data.substr(0, 2), nullptr, 16);
            return std::round(raw * 100.0 / 255.0 * 10.0) / 10.0;
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

SensorData PIDReader::readAll(OBDConnection& conn) {
    SensorData data;
    
    std::string resp_rpm = conn.send(PIDS.at("rpm"));
    data.rpm = parseRPM(resp_rpm);
    
    std::string resp_speed = conn.send(PIDS.at("speed"));
    data.speed = parseSpeed(resp_speed);
    
    std::string resp_coolant = conn.send(PIDS.at("coolant_temp"));
    data.coolant_temp = parseCoolantTemp(resp_coolant);
    
    std::string resp_intake = conn.send(PIDS.at("intake_temp"));
    data.intake_temp = parseIntakeTemp(resp_intake);
    
    std::string resp_fuel = conn.send(PIDS.at("fuel_level"));
    data.fuel_level = parseFuelLevel(resp_fuel);
    
    return data;
}
