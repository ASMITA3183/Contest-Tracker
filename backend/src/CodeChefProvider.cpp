#include "CodeChefProvider.h"

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

std::string CodeChefProvider::fetchContestData(
    const std::string &username)
{
    CURL *curl = curl_easy_init();

    if (!curl)
    {
        std::cerr << "Failed to initialize CURL\n";
        return "";
    }

    std::string response;

    std::string url =
        "https://www.codechef.com/users/" + username;

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
        std::cerr << "CodeChef request failed: "
                  << curl_easy_strerror(result)
                  << '\n';

        curl_easy_cleanup(curl);

        return "";
    }

    curl_easy_cleanup(curl);

    return response;
}