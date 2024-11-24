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

// Define benchmark stock
const std::string& BENCHMARK_STOCK = "SPY";

// Compare against function version and output
auto functionOutput = PairsTradingBackTesting::selectPairsForBackTesting(
	nyseListings,
	Config::getStockDataDir(),
	TEXT_EXTENSION,
	BENCHMARK_STOCK,
	PRICE_TYPE,
	START_DATE,
	END_DATE
);	

std::ofstream outFile(Config::getOutputDir() + "LN_Difference_Pairs_2.txt");
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
