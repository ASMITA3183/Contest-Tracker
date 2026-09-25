#include "LeetCodeParser.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cmath>
#include <ctime>
#include <stdexcept>

using json = nlohmann::json;

std::vector<Contest> LeetCodeParser::parse(
    const std::string &response)
{
    std::vector<Contest> contests;

    if (response.empty())
    {
        throw std::runtime_error("Could not connect to LeetCode");
    }

    json data = json::parse(response, nullptr, false);

    if (data.is_discarded())
    {
        throw std::runtime_error("Unexpected response from LeetCode");
    }

    // matchedUser is null when the username does not exist
        // matchedUser is null when the username does not exist
    if (data["data"]["matchedUser"].is_null())
    {
        // LeetCode explains the problem in "errors", e.g. "That user does not exist."
        std::string message = "Unexpected response from LeetCode";

        if (data.contains("errors") && data["errors"].is_array() && !data["errors"].empty())
        {
            message = data["errors"][0].value("message", message);
        }

        if (message.find("does not exist") != std::string::npos)
        {
            message = "User not found";
        }

        throw std::runtime_error(message);
    }

    const auto &history =
        data["data"]["userContestRankingHistory"];

    // User exists but has not taken part in any contest
    if (!history.is_array())
    {
        return contests;
    }

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
