#ifndef CONTEST_PERFORMANCE_FETCHER_H
#define CONTEST_PERFORMANCE_FETCHER_H

#include <chrono>
#include <string>

struct ContestStats
{
    int solved = 0;
    int totalQuestions = 0;

    std::chrono::system_clock::time_point dateTime;
};

class ContestPerformanceFetcher
{
public:
    // contestCode: "code" from all_rating, e.g. "START90D"
    // contestName: "name" from all_rating, e.g. "Starters 90 Division 4 (Rated)"
    // profileHtml: CodeChef profile page already fetched by CodeChefProvider
    ContestStats getContestStats(
        const std::string &contestCode,
        const std::string &contestName,
        const std::string &profileHtml);
};

#endif
