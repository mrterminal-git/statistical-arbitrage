#ifndef BACKTESTING_HPP
#define BACKTESTING_HPP

#include "Common.hpp"

class BackTesting {
public:
    // Constructor
    explicit BackTesting(double initialBalance);
	
	// Getters
	double getInitialBalance();
	double getCurrentBalance();
	int getTradeCount();

    // Virtual method for running back-tests
    virtual void run() = 0;

    // Performance metrics
    double calculateCumulativeReturn() const;
    double calculateSharpeRatio() const;
    double calculateMaximumDrawdown() const;

    // Log results
    virtual void logResults(const std::string& filename) const;

    virtual ~BackTesting() {}

protected:
    double initialBalance;
    double currentBalance;

	struct Trade {
		std::string entryDate;
		std::string exitDate;
		std::unordered_map<std::string, double> entryPrices; // Map of stock symbol -> entry price
		std::unordered_map<std::string, double> exitPrices;  // Map of stock symbol -> exit price
		double profitLoss;
	};
    std::vector<Trade> trades;

    // Helper function to record a trade
    void recordTrade(const Trade& trade);
};

#endif // BACKTESTING_HPP
