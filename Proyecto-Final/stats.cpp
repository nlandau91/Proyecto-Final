#include "stats.h"
namespace fp{

double get_mean(const std::vector<double> &scores, bool ignore_extremes)
{
    //calculamos la media
    double mean = 0;
    int n = 0;
    for(const double score : scores)
    {
        if(!ignore_extremes || (score > 0 && score < 1))
        {
            mean += score;
            n++;
        }
    }
    mean = mean/n;
    return mean;
}

double get_low_pcnt(std::vector<double> scores, double thresh)
{
    //calculamos el % bajo
    std::sort(scores.begin(),scores.end());
    int low_pcnt_index = std::trunc((scores.size() - 1) * thresh);
    double low_pcnt = scores[low_pcnt_index];
    return low_pcnt;
}

double get_high_pcnt(std::vector<double> scores, double thresh)
{
    //calculamos el % alto
    std::sort(scores.begin(),scores.end());
    int high_pcnt_index = std::trunc((scores.size() - 1) * (1-thresh));
    double high_pcnt = scores[high_pcnt_index];
    return high_pcnt;
}

double get_eer(std::vector<double> scores1, std::vector<double> scores2)
{
    double eer = 1.0;
    //tenemos que ver en que porcentaje de los vectores se cruzan los valores
    std::sort(scores1.begin(),scores1.end());
    std::sort(scores2.begin(),scores2.end());
    double min_dif = 1.0;
    for(int i = 0; i <= 100; i++)
    {
        double frr_pcnt = (double)i / 100.0;
        double far_pcnt = 1 - frr_pcnt;
        int index1 = std::trunc((scores1.size() -1) * frr_pcnt);
        int index2 = std::trunc((scores2.size() -1) * far_pcnt);
        double val1 = scores1[index1];
        double val2 = scores2[index2];
        double dif = std::abs(val1-val2);
        if(dif < min_dif)
        {
            min_dif = dif;
            //double eer_value = (val1+val2)/2.0;
            eer = frr_pcnt;
        }
    }
    return eer;
}

}
