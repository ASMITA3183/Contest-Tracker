#include "LeetCodeProvider.h"

#include <curl/curl.h>
#include <iostream>
#include <string>

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
}

std::string LeetCodeProvider::fetchContestData(
    const std::string &username)
{
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        std::cerr << "Failed to initialize CURL\n";
        return "";
    }

    std::string response;

    std::string query =
        R"({
        "operationName": "userContestRankingInfo",
        "query": "query userContestRankingInfo($username: String!) { matchedUser(username: $username) { username } userContestRanking(username: $username) { attendedContestsCount rating globalRanking totalParticipants topPercentage badge { name } } userContestRankingHistory(username: $username) { attended trendDirection problemsSolved totalProblems finishTimeInSeconds rating ranking contest { title startTime } } }",
        "variables": {
            "username": ")" +
        username + R"("
        }
    })";

    struct curl_slist *headers = nullptr;

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json");

    curl_easy_setopt(
        curl,
        CURLOPT_URL,
        "https://leetcode.com/graphql/");

    curl_easy_setopt(
        curl,
        CURLOPT_POST,
        1L);

    curl_easy_setopt(
        curl,
        CURLOPT_POSTFIELDS,
        query.c_str());

    curl_easy_setopt(
        curl,
        CURLOPT_HTTPHEADER,
        headers);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEFUNCTION,
        writeCallback);

    curl_easy_setopt(
        curl,
        CURLOPT_WRITEDATA,
        &response);

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK)
    {
        std::cerr << "LeetCode request failed: "
                  << curl_easy_strerror(result)
                  << '\n';

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return "";
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}