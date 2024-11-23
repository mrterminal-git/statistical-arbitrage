#include "PairsTradingBackTesting.hpp"
#include <chrono>

// Constructor
PairsTradingBackTesting::PairsTradingBackTesting(
    const std::string& stock1Name,
    const std::unordered_map<std::string, double>& stock1Data,
    const std::string& stock2Name,
    const std::unordered_map<std::string, double>& stock2Data,
    const std::unordered_map<std::string, double>& pairSelectionData1,
    const std::unordered_map<std::string, double>& pairSelectionData2,
    double initialBalance)
    : BackTesting(initialBalance),
      stock1Name(stock1Name),
      stock2Name(stock2Name),
      stock1Data(stock1Data),
      stock2Data(stock2Data),
      pairSelectionData1(pairSelectionData1),
      pairSelectionData2(pairSelectionData2) {

    normalizedStock1Data = StockAnalysis::normalizeToReferenceData(stock1Data, pairSelectionData1);
    normalizedStock2Data = StockAnalysis::normalizeToReferenceData(stock2Data, pairSelectionData2);
}

// Calculate spread between normalized prices
double PairsTradingBackTesting::calculateSpread(const std::string& date) const {
    auto it1 = normalizedStock1Data.find(date);
    auto it2 = normalizedStock2Data.find(date);
    if (it1 != normalizedStock1Data.end() && it2 != normalizedStock2Data.end()) {
        return it1->second - it2->second;
    }
    throw std::invalid_argument("Date not found in one or both stocks.");
}

// Check entry signal
bool PairsTradingBackTesting::isEntrySignal(double spread, double threshold) const {
    return std::fabs(spread) > threshold;
}

// Check exit signal
bool PairsTradingBackTesting::isExitSignal(double spread, double threshold) const {
    return std::fabs(spread) < threshold;
}

// Run back-test
void PairsTradingBackTesting::run(const BackTestingConfig& config) {
    std::string entryDate;
	std::string exitDate;
    std::unordered_map<std::string, double> entryPrices;
	
	// sort stock1Data by date
	std::vector<std::pair<std::string, double>> stock1DataSorted = StockAnalysis::sortMap(stock1Data);
	
    for (const auto& [date, price1] : stock1DataSorted) {
        if (stock2Data.count(date) == 0) continue;

        double spread = calculateSpread(date);

        if (entryDate.empty()) {
            if (currentBalance < config.tradeAmount * 2) {
                continue;
            }

            if (fabs(spread) > config.entryThreshold) {
                entryDate = date;
                entryPrices[stock1Name] = price1;
                entryPrices[stock2Name] = stock2Data.at(date);
                currentBalance -= config.tradeAmount * 2;
            }
        } else {
            if (fabs(spread) < config.exitThreshold) {
				exitDate = date;
                std::unordered_map<std::string, double> exitPrices;
                exitPrices[stock1Name] = price1;
                exitPrices[stock2Name] = stock2Data.at(date);

                double profitLoss = calculateProfitLoss(entryDate, exitDate, config.tradeAmount);

                currentBalance += config.tradeAmount * 2 + profitLoss;

                Trade trade{entryDate, date, entryPrices, exitPrices, profitLoss};
                recordTrade(trade);

                entryDate.clear();
				exitDate.clear();
                entryPrices.clear();
            }
        }
    }

    // Close any open trades
    if (!entryDate.empty()) {
        auto lastEntry = stock1DataSorted.back();
		std::string lastDate = lastEntry.first;
		double price1 = lastEntry.second;
		double price2 = stock2Data.at(lastEntry.first);

		std::cout << "Closing open trade at end date: " << lastEntry.first << "\n";

        std::unordered_map<std::string, double> exitPrices;
        exitPrices[stock1Name] = price1;
        exitPrices[stock2Name] = price2;
		
        double profitLoss = calculateProfitLoss(entryDate, lastDate, config.tradeAmount);

        currentBalance += config.tradeAmount * 2 + profitLoss;

        Trade trade{entryDate, lastDate, entryPrices, exitPrices, profitLoss};
        recordTrade(trade);
		
    }
}

// Override to BackTesting::run()
void PairsTradingBackTesting::run() {
    BackTestingConfig defaultConfig;
    defaultConfig.entryThreshold = 2.0;
    defaultConfig.exitThreshold = 1.0;
    defaultConfig.tradeAmount = 100.0;
    run(defaultConfig); // Call the overloaded run function
}

// Calculate profit-loss for a given trade
double PairsTradingBackTesting::calculateProfitLoss(
    const std::string& entryDate,
    const std::string& exitDate,
    const double& tradeAmount) const {

    // Fetch normalized and actual prices for entry and exit dates
    if (normalizedStock1Data.find(entryDate) == normalizedStock1Data.end() || 
        normalizedStock2Data.find(entryDate) == normalizedStock2Data.end() ||
        normalizedStock1Data.find(exitDate) == normalizedStock1Data.end() || 
        normalizedStock2Data.find(exitDate) == normalizedStock2Data.end()) {
        throw std::runtime_error("Normalized data for entry or exit date is missing.");
    }

    double normalizedEntryPriceStock1 = normalizedStock1Data.at(entryDate);
    double normalizedEntryPriceStock2 = normalizedStock2Data.at(entryDate);
    double entryPriceStock1 = stock1Data.at(entryDate);
    double entryPriceStock2 = stock2Data.at(entryDate);

    double exitPriceStock1 = stock1Data.at(exitDate);
    double exitPriceStock2 = stock2Data.at(exitDate);

    double stock1Profit = 0.0, stock2Profit = 0.0;

    // Compare normalized prices to decide long/short positions
    if (normalizedEntryPriceStock1 > normalizedEntryPriceStock2) {
        // Short stock1, long stock2
        stock1Profit = (tradeAmount / entryPriceStock1) * (entryPriceStock1 - exitPriceStock1);
        stock2Profit = (tradeAmount / entryPriceStock2) * (exitPriceStock2 - entryPriceStock2);
    } else {
        // Long stock1, short stock2
        stock1Profit = (tradeAmount / entryPriceStock1) * (exitPriceStock1 - entryPriceStock1);
        stock2Profit = (tradeAmount / entryPriceStock2) * (entryPriceStock2 - exitPriceStock2);
    }

    return stock1Profit + stock2Profit;
}

// Find the appropriate pairs via Distance Method
std::map<std::string, PairsTradingBackTesting::PairStatistics> PairsTradingBackTesting::selectPairsForBackTesting(
    std::vector<std::string> stockListings,
    const std::string& stockDataDir,
    const std::string& fileExtension,
    const std::string& priceType,
    const std::string& startDate,
    const std::string& endDate,
    double priceVolumeThreshold) {
	auto start = std::chrono::high_resolution_clock::now();

	// Calculate Mean, SDs, P-value, and ADF P-values for all stock pairs
	
	// Initialize the dictionary that will hold the pairs as keys and the mean of the difference as the values
	std::map<std::string, double> stockPairMean, stockPairStandardDeviation;

	// Initlaize pair statistics dictionary for output
    std::map<std::string, PairStatistics> pairStatistics;

	// Initlaize the dictionary that will hold the stock symbol and their object
	std::unordered_map<std::string, std::unordered_map<std::string, double>> stockAdjClose;

	// Filter out the Stocks that do not have valid dates (i.e. do not have data available within the start and end date) and non-liquid
	// This step can be done by comparing the stock dates to the SPY stock dates (since it's very hard to know what days the NYSE is open)
	std::vector<std::string> toRemoveListings;
	Stock SPY;
	FileReader::loadStockDataFromFile(stockDataDir + "SPY" + fileExtension, SPY);
	for(std::string nyseListing : stockListings){
		// Loading Stock
		Stock stock;
		std::cout << "Opening file: " << stockDataDir + nyseListing + fileExtension << "\n";
		FileReader::loadStockDataFromFile(stockDataDir + nyseListing + fileExtension, stock);
		
		try{
			// Check if Stock is liquid enough
			if(StockAnalysis::calculateAveragePriceVolume(stock, priceType, startDate, endDate) < priceVolumeThreshold){
				std::cout << nyseListing << " is not liquid enough in between dates " << startDate << " " << endDate << "\n";
				toRemoveListings.push_back(nyseListing);
				continue;
			}
		} catch(const std::exception& e){
			std::cout << nyseListing << " error " << e.what() << " occured when checking for price-volume " << "\n"; 
			toRemoveListings.push_back(nyseListing);
			continue;
		}
		
		// Check if Stock has all valid dates identical to the SPY dates
		if(!StockAnalysis::hasValidDataInRange(stock, SPY, priceType, startDate, endDate)){
			std::cout << nyseListing << " does not have a valid date in between dates " << startDate << " " << endDate << "\n";
			toRemoveListings.push_back(nyseListing);
			continue;
		}
		
		// Pre-load all the valid data into memory
		stockAdjClose[nyseListing] = StockUtils::getPriceDataInRange(stock, priceType, startDate, endDate); // Store the stock in the dictionary;
		std::cout << "--------------------------------------------------\n";
	}

	// Removing the filtered listings from the NYSE listings
	stockListings.erase(std::remove_if(stockListings.begin(), stockListings.end(),
									[&toRemoveListings](const std::string &str) {
										return std::find(toRemoveListings.begin(), toRemoveListings.end(), str) != toRemoveListings.end();
									}),
					 stockListings.end());
	toRemoveListings.clear();

	// Precompute normalized data
	std::unordered_map<std::string, std::unordered_map<std::string, double>> normalizedStockData;
	for (const auto& [stock, data] : stockAdjClose) {
		normalizedStockData[stock] = StockAnalysis::normalizeToEarliestDate(data);
	}

	// Loop through all valid NYSE listings to find the best pairs
	while(stockListings.size() > 1){
		// Get the first stock of the NYSE listings
		std::string baseStock = stockListings.at(0);
		std::cout << "current base stock: " << baseStock << "\n";
		
		// Normalize the comparison stock
		std::unordered_map<std::string, double> baseStockNormalized = normalizedStockData[baseStock];
		
		// Compare against all the other stock in NYSE listings
		double currentStandardDeviation;
		std::string comparisonStockToRemove;
		PairStatistics stats;
		for(int i = 1; i < stockListings.size(); ++i){

			
			// Get the comparison stock
			std::string comparisonStock = stockListings.at(i);
			//std::cout << "current comparison stock: " << comparisonStock << "\n";
			
			// Normalize the comparison stock
			std::unordered_map<std::string, double> comparisonStockNormalized = normalizedStockData[comparisonStock];
			
			// Find the difference between the comparison stock to the base stock
			std::unordered_map<std::string, double> differenceMap = StockAnalysis::calculateDifferenceBetweenData(baseStockNormalized, comparisonStockNormalized);
			
			// Calculate the statistics of the difference and store them dictionaries
			// Store the pair statistic in an unordered_map
			std::pair<double, double> differenceStatistics = StockAnalysis::calculateStatistics(differenceMap); // {mean, standard deviation}
			StockAnalysis::clearCache();
			
			// Store the statistics in the pair maps and the difference map
			if(i == 1 || differenceStatistics.second <  currentStandardDeviation){
				// Storing only pairs that result in lower standard deviations
				currentStandardDeviation = differenceStatistics.second;
				comparisonStockToRemove = comparisonStock;
				
				// Store the usual statistics
				stats.mean = differenceStatistics.first;
				stats.standardDeviation = differenceStatistics.second;
				stats.pValue = 2.0 * (1.0 - StandardNormalDistribution::cdf(std::fabs(differenceStatistics.first) / differenceStatistics.second));

				// For unit-root tests
				try {
					// Store the URT statistics
					stats.adfPValueAIC = StockAnalysis::calculatePValueForURT(differenceMap, "ADF", 10, "ct", "AIC");
					stats.adfPValueBIC = StockAnalysis::calculatePValueForURT(differenceMap, "ADF", 10, "ct", "BIC");
					stats.ppPValueShortRho = StockAnalysis::calculatePValueForURT(differenceMap, "PP", 10, "ct", "rho", "short");
					stats.ppPValueLongRho = StockAnalysis::calculatePValueForURT(differenceMap, "PP", 10, "ct", "rho", "long");
					stats.ppPValueShortTau = StockAnalysis::calculatePValueForURT(differenceMap, "PP", 10, "ct", "tau", "short");
					stats.ppPValueLongTau = StockAnalysis::calculatePValueForURT(differenceMap, "PP", 10, "ct", "tau", "long");
					stats.kpssPValueShort = StockAnalysis::calculatePValueForURT(differenceMap, "KPSS", 10, "ct", "", "short");
					stats.kpssPValueLong = StockAnalysis::calculatePValueForURT(differenceMap, "KPSS", 10, "ct", "", "long");
					
				} catch (const std::exception& e) {
					std::cout << "Encountered error during unit-root test with error \"" << e.what() << "\"\n";
					stats.adfPValueAIC = -1;
					stats.adfPValueBIC = -1;
					stats.ppPValueShortRho = -1;
					stats.ppPValueLongRho = -1;
					stats.ppPValueShortTau = -1;
					stats.ppPValueLongTau = -1;
					stats.kpssPValueShort = -1;
					stats.kpssPValueLong = -1;
					
				} // End of unit-root tests
				
			} // End of storing test statisitcs
			
		} // End of looping through comparison stocks
		
		// Add the pair object into the dictionary
		const std::string pairKey = baseStock + "-" + comparisonStockToRemove;
		pairStatistics[pairKey] = stats;
		
		// Removing the pairs from the NYSE listing
		for(auto toRemove : {baseStock, comparisonStockToRemove}){
			auto itr = std::find(stockListings.begin(), stockListings.end(), toRemove);
			if (itr != stockListings.end()) stockListings.erase(itr);
		}
		
		std::cout << "--------------------------------------------------\n";
	} //  End of looping through base stocks

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	std::cout << "Time taken by function: "
		 << duration.count() << " microseconds" << "\n";
	;
    return pairStatistics;
}

void PairsTradingBackTesting::logResults(const std::string& filename) const {
    std::ofstream file;

    // Check if the file exists
    if (std::filesystem::exists(filename)) {
        file.open(filename, std::ios::app); // Open in append mode
    } else {
        file.open(filename, std::ios::out); // Create a new file
        // Write the header if creating a new file
        file << "EntryDate,ExitDate";
        for (const auto& element : trades.at(0).entryPrices) {
            file << "," << element.first << " Entry Price";
        }
        for (const auto& element : trades.at(0).exitPrices) {
            file << "," << element.first << " Exit Price";
        }
        file << ",ProfitLoss\n";
    }

    // Check if file opened successfully
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }

    // Append trade details to the file
	file << stock1Name << "-" << stock2Name << "\n";
    for (const auto& trade : trades) {
        file << trade.entryDate << "," << trade.exitDate;
        for (const auto& element : trade.entryPrices) {
            file << "," << element.second;
        }
        for (const auto& element : trade.exitPrices) {
            file << "," << element.second;
        }
        file << "," << trade.profitLoss << "\n";
    }
	file << "\n";
    file.close();
}