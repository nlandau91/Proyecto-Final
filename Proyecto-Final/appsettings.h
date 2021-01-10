#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "common.h"

class AppSettings
{
public:
    AppSettings();
    void load_settings();

    fp::Preprocesser::EnhancementMethod enhancement_method;
    fp::Preprocesser::ThinningMethod thinning_method;
    bool masking;
    fp::Params keypoint_extractor;
    int keypoint_threshold;
    fp::Analyzer::FeatureExtractor feature_extractor;
    fp::Analyzer::MatcherMethod matcher_method;
    int max_match_dist;
    bool draw_over_output;
};

#endif // APPSETTINGS_H
