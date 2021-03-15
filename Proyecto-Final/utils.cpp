#include "utils.h"
#include <QDebug>

namespace fp
{

//convierte una imagen de tipo cv::Mat a una de tipo QImage
QImage cvMatToQImage( const cv::Mat &inMat )
{
    switch ( inMat.type() )
    {
    // 8-bit, 4 channel
    case CV_8UC4:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB32 );

        return image;
    }

        // 8-bit, 3 channel
    case CV_8UC3:
    {
        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_RGB888 );

        return image.rgbSwapped();
    }

        // 8-bit, 1 channel
    case CV_8UC1:
    {
        static QVector<QRgb>  sColorTable;

        // only create our color table once

        QImage image( inMat.data, inMat.cols, inMat.rows, inMat.step, QImage::Format_Indexed8 );
        return image;
    }

    default:
    {
        break;
    }

    }

    return QImage();
}

//convierte una imagen de tipo cv::Mat a una de tipo QPixmap
QPixmap cvMatToQPixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cvMatToQImage( inMat ) );
}

cv::Mat draw_keypoints( const cv::Mat &src, const std::vector<cv::KeyPoint> &keypoints)
{
    cv::Mat drawed = src.clone();
    for(const cv::KeyPoint &kp : keypoints)
    {
        //ridge ending
        if(kp.class_id == ENDING)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(255,0,0,255),cv::MARKER_CROSS,5);
        }
        else
        {
            //ridge bifurcation
            if(kp.class_id == BIFURCATION)
            {
                cv::drawMarker(drawed,kp.pt,cv::Scalar(0,0,255,255),cv::MARKER_CROSS,5);
            }
            else
            {
                cv::drawMarker(drawed,kp.pt,cv::Scalar(50,200,50,255),cv::MARKER_DIAMOND,5);
            }
        }

    }
    return drawed;
}

cv::Mat draw_singularities( const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities)
{
    cv::Mat drawed = src.clone();
    for(const cv::KeyPoint &kp : singularities)
    {
        if(kp.class_id == LOOP) // o core
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(255,0,0,255),cv::MARKER_SQUARE,10);
        }
        if(kp.class_id == WHORL)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(0,255,0,255),cv::MARKER_SQUARE,10);
        }
        if(kp.class_id == DELTA)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(0,0,255,255),cv::MARKER_SQUARE,10);
        }
    }
    return drawed;
}

cv::Mat mat_cos( const cv::Mat &src)
{
    cv::Mat cos_mat = cv::Mat::zeros(src.size(),CV_32FC1);
    for(int r = 0; r < src.rows; r++)
    {
        for(int c = 0; c < src.cols; c++)
        {

            cos_mat.at<float>(r,c) = cos(src.at<float>(r,c));
        }
    }
    return cos_mat;
}
cv::Mat mat_sin( const cv::Mat &src)
{
    cv::Mat sin_mat = cv::Mat::zeros(src.size(),CV_32FC1);
    for(int r = 0; r < src.rows; r++)
    {
        for(int c = 0; c < src.cols; c++)
        {
            sin_mat.at<float>(r,c) = sin(src.at<float>(r,c));
        }
    }
    return sin_mat;
}
cv::Mat mat_atan2( const cv::Mat &src1, const cv::Mat &src2)
{
    cv::Mat atan2_mat = cv::Mat::zeros(src1.size(),CV_32FC1);
    for(int r = 0; r < src1.rows; r++)
    {
        for(int c = 0; c < src1.cols; c++)
        {
            atan2_mat.at<float>(r,c) = atan2(src1.at<float>(r,c),src2.at<float>(r,c));
        }
    }
    return atan2_mat;
}

std::vector<float> unique(const cv::Mat& input, bool sort)
{
    if (input.channels() > 1 || input.type() != CV_32F)
    {
        qDebug() << "unique !!! Only works with CV_32F 1-channel Mat";
        return std::vector<float>();
    }

    std::vector<float> out;
    for(int y = 0; y < input.rows; ++y)
    {
        const float* row_ptr = input.ptr<float>(y);
        for(int x = 0; x < input.cols; ++x)
        {
            float value = row_ptr[x];

            if ( std::find(out.begin(), out.end(), value) == out.end() )
                out.push_back(value);
        }
    }

    if (sort)
        std::sort(out.begin(), out.end());

    return out;
}

cv::Mat visualize_angles( const cv::Mat &im, const cv::Mat &angles)
{
    int y = im.rows;
    int x = im.cols;
    int W = trunc(im.rows / angles.rows);
    cv::Mat result = cv::Mat::zeros(im.size(),CV_8UC1);
    cv::cvtColor(result,result,cv::COLOR_GRAY2BGR);
    for(int i = 1; i < x; i+=W)
    {
        for(int j = 1; j < y; j+=W)
        {
            float tang = tan(angles.at<float>(trunc((j-1)/W),trunc((i-1)/W)));
            cv::Point begin,end;
            if(-1 <=tang && tang <= 1)
            {
                begin = cv::Point(i,round((-W/2) * tang + j + W/2));
                end = cv::Point(i+W,round((W/2) * tang + j + W/2));
            }
            else
            {
                begin = cv::Point(round(i + W/2 + W/(2 * tang)), j + W/2);
                end = cv::Point(round(i + W/2 - W/(2 * tang)), j - W/2);
            }

            cv::line(result,begin,end,cv::Scalar(255,255,255,255));
        }
    }
    return result;
}

cv::Mat apply_mask(const cv::Mat &src, const cv::Mat &mask)
{
    cv::Mat masked = cv::Mat::zeros(src.size(),src.type());
    bitwise_and(src,src,masked,mask);
    return masked;
}

cv::Mat translate_mat(const cv::Mat &img, int offsetx, int offsety)
{
    cv::Mat translated = cv::Mat::zeros(img.size(),img.type());
    cv::Mat trans_mat = (cv::Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    cv::warpAffine(img,translated,trans_mat,img.size());
    return translated;
}

bool serialize_mat(const std::string& file, const cv::Mat& mat, const std::string &name)
{
    cv::FileStorage fs( file, cv::FileStorage::WRITE );
    if( fs.isOpened() )
    {
        fs << name << mat;
        return true;
    }
    return false;
}

bool read_serialized_mat(const std::string& file, cv::Mat& mat, const std::string &name)
{
    cv::FileStorage fs( file, cv::FileStorage::READ );
    if( fs.isOpened() )
    {
        fs[name] >> mat;
        return true;
    }
    return false;
}

}
