#ifndef DTC_H
#define DTC_H

#include <string>
#include <vector>
#include "connection.h"

class DTCReader {
public:
    static std::vector<std::string> readDTCs(OBDConnection& conn);
    static bool clearDTCs(OBDConnection& conn);

private:
    static std::string decodeDTC(const std::string& raw);
};

#endif // DTC_H
