#ifndef LEETCODE_PROVIDER_H
#define LEETCODE_PROVIDER_H

#include <string>

class LeetCodeProvider
{
public:
    std::string fetchContestData(const std::string &username);
};

#endif