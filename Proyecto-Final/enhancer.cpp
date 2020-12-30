#include "enhancer.h"

namespace fp
{


Enhancer::Enhancer()
{
    //esta clase solo tiene metodos estaticos
    //no es necesario instanciarla
}

//realiza una iteracion de reduccion, la misma se debe repetir hasta que la imagen este esquelitizada
void thinningIteration(cv::Mat& im, int iter)
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
void thinning(cv::Mat& im)
{
    // Enforce the range tob e in between 0 - 255
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningIteration(im, 0);
        thinningIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
}

cv::Mat skeletonize(cv::Mat &src)
{
    //pasamos la imagen de escala de gris a binario
    cv::Mat binary;
    cv::threshold(src,binary,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    //luego obtenemos el "esqueleto" de la imagen
    cv::Mat skeleton = binary.clone();
    thinning(skeleton);
    return skeleton;
}

/*
 * Compute the standard deviation of the image.
 */
float deviation(const cv::Mat &im, float average) {
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

/*
 * Normalization function of Anil Jain's algorithm.
 */
cv::Mat normalize_image(const cv::Mat &im, double reqMean,
                        double reqVar) {

    cv::Mat convertedIm;
    im.convertTo(convertedIm, CV_32FC1);

    cv::Scalar mean = cv::mean(convertedIm);
    cv::Mat normalizedImage = convertedIm - mean[0];

    cv::Scalar normMean = cv::mean(normalizedImage);
    float stdNorm = deviation(normalizedImage, normMean[0]);
    normalizedImage = normalizedImage / stdNorm;
    normalizedImage = reqMean + normalizedImage * cv::sqrt(reqVar);

    return normalizedImage;
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

    cv::Mat sub = grad_xx - grad_yy;
    cv::divide(grad_xy, denom, sin2theta);
    cv::divide(sub, denom, cos2theta);

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
    cv::Mat transpose = gv.t();
    cv::repeat(transpose, 1, total, meshY);
    std::cout << meshX << std::endl;
    std::cout << meshY << std::endl;
}

/*
 * Performing Gabor filtering for enhancement using previously calculated orientation
 * image and frequency. The output is final enhanced image.
 *
 * Refer to the paper for detailed description.
*/
cv::Mat filter_ridge(const cv::Mat &inputImage, const cv::Mat &orientationImage, const cv::Mat &frequency)
{
    double kx = 0.8;
    double ky = 0.8;
    bool addBorder = false;

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
    std::cout << "debug" << std::endl;
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

/*
 * Perform Gabor filter based image enhancement using orientation field and
 * frequency.
 */
cv::Mat gabor_filter(const cv::Mat &inputImage) {
    // The frequency is set to 0.11 experimentally. You can change this value
    // or use a dynamic calculation instead.
    double freqValue = 0.11;
    // Perform median blurring to smooth the image
    cv::Mat blurredImage;
    cv::medianBlur(inputImage, blurredImage, 3);

    // Check whether the input image is grayscale.
    // If not, convert it to grayscale.
    if (blurredImage.channels() != 1) {
        cv::cvtColor(blurredImage, blurredImage, cv::COLOR_BGR2GRAY);
    }


    std::cout << "Rows: " << blurredImage.rows << " / Cols: " << blurredImage.cols
              << std::endl;

    // Perform normalization using the method provided in the paper
    cv::Mat normalizedImage = normalize_image(blurredImage, 0, 1);
    std::cout << "Normalization done" << std::endl;

    // Calculate ridge orientation field
    cv::Mat orientationImage = orient_ridge(normalizedImage);
    std::cout << "Orientation done" << std::endl;

    cv::Mat freq = cv::Mat::ones(normalizedImage.rows, normalizedImage.cols,
                                 normalizedImage.type()) *
            freqValue;
    // Get the final enhanced image and return it as result
    cv::Mat enhancedImage =
            filter_ridge(normalizedImage, orientationImage, freq);

    std::cout << "Done with processing pipeling" << std::endl;
    return enhancedImage;
}

cv::Mat Enhancer::enhance(cv::Mat &src, EnhancementMethod method)
{
    cv::Mat enhanced;
    switch (method)
    {
    case NONE:
        enhanced = src.clone();
        break;
    case SKELETONIZE:
        enhanced = skeletonize(src);
        break;
    case GABORFILTERS:
        enhanced = gabor_filter(src);
        break;
    default:
        break;
    }
    return enhanced;
}
}
