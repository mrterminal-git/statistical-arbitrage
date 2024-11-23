#include "Stock.hpp"

// Add a data entry for a specific date
void Stock::addData(const std::string& date, const std::string& key, double value) {
    historicalData[date][key] = value;
}

// Retrieve data for a specific date and key
double Stock::getData(const std::string& date, const std::string& key) const {
    if (historicalData.count(date) && historicalData.at(date).count(key)) {
        return historicalData.at(date).at(key);
    } else {
        std::cerr << "Data not found for date: " << date << " and key: " << key << std::endl;
        return 0.0; // Or throw an exception if preferred
    }
}

// Print all historical data
void Stock::printHistoricalData() const {
    for (const auto& dateEntry : historicalData) {
        std::cout << "Date: " << dateEntry.first << "\n";
        for (const auto& dataEntry : dateEntry.second) {
            std::cout << "  " << dataEntry.first << ": " << dataEntry.second << "\n";
        }
    }
}

// Retrieve all historical data
const Stock::HistoricalData& Stock::getHistoricalData() const {
    return historicalData;
}