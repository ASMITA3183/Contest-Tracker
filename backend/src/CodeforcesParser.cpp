#include "CodeforcesParser.h"
#include "CodeforcesProvider.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <ctime>
#include <future>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>

using json = nlohmann::json;

namespace
{
    // Parses a Codeforces API response, returns null if it failed
    json parseApiResponse(
        const std::string &response,
        const std::string &apiName)
    {
        json data = json::parse(response, nullptr, false);

        if (data.is_discarded() || data.value("status", "") != "OK")
        {
            std::cerr << "Codeforces " << apiName << " failed";

            if (!data.is_discarded())
            {
                std::cerr << ": " << data.value("comment", "");
            }

            std::cerr << '\n';

            return nullptr;
        }

        return data["result"];
    }

    // standingsStart looks like:
    // {"status":"OK","result":{"contest":{...},"problems":[{..."index":"A"...},...],"rows":
    // It is cut off after "rows", so it is not valid JSON; count "index" entries instead.
    // problemset.problems is not used because in Div. 1 / Div. 2 rounds
    // it lists shared problems only under the Div. 1 contest.
    int countProblems(const std::string &standingsStart)
    {
        size_t end = standingsStart.find("\"rows\":");

        if (end == std::string::npos)
        {
            return 0;
        }

        const std::string key = "\"index\":\"";

        std::set<std::string> indexes;

        size_t pos = standingsStart.find(key);

        while (pos != std::string::npos && pos < end)
        {
            pos += key.size();

            size_t close = standingsStart.find('"', pos);

            if (close == std::string::npos || close > end)
            {
                break;
            }

            indexes.insert(standingsStart.substr(pos, close - pos));

            pos = standingsStart.find(key, close);
        }

        return static_cast<int>(indexes.size());
    }
}

std::vector<Contest> CodeforcesParser::parse(
    const std::string &ratingResponse,
    const std::string &submissionsResponse,
    const std::string &contestListResponse)
{
    std::vector<Contest> contests;

    if (ratingResponse.empty())
    {
        throw std::runtime_error("Could not connect to Codeforces");
    }

    json ratingData = json::parse(ratingResponse, nullptr, false);

    if (ratingData.is_discarded())
    {
        throw std::runtime_error("Unexpected response from Codeforces");
    }

    if (ratingData.value("status", "") != "OK")
    {
        std::string comment = ratingData.value("comment", "");

        // e.g. "handle: User abc not found"
        if (comment.find("not found") != std::string::npos)
        {
            throw std::runtime_error("User not found");
        }

        throw std::runtime_error(comment);
    }

    json ratingHistory = ratingData["result"];

    // 1. Start time of every contest
    std::map<int, std::time_t> startTimes;

    json contestList =
        parseApiResponse(contestListResponse, "contest.list");

    if (!contestList.is_null())
    {
        for (const auto &item : contestList)
        {
            startTimes[item.value("id", 0)] =
                item.value("startTimeSeconds", std::time_t{0});
        }
    }

    // 2. Problems solved during each contest:
    // accepted submissions made as a contestant (not practice / virtual)
    std::map<int, std::set<std::string>> solvedProblems;

    json submissions =
        parseApiResponse(submissionsResponse, "user.status");

    if (!submissions.is_null())
    {
        for (const auto &submission : submissions)
        {
            if (submission.value("verdict", "") != "OK")
            {
                continue;
            }

            if (submission.value(json::json_pointer("/author/participantType"), "") != "CONTESTANT")
            {
                continue;
            }

            solvedProblems[submission.value("contestId", 0)].insert(
                submission.value(json::json_pointer("/problem/index"), ""));
        }
    }

    // 3. One Contest for every rated contest
    std::vector<int> contestIds;

    for (const auto &item : ratingHistory)
    {
        int contestId = item.value("contestId", 0);

        Contest contest;

        contest.platform = "Codeforces";

        contest.contestName = item.value("contestName", "");

        contest.rank = item.value("rank", 0);

        contest.rating = item.value("newRating", 0);

        contest.dateTime =
            std::chrono::system_clock::from_time_t(startTimes[contestId]);

        contest.solved =
            static_cast<int>(solvedProblems[contestId].size());

        contest.totalQuestions = 0;

        contests.push_back(contest);
        contestIds.push_back(contestId);
    }

    // 4. Total questions of every contest.
    // Requests run in parallel, a few at a time, because the Codeforces API
    // answers "Call limit exceeded" when called too often.
    const size_t batchSize = 4;
    const int maxAttempts = 3;

    CodeforcesProvider provider;

    for (size_t batchStart = 0; batchStart < contests.size(); batchStart += batchSize)
    {
        size_t batchEnd = std::min(
            batchStart + batchSize,
            contests.size());

        std::vector<std::future<int>> results;

        for (size_t i = batchStart; i < batchEnd; i++)
        {
            int contestId = contestIds[i];

            results.push_back(std::async(
                std::launch::async,
                [&provider, contestId, maxAttempts]()
                {
                    for (int attempt = 1; attempt <= maxAttempts; attempt++)
                    {
                        int total = countProblems(
                            provider.fetchContestProblems(contestId));

                        if (total > 0)
                        {
                            return total;
                        }

                        // Probably "Call limit exceeded", wait and try again
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }

                    std::cerr << "Could not get problems of Codeforces contest "
                              << contestId << '\n';

                    return 0;
                }));
        }

        for (size_t i = batchStart; i < batchEnd; i++)
        {
            contests[i].totalQuestions =
                results[i - batchStart].get();
        }
    }

    return contests;
}
