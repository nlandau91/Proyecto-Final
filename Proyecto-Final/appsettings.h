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
    int blk_freq;
    int blk_segm;
    int blk_orient;
    float gabor_kx;
    float gabor_ky;

    //analyzer
    int minutiae_method;
    int keypoint_threshold;
    int descriptor_method;

    //comparator
    int matcher_method;
    double matcher_threshold;
    bool edge_matching;
    double edge_dist;
    double edge_angle;
    double triangle_min_edge;
    double triangle_max_edge;

    bool draw_over_output;

};

#endif // APPSETTINGS_H
