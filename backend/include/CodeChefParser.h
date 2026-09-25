#ifndef CODECHEF_PARSER_H
#define CODECHEF_PARSER_H

#include "Contest.h"

#include <string>
#include <vector>

class CodeChefParser
{
public:
    std::vector<Contest> parse(
        const std::string &response);
};

#endif