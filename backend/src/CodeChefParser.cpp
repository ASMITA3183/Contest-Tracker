#include "CodeChefParser.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

using json = nlohmann::json;

std::vector<Contest> CodeChefParser::parse(const std::string &response)
{
    std::vector<Contest> contests;

    // 1. Find the beginning of all_rating
    const std::string marker = "var all_rating =";

    size_t start = response.find(marker);

    if (start == std::string::npos)
    {
        std::cerr << "Could not find all_rating in response\n";
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

            // For now, these are not available from all_rating.
            contest.solved = 0;
            contest.totalQuestions = 0;

            contests.push_back(contest);
        }
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "JSON parsing failed: "
                  << e.what() << '\n';
    }

    return contests;
}