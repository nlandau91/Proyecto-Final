#include "fingerprinttemplate.h"

using namespace fp;

FingerprintTemplate::FingerprintTemplate()
{

}

FingerprintTemplate::FingerprintTemplate(const std::string& file)
{
    deserialize(file);
}

bool FingerprintTemplate::serialize(const std::string& file) const
{
    cv::FileStorage fs( file, cv::FileStorage::WRITE );
    if( fs.isOpened() )
    {
        cv::write(fs,"descriptors",descriptors);
        cv::write(fs,"keypoints",keypoints);
        cv::write(fs,"singularities",singularities);
        cv::write(fs,"minutiaes",minutiaes);
        fs.release();
        return true;
    }
    return false;
}

bool FingerprintTemplate::deserialize(const std::string& file)
{
    cv::FileStorage fs( file, cv::FileStorage::READ );
    if( fs.isOpened() )
    {
        cv::FileNode n1 = fs["descriptors"];
        cv::read(n1,descriptors);
        cv::FileNode n2 = fs["keypoints"];
        cv::read(n2,keypoints);
        cv::FileNode n3 = fs["singularities"];
        cv::read(n3,singularities);
        cv::FileNode n4 = fs["minutiaes"];
        cv::read(n4,minutiaes);
        fs.release();

        return true;
    }
    return false;
}
