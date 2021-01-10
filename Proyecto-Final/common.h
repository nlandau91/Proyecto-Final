#ifndef COMMON_H
#define COMMON_H

#include "preprocesser.h"
#include "analyzer.h"
#include "comparator.h"
#include "database.h"
#include "utils.h"
#include "configdialog.h"
#include "appsettings.h"

#include <opencv2/opencv.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>
#include <QDebug>

namespace fp
{
enum Params
{
    NONE,
    GABOR,
    ZHANGSUEN,
    MORPH,
    GUOHALL,
    HARRIS,
    SHITOMASI,
    SURF,
    ORB,
    SIFT,
    BRUTEFORCE,
    FLANN,

};
}

#endif // COMMON_H
