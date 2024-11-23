#ifndef STOCKUTILS_HPP
#define STOCKUTILS_HPP

#include "Common.hpp"
#include "Stock.hpp"

class StockUtils {
public:
    static std::unordered_map<std::string, double> getPriceData(const Stock& stock, const std::string& priceType);

    static std::unordered_map<std::string, double> getPriceDataInRange(
        const Stock& stock, const std::string& priceType, const std::string& startDate, const std::string& endDate
    );
};

#endif // STOCKUTILS_H