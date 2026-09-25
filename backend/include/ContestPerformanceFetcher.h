#ifndef CONTEST_PERFORMANCE_FETCHER_H
#define CONTEST_PERFORMANCE_FETCHER_H

#include <string>

struct ContestStats
{
    int solved;
    int totalQuestions;
};

class ContestPerformanceFetcher
{
public:
    ContestStats getContestStats(
        const std::string &contestCode,
        const std::string &username);
};

#endif