#include "StockAnalysis.hpp"
#include "FileReader.hpp"

// Initialize the static cache
std::unordered_map<const std::unordered_map<std::string, double>*, std::pair<double, double>> StockAnalysis::statisticsCache;

double StockAnalysis::calculateMean(const std::unordered_map<std::string, double>& data) {
    if (data.empty()) return 0.0;

    double sum = std::accumulate(data.begin(), data.end(), 0.0,
                                 [](double total, const auto& pair) { return total + pair.second; });
    return sum / data.size();
}

double StockAnalysis::calculateStandardDeviation(const std::unordered_map<std::string, double>& data) {
    if (data.empty()) return 0.0;

    double mean = calculateMean(data);
    double variance = std::accumulate(data.begin(), data.end(), 0.0,
                                      [mean](double total, const auto& pair) {
                                          double diff = pair.second - mean;
                                          return total + diff * diff;
                                      }) / data.size();
    return std::sqrt(variance);
}

// Method to calculate the mean and standard deviation and assign it to a cache location unique to each inputted data set
std::pair<double, double> StockAnalysis::calculateStatistics(const std::unordered_map<std::string, double>& data) {
    // Check if the result is already cached
    auto it = statisticsCache.find(&data);
    if (it != statisticsCache.end()) {
        return it->second;
    }

    // Calculate mean
    double sum = std::accumulate(data.begin(), data.end(), 0.0,
                                 [](double total, const auto& pair) { return total + pair.second; });
    double mean = sum / data.size();

    // Calculate standard deviation using the mean
    double variance = std::accumulate(data.begin(), data.end(), 0.0,
                                      [mean](double total, const auto& pair) {
                                          double diff = pair.second - mean;
                                          return total + diff * diff;
                                      }) / data.size();
    double stdDev = std::sqrt(variance);

    // Cache the result
    statisticsCache[&data] = {mean, stdDev};

    return {mean, stdDev};
}

// Method to calculate the difference between two stocks within the same date range for a given price type
std::unordered_map<std::string, double> StockAnalysis::calculateDifference(
    const Stock& stock1, const Stock& stock2, const std::string& priceType, 
    const std::string& startDate, const std::string& endDate
) {
    auto data1 = StockUtils::getPriceDataInRange(stock1, priceType, startDate, endDate);
    auto data2 = StockUtils::getPriceDataInRange(stock2, priceType, startDate, endDate);

    // Verify that both data dictionaries have identical dates
    if (data1.size() != data2.size()) {
        throw std::invalid_argument("Data dictionaries have different numbers of dates.");
    }
	std::unordered_map<std::string, double> difference;
    for (const auto& entry : data1) {
        const auto& date = entry.first;
        if (data2.count(date) == 0) {
            throw std::invalid_argument("Data dictionaries do not have identical dates. Missing date from second stock input: " + date);
        } else{
			difference[date] = entry.second - data2.at(date);
		}
    }

    return difference;
}

// Method to calculate the difference between two data dictionaries
std::unordered_map<std::string, double> StockAnalysis::calculateDifferenceBetweenData(
    const std::unordered_map<std::string, double>& data1,
    const std::unordered_map<std::string, double>& data2
) {
    std::unordered_map<std::string, double> difference;

    for (const auto& entry : data1) {
        const auto& date = entry.first;
        double value1 = entry.second;

        // Ensure the date exists in both dictionaries
        auto it = data2.find(date);
        if (it != data2.end()) {
            double value2 = it->second;
            difference[date] = value1 - value2;
        } else {
            throw std::invalid_argument("Data dictionaries do not have identical dates.");
        }
    }

    return difference;
}

// Normalize the values of a dictionary to the value corresponding to the earliest date
std::unordered_map<std::string, double> StockAnalysis::normalizeToEarliestDate(
    const std::unordered_map<std::string, double>& data
) {
    if (data.empty()) {
        throw std::invalid_argument("Data dictionary is empty.");
    }

    // Find the earliest date in the dictionary
    auto earliestEntry = std::min_element(data.begin(), data.end(),
                                          [](const auto& a, const auto& b) {
                                              return a.first < b.first; // Lexicographical comparison of dates
                                          });

    double baseValue = earliestEntry->second;

    // Check if the base value is non-zero to prevent division by zero
    if (baseValue == 0) {
        throw std::runtime_error("The price value on the earliest date is zero, cannot normalize.");
    }

    // Normalize each value by dividing by the base value
    std::unordered_map<std::string, double> normalizedData;
    for (const auto& entry : data) {
        normalizedData[entry.first] = entry.second / baseValue;
    }

    return normalizedData;
}

// Check if a Stock object has valid data within a specified date range
bool StockAnalysis::hasValidDataInRange(
    const Stock& stock1, const Stock& stock2, const std::string& priceType,
    const std::string& startDate, const std::string& endDate
) {
    // Retrieve data for the specified price type within the date range for both stocks
    auto data1 = StockUtils::getPriceDataInRange(stock1, priceType, startDate, endDate);
    auto data2 = StockUtils::getPriceDataInRange(stock2, priceType, startDate, endDate);

    // If either stock has no data in the specified range, return false
    if (data1.empty() || data2.empty()) {
        std::cout << "One of the stocks has no data in the specified range.\n";
        return false;
    }

    // Compare dates in both datasets
    if (data1.size() != data2.size()) {
        std::cout << "The stocks have a different number of dates in the specified range.\n";
        return false;
    }

    for (const auto& entry : data1) {
        const std::string& date = entry.first;
        auto it2 = data2.find(date);

        // Check if the date exists in both datasets and has valid (non-zero) data
        if (it2 == data2.end() || entry.second == 0 || it2->second == 0) {
            std::cout << "Mismatch or invalid data on date: " << date << "\n";
            return false;
        }
    }

    return true;
}

// Calculate the price-volume for a given date and price type
double StockAnalysis::calculatePriceVolume(const Stock& stock, const std::string& date, const std::string& priceType) {
    // Retrieve historical data from the stock
    const auto& historicalData = stock.getHistoricalData();

    // Check if the specified date exists in historical data
    auto dateIt = historicalData.find(date);
    if (dateIt == historicalData.end()) {
        throw std::invalid_argument("No data available for the specified date: " + date);
    }

    // Retrieve the price and volume for the given date
    const auto& dataMap = dateIt->second;
    auto priceIt = dataMap.find(priceType);
    auto volumeIt = dataMap.find("volume");

    // Ensure both price and volume are available
    if (priceIt == dataMap.end()) {
        throw std::invalid_argument("No price data found for type: " + priceType + " on date: " + date);
    }
    if (volumeIt == dataMap.end()) {
        throw std::invalid_argument("No volume data found on date: " + date);
    }

    // Calculate and return the price-volume product
    return priceIt->second * volumeIt->second;
}

// Helper function to increment a date by one day
std::string incrementDate(const std::string& dateStr) {
    std::tm tm = {};
    std::istringstream ss(dateStr);
    ss >> std::get_time(&tm, "%Y-%m-%d");

    // Increment the day by one
    tm.tm_mday += 1;

    // Normalize the time structure, handling month and year transitions
    std::mktime(&tm);

    // Convert back to string in YYYY-MM-DD format
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d");
    return oss.str();
}

// Calculate the average price-volume of a stock within a specified date range
double StockAnalysis::calculateAveragePriceVolume(
    const Stock& stock, const std::string& priceType, const std::string& startDate,
	const std::string& endDate
) {
    double totalPriceVolume = 0.0;
    int validDateCount = 0;

    // Loop through each date in the range
    std::string currentDate = startDate;
    while (currentDate <= endDate) {
        try {
            // Attempt to calculate price-volume for the current date
            double priceVolume = calculatePriceVolume(stock, currentDate, priceType);
            totalPriceVolume += priceVolume;
            validDateCount++;
        } catch (const std::exception& e) {
            // Ignore dates with missing data or other issues
        }

        // Move to the next date
        currentDate = incrementDate(currentDate);
    }

    // Calculate the average if there are valid dates; otherwise, return 0
    if (validDateCount > 0) {
        return totalPriceVolume / validDateCount;
    } else {
        throw std::runtime_error("No valid price-volume data found within the specified date range.");
    }
}

// Helper function to perform key-value pair comparison// Create a comparator function to perform key-value pair comparison
bool compare (std::pair <std::string, double> &a, std::pair <std::string, double> &b ){
   return a.second < b.second;
}

// Define sorting function to sort given dictionary or map
std::vector<std::pair <std::string, double> > StockAnalysis::sortMap( std::map <std::string, double> givenMap ){
   std::vector<std::pair <std::string, double> > pairVec;
   std::map<std::string, double> newMap;
   
   for ( auto& it : givenMap ) {
      pairVec.push_back( it );
   }
   
   std::sort( pairVec.begin(), pairVec.end(), compare);

   return pairVec;
}

// Overload sortMap function
std::vector<std::pair<std::string, double>> StockAnalysis::sortMap(const std::unordered_map<std::string, double>& givenMap) {
    std::vector<std::pair<std::string, double>> vec(givenMap.begin(), givenMap.end());
    std::sort(vec.begin(), vec.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    return vec;
}

double StockAnalysis::calculatePValueForURT(
    const std::unordered_map<std::string, double>& data,
    const std::string& testType, int lags, const std::string& trend,
    const std::string& method, const std::string& lagLength) {
    
    if (data.empty()) {
        throw std::invalid_argument("Input data is empty.");
    }

    // Sort the input data by date
    std::vector<std::pair<std::string, double>> sortedData(data.begin(), data.end());
    std::sort(sortedData.begin(), sortedData.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });

    // Convert sorted data to URT-compatible Eigen::VectorXd
    Eigen::VectorXd prices(sortedData.size());
    for (size_t i = 0; i < sortedData.size(); ++i) {
        prices[i] = sortedData[i].second;
    }

    // Select the appropriate test type and calculate the p-value
    double pValue = 0.0;

    if (testType == "ADF") {
        urt::ADF<double> adfTest(prices, lags, trend);

        // Apply lag length optimization method if provided
        if (!method.empty()) {
            adfTest.method = method; // e.g., "AIC", "BIC"
        }

        pValue = adfTest.pvalue();
    } else if (testType == "PP") {
        std::string ppMethod = method.empty() ? "tau" : method; // Default to "tau"
        if (ppMethod != "tau" && ppMethod != "rho") {
            throw std::invalid_argument("Invalid method for PP test: " + ppMethod);
        }

        urt::PP<double> ppTest = lagLength.empty()
            ? urt::PP<double>(prices, lags, trend, ppMethod)
            : urt::PP<double>(prices, lagLength, trend, ppMethod);

        pValue = ppTest.pvalue();
    } else if (testType == "KPSS") {
        urt::KPSS<double> kpssTest = lagLength.empty()
            ? urt::KPSS<double>(prices, lags, trend)
            : urt::KPSS<double>(prices, lagLength, trend);

        pValue = kpssTest.pvalue();
    } else if (testType == "DFGLS") {
        urt::DFGLS<double> dfglsTest(prices, lags, trend);

        // Apply lag length optimization method if provided
        if (!method.empty()) {
            dfglsTest.method = method; // e.g., "AIC", "BIC"
        }

        pValue = dfglsTest.pvalue();
    } else {
        throw std::invalid_argument("Invalid test type specified: " + testType);
    }

    return pValue;
}

// Utility function for creating cache keys
std::string StockAnalysis::makeCacheKey(const std::string& stock1, const std::string& stock2) {
	return stock1 < stock2 ? stock1 + "-" + stock2 : stock2 + "-" + stock1;
}

std::unordered_map<std::string, double> StockAnalysis::normalizeToReferenceData(
    const std::unordered_map<std::string, double>& dataToNormalize,
    const std::unordered_map<std::string, double>& referenceData) {

    if (referenceData.empty()) {
        throw std::invalid_argument("Reference data is empty.");
    }
    
	// Find the earliest date in the reference data
    auto earliestEntry = std::min_element(referenceData.begin(), referenceData.end(),
                                          [](const auto& a, const auto& b) {
                                              return a.first < b.first; // Lexicographical comparison of dates
                                          });

    const std::string& earliestDate = earliestEntry->first;

    // Check if the base value is non-zero to prevent division by zero
    double baseValue = earliestEntry->second;
    if (baseValue == 0) {
        throw std::runtime_error("The price value on the reference earliest date is zero, cannot normalize.");
    }

    // Normalize the dataToNormalize based on the base value
    std::unordered_map<std::string, double> normalizedData;
    for (const auto& entry : dataToNormalize) {
        normalizedData[entry.first] = entry.second / baseValue;
    }

    return normalizedData;
}

// Function to apply a mathematical operation to values in a map
std::unordered_map<std::string, double> StockAnalysis::applyMapOperation(
	const std::unordered_map<std::string, double>& data,
	const std::function<double(double)>& operation
) {
	std::unordered_map<std::string, double> result;

	for (const auto& [key, value] : data) {
		try {
			// Attempt to apply the operation
			double operatedValue = operation(value);

			// Check for invalid results (e.g., NaN, infinity)
			if (std::isnan(operatedValue) || std::isinf(operatedValue)) {
				operatedValue = 0.0; // Set invalid results to 0
			}

			result[key] = operatedValue;
		} catch (...) {
			// Catch any exceptions and set the value to 0
			result[key] = 0.0;
		}
	}

	return result;
}

// Method to clear the cache
void StockAnalysis::clearCache(){
	statisticsCache.clear();
}