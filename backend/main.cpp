#include <curl/curl.h>

#include <iostream>
#include <string>

size_t WriteCallback(
    void *contents,
    size_t size,
    size_t nmemb,
    void *userp)
{
    size_t totalSize = size * nmemb;

    std::string *response =
        static_cast<std::string *>(userp);

    response->append(
        static_cast<char *>(contents),
        totalSize);

    return totalSize;
}

int main()
{
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        std::cerr << "Failed to initialize CURL\n";
        return 1;
    }

    std::string response;

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        "https://www.codechef.com/sites/all/themes/abessive/js/contest-problem.js");

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        WriteCallback);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response);

    curl_easy_setopt(
        curl,
        CURLOPT_FOLLOWLOCATION,
        1L);

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cerr << "CURL failed: "
                  << curl_easy_strerror(result)
                  << '\n';

        curl_easy_cleanup(curl);
        return 1;
    }

    curl_easy_cleanup(curl);

    std::cout << "JS size: "
              << response.size()
              << "\n\n";

    std::string keywords[] =
        {
            "isProblemSolved",
            "contest",
            "submission",
            "ajax",
            "api",
            "status",
            "problem"};

    for (const std::string &keyword : keywords)
    {
        size_t pos = response.find(keyword);

        std::cout << "\n=============================\n";
        std::cout << "Keyword: " << keyword << '\n';
        std::cout << "Position: " << pos << '\n';

        if (pos != std::string::npos)
        {
            size_t start =
                (pos > 1000) ? pos - 1000 : 0;

            std::cout << response.substr(
                             start,
                             3000)
                      << '\n';
        }
    }

    return 0;
}

// #include "LeetCodeProvider.h"
// #include "LeetCodeParser.h"

// #include "httplib.h"
// #include <nlohmann/json.hpp>

// #include <iostream>
// #include <algorithm>

// using json = nlohmann::json;

// int main()
// {
//     LeetCodeProvider provider;
//     LeetCodeParser parser;

//     std::string response =
//         provider.fetchContestData("asmita3183");

//     std::vector<Contest> contests =
//         parser.parse(response);

//     std::sort(
//         contests.begin(),
//         contests.end(),
//         [](const Contest &a, const Contest &b)
//         {
//             return a.dateTime > b.dateTime;
//         });

//     httplib::Server server;

//     server.set_default_headers({{"Access-Control-Allow-Origin", "*"}});

//     server.Get("/contests",
//                [&contests](const httplib::Request &req,
//                            httplib::Response &res)
//                {
//                    json result = json::array();

//                    for (const Contest &contest : contests)
//                    {
//                        json item;

//                        item["platform"] = contest.platform;
//                        item["contestName"] = contest.contestName;

//                        std::time_t time =
//                            std::chrono::system_clock::to_time_t(
//                                contest.dateTime);

//                        item["dateTime"] =
//                            std::ctime(&time);

//                        item["solved"] = contest.solved;
//                        item["totalQuestions"] =
//                            contest.totalQuestions;
//                        item["rank"] = contest.rank;

//                        result.push_back(item);
//                    }

//                    res.set_content(
//                        result.dump(),
//                        "application/json");
//                });

//     std::cout << "Server running at http://localhost:8080\n";

//     server.listen("localhost", 8080);

//     return 0;
// }