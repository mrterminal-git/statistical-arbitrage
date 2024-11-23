#define _USE_MATH_DEFINES // for M_PI

#include "StandardNormalDistribution.hpp"

// Probability density function
double StandardNormalDistribution::pdf(const double& x) {
  return (1.0 / sqrt(2.0 * M_PI)) * exp(-0.5 * x * x);
}

// Cumulative density function
double StandardNormalDistribution::cdf(const double& x) {
  double k = 1.0 / (1.0 + 0.2316419 * x);
  double k_sum = k * (0.319381530 + k * (-0.356563782 + k * (1.781477937 + k * (-1.821255978 + 1.330274429 * k))));

  if (x >= 0.0) {
    return (1.0 - (1.0 / sqrt(2.0 * M_PI)) * exp(-0.5 * x * x) * k_sum);
  } else {
    return 1.0 - cdf(-x);
  }
}

// Inverse cumulative distribution function (aka the probit function)
double StandardNormalDistribution::inv_cdf(const double& quantile) {
  static double a[4] = { 2.50662823884, -18.61500062529, 41.39119773534, -25.44106049637 };
  static double b[4] = { -8.47351093090, 23.08336743743, -21.06224101826, 3.13082909833 };
  static double c[9] = { 0.3374754822726147, 0.9761690190917186, 0.1607979714918209, 0.0276438810333863,
                         0.0038405729373609, 0.0003951896511919, 0.0000321767881768, 0.0000002888167364,
                         0.0000003960315187 };

  if (quantile >= 0.5 && quantile <= 0.92) {
    double num = 0.0;
    double denom = 1.0;

    for (int i = 0; i < 4; i++) {
      num += a[i] * pow((quantile - 0.5), 2 * i + 1);
      denom += b[i] * pow((quantile - 0.5), 2 * i);
    }
    return num / denom;

  } else if (quantile > 0.92 && quantile < 1) {
    double num = 0.0;

    for (int i = 0; i < 9; i++) {
      num += c[i] * pow(log(-log(1 - quantile)), i);
    }
    return num;

  } else {
    return -1.0 * inv_cdf(1 - quantile);
  }
}

// Expectation/mean
double StandardNormalDistribution::mean() { return 0.0; }

// Variance 
double StandardNormalDistribution::var() { return 1.0; }

// Standard Deviation
double StandardNormalDistribution::stdev() { return 1.0; }

// Obtain a sequence of random draws from this distribution
void StandardNormalDistribution::random_draws(const std::vector<double>& uniform_draws,
                                              std::vector<double>& dist_draws) {
  if (uniform_draws.size() != dist_draws.size()) {
    std::cout << "Draws vectors are of unequal size in standard normal dist." << std::endl;
    return;
  }
  
  if (uniform_draws.size() % 2 != 0) {
    std::cout << "Uniform draw vector size not an even number." << std::endl;
    return;
  }

  for (size_t i = 0; i < uniform_draws.size() / 2; i++) {
    dist_draws[2 * i] = sqrt(-2.0 * log(uniform_draws[2 * i])) * sin(2 * 3.141592653589793 * uniform_draws[2 * i + 1]);
    dist_draws[2 * i + 1] = sqrt(-2.0 * log(uniform_draws[2 * i])) * cos(2 * 3.141592653589793 * uniform_draws[2 * i + 1]);
  }

  return;
}