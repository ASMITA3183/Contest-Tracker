#include "ContestPerformanceFetcher.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <ctime>
#include <iostream>
#include <set>
#include <string>
#include <utility>

using json = nlohmann::json;

namespace
{
    size_t writeCallback(
        void *contents,
        size_t size,
        size_t nmemb,
        void *userData)
    {
        size_t totalSize = size * nmemb;

        std::string *response =
            static_cast<std::string *>(userData);

        response->append(
            static_cast<char *>(contents),
            totalSize);

        return totalSize;
    }

    // Fetches https://www.codechef.com/api/contests/<contestCode>
    // which contains the contest problems and start time.
    std::string fetchContestJson(
        const std::string &contestCode)
    {
        CURL *curl = curl_easy_init();

        if (!curl)
        {
            std::cerr << "Failed to initialize CURL\n";
            return "";
        }

        std::string response;

        std::string url =
            "https://www.codechef.com/api/contests/" + contestCode;

        curl_easy_setopt(
            curl,
            CURLOPT_URL,
            url.c_str());

        curl_easy_setopt(
            curl,
            CURLOPT_FOLLOWLOCATION,
            1L);

        curl_easy_setopt(
            curl,
            CURLOPT_USERAGENT,
            "Mozilla/5.0");

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            writeCallback);

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEDATA,
            &response);

        CURLcode result =
            curl_easy_perform(curl);

        if (result != CURLE_OK)
        {
            std::cerr << "CodeChef contest request failed for "
                      << contestCode << ": "
                      << curl_easy_strerror(result)
                      << '\n';

            curl_easy_cleanup(curl);

            return "";
        }

        curl_easy_cleanup(curl);

        return response;
    }

    // Profile HTML escapes characters like & and ',
    // while the contest API returns them as plain text.
    std::string decodeHtml(std::string text)
    {
        const std::pair<std::string, std::string> entities[] =
            {
                {"&quot;", "\""},
                {"&#039;", "'"},
                {"&#39;", "'"},
                {"&lt;", "<"},
                {"&gt;", ">"},
                {"&amp;", "&"}};

        for (const auto &entity : entities)
        {
            size_t pos = 0;

            while ((pos = text.find(entity.first, pos)) != std::string::npos)
            {
                text.replace(
                    pos,
                    entity.first.size(),
                    entity.second);

                pos += entity.second.size();
            }
        }

        return text;
    }

    // The profile's "Problems Solved -> Contests" section looks like:
    // <h5><span ...>Starters 90 Division 4 (Rated)</span></h5>
    // <p><span><span style="font-size: 12px";>Learning SQL</span>, ...</span></p>
    std::set<std::string> findSolvedProblemNames(
        const std::string &profileHtml,
        const std::string &contestName)
    {
        std::set<std::string> names;

        const std::string header =
            ">" + contestName + "</span></h5>";

        size_t start = profileHtml.find(header);

        if (start == std::string::npos)
        {
            // Contest attended but nothing solved.
            return names;
        }

        size_t end = profileHtml.find("</p>", start);

        if (end == std::string::npos)
        {
            return names;
        }

        const std::string nameStart = "12px\";>";
        const std::string nameEnd = "</span>";

        size_t pos = profileHtml.find(nameStart, start);

        while (pos != std::string::npos && pos < end)
        {
            pos += nameStart.size();

            size_t close = profileHtml.find(nameEnd, pos);

            if (close == std::string::npos || close > end)
            {
                break;
            }

            names.insert(decodeHtml(
                profileHtml.substr(pos, close - pos)));

            pos = profileHtml.find(nameStart, close);
        }

        return names;
    }
}

ContestStats ContestPerformanceFetcher::getContestStats(
    const std::string &contestCode,
    const std::string &contestName,
    const std::string &profileHtml)
{
    ContestStats stats;

    std::string response =
        fetchContestJson(contestCode);

    if (response.empty())
    {
        return stats;
    }

    try
    {
        json data = json::parse(response);

        if (data.value("status", "") != "success")
        {
            std::cerr << "CodeChef contest API returned an error for "
                      << contestCode << '\n';
            return stats;
        }

        // 1. Date & time: contest start as a Unix timestamp
        std::time_t startTime =
            data.at("time").at("start").get<std::time_t>();

        stats.dateTime =
            std::chrono::system_clock::from_time_t(startTime);

        // 2. Problems solved by the user in this contest
        std::set<std::string> solvedNames =
            findSolvedProblemNames(profileHtml, contestName);

        // 3. Count scored problems and how many of them were solved
        const json &problems = data["problems"];

        if (problems.is_object())
        {
            for (const auto &problem : problems)
            {
                // "unscored" problems do not count towards the contest
                if (problem.value("category_name", "main") == "unscored")
                {
                    continue;
                }

                stats.totalQuestions++;

                if (solvedNames.count(problem.value("name", "")))
                {
                    stats.solved++;
                }
            }
        }
    }
    catch (const json::exception &e)
    {
        std::cerr << "Parsing contest " << contestCode
                  << " failed: " << e.what() << '\n';
    }

    return stats;
}
