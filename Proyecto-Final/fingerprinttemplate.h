#ifndef FINGERPRINTTEMPLATE_H
#define FINGERPRINTTEMPLATE_H

#include "common.h"

namespace fp
{
class FingerprintTemplate
{
public:
    FingerprintTemplate();
    FingerprintTemplate(const std::string& file);
    bool serialize(const std::string& file);
    bool deserialize(const std::string& file);

    cv::Mat descriptors;
    std::vector<cv::KeyPoint> keypoints;
    std::vector<cv::KeyPoint> singularities;
    std::vector<cv::KeyPoint> minutiaes;
};
}

#endif // FINGERPRINTTEMPLATE_H
