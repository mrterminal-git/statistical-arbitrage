#include "Common.hpp"
#include "FileReader.hpp"
#include "Stock.hpp"
#include "StockUtils.hpp"
#include "StockAnalysis.hpp"
#include "StandardNormalDistribution.hpp"
#include "PairsTradingBackTesting.hpp"
#include "Config.hpp"

int main()
{
// Creating the vector containing the NYSE stock listings from text file
const std::string& NYSE_LISTINGS_FILE =  Config::getListingsFilePath();
std::vector<std::string> nyseListings = FileReader::readNYSEListings(NYSE_LISTINGS_FILE);

// Define the stock bin location
const std::string& STOCK_BIN_LOCATION = Config::getStockDataDir();
const std::string& TEXT_EXTENSION = ".txt";

// Define the start and end date
const std::string& START_DATE = "2022-11-01", END_DATE = "2023-11-01";

// Define the price type
const std::string& PRICE_TYPE = "adj close";

// Define the price-volume threshold for liquidity
const double PRICE_VOLUME_THRESH = 500000;

std::map<std::string, PairsTradingBackTesting::PairStatistics> pairs = PairsTradingBackTesting::selectPairsForBackTesting(
    nyseListings, STOCK_BIN_LOCATION, TEXT_EXTENSION, PRICE_TYPE, START_DATE, END_DATE, PRICE_VOLUME_THRESH);

std::ofstream outFile(Config::getOutputDir() + "Pairs_Backtesting_Results.txt");
outFile << "Pair,Mean,Standard Deviation,P-value,ADF P-value AIC,ADF P-value BIC,PP Short Rho,PP Long Rho,PP Short Tau,PP Long Tau,KPSS Short,KPSS Long," <<
"Rate Return,Trade Count,Max Drawdown,Sharpe Ratio\n";

// Back-test for all stocks and record the distribution of the percentage change in portfolio
const std::string& BACK_TEST_START_DATE = "2023-11-02";
const std::string& BACK_TEST_END_DATE = "2024-05-02";
for (const auto& [pair, stats] : pairs) {
    try {
        size_t delimiterPos = pair.find("-");
        std::string stock1Name = pair.substr(0, delimiterPos);
        std::string stock2Name = pair.substr(delimiterPos + 1);

        Stock stock1, stock2;
        FileReader::loadStockDataFromFile(STOCK_BIN_LOCATION + stock1Name + TEXT_EXTENSION, stock1);
        FileReader::loadStockDataFromFile(STOCK_BIN_LOCATION + stock2Name + TEXT_EXTENSION, stock2);
		
        auto stock1Data = StockUtils::getPriceDataInRange(stock1, PRICE_TYPE, BACK_TEST_START_DATE, BACK_TEST_END_DATE);
        auto stock2Data = StockUtils::getPriceDataInRange(stock2, PRICE_TYPE, BACK_TEST_START_DATE, BACK_TEST_END_DATE);
		
		auto pairSelectionData1 = StockUtils::getPriceDataInRange(stock1, PRICE_TYPE, START_DATE, END_DATE);
		auto pairSelectionData2 = StockUtils::getPriceDataInRange(stock2, PRICE_TYPE, START_DATE, END_DATE);

		PairsTradingBackTesting backTesting(stock1Name, stock1Data, stock2Name, stock2Data, pairSelectionData1, pairSelectionData2, 1000);

        PairsTradingBackTesting::BackTestingConfig config;
        config.entryThreshold = 2.0 * stats.standardDeviation;
        config.exitThreshold = 1.5 * stats.standardDeviation;
        config.tradeAmount = 100.0;
		config.slippage = 0;

        backTesting.run(config);

        double rateReturn = 100.0 / backTesting.getInitialBalance() * (backTesting.getCurrentBalance() - backTesting.getInitialBalance()); // Convert to %
        double sharpeRatio = backTesting.calculateSharpeRatio();
        double maxDrawdown = backTesting.calculateMaximumDrawdown();

        outFile << pair << ","
                << stats.mean << ","
                << stats.standardDeviation << ","
                << stats.pValue << ","
                << stats.adfPValueAIC << ","
                << stats.adfPValueBIC << ","
                << stats.ppPValueShortRho << ","
                << stats.ppPValueLongRho << ","
                << stats.ppPValueShortTau << ","
                << stats.ppPValueLongTau << ","
                << stats.kpssPValueShort << ","
                << stats.kpssPValueLong << ","
                << rateReturn << ","
                << backTesting.getTradeCount() << ","
                << maxDrawdown << ","
                << sharpeRatio << "\n";

		// Output all trades
		backTesting.logResults(Config::getOutputDir() + "Pairs_Backtesting_Results_Trades.txt");

    } catch (const std::exception& e) {
        std::cerr << "Error processing pair " << pair << ": " << e.what() << "\n";
    }
}
outFile.close();

return 0;
}
