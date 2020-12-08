#ifndef PATTERNMATCHING_H
#define PATTERNMATCHING_H

#include <fstream>
#include <exception>
#include <iostream>
#include <regex>
#include <codecvt>
#include "json.h"

#define MAX_PATTERN_RESULTS 100

using json = nlohmann::json;
using namespace std;

class Match {
public:
    unsigned long index;
    unsigned long length;
    string color;
    string message;
    Match(string color, string message, unsigned long index, unsigned long length) {
        this->color = color;
        this->message = message;
        this->index = index;
        this->length = length;
    }
    Match& operator=(const Match &m) {
        this->color = m.color;
        this->message = m.message;
        this->index = m.index;
        this->length = m.length;
        return *this;
    }
};

class PatternMatching
{
public:
    PatternMatching(string path);
    vector<Match *> hasMatches(vector<uint8_t> &content);
private:
    json jconfig;
};

#endif // PATTERNMATCHING_H
