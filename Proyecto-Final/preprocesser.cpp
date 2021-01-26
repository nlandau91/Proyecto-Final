#include "preprocesser.h"
#include <QDebug>

namespace fp
{

cv::Mat morphological_thinning(const cv::Mat &src)
{

    cv::Mat bin;
    cv::threshold(src,bin,127,255,cv::THRESH_BINARY);
    cv::Mat skel(src.size(), CV_8UC1,cv::Scalar(0));
    cv::Mat temp;
    cv::Mat eroded;
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,cv::Size(3,3));
    bool done;
    do
    {
        cv::erode(bin, eroded, element);
        cv::dilate(eroded, temp, element); // temp = open(img)
        cv::subtract(bin, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        eroded.copyTo(bin);

        done = (cv::countNonZero(bin) == 0);
    } while (!done);
    return skel;

}

cv::Mat Preprocesser::normalize(const cv::Mat &src, float req_mean, float req_var, const cv::_InputOutputArray &mask)
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev,mask);
    cv::Mat normalized_im(src.size(),CV_32FC1);
    normalized_im = src - mean[0];
    normalized_im = normalized_im / stddev[0];
    normalized_im = req_mean + normalized_im * std::sqrt(req_var);
    return normalized_im;
}


//This is equivalent to Matlab's 'meshgrid' function
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

//calcula el mapa de orientacion de la imagen
//devuelve valores entre 0 y pi.
cv::Mat calculate_angles(const cv::Mat &im, int W, bool smooth = true)
{
    int x = im.cols;
    int y = im.rows;


    cv::Mat Gx_;
    //cv::Sobel(im/125,Gx_,-1,1,0);
    cv::Sobel(im,Gx_,-1,1,0);
    //Gx_ *=125;
    cv::Mat Gy_;
    //cv::Sobel(im/125,Gy_,-1,0,1);
    //Gy_ *= 125;
    cv::Sobel(im,Gy_,-1,0,1);

    cv::Mat result = cv::Mat::zeros(trunc(y/W),trunc(x/W),CV_32FC1);
    cv::Mat sines = result.clone();
    cv::Mat cosines = result.clone();
    for(int j = 0; j < y - W; j+=W)
    {
        for(int i = 0; i < x - W; i+=W)
        {
            int denominator = 0;
            int nominator = 0;

            for(int l = j; l < std::min(j + W, y - 1); l++)
            {
                for(int k = i; k < std::min(i + W, x - 1); k++)
                {
                    int Gx = std::round(Gx_.at<float>(l,k));
                    int Gy = std::round(Gy_.at<float>(l,k));
                    int j1 = 2 * Gx * Gy;
                    nominator += j1;
                    int j2 = Gx * Gx - Gy * Gy;
                    denominator += j2;
                }
            }
            cv::Point p(trunc(i/W),trunc(j/W));
            cosines.at<float>(p) = denominator;
            sines.at<float>(p) = nominator;
        }
    }

    //filtro pasabajo al campo direccional
    if(smooth)
    {
        int ksize = 3;
        float sigma = 1.0;
        cv::Mat kernel = cv::getGaussianKernel(ksize,sigma);
        cv::filter2D(cosines, cosines, -1, kernel);
        cv::filter2D(sines, sines, -1, kernel);
    }
    //finalmente armamos el mapa de angulos
    for (int i = 0; i < sines.rows; i++)
    {
        const float *sines_i = sines.ptr<float>(i);
        const float *cosines_i = cosines.ptr<float>(i);
        auto *result_i = result.ptr<float>(i);
        for (int j = 0; j < sines.cols; j++)
        {
            result_i[j] = (M_PI + std::atan2(sines_i[j], cosines_i[j])) / 2;
            qDebug() << result_i[j];
        }
    }
    return result;

}

cv::Mat filter_ridge(const cv::Mat &src,const cv::Mat &orientation_map,const cv::Mat &frequency_map, float kx = 0.5, float ky = 0.5)
{

    // Fixed angle increment between filter orientations in degrees
    // Deberia ser divisor de 180
    int angleInc = 3;

    cv::Mat im;
    src.convertTo(im, CV_32FC1);
    int blk_sze = im.rows/orientation_map.rows;

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

    //Generate filters corresponding to these distinct frequencies and
    //orientations in 'angleInc' increments.
    cv::Mat filter[unfreq.size()][180/angleInc];
    uchar sze[unfreq.size()];
    for(size_t i = 0; i < unfreq.size(); i++)
    {
        qDebug() << "armando reffilter...";
        float sigmax = (1.0/unfreq[i]) * kx;
        float sigmay = (1.0/unfreq[i]) * ky;
        sze[i] = round(3.0*std::max(sigmax,sigmay));
        cv::Mat x, y;
        meshgrid(sze[i],x,y);
        x.convertTo(x,CV_32FC1);
        y.convertTo(y,CV_32FC1);

        cv::Mat term1(x.size(),CV_32FC1);

        cv::Mat term2(x.size(),CV_32FC1);

        cv::Mat term3(x.size(),CV_32FC1);


        cv::pow(x,2,term1);
        term1 *= 1.0f / (sigmax*sigmax);

        cv::pow(y,2,term2);
        term2 *= 1.0f / (sigmay*sigmay);

        term3 = fp::mat_cos(2.0*M_PI*unfreq[i]*x);

        cv::Mat reffilter = cv::Mat::zeros(x.size(),CV_32FC1);
        cv::exp(-(term1 + term2)/2.0,reffilter);
        reffilter = reffilter.mul(term3);

        // Generate rotated versions of the filter.  Note orientation
        // image provides orientation *along* the ridges, hence +90
        // degrees, and imrotate requires angles +ve anticlockwise, hence
        // the minus sign.
        for(int o = 0; o < 180/angleInc; o++)
        {
            //qDebug() << o;
            cv::Mat new_filter(reffilter.size(),reffilter.type());
            float angle = -(o*angleInc+90);
            cv::Mat M = cv::getRotationMatrix2D(cv::Point((reffilter.cols)/2,(reffilter.rows)/2),angle,1);
            cv::warpAffine(reffilter,new_filter,M,reffilter.size());
            filter[i][o] = new_filter;
            //descomentar para guardar los filtros en disco
            cv::imwrite("reffilter.jpg",reffilter*255);
            cv::imwrite(std::to_string(i)+"_"+std::to_string(o)+"filter.jpg",new_filter*255);
        }

    }
    // Convert orientation matrix values from radians to an index value
    // that corresponds to round(degrees/angleInc)
    int maxOrientIndex = round(180/angleInc);
    //cv::Mat orientindex = orient;
    cv::Mat orientindex(orient.size(),CV_8UC1);
    //orientindex /= M_PI;
    //orientindex *= 180;
    //orientindex /= angleInc;
    //orientindex.convertTo(orientindex,CV_8UC1);
    for(int r = 0; r < orientindex.rows; r++)
    {
        for(int c = 0; c < orientindex.cols; c++)
        {
            int orientpix = static_cast<int>(
                        std::round(orient.at<float>(r,c) / M_PI * 180 / angleInc));

            if (orientpix < 0) {
                orientpix += maxOrientIndex;
            }
            if (orientpix >= maxOrientIndex) {
                orientpix -= maxOrientIndex;
            }

            orientindex.at<uchar>(r,c) = orientpix;
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

        //cv::Rect roi(c-s,r-s,2*s,2*s);
        cv::Rect roi(c-s,r-s,2*s,2*s);
        std::cout << roi << std::endl;
        cv::Mat subim(im(roi));
        cv::Mat subFilter = filter[filterindex][orientindex.at<uchar>(trunc(r/blk_sze),trunc(c/blk_sze))];
        cv::Mat mulResult;
        cv::multiply(subim,subFilter,mulResult);

        if(cv::sum(mulResult)[0] > 0)
        {
            newim.at<float>(r,c) = 255;
        }
    }
    return newim;
}

cv::Mat ridge_freq(const cv::Mat &im, const cv::Mat &mask, const cv::Mat &angles,int blk_sze)
{
    Q_UNUSED(angles);
    Q_UNUSED(blk_sze;)
    //todo, por el momento devuelve un valor fijo
    float freq = 0.11;
    cv::Mat freq_map = freq * cv::Mat::ones(im.size(),im.type());
    cv::bitwise_and(freq_map,freq_map,freq_map,mask);
    return freq_map;
}

cv::Mat Preprocesser::thin(const cv::Mat &src, int thinning_method)
{
    cv::Mat thinned;
    cv::Mat binary;
    cv::threshold(src,binary,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    thinned = binary.clone();
    switch(thinning_method)
    {
    case NONE:
    {
        thinned = src;
        break;
    }
    case ZHANGSUEN:
    {
        //zhangsuen_thinning(thinned);
        cv::ximgproc::thinning(binary,thinned,cv::ximgproc::THINNING_ZHANGSUEN);
        break;
    }
    case MORPH:
    {
        thinned = morphological_thinning(binary);
        break;
    }
    case GUOHALL:
    {
        //guohall_thinning(thinned);
        cv::ximgproc::thinning(binary,thinned,cv::ximgproc::THINNING_GUOHALL);
    }
    default:
        break;
    }
    return thinned;
}

//calcula la roi de una imagen a partir de la variacion local
cv::Mat Preprocesser::get_roi(const cv::Mat &src,int blk_sze, float threshold_ratio)
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev);
    float threshold = stddev[0] * threshold_ratio;
    cv::Mat image_variance(src.size(),CV_32FC1);
    for(int i = 0; i < src.cols; i+=blk_sze)
    {
        for(int j = 0; j < src.rows; j+=blk_sze)
        {
            cv::Rect rect(i,j,std::min(blk_sze,src.cols-i-1),std::min(blk_sze,src.rows-j-1));
            cv::Scalar b_mean, b_stddev;
            cv::meanStdDev(src(rect),b_mean,b_stddev);
            float block_stddev = b_stddev[0];
            image_variance(rect).setTo(block_stddev);
        }
    }

    cv::Mat mask = (image_variance >= threshold);

    cv::Mat kernel;
    cv::Size k_size(2*blk_sze,2*blk_sze);
    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,k_size);
    cv::morphologyEx(mask,mask,cv::MORPH_OPEN,kernel);
    cv::morphologyEx(mask,mask,cv::MORPH_CLOSE,kernel);
    return mask;

}


fp::Preprocessed Preprocesser::preprocess(const cv::Mat &src)
{
    cv::Mat result;
    //pipeline de preprocesamiento

    //convertimos a 32f
    cv::Mat src_32f;
    src.convertTo(src_32f,CV_32FC1);

    //normalizacion para eliminar ruido e imperfecciones
    qDebug() << "Preprocesser: normalizando para mejorar contraste 1...";
    cv::Mat norm_req = normalize(src_32f,norm_req_mean,norm_req_var);

    //obtencion del roi
    qDebug() << "Preprocesser: calculando roi...";
    cv::Mat mask = get_roi(norm_req,blk_sze,roi_threshold_ratio);

    //normalizacion a media 0 y desviacion unitaria
    qDebug() << "Preprocesser: normalizando a media 0 y desviacion unitaria...";
    cv::Mat norm_m0d1 = normalize(src_32f,0.0,1.0,mask);

    //segmentacion
    qDebug() << "Preprocesser: segmentando...";
    cv::Mat segmented;
    cv::bitwise_and(norm_req,norm_req,segmented,mask);

    cv::Mat filtered;
    //estimacion de la orientacion local
    qDebug() << "Preprocesser: calculando mapa de orientacion...";
    cv::Mat angles = calculate_angles(norm_req,blk_sze,true);
    if(enhancement_method == GABOR)
    {
        //todo, mapa de frecuencia
        qDebug() << "Preprocesser: calculando mapa de frecuencias...";
        cv::Mat freq = ridge_freq(norm_m0d1,mask,angles,blk_sze);
        //filtro
        qDebug() << "Preprocesser: armando y aplicando filtros orientados de Gabor...";
        filtered = filter_ridge(norm_m0d1,angles,freq,0.5,0.5);
    }
    else
    {
        filtered = norm_req.clone();
    }
    filtered.convertTo(filtered,CV_8UC1);

    //esqueletizamos la imagen
    qDebug() << "Preprocesser: esqueletizamos...";
    cv::Mat thinned = thin(filtered,thinning_method);
    result = thinned.mul(mask);

    Preprocessed pre;
    pre.original = src;
    pre.normalized = norm_req;
    pre.filtered = filtered;
    pre.thinned = thinned;
    pre.orientation = angles;
    pre.roi = mask;
    pre.result = result;

    cv::imwrite("src.jpg",src);
    cv::imwrite("norm_req.jpg",norm_req);
    cv::imwrite("filtered.jpg",filtered);
    cv::imwrite("thinned.jpg",thinned);
    cv::imwrite("angles.jpg",visualize_angles(src,angles,blk_sze));
    cv::imwrite("mask.jpg",mask);
    cv::imwrite("result.jpg",result);

    qDebug() << "Preprocess:: listo.";
    return pre;

}

}
