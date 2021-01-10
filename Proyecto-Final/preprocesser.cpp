#include "preprocesser.h"
#include <QDebug>

namespace fp
{


Preprocesser::Preprocesser()
{
    //esta clase solo tiene metodos estaticos
    //no es necesario instanciarla
}

//realiza una iteracion de reduccion del algoritmo de zhang-suen, la misma se debe repetir hasta que la imagen este esquelitizada
void zhangsuen_iteration(cv::Mat& im, int iter)
{
    cv::Mat marker =cv::Mat::zeros(im.size(), CV_8UC1);
    for (int i = 1; i < im.rows-1; i++)
    {
        for (int j = 1; j < im.cols-1; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int A  = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
                    (p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
                    (p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
                    (p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
            int B  = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
            int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
            int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

            if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

//funcion para reducir una imagen binaria, debe estar en el rango de 0-255.
//basado en el algoritmo de Zhang-Suen https://rosettacode.org/wiki/Zhang-Suen_thinning_algorithm
void zhangsuen_thinning(cv::Mat& im)
{
    // Enforce the range tob e in between 0 - 255
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        zhangsuen_iteration(im, 0);
        zhangsuen_iteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}

/**
         * Perform one thinning iteration.
         * Normally you wouldn't call this function directly from your code.
         *
         * @param  im    Binary image with range = 0-1
         * @param  iter  0=even, 1=odd
         */
void guohall_iteration(cv::Mat& im, int iter)
{
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows; i++)
    {
        for (int j = 1; j < im.cols; j++)
        {
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int C  = ((!p2) & (p3 | p4)) + ((!p4) & (p5 | p6)) +
                    ((!p6) & (p7 | p8)) + ((!p8) & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N  = N1 < N2 ? N1 : N2;
            int m  = iter == 0 ? ((p6 | p7 | (!p9)) & p8) : ((p2 | p3 | (!p5)) & p4);

            if (C == 1 && (N >= 2 && N <= 3) & (m == 0))
                marker.at<uchar>(i,j) = 1;
        }
    }

    im &= ~marker;
}

/**
         * Function for thinning the given binary image
         *
         * @param  im  Binary image with range = 0-255
         */
void guohall_thinning(cv::Mat& im)
{
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        guohall_iteration(im, 0);
        guohall_iteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}

cv::Mat morphological_thinning(cv::Mat &src)
{
    cv::Mat bin;
    cv::threshold(src,bin,127,255,cv::THRESH_BINARY);
    cv::Mat skel(src.size(), CV_8UC1,cv::Scalar(0));
    cv::Mat temp(src.size(),CV_8UC1);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS,cv::Size(3,3));
    bool done;
    do
    {
        cv::morphologyEx(src, temp, cv::MORPH_OPEN, element);
        cv::bitwise_not(temp, temp);
        cv::bitwise_and(src, temp, temp);
        cv::bitwise_or(skel, temp, skel);
        cv::erode(src, src, element);

        double max;
        cv::minMaxLoc(src, 0, &max);
        done = (max == 0);
    } while (!done);
    return skel;
}

/*
         * Compute the standard deviation of the image.
         */
float deviation(const cv::Mat &im, float average)
{
    float sdev = 0.0;

    for (int i = 0; i < im.rows; i++) {
        for (int j = 0; j < im.cols; j++) {
            float pixel = im.at<float>(i, j);
            float dev = (pixel - average) * (pixel - average);
            sdev = sdev + dev;
        }
    }

    int size = im.rows * im.cols;
    float var = sdev / (size - 1);
    float sd = std::sqrt(var);

    return sd;
}


//normaliza una imagen para que tenga la media y varianza deseadas
cv::Mat normalize_1(cv::Mat &src, float req_mean, float req_var)
{
    cv::Scalar mean = cv::mean(src);
    cv::Mat normalized_im = src - mean[0];

    cv::Scalar norm_mean = cv::mean(normalized_im);
    float std_norm = deviation(normalized_im, norm_mean[0]);
    normalized_im = normalized_im / std_norm;
    normalized_im = req_mean + normalized_im * std::sqrt(req_var);


    return normalized_im;

}

float normalize_pixel(float x, float req_var, float v, float m, float req_mean)
{
    float normalized_pixel = 0;
    float dev_coeff = sqrt((req_var * ((x - m)*(x - m)))/v);
    if(x > m)
    {
        normalized_pixel = req_mean + dev_coeff;
    }
    else
    {
        normalized_pixel = req_mean - dev_coeff;
    }
    return normalized_pixel;
}

cv::Mat normalize_2(cv::Mat &src, float req_mean, float req_var)
{
    cv::Scalar mean = cv::mean(src);
    float m = mean[0];
    float v = deviation(src,m);
    cv::Mat normalized = src.clone();
    for(int row = 0; row < src.rows; row++)
    {
        for(int col = 0; col < src.cols; col++)
        {
            normalized.at<float>(row,col) = normalize_pixel(src.at<float>(row,col),req_var,v,m,req_mean);
        }
    }
    return normalized;
}

cv::Mat Preprocesser::normalize(cv::Mat &src, float req_mean, float req_var)
{
    cv::Mat normalized_image;
    normalized_image = normalize_1(src,req_mean,req_var);
    return normalized_image;

}

/*
         * Calculate gradient in x- and y-direction of the image
         */
void gradient(const cv::Mat &image, cv::Mat &xGradient, cv::Mat &yGradient)
{

    int ddepth = CV_32FC1;
    xGradient = cv::Mat::zeros(image.rows, image.cols, ddepth);
    yGradient = cv::Mat::zeros(image.rows, image.cols, ddepth);

    // Pointer access more effective than Mat.at<T>()
    for (int i = 1; i < image.rows - 1; i++) {
        const auto *image_i = image.ptr<float>(i);
        auto *xGradient_i = xGradient.ptr<float>(i);
        auto *yGradient_i = yGradient.ptr<float>(i);
        for (int j = 1; j < image.cols - 1; j++) {
            float xPixel1 = image_i[j - 1];
            float xPixel2 = image_i[j + 1];

            float yPixel1 = image.at<float>(i - 1, j);
            float yPixel2 = image.at<float>(i + 1, j);

            float xGrad;
            float yGrad;

            if (j == 0) {
                xPixel1 = image_i[j];
                xGrad = xPixel2 - xPixel1;
            } else if (j == image.cols - 1) {
                xPixel2 = image_i[j];
                xGrad = xPixel2 - xPixel1;
            } else {
                xGrad = 0.5f * (xPixel2 - xPixel1);
            }

            if (i == 0) {
                yPixel1 = image_i[j];
                yGrad = yPixel2 - yPixel1;
            } else if (i == image.rows - 1) {
                yPixel2 = image_i[j];
                yGrad = yPixel2 - yPixel1;
            } else {
                yGrad = 0.5f * (yPixel2 - yPixel1);
            }

            xGradient_i[j] = xGrad;
            yGradient_i[j] = yGrad;
        }
    }
}

/*
         * Estimate orientation field of fingerprint ridges.
         */
cv::Mat orient_ridge(const cv::Mat &im) {


    double blockSigma = 5.0;
    double gradientSigma = 1.0;
    double orientSmoothSigma = 5.0;

    int ddepth = CV_32FC1;


    cv::Mat gradX, gradY;
    cv::Mat sin2theta;
    cv::Mat cos2theta;

    int kernelSize = 6 * round(gradientSigma);

    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    // Define Gaussian kernel
    cv::Mat gaussKernelX =
            cv::getGaussianKernel(kernelSize, gradientSigma, CV_32FC1);
    cv::Mat gaussKernelY =
            cv::getGaussianKernel(kernelSize, gradientSigma, CV_32FC1);
    cv::Mat gaussKernel = gaussKernelX * gaussKernelY.t();

    // Peform Gaussian filtering
    cv::Mat fx, fy;
    cv::Mat kernelx = (cv::Mat_<float>(1, 3) << -0.5, 0, 0.5);
    cv::Mat kernely = (cv::Mat_<float>(3, 1) << -0.5, 0, 0.5);
    cv::filter2D(gaussKernel, fx, -1, kernelx);
    cv::filter2D(gaussKernel, fy, -1, kernely);

    // Gradient of Gaussian
    gradient(gaussKernel, fx, fy);

    gradX.convertTo(gradX, CV_32FC1);
    gradY.convertTo(gradY, CV_32FC1);

    // Gradient of the image in x
    cv::filter2D(im, gradX, -1, fx, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
    // Gradient of the image in y
    cv::filter2D(im, gradY, -1, fy, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

    cv::Mat grad_xx, grad_xy, grad_yy;
    cv::multiply(gradX, gradX, grad_xx);
    cv::multiply(gradX, gradY, grad_xy);
    cv::multiply(gradY, gradY, grad_yy);

    // Now smooth the covariance data to perform a weighted summation of the data
    int sze2 = 6 * round(blockSigma);

    if (sze2 % 2 == 0) {
        sze2++;
    }

    cv::Mat gaussKernelX2 = cv::getGaussianKernel(sze2, blockSigma, CV_32FC1);
    cv::Mat gaussKernelY2 = cv::getGaussianKernel(sze2, blockSigma, CV_32FC1);
    cv::Mat gaussKernel2 = gaussKernelX2 * gaussKernelY2.t();

    cv::filter2D(grad_xx, grad_xx, -1, gaussKernel2, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
    cv::filter2D(grad_xy, grad_xy, -1, gaussKernel2, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
    cv::filter2D(grad_yy, grad_yy, -1, gaussKernel2, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);

    grad_xy *= 2;

    // Analytic solution of principal direction
    cv::Mat G1, G2, G3;
    cv::multiply(grad_xy, grad_xy, G1);
    G2 = grad_xx - grad_yy;
    cv::multiply(G2, G2, G2);

    cv::Mat denom;
    G3 = G1 + G2;
    cv::sqrt(G3, denom);

    cv::divide(grad_xy, denom, sin2theta);
    cv::Mat sub1 = grad_xx - grad_yy;
    cv::divide(sub1, denom, cos2theta);

    int sze3 = 6 * round(orientSmoothSigma);

    if (sze3 % 2 == 0) {
        sze3 += 1;
    }

    cv::Mat gaussKernelX3 =
            cv::getGaussianKernel(sze3, orientSmoothSigma, CV_32FC1);
    cv::Mat gaussKernelY3 =
            cv::getGaussianKernel(sze3, orientSmoothSigma, CV_32FC1);
    cv::Mat gaussKernel3 = gaussKernelX3 * gaussKernelY3.t();

    cv::filter2D(cos2theta, cos2theta, -1, gaussKernel3, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
    cv::filter2D(sin2theta, sin2theta, -1, gaussKernel3, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);

    sin2theta.convertTo(sin2theta, ddepth);
    cos2theta.convertTo(cos2theta, ddepth);
    cv::Mat orientim = cv::Mat::zeros(sin2theta.rows, sin2theta.cols, ddepth);

    // Pointer access more effective than Mat.at<T>()
    for (int i = 0; i < sin2theta.rows; i++) {
        const float *sin2theta_i = sin2theta.ptr<float>(i);
        const float *cos2theta_i = cos2theta.ptr<float>(i);
        auto *orientim_i = orientim.ptr<float>(i);
        for (int j = 0; j < sin2theta.cols; j++) {
            orientim_i[j] = (M_PI + std::atan2(sin2theta_i[j], cos2theta_i[j])) / 2;
        }
    }

    return orientim;
}

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

/*
         * Performing Gabor filtering for enhancement using previously calculated orientation
         * image and frequency. The output is final enhanced image.
         *
         * Refer to the paper for detailed description.
        */
cv::Mat filter_ridge(const cv::Mat &inputImage,
                     const cv::Mat &orientationImage,
                     const cv::Mat &frequency)
{
    double kx = 0.8;
    double ky = 0.8;
    bool addBorder = true;
    // Fixed angle increment between filter orientations in degrees
    int angleInc = 3;

    inputImage.convertTo(inputImage, CV_32FC1);
    int rows = inputImage.rows;
    int cols = inputImage.cols;

    orientationImage.convertTo(orientationImage, CV_32FC1);

    cv::Mat enhancedImage = cv::Mat::zeros(rows, cols, CV_32FC1);
    std::vector<int> validr;
    std::vector<int> validc;

    double unfreq = frequency.at<float>(1, 1);

    cv::Mat freqindex = cv::Mat::ones(100, 1, CV_32FC1);

    double sigmax = (1 / unfreq) * kx;
    double sigmax_squared = sigmax * sigmax;
    double sigmay = (1 / unfreq) * ky;
    double sigmay_squared = sigmay * sigmay;

    int szek = (int) round(3 * (std::max(sigmax, sigmay)));

    cv::Mat meshX, meshY;
    meshgrid(szek, meshX, meshY);

    cv::Mat refFilter = cv::Mat::zeros(meshX.rows, meshX.cols, CV_32FC1);

    meshX.convertTo(meshX, CV_32FC1);
    meshY.convertTo(meshY, CV_32FC1);

    double pi_by_unfreq_by_2 = 2 * M_PI * unfreq;

    for (int i = 0; i < meshX.rows; i++) {
        const float *meshX_i = meshX.ptr<float>(i);
        const float *meshY_i = meshY.ptr<float>(i);
        auto *reffilter_i = refFilter.ptr<float>(i);
        for (int j = 0; j < meshX.cols; j++) {
            float meshX_i_j = meshX_i[j];
            float meshY_i_j = meshY_i[j];
            float pixVal2 = -0.5f * (meshX_i_j * meshX_i_j / sigmax_squared +
                                     meshY_i_j * meshY_i_j / sigmay_squared);
            float pixVal = std::exp(pixVal2);
            float cosVal = pi_by_unfreq_by_2 * meshX_i_j;
            reffilter_i[j] = pixVal * std::cos(cosVal);
        }
    }

    std::vector<cv::Mat> filters;

    for (int m = 0; m < 180 / angleInc; m++) {
        double angle = -(m * angleInc + 90);
        cv::Mat rot_mat =
                cv::getRotationMatrix2D(cv::Point((float) (refFilter.rows / 2.0F),
                                                  (float) (refFilter.cols / 2.0F)),
                                        angle, 1.0);
        cv::Mat rotResult;
        cv::warpAffine(refFilter, rotResult, rot_mat, refFilter.size());
        filters.push_back(rotResult);
    }

    // Find indices of matrix points greater than maxsze from the image boundary
    int maxsze = szek;
    // Convert orientation matrix values from radians to an index value that
    // corresponds to round(degrees/angleInc)
    int maxorientindex = std::round(180 / angleInc);

    cv::Mat orientindex(rows, cols, CV_32FC1);

    int rows_maxsze = rows - maxsze;
    int cols_maxsze = cols - maxsze;

    for (int y = 0; y < rows; y++) {
        const auto *orientationImage_y = orientationImage.ptr<float>(y);
        auto *orientindex_y = orientindex.ptr<float>(y);
        for (int x = 0; x < cols; x++) {
            if (x > maxsze && x < cols_maxsze && y > maxsze && y < rows_maxsze) {
                validr.push_back(y);
                validc.push_back(x);
            }

            int orientpix = static_cast<int>(
                        std::round(orientationImage_y[x] / M_PI * 180 / angleInc));

            if (orientpix < 0) {
                orientpix += maxorientindex;
            }
            if (orientpix >= maxorientindex) {
                orientpix -= maxorientindex;
            }

            orientindex_y[x] = orientpix;
        }
    }

    // Finally, do the filtering
    for (long unsigned k = 0; k < validr.size(); k++) {
        int r = validr[k];
        int c = validc[k];

        cv::Rect roi(c - szek - 1, r - szek - 1, meshX.cols, meshX.rows);
        cv::Mat subim(inputImage(roi));

        cv::Mat subFilter = filters.at(orientindex.at<float>(r, c));
        cv::Mat mulResult;
        cv::multiply(subim, subFilter, mulResult);

        if (cv::sum(mulResult)[0] > 0) {
            enhancedImage.at<float>(r, c) = 255;
        }
    }

    // Add a border.
    if (addBorder) {
        enhancedImage.rowRange(0, rows).colRange(0, szek + 1).setTo(255);
        enhancedImage.rowRange(0, szek + 1).colRange(0, cols).setTo(255);
        enhancedImage.rowRange(rows - szek, rows).colRange(0, cols).setTo(255);
        enhancedImage.rowRange(0, rows)
                .colRange(cols - 2 * (szek + 1) - 1, cols)
                .setTo(255);
    }

    return enhancedImage;
}

//mejora una imagen aplicando filtros de gabor
//supone que la imagen ya esta normalizada
//entrada : imagen CV_32FC1
//salida : imagen CV_32FC1
cv::Mat gabor(cv::Mat &normalized)
{
    //estimacion de la orientacion local
    cv::Mat orient_image = orient_ridge(normalized);
    double freq_val = 0.11;
    cv::Mat freq = cv::Mat::ones(normalized.rows, normalized.cols, normalized.type()) * freq_val;
    //filtro
    cv::Mat filtered = filter_ridge(normalized, orient_image, freq);
    //devolvemos la imagen mejorada
    return filtered;
}

//mejora una imagen con el metodo elegido
//entrada CV_32FC1
//salida CV_32FC1
cv::Mat Preprocesser::enhance(cv::Mat &src, EnhancementMethod enhancement_method)
{
    cv::Mat enhanced;
    switch (enhancement_method)
    {
    case ENH_NONE:
    {
        enhanced = src.clone();
        break;
    }
    case ENH_GABOR:
    {
        enhanced = gabor(src);
        break;
    }
    default:
        break;
    }
    return enhanced;
}

cv::Mat Preprocesser::thin(cv::Mat &src, ThinningMethod thinning_method)
{
    cv::Mat thinned;
    cv::Mat binary;
    cv::threshold(src,binary,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    thinned = binary.clone();
    switch(thinning_method)
    {
    case THI_NONE:
    {
        thinned = src;
        break;
    }
    case THI_ZHANGSUEN:
    {
        zhangsuen_thinning(thinned);
        break;
    }
    case THI_MORPH:
    {
        thinned = morphological_thinning(binary);
        break;
    }
    case THI_GUOHALL:
    {
        guohall_thinning(thinned);
    }
    default:
        break;
    }
    return thinned;
}

//calcula la roi de una imagen
//supone que la imagen ya esta normalizada
cv::Mat Preprocesser::get_roi(cv::Mat &src,int block_size, float threshold_ratio)
{
    int w = block_size;
    float t = threshold_ratio;
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev);
    float threshold = stddev[0] * t;
    cv::Mat image_variance(src.size(),CV_32FC1);
    for(int i = 0; i < src.cols; i+=w)
    {
        for(int j = 0; j < src.rows; j+=w)
        {
            cv::Rect rect(i,j,std::min(w,src.cols-i-1),std::min(w,src.rows-j-1));
            cv::Scalar b_mean, b_stddev;
            cv::meanStdDev(src(rect),b_mean,b_stddev);
            float block_stddev = b_stddev[0];
            image_variance(rect).setTo(block_stddev);
        }
    }
    cv::Mat mask(src.size(),CV_8UC1,255);
    for(int i = 0; i < mask.cols; i++)
    {
        for(int j = 0; j < mask.rows; j++)
        {
            if(image_variance.at<float>(j,i) < threshold)
            {

                mask.at<uchar>(j,i) = 0;
            }
        }
    }
    cv::Mat kernel;
    cv::Size k_size(2*w,2*w);
    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,k_size);
    cv::morphologyEx(mask,mask,cv::MORPH_OPEN,kernel);
    cv::morphologyEx(mask,mask,cv::MORPH_CLOSE,kernel);
    return mask;

}

cv::Mat Preprocesser::preprocess(cv::Mat &src, EnhancementMethod enhancement_method, ThinningMethod thinning_method, bool roi_masking)
{
    Q_UNUSED(roi_masking);
    //pipeline de preprocesamiento

    //convertimos a 32f
    cv::Mat src_32f;
    src.convertTo(src_32f,CV_32FC1);

    //normalizacion
    cv::Mat normalized = normalize(src_32f,100.0,100.0);

    //mejora
    cv::Mat enhanced = enhance(normalized, enhancement_method);
    enhanced.convertTo(enhanced,CV_8UC1);

    //eskeletizamos la imagen
    cv::Mat thinned = thin(enhanced,thinning_method);

    //segmentamos la imagen
    cv::Mat mask = get_roi(normalized,16,0.2);
    cv::bitwise_and(thinned,mask,thinned);

    cv::Mat result = thinned;


    return result;

}
}
