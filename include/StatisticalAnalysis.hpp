#ifndef STATISTICAL_ANALYSIS_H
#define STATISTICAL_ANALYSIS_H

#include <Eigen/Dense>
#include "Common.hpp"

class StatisticalAnalysis {
public:
    // Constructor
    StatisticalAnalysis() = default;

    // Destructor
    ~StatisticalAnalysis() = default;

    // Struct for Linear Regression Results
    struct LinearRegressionResult {
        double intercept;      // b0
        double slope;          // b1
        double interceptError; // SE of b0
        double slopeError;     // SE of b1
        double rSquared;       // Coefficient of determination (R^2)
    };

    // Linear regression: Returns intercept, slope, and their standard errors
    static LinearRegressionResult 
    linearRegression(const Eigen::VectorXd& x, const Eigen::VectorXd& y);

	// Wrapper/s for linear regression function
	static LinearRegressionResult 
	linearRegression(const std::unordered_map<std::string, double>& data);

    // Hypothesis testing for regression coefficients
    static bool hypothesisTest(double estimatedValue, double standardError, double nullValue, double alpha = 0.05);

    // Confidence intervals for regression parameters
    static std::pair<double, double> confidenceInterval(double estimate, double standardError, double alpha = 0.05);

    // Calculation of summary statistics
    static double mean(const Eigen::VectorXd& data);
    static double variance(const Eigen::VectorXd& data);
    static double standardDeviation(const Eigen::VectorXd& data);
    static double median(const Eigen::VectorXd& data);

    // Covariance and correlation
    static double covariance(const Eigen::VectorXd& x, const Eigen::VectorXd& y);
    static double correlation(const Eigen::VectorXd& x, const Eigen::VectorXd& y);

    // Goodness-of-fit test (e.g., Chi-Square)
    static double chiSquareGoodnessOfFit(const Eigen::VectorXd& observed, const Eigen::VectorXd& expected);

    // One-sample t-test
    static double tTestOneSample(const Eigen::VectorXd& data, double populationMean, double alpha = 0.05);

    // Two-sample t-test (independent samples)
    static double tTestTwoSample(const Eigen::VectorXd& sample1, const Eigen::VectorXd& sample2, double alpha = 0.05);

    // Analysis of Variance (ANOVA)
    static double singleFactorANOVA(const std::vector<Eigen::VectorXd>& groups);

    // Probability distributions
    static Eigen::VectorXd generateNormalDistribution(int n, double mean = 0.0, double stddev = 1.0);
    static Eigen::VectorXd generateBinomialDistribution(int n, int trials, double probability);
    static Eigen::VectorXd generatePoissonDistribution(int n, double lambda);

    // Simulation: Resampling methods (e.g., bootstrapping)
    static std::vector<Eigen::VectorXd> bootstrapSamples(const Eigen::VectorXd& data, int numSamples);

    // Empirical CDF
    static Eigen::VectorXd empiricalCDF(const Eigen::VectorXd& data);

    // Quantile-Quantile Plot Data
    static std::pair<Eigen::VectorXd, Eigen::VectorXd> qqPlotData(const Eigen::VectorXd& data, const std::string& distribution = "normal");

    // Export data for visualization
    static void exportDataForPlot(const Eigen::VectorXd& x, const Eigen::VectorXd& y, const std::string& fileName);

	// Function to calculate p-value given a test statistic and degrees of freedom
    static double calculatePValue(double testStatistic, int degreesOfFreedom, const std::string& testType);

private:
    // Internal helper functions
    static double calculatePValue(double tStatistic, int degreesOfFreedom);
    static double fStatistic(const std::vector<Eigen::VectorXd>& groups);
    static double tDistributionPValue(double tStatistic, int degreesOfFreedom);
    static double normalDistributionPValue(double zStatistic);
	
};

#endif // STATISTICAL_ANALYSIS_H
