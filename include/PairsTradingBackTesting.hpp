#ifndef PAIRSTRADINGBACKTESTING_HPP
#define PAIRSTRADINGBACKTESTING_HPP

#include "BackTesting.hpp"
#include "StockAnalysis.hpp"
#include "FileReader.hpp"
#include "StockUtils.hpp"
#include "StandardNormalDistribution.hpp"
#include "Config.hpp"

class PairsTradingBackTesting : public BackTesting {
public:
	struct PairStatistics {
		double mean;
		double standardDeviation;
		double pValue;
		double adfPValueAIC;
		double adfPValueBIC;
		double ppPValueShortRho;
		double ppPValueLongRho;
		double ppPValueShortTau;
		double ppPValueLongTau;
		double kpssPValueShort;
		double kpssPValueLong;
	};
	
    struct BackTestingConfig {
        double entryThreshold = 0.4;
        double exitThreshold = 0.2;
        double tradeAmount = 1000.0;
        double slippage = 0.0;
    };

    PairsTradingBackTesting(const std::string& stock1Name,
                            const std::unordered_map<std::string, double>& stock1Data,
                            const std::string& stock2Name,
                            const std::unordered_map<std::string, double>& stock2Data,
                            const std::unordered_map<std::string, double>& pairSelectionData1,
                            const std::unordered_map<std::string, double>& pairSelectionData2,
                            double initialBalance);

    // Run the back-test
	void run() override;
	void run(const BackTestingConfig& config);

	static std::map<std::string, PairStatistics> selectPairsForBackTesting(
		std::vector<std::string> stockListings,
		const std::string& stockDataDir,
		const std::string& fileExtension,
		const std::string& priceType,
		const std::string& startDate,
		const std::string& endDate,
		double priceVolumeThreshold);

	// Override the logResults functions
	void logResults(const std::string& filename) const override; // Override logResults

private:
    std::string stock1Name;
    std::string stock2Name;
    std::unordered_map<std::string, double> stock1Data;
    std::unordered_map<std::string, double> stock2Data;
    std::unordered_map<std::string, double> normalizedStock1Data;
    std::unordered_map<std::string, double> normalizedStock2Data;
    std::unordered_map<std::string, double> pairSelectionData1;
    std::unordered_map<std::string, double> pairSelectionData2;
    
	// Helper functions
    double calculateSpread(const std::string& date) const;
	double calculateProfitLoss(const std::string& entryDate,
						   const std::string& exitDate,
						   const double& tradeAmount) const;
    bool isEntrySignal(double spread, double threshold) const;
    bool isExitSignal(double spread, double threshold) const;
};

#endif // PAIRSTRADINGBACKTESTING_HPP
