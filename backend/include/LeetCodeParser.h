#ifndef LEETCODE_PARSER_H
#define LEETCODE_PARSER_H

#include "Contest.h"

#include <string>
#include <vector>

class LeetCodeParser
{
public:
    std::vector<Contest> parse(const std::string &response);
};

#endif