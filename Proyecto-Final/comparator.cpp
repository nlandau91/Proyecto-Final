#include "comparator.h"

namespace fp
{
Comparator::Comparator()
{

}

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
std::vector<std::vector<cv::DMatch>> obtener_matches(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(cv::NORM_HAMMING, true);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > all_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<cv::DMatch> matches;
        matcher.match(lista_descriptores[entry],descriptors,matches);
        all_matches.push_back(matches);
    }
    return all_matches;
}

int Comparator::compare(cv::Mat &descriptors, std::vector<cv::Mat> &lista_descriptores)
{
    int lowest_dist = 999;
    std::vector<std::vector<cv::DMatch>> all_matches;
    all_matches = obtener_matches(descriptors, lista_descriptores);
    for(std::vector<cv::DMatch> vm : all_matches)
    {
        if(vm.size() > 0)
        {
            float dist = 0.0;
            float sum = 0.0;
            for(cv::DMatch m : vm)
            {
                sum = sum + m.distance;
            }
            dist = sum/vm.size();
            if(dist < lowest_dist)
            {
                lowest_dist = dist;
            }
            //std::cout << score << std::endl;
        }
    }
    return lowest_dist;
}
}
