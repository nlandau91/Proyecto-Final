#include "utils.h"
#include "appsettings.h"
#include <opencv2/imgproc.hpp>
#include <iostream>
/*!
 * El namespace fp contiene todas las clases, structs, enums y funciones
 * que necesita el programa para manejar las huellas dactilares
 */
namespace fp{

QImage cv_mat_to_qimage(const cv::Mat &inMat)
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

QPixmap cv_mat_to_qpixmap( const cv::Mat &inMat )
{
    return QPixmap::fromImage( cv_mat_to_qimage( inMat ) );
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

cv::Mat visualize_angles( const cv::Mat &im, const cv::Mat &angles)
{
    int y = im.rows;
    int x = im.cols;
    int W = trunc(im.rows / angles.rows);
    cv::Mat result = cv::Mat::zeros(im.size(),CV_8UC1);
    //cv::cvtColor(result,result,cv::COLOR_GRAY2BGR);
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

            cv::line(result,begin,end,cv::Scalar(255));
        }
    }
    return result;
}

cv::Mat visualize_frequencies(const cv::Mat &im, const cv::Mat &frequencies)
{
    int W = trunc(im.rows / frequencies.rows);
    cv::Mat result = cv::Mat::zeros(im.size(),CV_8UC1);
    for(int j = 0; j < im.rows-W; j+=W)
    {
        for(int i = 0; i < im.cols-W; i+=W)
        {
            float freq = frequencies.at<float>(trunc(j/W),trunc(i/W));
            cv::Rect blk(i,j,W,W);
            result(blk) = (unsigned int)(freq*255);
        }
    }
    return result;
}



}
