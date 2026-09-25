#ifndef CODEFORCES_PARSER_H
#define CODEFORCES_PARSER_H

#include "Contest.h"

#include <string>
#include <vector>

class CodeforcesParser
{
public:
    std::vector<Contest> parse(
        const std::string &ratingResponse,
        const std::string &submissionsResponse,
        const std::string &contestListResponse);
};

#endif
