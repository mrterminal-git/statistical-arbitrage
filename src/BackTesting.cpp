#include "BackTesting.hpp"

// Constructor
BackTesting::BackTesting(double initialBalance)
    : initialBalance(initialBalance), currentBalance(initialBalance) {}

// Getters
double BackTesting::getInitialBalance(){return initialBalance;}
double BackTesting::getCurrentBalance(){return currentBalance;}
int BackTesting::getTradeCount(){return trades.size();}

// Calculate cumulative return
double BackTesting::calculateCumulativeReturn() const {
    return (currentBalance - initialBalance) / initialBalance;
}

// Calculate Sharpe ratio
double BackTesting::calculateSharpeRatio() const {
    if (trades.empty()) return 0.0;

    double meanReturn = 0.0;
    double variance = 0.0;
    std::vector<double> returns;

    for (const auto& trade : trades) {
        double tradeReturn = trade.profitLoss / initialBalance;
        returns.push_back(tradeReturn);
        meanReturn += tradeReturn;
    }
    meanReturn /= trades.size();

    for (const auto& r : returns) {
        variance += (r - meanReturn) * (r - meanReturn);
    }
    double stddev = std::sqrt(variance / trades.size());

    return stddev == 0.0 ? 0.0 : meanReturn / stddev;
}

// Calculate maximum drawdown
double BackTesting::calculateMaximumDrawdown() const {
    double maxBalance = initialBalance;
    double runningBalance = initialBalance;  // Track balance incrementally
    double maxDrawdown = 0.0;

    for (const auto& trade : trades) {
        runningBalance += trade.profitLoss;  // Update balance with each trade
        if (runningBalance > maxBalance) maxBalance = runningBalance;
        double drawdown = (maxBalance - runningBalance) / maxBalance;
        if (drawdown > maxDrawdown) maxDrawdown = drawdown;
    }

    return maxDrawdown;
}

// Record a trade
void BackTesting::recordTrade(const Trade& trade) {
    trades.push_back(trade);
    currentBalance += trade.profitLoss;
}

// Log results
void BackTesting::logResults(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << "\n";
        return;
    }
	
	// Write the header of the file
    file << "EntryDate,ExitDate";
	for(auto element : trades.at(0).entryPrices){
		file << "," << element.first << " Entry Price";
	}
	for(auto element : trades.at(0).exitPrices){
		file << "," << element.first << " Exit Price";
	}
	file << ",ProfitLoss\n";
	
	// Write the trades information into the file
    for (const auto& trade : trades) {
        file << trade.entryDate << "," << trade.exitDate << ",";
		for(auto element : trade.entryPrices){
			file << element.second << ",";
		}
		for(auto element : trade.exitPrices){
			file << element.second << ",";
		}
		file << trade.profitLoss << "\n";
    }
	file << "\n";
}
