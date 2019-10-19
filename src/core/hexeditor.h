#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <vector>
#include <istream>
#include <fstream>
#include <iostream>
#include <future>
#include <atomic>

#include "patternmatching.h"

using namespace std;

class HexEditor
{
public:
    unsigned long fileSize;
    atomic<unsigned long> bytesRead;
    HexEditor();
    HexEditor(string path);
    ~HexEditor();
    bool loadFile(string path);
    bool loadFileAsync(string path);
    bool isFileLoaded();
    vector<uint8_t> &getCurrentData();
    string getCurrentDataAsString();
    void setCurrentData(vector<uint8_t> &data);
    void updateByte(uint8_t new_byte, unsigned long file_offset);
    bool saveDataToFile(string path);
    void saveDataToFileAsync(string path);
    string getCurrentPath();
    vector<Match *> findPatterns();

private:
    PatternMatching *patternMatching;
    unsigned int task_num = std::thread::hardware_concurrency();
    vector<std::future<vector<uint8_t>*>> tasks;
    vector<std::future<vector<Match*>>> pattern_tasks;
    string current_path;
    vector<uint8_t> current_data;
    vector<uint8_t>* loadFilePart(string path, unsigned long start, unsigned long len);
    vector<Match *> findPatternsInChunk(unsigned long start, unsigned long len);

};

#endif // HEXEDITOR_H
