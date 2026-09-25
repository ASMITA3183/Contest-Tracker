#ifndef CONTEST_H
#define CONTEST_H

#include <string>
#include <chrono>

struct Contest
{
    std::string platform;
    std::string contestName;

    std::chrono::system_clock::time_point dateTime;

    int solved;
    int totalQuestions;
    int rank;
    int rating;
};

#endif