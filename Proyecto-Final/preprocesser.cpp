#include "preprocesser.h"
#include <QDebug>

namespace fp
{

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

//normaliza una imagen para que tenga la media y varianza deseadas
cv::Mat Preprocesser::normalize(cv::Mat &src, float req_mean, float req_var, const cv::_InputOutputArray &mask)
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev,mask);
    cv::Mat normalized_im(src.size(),CV_32FC1);
    normalized_im = src - mean[0];
    normalized_im = normalized_im / stddev[0];
    normalized_im = req_mean + normalized_im * std::sqrt(req_var);
    return normalized_im;
}

//Calculate gradient in x- and y-direction of the image
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

/*!
 * \brief orient_ridge crea un mapa de orientacion a partir de la orientacion local
 * \param im imagen normalizada
 * \param gradientSigma sigma de la derivada del gaussiano a usar para computar los gradientes
 * \param blockSigma sigma de peso gaussiano usado para sumar los momentos de gradiente
 * \param orientSmoothSigma sigma del gaussiano usado para suavisar el campo vectorial de orientaciones
 * \return el mapa de orientacion en radianes
 */
cv::Mat orient_ridge(const cv::Mat &im, double gradientSigma = 1.0, double blockSigma = 3.0, double orientSmoothSigma = 3.0) {

    int ddepth = CV_32FC1;

    cv::Mat gradX, gradY;
    cv::Mat sin2theta;
    cv::Mat cos2theta;

    int kernelSize = trunc(6 * gradientSigma);

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
    int sze2 = trunc(6 * blockSigma);

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

/*!
 * \brief filter_ridge mejora una imagen de huella dactilar a partir de filtros orientados
 * \param im imagen a ser procesada
 * \param orient mapa de orientaciones de los riscos
 * \param freq mapa de frecuencia de la imagen
 * \param kx controla el ancho de banda del filtro
 * \param ky controla la selectividad orientacional del filtro
 * \return la imagen mejorada
 */
cv::Mat filter_ridge(const cv::Mat &im,
                     const cv::Mat &orient,
                     const cv::Mat &freq, double kx = 0.5, double ky = 0.5)
{
    bool addBorder = false;
    // Fixed angle increment between filter orientations in degrees
    // Deberia ser divisor de 180
    int angleInc = 3;


    im.convertTo(im, CV_32FC1);
    int rows = im.rows;
    int cols = im.cols;

    orient.convertTo(orient, CV_32FC1);

    cv::Mat enhancedImage = cv::Mat::zeros(rows, cols, CV_32FC1);

    std::vector<int> validr;
    std::vector<int> validc;

    double unfreq = freq.at<float>(1, 1);

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
    for(int r = 0; r < meshX.rows; r++)
    {
        for(int c = 0; c < meshX.cols; c++)
        {

            float x_val = meshX.at<float>(r,c);
            float y_val = meshY.at<float>(r,c);
            float result = exp(-((x_val*x_val)/sigmax_squared + (y_val*y_val)/sigmay_squared)/2);
            result *= cos(pi_by_unfreq_by_2*x_val);
            refFilter.at<float>(r,c) = result;
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
        //descomentar para guardar los filtros
        //cv::imwrite(std::to_string(m)+"_filter.jpg",rotResult*255);
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
        const auto *orientationImage_y = orient.ptr<float>(y);
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
        cv::Mat subim(im(roi));
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



cv::Mat calculate_angles(cv::Mat &im, int W)
{
    int x = im.cols;
    int y = im.rows;


    cv::Mat Gx_;
    cv::Sobel(im/125,Gx_,-1,1,0);
    Gx_ *=125;
    cv::Mat Gy_;
    cv::Sobel(im/125,Gy_,-1,0,1);
    Gy_ *= 125;
    cv::Mat result = cv::Mat::zeros(trunc(y/W),trunc(x/W),CV_32FC1);
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
            cv::Point p(trunc((i)/W),trunc((j)/W));
            if(nominator || denominator)
            {
                float angle = (M_PI + atan2(nominator,denominator))/2;
                result.at<float>(p) = angle;
            }
            else
            {
                result.at<float>(p) = 0;
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
        sze[i] = round(3*std::max(sigmax,sigmay));
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

        term3 = fp::mat_cos(2*M_PI*unfreq[i]*x);

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
            cv::Mat new_filter(reffilter.size(),reffilter.type());
            float angle = -(o*angleInc+90);
            cv::Mat M = cv::getRotationMatrix2D(cv::Point((reffilter.cols-1)/2,(reffilter.rows-1)/2),angle,1);
            cv::warpAffine(reffilter,new_filter,M,reffilter.size());
            filter[i][o] = new_filter;
            //descomentar para guardar los filtros en disco
            cv::imwrite("reffilter.jpg",reffilter*255);
            // cv::imwrite(std::to_string(i)+"_"+std::to_string(o)+"filter.jpg",new_filter);
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
    std::cout << orientindex.size() << std::endl;;
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
        cv::Mat subim(im(roi));
        cv::Mat subFilter = filter[filterindex][orientindex.at<uchar>(trunc(r/blk_sze),trunc(c/blk_sze))];
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

cv::Mat ridge_freq(cv::Mat &im, cv::Mat &mask, cv::Mat &angles,int blk_sze)
{
    Q_UNUSED(angles);
    Q_UNUSED(blk_sze;)
    //todo, por el momento devuelve un valor fijo
    float freq = 0.11;
    cv::Mat freq_map = freq * cv::Mat::ones(im.size(),im.type());
    cv::bitwise_and(freq_map,freq_map,freq_map,mask);
    return freq_map;
}

//mejora una imagen aplicando filtros de gabor
//supone que la imagen ya esta normalizada
//entrada : imagen CV_32FC1
//salida : imagen CV_32FC1
cv::Mat gabor(cv::Mat &normalized, int blk_sze)
{
    cv::Mat im = normalized.clone();
    //estimacion de la orientacion local
    qDebug() << "calculating angles...";
    // cv::Mat orient_image = orient_ridge(im);
    cv::Mat orient_image = calculate_angles(im,blk_sze);
    cv::imwrite("orient_image.jpg",visualize_angles(im,orient_image,blk_sze));
    //todo, mapa de frecuencia
    qDebug() << "Armando frecuencia1...";
    float freq_val = 0.11;
    cv::Mat freq = cv::Mat::ones(im.size(),im.type());
    freq *= freq_val;
    //filtro
    qDebug() << "filtering...";
    //cv::Mat filtered = filter_ridge(im, orient_image, freq);;
    cv::Mat filtered = filter_ridge2(im,orient_image,freq,0.5,0.5);
    //devolvemos la imagen mejorada
    return filtered;
}

cv::Mat Preprocesser::enhance(cv::Mat &src, int enhancement_method)
{
    cv::Mat enhanced;
    switch (enhancement_method)
    {
    case NONE:
    {
        enhanced = src.clone();
        break;
    }
    case GABOR:
    {
        enhanced = gabor(src,blk_sze);
        break;
    }
    default:
        break;
    }
    return enhanced;
}

cv::Mat Preprocesser::thin(cv::Mat &src, int thinning_method)
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
        zhangsuen_thinning(thinned);
        break;
    }
    case MORPH:
    {
        thinned = morphological_thinning(binary);
        break;
    }
    case GUOHALL:
    {
        guohall_thinning(thinned);
    }
    default:
        break;
    }
    return thinned;
}

//calcula la roi de una imagen a partir de la variacion local
cv::Mat Preprocesser::get_roi(cv::Mat &src,int blk_sze, float threshold_ratio)
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
    cv::Size k_size(2*blk_sze,2*blk_sze);
    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,k_size);
    cv::morphologyEx(mask,mask,cv::MORPH_OPEN,kernel);
    cv::morphologyEx(mask,mask,cv::MORPH_CLOSE,kernel);
    return mask;

}


fp::Preprocessed Preprocesser::preprocess(cv::Mat &src)
{
    cv::Mat result;
    //pipeline de preprocesamiento

    //convertimos a 32f
    cv::Mat src_32f;
    src.convertTo(src_32f,CV_32FC1);

    //normalizacion para eliminar ruido e imperfectiones
    qDebug() << "Preprocesser: normalizing 1...";
    cv::Mat norm_req = normalize(src_32f,norm_req_mean,norm_req_var);

    //obtencion del roi
    qDebug() << "Preprocesser: calculating roi...";
    cv::Mat mask = get_roi(norm_req,blk_sze,roi_threshold_ratio);

    //normalizacion a media 0 y desviacion unitaria
    qDebug() << "Preprocesser: normalizing 2...";
    cv::Mat norm_m0d1 = normalize(src_32f,0.0,1.0,mask);

    //segmentacion
    qDebug() << "Preprocesser: segmentando...";
    cv::Mat segmented;
    cv::bitwise_and(norm_req,norm_req,segmented,mask);

    //    //mejora
    qDebug() << "Preprocesser: enhancing...";
    //    cv::Mat enhanced = enhance(norm_req, enhancement_method);
    //    enhanced.convertTo(enhanced,CV_8UC1);

    cv::Mat enhanced;
    if(enhancement_method == GABOR)
    {
        //estimacion de la orientacion local
        qDebug() << "calculating angles...";
        cv::Mat angles = calculate_angles(norm_req,blk_sze);
        cv::imwrite("orient_image.jpg",visualize_angles(segmented,angles,blk_sze));
        //todo, mapa de frecuencia
        qDebug() << "Armando frecuencia...";
        cv::Mat freq = ridge_freq(norm_m0d1,mask,angles,blk_sze);
        //filtro
        qDebug() << "filtering...";
        enhanced = filter_ridge2(norm_m0d1,angles,freq,0.5,0.5);
    }
    else
    {
        enhanced = norm_req.clone();
    }
    enhanced.convertTo(enhanced,CV_8UC1);

    //esqueletizamos la imagen
    qDebug() << "Preprocesser: thinning...";
    cv::Mat thinned = thin(enhanced,thinning_method);
    result = thinned;

    cv::Mat orient = orient_ridge(norm_req);
    Preprocessed pre;
    pre.original = src;
    pre.normalized = norm_req;
    pre.filtered = enhanced;
    pre.thinned = thinned;
    pre.orientation = orient;
    pre.roi = mask;
    pre.result = result;
    qDebug() << "finished preprocess";
    return pre;

}

}
