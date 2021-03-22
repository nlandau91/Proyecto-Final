#include "preprocesser.h"
#include <QDebug>

using namespace fp;

Preprocesser::Preprocesser()
{

}

cv::Mat sharpening(const cv::Mat &im, double scale)
{
    cv::Mat sharp = im.clone();
    cv::Mat smooth;
    cv::GaussianBlur(im,smooth,cv::Size(3,3),0);
    sharp = (1+scale)*im - scale*smooth;

    return sharp;
}

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

cv::Mat normalize(const cv::Mat &src, float req_mean, float req_var, const cv::Mat &mask = cv::Mat())
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev,mask);

    cv::Mat normalized_im(src.size(), CV_32FC1);

    normalized_im = src - mean[0];
    normalized_im = normalized_im / stddev[0];
    normalized_im = req_mean + normalized_im * std::sqrt(req_var);
    return normalized_im;
}

//normaliza un pixel, usado por normalize2
float normalize_pixel(float x, float v, float v0, float m, float m0)
{
    float normalized = 0;
    normalized = (x - m) * (x - m);
    normalized *= v0/v;
    normalized = std::sqrt(normalized);
    if(x > m)
    {
        normalized = m0 + normalized;
    }
    else
    {
        normalized = m0 - normalized;
    }
    return normalized;
}

/*!
 * \brief normalize2 normaliza una imagne para que tenga la madia y varianza indicadas
 * implementa el metodo de Hong, Wan y Jain 98
 * \param src
 * \param m0 media deseada
 * \param v0 varianza deseada
 * \param mask mascara opcional
 * \return
 */
cv::Mat normalize2(const cv::Mat &src, float m0, float v0, const cv::Mat &mask = cv::Mat())
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev,mask);
    float v = stddev[0] * stddev[0];
    float m = mean[0];

    cv::Mat normalized_im(src.size(), CV_32FC1);
    src.convertTo(normalized_im, CV_32FC1);

    for(int r = 0; r < normalized_im.rows; r++)
    {
        for(int c = 0; c < normalized_im.cols; c++)
        {
            if(mask.empty() || (!mask.empty() && mask.at<uchar>(r,c) > 0))
            {
                normalized_im.at<float>(r,c) = normalize_pixel(normalized_im.at<float>(r,c),v,v0,m,m0);
            }
        }
    }

    return normalized_im;
}

//This is equivalent to Matlab's 'meshgrid' function
void meshgrid(int kernelSize, cv::Mat &meshX, cv::Mat &meshY)
{
    std::vector<int> t;

    for(int i = -kernelSize; i < kernelSize; i++)
    {
        t.push_back(i);
    }

    cv::Mat gv = cv::Mat(t);
    int total = static_cast<int>(gv.total());
    gv = gv.reshape(1, 1);

    cv::repeat(gv, total, 1, meshX);
    cv::Mat traspose = gv.t();
    cv::repeat(traspose, 1, total, meshY);
}

/*!
 * \brief calculate_angles calcula el mapa de orientacion de la imagen
 * \param im imagen a la que se le calculara el mapa de orientacion
 * \param W tamaño de bloque a utilizar
 * \param smooth decide si se realiza un suavizado a los angulos
 * \return mapa con los angulos de la imagen. Tamaño de im/W
 */
cv::Mat calculate_angles(const cv::Mat &im, int W, int blocksigma = 3, int orientsmoothsigma = 3, cv::Mat mask = cv::Mat())
{
    int x = im.cols;
    int y = im.rows;

    cv::Mat Gx_;
    cv::Sobel(im,Gx_,-1,1,0);
    cv::Mat Gy_;
    cv::Sobel(im,Gy_,-1,0,1);

    cv::Mat Gxx_ = Gx_.mul(Gx_);
    cv::Mat Gxy_ = Gx_.mul(Gy_);
    Gxy_ *= 2;
    cv::Mat Gyy_ = Gy_.mul(Gy_);

    //suavizamos la covarianza
    int ksize = std::trunc(6 * blocksigma);
    if(ksize % 2 == 0)
    {
        ksize += 1;
    }
    cv::Mat kernel1 = cv::getGaussianKernel(ksize,blocksigma, CV_32FC1);
    cv::Mat kernel2 = cv::getGaussianKernel(ksize,blocksigma, CV_32FC1);
    cv::Mat kernel = kernel1 * kernel2.t();
    cv::filter2D(Gxx_, Gxx_, -1, kernel, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
    cv::filter2D(Gxy_, Gxy_, -1, kernel, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);
    cv::filter2D(Gyy_, Gyy_, -1, kernel, cv::Point(-1, -1), 0,
                 cv::BORDER_DEFAULT);

    cv::Mat result = cv::Mat::zeros(static_cast<int>(trunc(y/W)),static_cast<int>(trunc(x/W)),CV_32FC1);
    cv::Mat sines = cv::Mat::zeros(result.size(),CV_64FC1);
    cv::Mat cosines = cv::Mat::zeros(result.size(),CV_64FC1);
    for(int j = 0; j < y - W; j+=W)
    {
        auto *sines_j = sines.ptr<double>(j/W);
        auto *cosines_j = cosines.ptr<double>(j/W);
        for(int i = 0; i < x - W; i+=W)
        {
            int denominator = 0;
            int nominator = 0;

            for(int l = j; l < std::min(j + W, y - 1); l++)
            {
                const float *Gxx_l = Gxx_.ptr<float>(l);
                const float *Gyy_l = Gyy_.ptr<float>(l);
                const float *Gxy_l = Gxy_.ptr<float>(l);
                for(int k = i; k < std::min(i + W, x - 1); k++)
                {
                    if(mask.empty() || (!mask.empty() && mask.at<uchar>(l,k) > 0))
                    {
                        int Gxx = std::round(Gxx_l[k]);
                        int Gyy = std::round(Gyy_l[k]);
                        int Gxy = std::round(Gxy_l[k]);

                        int j1 = Gxy;;
                        nominator += j1;
                        int j2 = Gxx - Gyy;
                        denominator += j2;
                    }
                }
            }
            cv::Point p(i/W,j/W);
            cosines_j[i/W] = denominator;
            sines_j[i/W] = nominator;
        }
    }

    //filtro pasabajo al campo direccional
    ksize = std::trunc(6 * orientsmoothsigma);
    if(ksize % 2 == 0)
    {
        ksize += 1;
    }
    kernel1 = cv::getGaussianKernel(ksize,orientsmoothsigma);
    kernel2 = cv::getGaussianKernel(ksize,orientsmoothsigma);
    kernel = kernel1 * kernel2.t();
    cv::filter2D(cosines, cosines, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);
    cv::filter2D(sines, sines, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

    //finalmente armamos el mapa de angulos
    for(int i = 0; i < sines.rows; i++)
    {
        const double *sines_i = sines.ptr<double>(i);
        const double *cosines_i = cosines.ptr<double>(i);
        auto *result_i = result.ptr<float>(i);
        for(int j = 0; j < sines.cols; j++)
        {
            result_i[j] = static_cast<float>((M_PI + std::atan2(sines_i[j], cosines_i[j])) / 2.0);
        }
    }
    return result;

}

/*!
 * \brief filter_ridge aplica filtro orientados de gabor a una imagen
 * \param src imagen que sera filtrada
 * \param orientation_map mapa de orientaciones de la imagen
 * \param frequency_map mapa de frecuencias de la imagen
 * \param mask mascara de la imagen, sera modificada para incluir el borde a partir del procesamiento en bloques
 * \param kx controla el ancho de banda del filtro
 * \param ky controla la selectividad rotacional del filtro
 * \return
 */
cv::Mat filter_ridge(const cv::Mat &src,const cv::Mat &orientation_map,const cv::Mat &frequency_map, cv::Mat &mask, float kx = 0.5, float ky = 0.5)
{

    // Fixed angle increment between filter orientations in degrees
    // Deberia ser divisor de 180
    int angleInc = 3;
    cv::Mat im;
    src.convertTo(im, CV_32FC1);
    int orient_blk_sze = im.rows/orientation_map.rows;
    int freq_blk_sze = im.rows/frequency_map.rows;

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
    for(int i = 0; i < valid_points.rows;i++)
    {
        int row = valid_points.at<cv::Point>(i).y;
        int col = valid_points.at<cv::Point>(i).x;
        validr.push_back(row);
        validc.push_back(col);
        float new_freq = static_cast<float>(round(100.0*freq.at<float>(row,col))/100.0);
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

    float freqindex[101] = {0};
    //std::vector<float> freqindex((int)round(unfreq.back()*100.0),0);
    for(size_t i = 0; i < unfreq.size(); i++)
    {
        float f = unfreq[i];
        int index = (int)round(f*100.0);
        freqindex[index] = i;
    }
    //Generate filters corresponding to these distinct frequencies and
    //orientations in 'angleInc' increments.
    cv::Mat filter[unfreq.size()][180/angleInc];
    int sze[unfreq.size()];
    for(size_t i = 0; i < unfreq.size(); i++)
    {
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

        term3 = mat_cos(2.0*M_PI*unfreq[i]*x);

        cv::Mat reffilter = cv::Mat::zeros(x.size(),CV_32FC1);
        cv::exp(-(term1 + term2)/2.0,reffilter);
        reffilter = reffilter.mul(term3);
        //descomentar para guardar los filtros en disco
        //cv::imwrite(std::to_string(i)+"_reffilter.jpg",reffilter*255);

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
            //cv::imwrite(std::to_string(i)+"_"+std::to_string(o)+"_filter.jpg",new_filter*255);
        }

    }
    // Convert orientation matrix values from radians to an index value
    // that corresponds to round(degrees/angleInc)
    int maxOrientIndex = round(180/angleInc);
    cv::Mat orientindex(orient.size(),CV_8UC1);
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
    cv::Mat newim = cv::Mat::zeros(im.size(),im.type());
    //std::cout << "debug: " << 1 << std::endl;
    for(int r = maxsze + 1; r < im.rows - maxsze - 1; r++)
    {
        //std::cout << "debug: r" << r << std::endl;
        for(int c = maxsze + 1; c < im.cols - maxsze - 1; c++)
        {
            //vemos si este punto es valido
            if(freq.at<float>(r/freq_blk_sze,c/freq_blk_sze) > 0)
            {
                //int filterindex = freqindex[(int)trunc(freq.at<float>(r/freq_blk_sze,c/freq_blk_sze)*100)];
                float f = freq.at<float>(r/freq_blk_sze,c/freq_blk_sze);

                int filterindex = freqindex[(int)round(f*100.0)];
                // std::cout << filterindex << std::endl;
                int s = sze[filterindex];
                //std::cout << s << std::endl;


                int orientind = orientindex.at<uchar>(r/orient_blk_sze,c/orient_blk_sze);
                //std::cout << orientind << std::endl;
                cv::Mat subFilter = filter[filterindex][orientind];

                cv::Rect roi(c-s, r-s, subFilter.cols, subFilter.rows);

                cv::Mat subim = im(roi);

                cv::Mat mulResult(subim.size(),subim.type());

                cv::multiply(subim,subFilter,mulResult);

                if(cv::sum(mulResult)[0] > 0)
                {
                    newim.at<float>(r,c) = 255;
                }
            }
        }
    }
    //std::cout <<"borde" << std::endl;
    //ponemos un borde
    newim.rowRange(0, im.rows).colRange(0, maxsze + 1).setTo(255);
    newim.rowRange(0, maxsze + 1).colRange(0, im.cols).setTo(255);
    newim.rowRange(im.rows - maxsze, im.rows).colRange(0, im.cols).setTo(255);
    newim.rowRange(0, im.rows).colRange(im.cols - 1 * (maxsze + 1) - 1, im.cols).setTo(255);
    //actualizamos la mascara
    mask.rowRange(0, mask.rows).colRange(0, maxsze + 1).setTo(0);
    mask.rowRange(0, maxsze + 1).colRange(0, mask.cols).setTo(0);
    mask.rowRange(mask.rows - maxsze, mask.rows).colRange(0, mask.cols).setTo(0);
    mask.rowRange(0, mask.rows).colRange(mask.cols - 1 * (maxsze + 1) - 1, mask.cols).setTo(0);
    return newim;
}

/*!
 * \brief get_oriented_window devuelve una ventana orientada
 * \param im imagen de la cual obtener la ventana orientada
 * \param x posicion x del centro de la ventana
 * \param y posicion y del centro de la ventana
 * \param angle angulo que debemos cancelar
 * \param w ancho de la ventana
 * \param h alto de la ventana
 * \return ventana orientada
 */
cv::Mat get_oriented_window(const cv::Mat &im, int x, int y, float angle, int w, int h)
{
    //rotamos la imagen para cancelar el angulo
    cv::Mat oriented_window(h,w,im.type());
    cv::Mat rot = cv::getRotationMatrix2D(cv::Point2f(x,y),angle/M_PI*180 + 90,1.0);
    cv::Mat rotated;
    cv::warpAffine(im,rotated,rot,im.size());
    //obtenemos una ventana de la imagen rotada
    oriented_window = rotated(cv::Rect(x - w/2,y - h/2,w,h));
    return oriented_window;
}

/*!
 * \brief sig_smooth suavia y elimina ruido de un arreglo de una senial seno.
 * \param sig arreglo a suavizar, reemplaza los valores del mismo
 */
void sig_smooth(std::vector<float> &sig)
{
    for(size_t i = 1; i < sig.size() -1; i++)
    {
        int smaller = sig[i-1] < sig[i+1] ? sig[i-1] : sig[i+1];
        if(smaller > sig[i])
        {
            sig[i] = smaller;
        }
    }
}

/*!
 * \brief calc_freq calcula la frecuencia de un arreglo de valores representando una senial cuasi sinuoidal
 * \param sig signature a la cual calcular la frecuencia
 * \param min minimo valor que debe tener un pico
 * \param min_wavelength minima distancia entre picos
 * \param max_wavelength maxima distancia entre picos
 * \return frecuencia calculada
 */
float calc_freq(std::vector<float> &sig, float min, int min_wavelength, int max_wavelength)
{
    std::vector<int> peak_ind;
    //detectamos los picos
    for(int i = 1; i < (int)sig.size() -1; i++)
    {
        int cur = sig[i];
        if(cur >= min)
        {
            int pre = sig[i-1];
            if(cur > pre)
            {
                int nex = sig[i+1];
                if(!(nex > cur))
                {
                    peak_ind.push_back(i);
                }
            }
        }
    }
    float freq = 0;
    //si encontramos al menos dos picos
    if(peak_ind.size() > 1)
    {
        //calculamos la longitud y vemos si esta dentro de los limites
        float wavelength = (float)(peak_ind.back() - peak_ind[0]) / (float)(peak_ind.size() - 1);
        if(min_wavelength < wavelength && wavelength < max_wavelength)
        {
            //calculamos la frecuencia
            freq = (float)1.0/(float)wavelength;
        }
    }
    return freq;
}
/*!
 * \brief get_block_freq calcula la frecuencia de un bloque. Se supone que esta orientado
 * \param im bloque orientado
 * \param min_wavelength minima distancia entre crestas
 * \param max_wavelength maxima distancia entre crestas
 * \return frecuencia del bloque
 */
float get_block_freq(const cv::Mat &im, int min_wavelength = 5, int max_wavelength = 25)
{
    float freq = 0;
    std::vector<float> sig;
    //sumamos las columnas
    for(int c = 0; c < im.cols; c++)
    {
        float sum = 0;
        for(int r = 0; r < im.rows; r++)
        {
            sum += im.at<float>(r,c);
        }
        sum /=  (float)im.rows;
        sig.push_back(sum);
    }
    cv::Scalar mean = cv::mean(sig);
    //suavizamos la senial
    sig_smooth(sig);
    //calculamos la frecuencia
    freq = calc_freq(sig,mean[0],min_wavelength,max_wavelength);
    return freq;
}

/*!
 * \brief ridge_freq calcula el mapa de frecuencias de una imagen de huella dactilar.
 * Implementa el algoritmo de hong 98
 * \param im imagen normalizada
 * \param angles mapa de orientacion
 * \param f_blk_size tamanio de bloque del mapa de frecuencia, esto tambien establece la ventana orientada como f_blk_sze x 2*f_blk_sze
 * \param min_wavelength minima distancia entre crestas
 * \param max_wavelength maxima distancia entre crestas
 * \param mask mascara opcional
 * \param average flag que indica se el mapa sera un promedio de los bloques, o si cada bloque tiene su propia frecuencia
 * \return mapa de frecuencia
 */
cv::Mat ridge_freq(const cv::Mat &im, const cv::Mat &angles, int f_blk_sze = -1, int min_wavelength = 5, int max_wavelength = 15, const cv::Mat &mask = cv::Mat(), bool average = false)
{
    //creamos valor y estructuras a utilizar
    int o_blk_sze = im.rows / angles.rows;
    int w = f_blk_sze == -1 ? o_blk_sze : f_blk_sze;
    int l = w * 2;
    std::vector<float> frequencies;
    cv::Mat freq = cv::Mat::zeros(im.rows/w,im.cols/w,im.type());
    for(int j = w; j < im.rows - w; j+=w)
    {
        for(int i = w; i < im.cols - w; i+=w)
        {
            //si estamos en una posicion valida
            if(mask.empty() || (!mask.empty() && cv::countNonZero(mask(cv::Rect(i - w/2,j - w/2,w,w))) == w*w))
            {
                float block_orientation = angles.at<float>(j/o_blk_sze,i/o_blk_sze);
                if(block_orientation > 0)
                {
                    //calculamos la ventana orientada
                    cv::Mat oriented_window = get_oriented_window(im,i,j,block_orientation,l,w);
                    //calculamos la frecuencia de esa ventana
                    float block_freq = get_block_freq(oriented_window, min_wavelength, max_wavelength);
                    if(block_freq > 0)
                    {
                        freq.at<float>(j/w,i/w) = block_freq;
                        frequencies.push_back(block_freq);
                    }
                }
            }
        }
    }
    cv::Mat mean_freqs(frequencies);
    cv::Scalar mean = cv::mean(mean_freqs, mean_freqs > 0);

    if(average)
    {
        //seteamos a todo el mapa de frecuencia la frecuencia media
        //esto da un resultado mas estable en muchos casos
        freq.setTo(mean[0]);
    }
    else
    {
        //nos quedamos con el mapa de frecuencias calculado
        //esto es mejor en algunos casos en los que la huella tiene grandes variaciones de frecuencia
        //ademas, le asignamos la frecuencia media a los bloques en los que no se pudo calcular frecuencia
        freq.setTo(mean[0], freq == 0);
    }
    return freq;
}

cv::Mat thin(const cv::Mat &src, int thinning_method)
{
    cv::Mat binary;
    cv::threshold(src,binary,0,255,cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
    cv::Mat thinned = cv::Mat::zeros(binary.size(),binary.type());
    switch(thinning_method)
    {
    case NONE:
    {
        thinned = src.clone();
        break;
    }
    case ZHANGSUEN:
    {
        //zhangsuen_thinning(thinned);
        cv::ximgproc::thinning(binary,thinned,cv::ximgproc::THINNING_ZHANGSUEN);
        cv::threshold(thinned,thinned,0,255,cv::THRESH_BINARY_INV);
        break;
    }
    case MORPH:
    {
        thinned = morphological_thinning(binary);
        cv::threshold(thinned,thinned,0,255,cv::THRESH_BINARY_INV);
        break;
    }
    case GUOHALL:
    {
        //guohall_thinning(thinned);
        cv::ximgproc::thinning(binary,thinned,cv::ximgproc::THINNING_GUOHALL);
        cv::threshold(thinned,thinned,0,255,cv::THRESH_BINARY_INV);
    }
    default:
        break;
    }
    return thinned;
}

//calcula la roi de una imagen a partir de la variacion local
cv::Mat get_roi(const cv::Mat &src,int blk_sze, float threshold_ratio)
{
    cv::Scalar mean,stddev;
    cv::meanStdDev(src,mean,stddev);
    float threshold = stddev[0] * threshold_ratio;
    cv::Mat image_variance = cv::Mat::zeros(src.size(),CV_32FC1);
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

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(2*blk_sze,2*blk_sze));
    cv::morphologyEx(mask,mask,cv::MORPH_OPEN,kernel);
    cv::morphologyEx(mask,mask,cv::MORPH_CLOSE,kernel);
    return mask;

}

Preprocessed Preprocesser::preprocess(const cv::Mat &src)
{
    //pipeline de preprocesamiento

    //convertimos a 32f
    cv::Mat src_32f;
    src.convertTo(src_32f,CV_32FC1);
    //cv::GaussianBlur(src_32f,src_32f,cv::Size(5,5),0);
    cv::Mat sharpened = sharpening(src_32f,2.0);
    sharpened.convertTo(sharpened,CV_8UC1);

    //normalizacion para eliminar ruido e imperfecciones
    qDebug() << "Preprocesser: normalizando para mejorar contraste...";
    cv::Mat norm_req = normalize2(src_32f,norm_req_mean,norm_req_var);
    // cv::normalize(norm_req,norm_req,0,255,cv::NORM_MINMAX);

    //obtencion del roi
    qDebug() << "Preprocesser: calculando roi...";
    cv::Mat mask = get_roi(norm_req,blk_mask,roi_threshold_ratio);

    //normalizacion a media 0 y desviacion unitaria
    qDebug() << "Preprocesser: normalizando a media 0 y desviacion unitaria...";
    cv::Mat norm_m0d1;
    //norm_m0d1 = normalize2(norm_req,0,1);
    norm_m0d1 = normalize(norm_req,0,1,mask);

    //estimacion de la orientacion local
    qDebug() << "Preprocesser: calculando mapa de orientacion...";
    cv::Mat angles = calculate_angles(norm_req,blk_orient,1,1,mask);

    //todo, mapa de frecuencia
    qDebug() << "Preprocesser: calculando mapa de frecuencias...";
    cv::Mat freq = ridge_freq(norm_req,angles,blk_freq,5,15,mask,true);
    //filtro
    qDebug() << "Preprocesser: armando y aplicando filtros orientados de Gabor...";
    cv::Mat filtered;
    filtered = filter_ridge(norm_m0d1,angles,freq,mask,gabor_kx,gabor_ky);


    filtered.convertTo(filtered,CV_8UC1);
    //esqueletizamos la imagen
    qDebug() << "Preprocesser: esqueletizamos...";
    cv::Mat thinned = thin(filtered,thinning_method);
    cv::Mat result = cv::Mat::zeros(thinned.size(),thinned.type());
    if(segment)
    {
        cv::Mat mask_inv;
        cv::bitwise_not(mask,mask_inv);
        result = thinned + mask_inv;
    }
    else
    {
        result = thinned;
    }

    Preprocessed pre;
    pre.original = src;
    pre.normalized = norm_req;
    pre.roi = mask;
    pre.orientation = angles;
    pre.frequency = freq;
    pre.filtered = filtered;
    pre.thinned = thinned;
    pre.grayscale = sharpened;
    pre.result = result;

    qDebug() << "Preprocess:: listo.";
    return pre;

}
