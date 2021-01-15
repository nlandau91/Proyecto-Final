#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "common.h"

class AppSettings
{
public:
    AppSettings();
    void load_settings();

    int enhancement_method;
    int thinning_method;
    bool segment;
    int keypoint_extractor;
    int keypoint_threshold;
    int feature_extractor;
    int matcher_method;
    int max_match_dist;
    bool draw_over_output;
    int blk_size;
};

#endif // APPSETTINGS_H
