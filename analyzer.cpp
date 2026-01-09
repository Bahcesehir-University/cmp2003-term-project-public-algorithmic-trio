#include "analyzer.h"
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <array>

void TripAnalyzer::processLine(const std::string& line) {
    int commaCount = 0;
    for (char c : line) {
        if (c == ',') commaCount++;
    }
    if (commaCount < 5) return;

    std::size_t c1 = line.find(',');
    
    std::size_t c2 = line.find(',', c1 + 1);
    
    std::size_t c3 = line.find(',', c2 + 1);
    
    std::size_t c4 = line.find(',', c3 + 1);
    
    std::string_view pickupZone(line.data() + c1 + 1, c2 - c1 - 1);
    if (pickupZone.empty()) return;
    
    std::string_view pickupTime(line.data() + c3 + 1, c4 - c3 - 1);
    if (pickupTime.empty()) return;
    
    std::size_t spacePos = pickupTime.find(' ');
    if (spacePos == std::string::npos || spacePos + 3 > pickupTime.length()) {
        return;
    }
    
    char c1_char = pickupTime[spacePos + 1];
    char c2_char = pickupTime[spacePos + 2];
    if (pickupTime[spacePos + 3] != ':') return;
    
    int hour = (c1_char - '0') * 10 + (c2_char - '0');
    if (hour > 23) return;
    
    std::string zone(pickupZone);
    zoneCount[zone]++;
    
    slotCount.try_emplace(zone, std::array<long long, 24>{}).first->second[hour]++;
}

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    std::ifstream file(csvPath);
    
    std::string line;
    line.reserve(256);
    bool isFirstLine = true;

    zoneCount.reserve(50000);
    slotCount.reserve(50000);
    
    while (std::getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }
        processLine(line);
    }
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    std::vector<ZoneCount> result;
    
    for (auto zone = zoneCount.begin(); zone != zoneCount.end(); ++zone) {
        result.push_back({ zone->first, zone->second });
    }

    std::sort(result.begin(), result.end(), 
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            return a.zone < b.zone;
        });
    
    if (result.size() > static_cast<size_t>(k)) {
        result.resize(k);
    }
    
    return result;
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    std::vector<SlotCount> result;
    
    for (const auto& zonePair : slotCount) {
        const auto& hourArray = zonePair.second;
        
        for (int hour = 0; hour < 24; hour++) {
            if (hourArray[hour] > 0) {
                result.push_back({zonePair.first, hour, hourArray[hour]});
            }
        }
    }
    std::sort(result.begin(), result.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            if (a.zone != b.zone) {
                return a.zone < b.zone;
            }
            return a.hour < b.hour;
        });
    if (result.size() > static_cast<size_t>(k)) {
        result.resize(k);
    }
    return result;
}
