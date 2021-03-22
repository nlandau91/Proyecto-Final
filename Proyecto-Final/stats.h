#ifndef STATS_H
#define STATS_H
#include "common.h"
#include "utils.h"

namespace fp{

double get_mean(const std::vector<double> &scores, bool ignore_extremes = true);

double get_low_pcnt(std::vector<double> scores, double thresh);

double get_high_pcnt(std::vector<double> scores, double thresh);

double get_eer(std::vector<double> scores1, std::vector<double> scores2);
}

#endif // STATS_H
