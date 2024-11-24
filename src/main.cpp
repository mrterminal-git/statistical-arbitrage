#include "Common.hpp"
#include "FileReader.hpp"
#include "Stock.hpp"
#include "StockUtils.hpp"
#include "StockAnalysis.hpp"
#include "StandardNormalDistribution.hpp"
#include "PairsTradingBackTesting.hpp"
#include "Config.hpp"
#include "StatisticalAnalysis.hpp"

void processPairsForDateRange(
    const std::vector<std::string>& nyseListings,
    const std::string& benchmarkStock,
    const std::string& priceType,
    const std::string& startDate,
    const std::string& endDate,
    const std::string& filename
) {
    // Get pairs statistics for the given date range
    auto functionOutput = PairsTradingBackTesting::selectPairsForBackTesting(
        nyseListings,
        Config::getStockDataDir(),
        ".txt",
        benchmarkStock,
        priceType,
        startDate,
        endDate
    );

    // Check if the file already exists
    bool fileExists = std::filesystem::exists(filename);

    // Open the file in append mode
    std::ofstream outFile(filename, std::ios_base::app);
    if (!outFile.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << "\n";
        return;
    }

    // Write the header only if the file is newly created
    if (!fileExists) {
        outFile << "Pair,Date Range,Slope,Slope Error,Test Statistic,DF,p-value,R-squared,Last Difference\n";
    }

    // Append the data
    for (const auto& element : functionOutput) {
        outFile << element.second.pair << ","
                << element.second.pairDateRange << ","
                << element.second.slope << ","
                << element.second.slopeError << ","
                << element.second.df << ","
                << element.second.testStatistic << ","
                << element.second.pValue << ","
                << element.second.rSquared << ","
                << element.second.lastDifference << "\n";
    }

    outFile.close();
}

int main() {
    // Create the vector containing the NYSE stock listings from the text file
    const std::string& nyseListingsFile = Config::getListingsFilePath();
    std::vector<std::string> nyseListings = FileReader::readNYSEListings(nyseListingsFile);

    // Define benchmark stock and price type
    const std::string& benchmarkStock = "SPY";
    const std::string& priceType = "adj close";

    // Define output file
    const std::string& filename = Config::getOutputDir() + "LN_Difference_Pairs.txt";

    // Process different date ranges
    std::vector<std::pair<std::string, std::string>> dateRanges = {
        {"2014-11-23", "2024-11-23"},
        {"2019-11-23", "2024-11-23"},
        {"2021-11-23", "2024-11-23"},
        {"2022-11-23", "2024-11-23"},
        {"2023-11-23", "2024-11-23"},
		{"2024-05-23", "2024-11-23"},
		{"2024-08-23", "2024-11-23"},
		{"2024-09-23", "2024-11-23"},
		{"2024-10-23", "2024-11-23"}
    };

    for (const auto& [startDate, endDate] : dateRanges) {
        processPairsForDateRange(nyseListings, benchmarkStock, priceType, startDate, endDate, filename);
    }

    return 0;
}
