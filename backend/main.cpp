#include "LeetCodeProvider.h"
#include "LeetCodeParser.h"

#include "httplib.h"
#include <nlohmann/json.hpp>

#include <iostream>
#include <algorithm>

using json = nlohmann::json;

int main()
{
    LeetCodeProvider provider;
    LeetCodeParser parser;

    std::string response =
        provider.fetchContestData("asmita3183");

    std::vector<Contest> contests =
        parser.parse(response);

    std::sort(
        contests.begin(),
        contests.end(),
        [](const Contest &a, const Contest &b)
        {
            return a.dateTime > b.dateTime;
        });

    httplib::Server server;

    server.set_default_headers({{"Access-Control-Allow-Origin", "*"}});

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

                       std::time_t time =
                           std::chrono::system_clock::to_time_t(
                               contest.dateTime);

                       item["dateTime"] =
                           std::ctime(&time);

                       item["solved"] = contest.solved;
                       item["totalQuestions"] =
                           contest.totalQuestions;
                       item["rank"] = contest.rank;

                       result.push_back(item);
                   }

                   res.set_content(
                       result.dump(),
                       "application/json");
               });

    std::cout << "Server running at http://localhost:8080\n";

    server.listen("localhost", 8080);

    return 0;
}