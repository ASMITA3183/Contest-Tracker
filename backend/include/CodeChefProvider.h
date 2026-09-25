#ifndef CODECHEF_PROVIDER_H
#define CODECHEF_PROVIDER_H

#include <string>

class CodeChefProvider
{
public:
    std::string fetchContestData(
        const std::string &username);
};

#endif