#ifndef STOCK_HPP
#define STOCK_HPP

#include "Common.hpp"

class Stock {
public:
    // Define the inner map type for holding specific data points like "close", "high", etc.
    using DataMap = std::unordered_map<std::string, double>;

    // Define the outer map type for holding historical data with date as the key
    using HistoricalData = std::unordered_map<std::string, DataMap>;

    // Constructor and Destructor
    Stock() = default;
    ~Stock() = default;

    // Method to add a data entry for a specific date and key
    void addData(const std::string& date, const std::string& key, double value);

    // Method to retrieve data for a specific date and key
    double getData(const std::string& date, const std::string& key) const;

    // Method to print all historical data
    void printHistoricalData() const;

	// Method to retrieve all historical data
	const Stock::HistoricalData& getHistoricalData() const;

private:
    HistoricalData historicalData; // Holds the historical data
};

#endif // STOCK_H