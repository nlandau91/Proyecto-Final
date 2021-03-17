#ifndef TESTER_H
#define TESTER_H

#include "database.h"
#include "preprocesser.h"
#include "analyzer.h"
#include "comparator.h"

namespace fp
{
class Tester
{
public:
    Tester();
    void load_database(Database &db);
    double test_far(const Database &db);
    double test_frr(const Database &db);

    Preprocesser preprocesser;
    Analyzer analyzer;
    Comparator comparator;
};
}
#endif // TESTER_H
