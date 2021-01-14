#include "utils.h"

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

cv::Mat draw_minutiae( const cv::Mat &src, const std::vector<cv::KeyPoint> &mintiaes)
{
    cv::Mat drawed = src.clone();
    for(cv::KeyPoint kp : mintiaes)
    {
        //ridge ending
        if(kp.size == 1)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(255,255,0,255),cv::MARKER_CROSS,3);
        }
        //ridge bifurcation
        if(kp.size == 3)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(0,255,255,255),cv::MARKER_CROSS,3);
        }

    }
    return drawed;
}

cv::Mat draw_singularities( const cv::Mat &src, const std::vector<cv::KeyPoint> &singularities)
{
    cv::Mat drawed = src.clone();
    for(cv::KeyPoint kp : singularities)
    {
        if(kp.size == LOOP)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(255,0,0,255),cv::MARKER_SQUARE);
        }
        if(kp.size == WHORL)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(0,255,0,255),cv::MARKER_SQUARE);
        }
        if(kp.size == DELTA)
        {
            cv::drawMarker(drawed,kp.pt,cv::Scalar(0,0,255,255),cv::MARKER_SQUARE);
        }
    }
    return drawed;
}

}
