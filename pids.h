#ifndef PIDS_H
#define PIDS_H

#include <string>
#include <map>
#include <optional>
#include <math.h>
#include "connection.h"

struct SensorData {
    std::optional<int> rpm;
    std::optional<int> speed;
    std::optional<int> coolant_temp;
    std::optional<int> intake_temp;
    std::optional<double> fuel_level;
};

class PIDReader {
public:
    static SensorData readAll(OBDConnection& conn);

private:
    static std::string clean(const std::string& resp);
    static std::string extract(const std::string& resp, const std::string& header);
    
    static std::optional<int> parseRPM(const std::string& resp);
    static std::optional<int> parseSpeed(const std::string& resp);
    static std::optional<int> parseCoolantTemp(const std::string& resp);
    static std::optional<int> parseIntakeTemp(const std::string& resp);
    static std::optional<double> parseFuelLevel(const std::string& resp);
};

#endif // PIDS_H
