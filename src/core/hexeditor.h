#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#ifdef __MINGW32__
#define WINDOWS
#endif

#include <vector>
#include <istream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <future>
#include <atomic>
#include <iomanip>

#include "patternmatching.h"

using namespace std;

class HexEditor
{
public:
    unsigned long fileSize;
    atomic<unsigned long> bytesRead;
    HexEditor(string patternsFile);
    HexEditor(string path, string patternsFile);
    ~HexEditor();
    bool loadFile(string path);
    bool loadFileAsync(string path);
    bool isFileLoaded();
    vector<uint8_t> &getCurrentData();
    string getCurrentDataAsString(unsigned long start, unsigned long len);
    wstring getCurrentDataAsWString(unsigned long start, unsigned long len);
    void setCurrentData(vector<uint8_t> &data);
    void updateByte(uint8_t new_byte, unsigned long file_offset);
    bool saveDataToFile(string path);
    void saveDataToFileAsync(string path);
    string getCurrentPath();
    vector<Match *> findPatterns();
    vector<pair<unsigned long, uint8_t>> compareTo(HexEditor &hexEditor);

private:
    PatternMatching *patternMatching;
    unsigned int task_num = std::thread::hardware_concurrency();
    vector<std::future<vector<uint8_t>*>> tasks;
    vector<std::future<vector<Match*>>> pattern_tasks;
    string current_path;
    vector<uint8_t> current_data;
    vector<uint8_t>* loadFilePart(string path, unsigned long start, unsigned long len);
    vector<Match *> findPatternsInChunk(unsigned long start, unsigned long len);
    string fromUintVectorToPrintableString(vector<uint8_t> &vec, long start, long len);

};

#endif // HEXEDITOR_H
