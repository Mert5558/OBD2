#include "dtc.h"
#include <algorithm>
#include <cctype>
#include <map>

const std::map<char, std::string> DTC_PREFIXES = {
    {'0', "P0"}, {'1', "P1"}, {'2', "P2"}, {'3', "P3"},
    {'4', "C0"}, {'5', "C1"}, {'6', "C2"}, {'7', "C3"},
    {'8', "B0"}, {'9', "B1"}, {'A', "B2"}, {'B', "B3"},
    {'C', "U0"}, {'D', "U1"}, {'E', "U2"}, {'F', "U3"}
};

std::string DTCReader::decodeDTC(const std::string& raw) {
    if (raw.length() < 4 || raw == "0000") {
        return "";
    }
    
    char first = std::toupper(raw[0]);
    auto it = DTC_PREFIXES.find(first);
    
    std::string prefix = (it != DTC_PREFIXES.end()) ? it->second : "P0";
    return prefix[0] + raw.substr(1);
}

std::vector<std::string> DTCReader::readDTCs(OBDConnection& conn) {
    std::string resp = conn.send("03", 8);
    std::string clean = resp;
    
    // Remove spaces and convert to uppercase
    clean.erase(std::remove(clean.begin(), clean.end(), ' '), clean.end());
    std::transform(clean.begin(), clean.end(), clean.begin(), ::toupper);
    
    std::vector<std::string> codes;
    
    size_t pos = clean.find("43");
    if (pos == std::string::npos) {
        return codes;
    }
    
    // Skip past "43" and count byte
    size_t idx = pos + 2 + 2;
    
    while (idx + 4 <= clean.length()) {
        std::string raw = clean.substr(idx, 4);
        std::string code = decodeDTC(raw);
        
        if (!code.empty()) {
            codes.push_back(code);
        }
        
        idx += 4;
    }
    
    return codes;
}

bool DTCReader::clearDTCs(OBDConnection& conn) {
    std::string resp = conn.send("04", 8);
    std::string clean = resp;
    
    clean.erase(std::remove(clean.begin(), clean.end(), ' '), clean.end());
    std::transform(clean.begin(), clean.end(), clean.begin(), ::toupper);
    
    return clean.find("44") != std::string::npos || 
           clean.find("OK") != std::string::npos;
}
