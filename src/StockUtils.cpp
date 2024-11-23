#include "StockUtils.hpp"

// Method to retrieve just a single value (adj close, close, high, low, open, volume) from Stock object
std::unordered_map<std::string, double> StockUtils::getPriceData(const Stock& stock, const std::string& priceType) {
    std::unordered_map<std::string, double> priceData;
    const auto& historicalData = stock.getHistoricalData();

    for (const auto& dateEntry : historicalData) {
        const auto& date = dateEntry.first;
        const auto& dataMap = dateEntry.second;

        auto it = dataMap.find(priceType);
        if (it != dataMap.end()) {
            priceData[date] = it->second;
        }
    }

    return priceData;
}

// Helper function to check if a date is within the specified range
bool isDateInRange(const std::string& date, const std::string& startDate, const std::string& endDate) {
    return date >= startDate && date <= endDate;
}

// Method to retrieve a price type (adj close, close, high, low, open, volume) from Stock object within a date range
std::unordered_map<std::string, double> StockUtils::getPriceDataInRange(
    const Stock& stock, const std::string& priceType, const std::string& startDate, const std::string& endDate
) {
    std::unordered_map<std::string, double> priceData;
    const auto& historicalData = stock.getHistoricalData();

    for (const auto& dateEntry : historicalData) {
        const auto& date = dateEntry.first;

        // Check if the date falls within the specified range
        if (isDateInRange(date, startDate, endDate)) {
            const auto& dataMap = dateEntry.second;

            auto it = dataMap.find(priceType);
            if (it != dataMap.end()) {
                priceData[date] = it->second;
            }
        }
    }

    return priceData;
}