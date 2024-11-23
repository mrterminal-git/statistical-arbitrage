#ifndef STANDARDNORMALDISTRIBUTION_HPP
#define STANDARDNORMALDISTRIBUTION_HPP

#include "Common.hpp"

class StandardNormalDistribution{
 public:
  static double pdf(const double& x);
  static double cdf(const double& x);

  // Inverse cumulative distribution function (aka the probit function)
  static double inv_cdf(const double& quantile);
  
  // Descriptive stats
  static double mean();   // equal to 0
  static double var();    // equal to 1 
  static double stdev();  // equal to 1

  // Obtain a sequence of random draws from the standard normal distribution
  static void random_draws(const std::vector<double>& uniform_draws,
                           std::vector<double>& dist_draws);
};

#endif // STANDARDNORMALDISTRIBUTION_HPP