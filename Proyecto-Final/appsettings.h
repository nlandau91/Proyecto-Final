#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "common.h"

class AppSettings
{
public:
    AppSettings();
    void load_settings();

    //preprocess
    int enhancement_method;
    int thinning_method;
    bool segment;
    int blk_size;
    float roi_threshold;

    //analyzer
    int minutiae_method;
    int keypoint_threshold;
    int descriptor_method;

    //comparator
    int matcher_method;
    int max_match_dist;

    bool draw_over_output;

};

#endif // APPSETTINGS_H
