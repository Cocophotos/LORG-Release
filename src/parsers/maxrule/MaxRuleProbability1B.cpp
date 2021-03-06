// // -*- mode: c++ -*-
#ifndef _MAXRULEPROBABILITY_CPP_
#define _MAXRULEPROBABILITY_CPP_

#include "MaxRuleProbability1B.h"

#include <numeric>

double MaxRuleProbability1B::log_normalisation_factor = 0;

void MaxRuleProbability1B::set_log_normalisation_factor(double lnf)
{
  log_normalisation_factor = lnf;
}


template<>
ParserCKYAllFactory::MaxParsing_Calculation
MaxRuleTreeLogProbaComputer<MaxRuleProbability1B>::calculation = ParserCKYAllFactory::Product;

#endif /* _MAXRULEPROBABILITY_H_ */
