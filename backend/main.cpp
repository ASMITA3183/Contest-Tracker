#include "CodeChefParser.h"
#include "CodeChefProvider.h"
#include "CodeforcesParser.h"
#include "CodeforcesProvider.h"
#include "LeetCodeParser.h"
#include "LeetCodeProvider.h"

#include "httplib.h"
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <vector>

using json = nlohmann::json;

const std::string LEETCODE_USERNAME = "asmita3183";
const std::string CODECHEF_USERNAME = "asmitamhetre20";
const std::string CODEFORCES_USERNAME = "asmita3183";

int main()
{
    std::vector<Contest> contests;

    // 1. LeetCode contests
    try
    {
        LeetCodeProvider provider;
        LeetCodeParser parser;

        std::vector<Contest> leetCodeContests =
            parser.parse(provider.fetchContestData(LEETCODE_USERNAME));

        contests.insert(
            contests.end(),
            leetCodeContests.begin(),
            leetCodeContests.end());

        std::cout << "LeetCode contests: "
                  << leetCodeContests.size() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "LeetCode failed: " << e.what() << '\n';
    }

    // 2. CodeChef contests
    try
    {
        CodeChefProvider provider;
        CodeChefParser parser;

        std::vector<Contest> codeChefContests =
            parser.parse(provider.fetchContestData(CODECHEF_USERNAME));

        contests.insert(
            contests.end(),
            codeChefContests.begin(),
            codeChefContests.end());

        std::cout << "CodeChef contests: "
                  << codeChefContests.size() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "CodeChef failed: " << e.what() << '\n';
    }

        // 3. Codeforces contests
    try
    {
        CodeforcesProvider provider;
        CodeforcesParser parser;

        std::vector<Contest> codeforcesContests =
            parser.parse(
                provider.fetchRatingHistory(CODEFORCES_USERNAME),
                provider.fetchSubmissions(CODEFORCES_USERNAME),
                provider.fetchContestList());

        contests.insert(
            contests.end(),
            codeforcesContests.begin(),
            codeforcesContests.end());

        std::cout << "Codeforces contests: "
                  << codeforcesContests.size() << '\n';
    }
    catch (const std::exception &e)
    {
        std::cerr << "Codeforces failed: " << e.what() << '\n';
    }

    // 3. Newest contest first
    std::sort(
        contests.begin(),
        contests.end(),
        [](const Contest &a, const Contest &b)
        {
            return a.dateTime > b.dateTime;
        });

    httplib::Server server;

    // Serve index.html, style.css and script.js from the frontend folder
    server.set_mount_point("/", "../frontend");

    server.Get("/contests",
               [&contests](const httplib::Request &req,
                           httplib::Response &res)
               {
                   json result = json::array();

                   for (const Contest &contest : contests)
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
                       item["totalQuestions"] =
                           contest.totalQuestions;
                       item["rank"] = contest.rank;
                       item["rating"] = contest.rating;

                       result.push_back(item);
                   }

                   res.set_content(
                       result.dump(),
                       "application/json");
               });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("0.0.0.0", 8080);

    return 0;
}
