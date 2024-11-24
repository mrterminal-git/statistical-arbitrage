#include "StatisticalAnalysis.hpp"

// Linear regression: Returns intercept, slope, and their standard errors
StatisticalAnalysis::LinearRegressionResult StatisticalAnalysis::linearRegression (
	const Eigen::VectorXd& x, 
	const Eigen::VectorXd& y) {
	int n = x.size();
	if (n != y.size()) {
		throw std::invalid_argument("x and y must have the same length");
	}

	// Design matrix with intercept term
	Eigen::MatrixXd X(n, 2);
	X.col(0) = Eigen::VectorXd::Ones(n);  // Intercept column
	X.col(1) = x;

	// Solve for coefficients: beta = (X^T X)^-1 X^T y
	Eigen::VectorXd beta = (X.transpose() * X).inverse() * X.transpose() * y;
	double b0 = beta(0);  // Intercept
	double b1 = beta(1);  // Slope

	// Calculate residuals and residual variance
	Eigen::VectorXd residuals = y - X * beta;
	double residualVariance = residuals.squaredNorm() / (n - 2);

	// Standard errors: sqrt(residualVariance * diag((X^T X)^-1))
	Eigen::MatrixXd XtX_inv = (X.transpose() * X).inverse();
	double se_b0 = std::sqrt(residualVariance * XtX_inv(0, 0));
	double se_b1 = std::sqrt(residualVariance * XtX_inv(1, 1));

    // Calculate R^2
    double totalVariance = (y.array() - y.mean()).matrix().squaredNorm();
    double rSquared = 1.0 - residuals.squaredNorm() / totalVariance;

	// Return results
	return {b0, b1, se_b0, se_b1, rSquared};
}

// Wrapper for linear regression
StatisticalAnalysis::LinearRegressionResult StatisticalAnalysis::linearRegression(
	const std::unordered_map<std::string, double>& data) {
	// Step 1: Convert unordered_map to a vector of pairs and sort by date
	std::vector<std::pair<std::string, double>> sortedData(data.begin(), data.end());
	std::sort(sortedData.begin(), sortedData.end(), [](const auto& a, const auto& b) {
		return a.first < b.first;  // Compare dates lexicographically
	});

	// Step 2: Convert sorted data into Eigen vectors
	int n = sortedData.size();
	Eigen::VectorXd x(n);
	Eigen::VectorXd y(n);
	for (int i = 0; i < n; ++i) {
		x[i] = i;  // Chronological index
		y[i] = sortedData[i].second;  // Corresponding value
	}

	// Step 3: Call the original linear regression function
	return linearRegression(x, y);
}

// Function to calculate p-value given a test statistic and degrees of freedom
double StatisticalAnalysis::calculatePValue(double testStatistic, int degreesOfFreedom, const std::string& testType) {
    if (testType == "t") {
        return tDistributionPValue(testStatistic, degreesOfFreedom);
    } else if (testType == "normal") {
        return normalDistributionPValue(testStatistic);
    } else {
        throw std::invalid_argument("Unsupported test type: " + testType);
    }
}

// Helper function to calculate the p-value for t-distribution
double StatisticalAnalysis::tDistributionPValue(double tStatistic, int degreesOfFreedom) {
    if (degreesOfFreedom <= 0) {
        throw std::invalid_argument("Degrees of freedom must be greater than 0 for t-distribution.");
    }

    boost::math::students_t tDist(degreesOfFreedom);
    double cdfValue = boost::math::cdf(tDist, tStatistic);

    // One-tailed p-value
    return  1.0 - cdfValue;
}

// Helper function to calculate the p-value for normal distribution
double StatisticalAnalysis::normalDistributionPValue(double zStatistic) {
    boost::math::normal normalDist;
    double cdfValue = boost::math::cdf(normalDist, zStatistic);

    // Two-tailed p-value
    return 2.0 * (1.0 - cdfValue);
}