#include "Common.hpp"
#include "FileReader.hpp"
#include "Stock.hpp"
#include "StockUtils.hpp"
#include "StockAnalysis.hpp"
#include "StandardNormalDistribution.hpp"
#include "PairsTradingBackTesting.hpp"
#include "Config.hpp"
#include "StatisticalAnalysis.hpp"

int main()
{
// Creating the vector containing the NYSE stock listings from text file
const std::string& NYSE_LISTINGS_FILE =  Config::getListingsFilePath();
std::vector<std::string> nyseListings = FileReader::readNYSEListings(NYSE_LISTINGS_FILE);

// Define the stock bin location
const std::string& STOCK_BIN_LOCATION = Config::getStockDataDir();
const std::string& TEXT_EXTENSION = ".txt";

// Define the start and end date
const std::string& START_DATE = "2014-11-23", END_DATE = "2024-11-23";

// Define the price type
const std::string& PRICE_TYPE = "adj close";

// Loading base stock
Stock stock2;
const std::string& stock2Symbol = "SPY";
FileReader::loadStockDataFromFile(Config::getStockDataDir() + stock2Symbol + TEXT_EXTENSION, stock2);
std::unordered_map<std::string, double> stock2Data = StockUtils::getPriceDataInRange(stock2, PRICE_TYPE, START_DATE, END_DATE);
std::unordered_map<std::string, double> stock2NormalizedData = StockAnalysis::normalizeToEarliestDate(stock2Data);


struct LNpairStatistic{
	std::string pair;
	std::string pairDateRange;
	double slope;
	double slopeError;
	int df;
	double testStatistic;
	double pValue;
	double rSquared;
	double lastDifference;
};

// Initialize map to store results
std::unordered_map<std::string, LNpairStatistic> pairResults;
 
for(const std::string& nyseListing : nyseListings) {
	// Loading comparison data
	Stock stock1;
	
	const std::string& stock1Symbol = nyseListing;

	std::cout << "-----------------------------\n" << stock1Symbol << "-" << stock2Symbol  << "\n";
	
	// Loading the data into stock object
	FileReader::loadStockDataFromFile(Config::getStockDataDir() + stock1Symbol + TEXT_EXTENSION, stock1);
	
	// Get the specific price type and date range data
	std::unordered_map<std::string, double> stock1Data = StockUtils::getPriceDataInRange(stock1, PRICE_TYPE, START_DATE, END_DATE);

	// Normalize the data
	std::unordered_map<std::string, double> stock1NormalizedData;
	try{
		stock1NormalizedData = StockAnalysis::normalizeToEarliestDate(stock1Data);
	
	} catch (std::exception& e){
		std::cout << e.what() << "\n";
		continue;
		
	}

	// Get the difference
	std::unordered_map<std::string, double> differenceOfNormalized = StockAnalysis::calculateDifferenceBetweenData(stock1NormalizedData, stock2NormalizedData);

	// Get the sorted version
	std::vector<std::pair<std::string, double>> sortedDifferenceOfNormalzied = StockAnalysis::sortMap(differenceOfNormalized);

	// Get the LN(difference)
	std::unordered_map<std::string, double> LnDifferenceOfNormalized 
		= StockAnalysis::applyMapOperation(differenceOfNormalized, [](double value) { return std::log(value); });

	// Perform linear regression get linear fit
	StatisticalAnalysis::LinearRegressionResult stats = StatisticalAnalysis::linearRegression(LnDifferenceOfNormalized);

	// Get the t-student p-value of the linear fit
	double linearFitPvalue;
	try {
		linearFitPvalue = StatisticalAnalysis::calculatePValue(
		stats.slope / stats.slopeError,
		LnDifferenceOfNormalized.size(),
		"t");
	} catch (std::exception& e) {
		std::cout << e.what() << "\n";
		continue;
	}

	LNpairStatistic currentPairStatistics = {
		stock1Symbol + "-" + stock2Symbol,
		START_DATE + "-" + END_DATE,
		stats.slope, 
		stats.slopeError,
		LnDifferenceOfNormalized.size(),
		stats.slope / stats.slopeError,
		linearFitPvalue,
		stats.rSquared,
		sortedDifferenceOfNormalzied.back().second
	};
	
	pairResults[stock1Symbol + "-" + stock2Symbol] = currentPairStatistics;
}

// Output all results into a file
std::ofstream outFile(Config::getOutputDir() + "LN_Difference_Pairs.txt");
outFile << "Pair,Date Range,Slope,Slope Error,Test Statistic,DF,p-value,R-squared,last difference\n";
for(auto element : functionOutput) {
	outFile << 	
	element.second.pair << "," <<
	element.second.pairDateRange << "," <<
	element.second.slope << "," <<
	element.second.slopeError << "," <<
	element.second.df << "," <<
	element.second.testStatistic << "," <<
	element.second.pValue << "," << 
	element.second.rSquared << "," << 
	element.second.lastDifference << 
	"\n";
	
}
outFile.close();


// std::ofstream outFile(Config::getOutputDir() + "LN_Difference_Dutput.txt");
// outFile << "Date,Difference\n";
// for(auto element : sortedLnDiff) {
	// outFile << element.first << "," << element.second << "\n";
	
// }
// outFile.close();


// // Output results
// std::cout << "Linear fit parameters:\n" << 
// "b1,b1 error,b0,b0 error\n" << 
// stats.slope << "," << stats.slopeError << "," <<
// stats.intercept << "," << stats.interceptError <<
// "\n";

return 0;
}
