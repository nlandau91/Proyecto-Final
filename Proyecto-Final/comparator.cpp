#include "comparator.h"
#include <QDebug>

namespace fp
{
Comparator::Comparator()
{

}

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
std::vector<std::vector<cv::DMatch>> matches_orb(const cv::Mat &descriptors, const std::vector<cv::Mat> &lista_descriptores, int norm_type)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(norm_type, true);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > all_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<cv::DMatch> matches;
        matcher.match(lista_descriptores[entry],descriptors,matches);
        all_matches.push_back(matches);
    }
    return all_matches;
}

//busca matches entre descriptores de una imagen y una un arreglo de descriptores
//utiliza el test del ratio de lowe
std::vector<std::vector<cv::DMatch>> matches_orb(const cv::Mat &descriptors, const std::vector<cv::Mat> &lista_descriptores, int norm_type, float lowe_ratio)
{
    // Create the matcher interface
    cv::BFMatcher matcher = cv::BFMatcher(norm_type);
    // Now loop over the database and start the matching
    std::vector< std::vector< cv::DMatch > > good_matches;
    for(long unsigned entry=0; entry<lista_descriptores.size();entry++){
        std::vector<cv::DMatch> matches;
        //matcher.match(lista_descriptores[entry],descriptors,matches);
        std::vector<std::vector<cv::DMatch>> all_matches;
        matcher.knnMatch(lista_descriptores[entry],descriptors,all_matches,2);
        for(std::vector<cv::DMatch> match_pair : all_matches)
        {
            //test de ratio de lowe
            if(match_pair[0].distance < lowe_ratio*match_pair[1].distance)
            {
                matches.push_back(match_pair[0]);
            }
        }
        good_matches.push_back(matches);
    }
    return good_matches;
}

//std::vector<std::vector<cv::DMatch>> matches_surf(const cv::Mat &descriptors, const std::vector<cv::Mat> &lista_descriptores, int norm_type)
//{

//}

std::vector<std::vector<cv::DMatch>> obtener_matches(const cv::Mat &descriptors, const std::vector<cv::Mat> &lista_descriptores, int matcher_method)
{
    std::vector<std::vector<cv::DMatch>> matches;
    switch(matcher_method)
    {
    case ORB:
    {
        qDebug() << "orb";
        matches = matches_orb(descriptors,lista_descriptores,cv::NORM_HAMMING,0.75);
        break;
    }
    case SURF:
    {
        qDebug() << "surf";
        matches = matches_orb(descriptors,lista_descriptores,cv::NORM_L2,0.75);
        break;
    }
    }
    return matches;
}

int Comparator::compare(const cv::Mat &descriptors,const  std::vector<cv::Mat> &lista_descriptores)
{
    int lowest_dist = 999;
    std::vector<std::vector<cv::DMatch>> all_matches;
    qDebug() << "Comparator: obteniendo matches...";
    all_matches = obtener_matches(descriptors, lista_descriptores,matcher_method);
    qDebug() << "Comparator: decidiendo mejor match...";
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
            std::cout << dist << std::endl;
            if(dist < lowest_dist)
            {
                lowest_dist = dist;
            }

        }
    }
    qDebug() << "Comparator: listo.";
    return lowest_dist;
}
}
