#ifndef STOCKANALYSIS_HPP
#define STOCKANALYSIS_HPP

#include "Stock.hpp"
#include "StockUtils.hpp"
#include "Common.hpp"
#include <utility>
#include <ctime>
#include <iomanip>

class StockAnalysis {
public:
    static double calculateMean(const std::unordered_map<std::string, double>& data);
    static double calculateStandardDeviation(const std::unordered_map<std::string, double>& data);

	// Calculate both mean and standard deviation, with caching
    static std::pair<double, double> calculateStatistics(const std::unordered_map<std::string, double>& data);

	// Method to clear cache
	static void clearCache();
	
	// Method to find the difference of price between two stocks in their respective dates
	static std::unordered_map<std::string, double> calculateDifference(
        const Stock& stock1, const Stock& stock2, const std::string& priceType, 
        const std::string& startDate, const std::string& endDate
    );
	
    // New function to calculate the difference between two data dictionaries
    static std::unordered_map<std::string, double> calculateDifferenceBetweenData(
        const std::unordered_map<std::string, double>& data1,
        const std::unordered_map<std::string, double>& data2
    );
	
    // Method to normalize the values in a data dictionary based on the earliest date
    static std::unordered_map<std::string, double> normalizeToEarliestDate(
        const std::unordered_map<std::string, double>& data
    );

	// Method to check if a stock has valid data within a specified range
    static bool hasValidDataInRange(
        const Stock& stock1, const Stock& stock2, const std::string& priceType,
        const std::string& startDate, const std::string& endDate
    );

	// Method to calculate the price-volume of certain price type of a Stock on a specific date
    static double calculatePriceVolume(const Stock& stock, const std::string& date, const std::string& priceType = "close");

	// Method to calculate the average price-volume of a certain price type of a Stock within a range of dates
    static double calculateAveragePriceVolume(
        const Stock& stock, const std::string& priceType, const std::string& startDate,
		const std::string& endDate
    );
	
	// Method to sort map by values
	static std::vector<std::pair<std::string, double>> sortMap( std::map <std::string, double> givenMap );
	
	// Overload sortMap function
	static std::vector<std::pair<std::string, double>> sortMap(const std::unordered_map<std::string, double>& givenMap);
	
	// Method to calculate the p-value of unit-root test for a given dictionary of dates and prices
	static double calculatePValueForURT(
    const std::unordered_map<std::string, double>& data,
    const std::string& testType, int lags, const std::string& trend,
    const std::string& method, const std::string& lagLength = "");
	
	// Utility function for creating cacheKeys
	static std::string makeCacheKey(const std::string& stock1, const std::string& stock2);
	
	// Normalize data to some other reference data
	static std::unordered_map<std::string, double> normalizeToReferenceData(
    const std::unordered_map<std::string, double>& dataToNormalize,
    const std::unordered_map<std::string, double>& referenceData);
	
private:
    // Cache for calculated statistics; uses the address of the dataset as the key
    static std::unordered_map<const std::unordered_map<std::string, double>*, std::pair<double, double>> statisticsCache;
};

#endif // STOCKANALYSIS_HPP
