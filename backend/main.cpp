#include "CodeChefParser.h"
#include "CodeChefProvider.h"
#include "CodeforcesParser.h"
#include "CodeforcesProvider.h"
#include "LeetCodeParser.h"
#include "LeetCodeProvider.h"

#include "httplib.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <vector>

using json = nlohmann::json;

namespace
{
    // Usernames go into URLs and the LeetCode query,
    // so only letters, digits, '_', '-' and '.' are allowed.
    bool isValidUsername(const std::string &username)
    {
        if (username.size() > 40)
        {
            return false;
        }

        return std::all_of(
            username.begin(),
            username.end(),
            [](unsigned char c)
            {
                return std::isalnum(c) || c == '_' || c == '-' || c == '.';
            });
    }

    std::vector<Contest> fetchLeetCode(const std::string &username)
    {
        LeetCodeProvider provider;
        LeetCodeParser parser;

        return parser.parse(provider.fetchContestData(username));
    }

    std::vector<Contest> fetchCodeChef(const std::string &username)
    {
        CodeChefProvider provider;
        CodeChefParser parser;

        return parser.parse(provider.fetchContestData(username));
    }

    std::vector<Contest> fetchCodeforces(const std::string &username)
    {
        CodeforcesProvider provider;
        CodeforcesParser parser;

        return parser.parse(
            provider.fetchRatingHistory(username),
            provider.fetchSubmissions(username),
            provider.fetchContestList());
    }

    json toJson(const Contest &contest)
    {
        json item;

        item["platform"] = contest.platform;
        item["contestName"] = contest.contestName;

        // Unix timestamp in seconds, formatted by the frontend
        item["dateTime"] =
            std::chrono::duration_cast<std::chrono::seconds>(
                contest.dateTime.time_since_epoch())
                .count();

        item["solved"] = contest.solved;
        item["totalQuestions"] = contest.totalQuestions;
        item["rank"] = contest.rank;
        item["rating"] = contest.rating;

        return item;
    }

    struct Platform
    {
        std::string name;
        std::string username;
        std::vector<Contest> (*fetch)(const std::string &);
    };
}

int main()
{
    // Must be called once before curl is used from multiple threads
    curl_global_init(CURL_GLOBAL_DEFAULT);

    httplib::Server server;

    // Serve index.html, style.css and script.js from the frontend folder
    server.set_mount_point("/", "../frontend");

    // GET /contests?leetcode=<name>&codechef=<name>&codeforces=<name>
    // Empty usernames are skipped.
    // Returns {"contests": [...], "errors": [{"platform": ..., "message": ...}]}
    server.Get("/contests",
               [](const httplib::Request &req,
                  httplib::Response &res)
               {
                   std::vector<Platform> platforms =
                       {
                           {"LeetCode", req.get_param_value("leetcode"), fetchLeetCode},
                           {"CodeChef", req.get_param_value("codechef"), fetchCodeChef},
                           {"Codeforces", req.get_param_value("codeforces"), fetchCodeforces}};

                   json errors = json::array();

                   // 1. Fetch all platforms in parallel
                   std::vector<std::future<std::vector<Contest>>> results(platforms.size());

                   for (size_t i = 0; i < platforms.size(); i++)
                   {
                       const Platform &platform = platforms[i];

                       if (platform.username.empty())
                       {
                           continue;
                       }

                       if (!isValidUsername(platform.username))
                       {
                           errors.push_back(json{
                               {"platform", platform.name},
                               {"message", "Invalid username \"" + platform.username + "\""}});
                           continue;
                       }

                       results[i] = std::async(
                           std::launch::async,
                           platform.fetch,
                           platform.username);
                   }

                   // 2. Collect contests, and errors such as "User not found"
                   std::vector<Contest> contests;

                   for (size_t i = 0; i < platforms.size(); i++)
                   {
                       if (!results[i].valid())
                       {
                           continue;
                       }

                       try
                       {
                           std::vector<Contest> platformContests = results[i].get();

                           contests.insert(
                               contests.end(),
                               platformContests.begin(),
                               platformContests.end());
                       }
                       catch (const std::exception &e)
                       {
                           errors.push_back(json{
                               {"platform", platforms[i].name},
                               {"message", e.what()}});
                       }
                   }

                   // 3. Newest contest first
                   std::sort(
                       contests.begin(),
                       contests.end(),
                       [](const Contest &a, const Contest &b)
                       {
                           return a.dateTime > b.dateTime;
                       });

                   json contestsJson = json::array();

                   for (const Contest &contest : contests)
                   {
                       contestsJson.push_back(toJson(contest));
                   }

                   json result;
                   result["contests"] = contestsJson;
                   result["errors"] = errors;

                   res.set_content(
                       result.dump(),
                       "application/json");
               });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    curl_global_cleanup();

    return 0;
}
