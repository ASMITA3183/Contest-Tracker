#include "CodeChefParser.h"
#include "ContestPerformanceFetcher.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

std::vector<Contest> CodeChefParser::parse(const std::string &response)
{
    std::vector<Contest> contests;

    if (response.empty())
    {
        throw std::runtime_error("Could not connect to CodeChef");
    }

    // Unknown usernames are redirected to the CodeChef home page,
    // which does not have the profile details block.
    if (response.find("user-details-container") == std::string::npos)
    {
        throw std::runtime_error("User not found");
    }

    // 1. Find the beginning of all_rating
    const std::string marker = "var all_rating =";

    size_t start = response.find(marker);

    if (start == std::string::npos)
    {
        // User exists but has no rated contests
        return contests;
    }

    // Move to the '[' of the JSON array
    start = response.find('[', start);

    if (start == std::string::npos)
    {
        std::cerr << "Could not find all_rating array\n";
        return contests;
    }

    // 2. Find the end of the array
    size_t end = response.find("];", start);

    if (end == std::string::npos)
    {
        std::cerr << "Could not find end of all_rating array\n";
        return contests;
    }

    // 3. Extract only the JSON array
    std::string jsonData = response.substr(
        start,
        end - start + 1);

    // Contest codes (e.g. "START90D"), same order as contests
    std::vector<std::string> contestCodes;

    try
    {
        json data = json::parse(jsonData);

        // 4. Parse every contest
        for (const auto &item : data)
        {
            Contest contest;

            contest.platform = "CodeChef";

            contest.contestName = item.value("name", "");

            contest.rank = std::stoi(
                item.value("rank", "0"));

            contest.rating = std::stoi(
                item.value("rating", "0"));

            contests.push_back(contest);
            contestCodes.push_back(item.value("code", ""));
        }
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parsing failed: "
                  << e.what() << '\n';

        return contests;
    }

    // 5. Fetch solved, total questions and date for every contest.
    // Requests run in parallel, a few at a time, so CodeChef does not block us.
    const size_t batchSize = 8;

    ContestPerformanceFetcher fetcher;

    for (size_t batchStart = 0; batchStart < contests.size(); batchStart += batchSize)
    {
        size_t batchEnd = std::min(
            batchStart + batchSize,
            contests.size());

        std::vector<std::future<ContestStats>> results;

        for (size_t i = batchStart; i < batchEnd; i++)
        {
            std::string code = contestCodes[i];
            std::string name = contests[i].contestName;

            results.push_back(std::async(
                std::launch::async,
                [&fetcher, &response, code, name]()
                {
                    return fetcher.getContestStats(
                        code,
                        name,
                        response);
                }));
        }

        for (size_t i = batchStart; i < batchEnd; i++)
        {
            ContestStats stats = results[i - batchStart].get();

            contests[i].solved = stats.solved;
            contests[i].totalQuestions = stats.totalQuestions;
            contests[i].dateTime = stats.dateTime;
        }
    }

    return contests;
}
