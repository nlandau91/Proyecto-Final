#include "common.h"
#include "utils.h"
#include <QDebug>

/*
* This is equivalent to Matlab's 'meshgrid' function
*/
void meshgrid(int kernelSize, cv::Mat &meshX, cv::Mat &meshY) {
    std::vector<int> t;

    for (int i = -kernelSize; i < kernelSize; i++) {
        t.push_back(i);
    }

    cv::Mat gv = cv::Mat(t);
    int total = gv.total();
    gv = gv.reshape(1, 1);

    cv::repeat(gv, total, 1, meshX);
    cv::Mat traspose = gv.t();
    cv::repeat(traspose, 1, total, meshY);
}

cv::Mat calculate_angles(cv::Mat im, int W)
{
    int x = im.cols;
    int y = im.rows;

    //ojo, quizas deberia pasar a CV_16UC1 y sacar el abs
    cv::Mat Gx_;
    cv::Sobel(im/125,Gx_,-1,1,0);
    Gx_ *=125;
    cv::Mat Gy_;
    cv::Sobel(im/125,Gy_,-1,0,1);
    Gy_ *= 125;
    cv::Mat result(y/W,x/W,CV_32FC1);
    for(int j = 1; j < y; j+=W)
    {
        for(int i = 1; i < x; i+=W)
        {
            int denominator = 0;
            int nominator = 0;

            for(int l = i; l < std::min(j + W, y - 1); l++)
            {
                for(int k = j; k < std::min(i + W, x - 1); k++)
                {
                    int Gx = std::round(Gx_.at<float>(l,k));
                    int Gy = std::round(Gy_.at<float>(l,k));
                    int j1 = 2 * Gx * Gy;
                    nominator += j1;
                    int j2 = Gx * Gx - Gy * Gy;
                    denominator += j2;
                }
            }
            //deberia redondear?
            if(nominator || denominator)
            {
                float angle = (M_PI + atan2(nominator,denominator))/2;
                //float orientation = M_PI/2 + atan2(nominator,denominator)/2;
                result.at<float>((j-1)/W,(i-1)/W) = angle;
            }
            else
            {
                result.at<float>((j-1)/W,(i-1)/W) = 0;
            }

        }
    }
    return result;
}

cv::Mat filter_ridge2(const cv::Mat &src,const cv::Mat &orientation_map,const cv::Mat &frequency_map, float kx = 0.5, float ky = 0.5)
{

    // Fixed angle increment between filter orientations in degrees
    // Deberia ser divisor de 180
    int angleInc = 3;

    cv::Mat im;
    src.convertTo(im, CV_32FC1);
    int rows = im.rows;
    int cols = im.cols;

    cv::Mat orient;
    orientation_map.convertTo(orient, CV_32FC1);
    cv::Mat freq;
    frequency_map.convertTo(freq, CV_32FC1);

    //find where there is valid frequency data.
    //redondeamos el arreglo de frecuencias al 0.01 mas cercano
    // cargamos un arreglo con los valores
    // y genero un arreglo de frecuencias unicas
    cv::Mat valid_points;
    cv::findNonZero(freq,valid_points);
    std::vector<int> validr;
    std::vector<int> validc;
    std::vector<float> ind;
    std::vector<float> unfreq;
    for(size_t i = 0; i < valid_points.total();i++)
    {
        int row = valid_points.at<cv::Point>(i).y;
        int col = valid_points.at<cv::Point>(i).x;
        validr.push_back(row);
        validc.push_back(col);
        float new_freq = round(100*freq.at<float>(row,col))/100;
        freq.at<float>(row,col) = new_freq;
        ind.push_back(new_freq);
        if(std::find(unfreq.begin(),unfreq.end(),new_freq) == unfreq.end())
        {
            unfreq.push_back(new_freq);
        }
    }
    sort(unfreq.begin(),unfreq.end());
    // Generate a table, given the frequency value multiplied by 100 to obtain
    // an integer index, returns the index within the unfreq array that it
    // corresponds to

    float freqindex[100];
    for(size_t i = 0; i < unfreq.size(); i++)
    {
        float f = unfreq[i];
        int index = round(f*100);
        freqindex[index] = i;
    }
    int debugnumber = 0;
    qDebug() << debugnumber++;

    //Generate filters corresponding to these distinct frequencies and
    //orientations in 'angleInc' increments.
    cv::Mat filter[unfreq.size()][180/angleInc];
    uchar sze[unfreq.size()];
    for(size_t i = 0; i < unfreq.size(); i++)
    {

        float sigmax = (1/unfreq[i]) * kx;
        float sigmay = (1/unfreq[i]) * ky;
        sze[i] = round(3*std::max(sigmax,sigmay));
        cv::Mat x, y;
        meshgrid(sze[i],x,y);

        cv::Mat term1, term2, term3;

        cv::pow(x,2,term1);
        term1 /= (sigmax*sigmax);

        cv::pow(y,2,term2);
        term2 /= (sigmay*sigmay);

        term3 += fp::mat_cos(2*M_PI*unfreq[i]*x);

        cv::Mat reffilter = cv::Mat::zeros(x.size(),CV_32FC1);
        cv::exp(-(term1 + term2)/2,reffilter);
        reffilter *= term3;

        // Generate rotated versions of the filter.  Note orientation
        // image provides orientation *along* the ridges, hence +90
        // degrees, and imrotate requires angles +ve anticlockwise, hence
        // the minus sign.
        for(int o = 0; o < 180/angleInc; o++)
        {
            //qDebug() << o;
            cv::Mat new_filter;
            float angle = -(o*angleInc+90);
            cv::Mat M = cv::getRotationMatrix2D(cv::Point((reffilter.cols-1)/2,(reffilter.rows-1)/2),angle,1);
            cv::warpAffine(reffilter,new_filter,M,reffilter.size());
            filter[i][o] = new_filter;
            //descomentar para guardar los filtros en disco
            cv::imwrite(std::to_string(i)+"_"+std::to_string(o)+"filter.jpg",new_filter*255);
        }

    }

    // Convert orientation matrix values from radians to an index value
    // that corresponds to round(degrees/angleInc)
    int maxOrientIndex = round(180/angleInc);
    cv::Mat orientindex = orient;
    orientindex /= M_PI;
    orientindex *= 180;
    orientindex /= angleInc;
    orientindex.convertTo(orientindex,CV_8UC1);
    for(int r = 0; r < orientindex.rows; r++)
    {
        for(int c = 0; c < orientindex.cols; c++)
        {

            if(orientindex.at<uchar>(r,c) < 1)
            {
                orientindex.at<uchar>(r,c) += maxOrientIndex;
            }
            if(orientindex.at<uchar>(r,c) >= maxOrientIndex)
            {
                orientindex.at<uchar>(r,c) -= maxOrientIndex;
            }
        }
    }

    //Find indices of matrix points greater than maxsze from the image boundary
    int maxsze = sze[0];
    std::vector<int> finalind;
    for(long unsigned i = 0; i < validr.size();i++)
    {
        if(validr[i] > maxsze && validr[i] < im.rows - maxsze
                && validc[i] > maxsze && validc[i] < im.cols - maxsze)
        {
            finalind.push_back(i);
        }
    }
    // Finally do the filtering
    cv::Mat newim(im.size(),im.type());
    for(long unsigned i = 0; i < finalind.size(); i++)
    {

        int r = validr[finalind[i]];
        int c = validc[finalind[i]];

        //find filter corresponding to freq(r,c)
        int filterindex = freqindex[(int)round(freq.at<float>(r,c)*100)];

        int s = sze[filterindex];

        cv::Rect roi(c-s,r-s,2*s,2*s);
        cv::Mat subim(im(roi));
        cv::Mat subFilter = filter[filterindex][orientindex.at<uchar>(r,c)];
        cv::Mat mulResult;
        cv::multiply(subim,subFilter,mulResult);
        ;
        if(cv::sum(mulResult)[0] > 0)
        {
            newim.at<float>(r,c) = 255;
        }
    }
    qDebug() << "done filter!";
    return newim;
}

///*!
// * \brief freqest estima la frecuencia de crestas en un pequenio bloque
// * \param im imagen a procesar
// * \param orientim mapa de orientaciones del bloque
// * \param windsze longitud de la ventana para identificar picos. Impar.
// * \param minWaveLength longitud de onda minima de las crestas
// * \param maxWaveLength longitud de onda maxima de las crestas
// * \return
// */
//cv::Mat freqest(cv::Mat &im, cv::Mat &orientim, int windsze = 5, int minWaveLength = 5, int maxWaveLength = 15)
//{
//    int rows = im.rows;
//    int cols = im.cols;
//    // Find mean orientation within the block. This is done by averaging the
//    // sines and cosines of the doubled angles before reconstructing the
//    // angle again.  This avoids wraparound problems at the origin.
//    cv::Mat orientimx2 = orientim*2;
//    cv::Scalar cosorient = cv::mean(fp::mat_cos(orientimx2));
//    cv::Scalar sinorient = cv::mean(fp::mat_sin(orientimx2));
//    double orient = atan2( sinorient[0], cosorient[0]) /2;

//    //rotamos el blque para que las crestas sean verticales
//    cv::Mat M = cv::getRotationMatrix2D(cv::Point((cols-1)/2.0,(rows-1)/2.0),(orient/M_PI)*180+90,1);
//    cv::Mat rotim;
//    cv::warpAffine(im,rotim,M,im.size());
//    //recortamos la imagen rotada para que no contenga regiones invalidas
//    //OJO ACA
//    int cropsze = trunc(rows/sqrt(2));
//    int offset = trunc((rows-cropsze)/2);
//    rotim = rotim(cv::Rect(offset,offset,cropsze,cropsze));


//}


///*!
// * \brief ridge_freq estima la frecuencia de crestas en una huella dactilar
// * \param im imagen a procesar
// * \param mask roi
// * \param orientim mapa de orientaciones
// * \param blksze bloque de tamano a usar
// * \param windsze longitud de la ventana para identificar picos. Impar.
// * \param minWaveLength longitud de onda minima de las crestas
// * \param maxWaveLength longitud de onda maxima de las crestas
// * \return
// */
//cv::Mat ridge_freq(cv::Mat &im, cv::Mat &mask, cv::Mat &orientim, int blksze = 32, int windsze = 5, int minWaveLength = 5, int maxWaveLength = 15 )
//{
//    int rows = im.rows;
//    int cols = im.cols;
//    cv::Mat freq = cv::Mat::zeros(im.size(),im.type());
//    for(int r = 1; r < blksze;rows-blksze)
//    {
//        for(int c = 1; c < blksze; cols-blksze)
//        {
//            cv::Rect roi(c,r,c+blksze-1,r+blksze-1);
//            cv::Mat blkim(im(roi));
//            cv::Mat blkor(orientim(roi));
//            freq(roi) = freqest(blkim,blkor,windsze,minWaveLength,maxWaveLength);
//        }
//    }
//    cv::bitwise_and(freq,mask,freq);
//    return freq;

//}

cv::Mat gabor2(cv::Mat &normalized)
{
    cv::Mat im = normalized.clone();
    //estimacion de la orientacion local
    cv::Mat orient_image = calculate_angles(im,16);
    float freq_val = 0.11;
    cv::Mat freq = cv::Mat::ones(im.rows, im.cols, im.type()) * freq_val;

    //filtro
    cv::Mat filtered = filter_ridge2(im, orient_image, freq);
    //devolvemos la imagen mejorada
    return filtered;
}

