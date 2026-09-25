#include "LeetCodeParser.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <ctime>

using json = nlohmann::json;

std::vector<Contest> LeetCodeParser::parse(
    const std::string &response)
{
    std::vector<Contest> contests;

    json data = json::parse(response);

    const auto &history =
        data["data"]["userContestRankingHistory"];

    for (const auto &contestData : history)
    {
        if (!contestData["attended"].get<bool>())
        {
            continue;
        }

        Contest contest;

        contest.platform = "LeetCode";

        contest.contestName =
            contestData["contest"]["title"].get<std::string>();

        std::time_t startTime =
            contestData["contest"]["startTime"].get<std::time_t>();

        contest.dateTime =
            std::chrono::system_clock::from_time_t(startTime);

        contest.solved =
            contestData["problemsSolved"].get<int>();

        contest.totalQuestions =
            contestData["totalProblems"].get<int>();

        contest.rank =
            contestData["ranking"].get<int>();

        contest.rating = static_cast<int>(
            std::round(contestData["rating"].get<double>()));

        contests.push_back(contest);
    }

    return contests;
}