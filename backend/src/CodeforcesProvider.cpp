#include "CodeforcesProvider.h"

#include <curl/curl.h>

#include <iostream>
#include <string>

namespace
{
    struct Download
    {
        std::string data;

        // Stop downloading once this text arrives (empty = download everything)
        std::string stopAfter;

        bool stopped = false;
    };

    size_t writeCallback(
        void *contents,
        size_t size,
        size_t nmemb,
        void *userData)
    {
        size_t totalSize = size * nmemb;

        Download *download =
            static_cast<Download *>(userData);

        download->data.append(
            static_cast<char *>(contents),
            totalSize);

        if (!download->stopAfter.empty() &&
            download->data.find(download->stopAfter) != std::string::npos)
        {
            // Returning 0 makes curl stop the transfer
            download->stopped = true;
            return 0;
        }

        return totalSize;
    }

    std::string httpGet(
        const std::string &url,
        const std::string &stopAfter = "")
    {
        CURL *curl = curl_easy_init();

        if (!curl)
        {
            std::cerr << "Failed to initialize CURL\n";
            return "";
        }

        Download download;
        download.stopAfter = stopAfter;

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
            &download);

        CURLcode result =
            curl_easy_perform(curl);

        if (result != CURLE_OK && !download.stopped)
        {
            std::cerr << "Codeforces request failed for "
                      << url << ": "
                      << curl_easy_strerror(result)
                      << '\n';

            curl_easy_cleanup(curl);

            return "";
        }

        curl_easy_cleanup(curl);

        return download.data;
    }
}

std::string CodeforcesProvider::fetchRatingHistory(
    const std::string &username)
{
    return httpGet(
        "https://codeforces.com/api/user.rating?handle=" + username);
}

std::string CodeforcesProvider::fetchSubmissions(
    const std::string &username)
{
    return httpGet(
        "https://codeforces.com/api/user.status?handle=" + username);
}

std::string CodeforcesProvider::fetchContestList()
{
    return httpGet(
        "https://codeforces.com/api/contest.list?gym=false");
}

std::string CodeforcesProvider::fetchContestProblems(int contestId)
{
    // Full standings are ~10 MB, but the problem list comes before "rows",
    // so only the first few KB are downloaded.
    return httpGet(
        "https://codeforces.com/api/contest.standings?contestId=" +
            std::to_string(contestId),
        "\"rows\":");
}
