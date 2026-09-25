#ifndef CODEFORCES_PROVIDER_H
#define CODEFORCES_PROVIDER_H

#include <string>

class CodeforcesProvider
{
public:
    // Name, rank and new rating of every rated contest
    std::string fetchRatingHistory(
        const std::string &username);

    // All submissions of the user
    std::string fetchSubmissions(
        const std::string &username);

    // All Codeforces contests with their start time
    std::string fetchContestList();

    // Beginning of contest.standings, up to and including the problem list
    std::string fetchContestProblems(int contestId);
};

#endif
