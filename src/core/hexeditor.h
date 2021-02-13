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

/** HexEditor backend **/
class HexEditor
{
public:
    unsigned long fileSize; // real file size of the file
    unsigned long loadedFileSize; // bytes currently loaded in the hexeditor (length)
    unsigned long startOffset;
    atomic<unsigned long> bytesRead;
    atomic<unsigned long> bytesSaved;
    HexEditor();
    HexEditor(string path);
    ~HexEditor();
    void loadPatterns(string patternsFile);
    bool loadFileAsync(string path, unsigned long start = 0, unsigned long offset = 0);
    bool saveFileAsync(string path);
    bool isFileLoaded();
    bool isFileSaved();
    vector<uint8_t> &getCurrentData();
    string getCurrentDataAsString(unsigned long start, unsigned long len);
    wstring getCurrentDataAsWString(unsigned long start, unsigned long len);
    void setCurrentData(vector<uint8_t> &data);
    void updateByte(uint8_t new_byte, unsigned long file_offset);
    string getCurrentPath();
    vector<Match *> findPatterns();
    vector<pair<unsigned long, uint8_t>> compareTo(HexEditor &hexEditor);

private:
    PatternMatching *patternMatching;
    unsigned int task_num = std::thread::hardware_concurrency();
    vector<std::future<vector<Match*>>> pattern_tasks;
    string current_path;
    vector<uint8_t> current_data;
    void loadFilePart(string path, unsigned long start, unsigned long offset);
    vector<Match *> findPatternsInChunk(unsigned long start, unsigned long len);
    string fromUintVectorToPrintableString(vector<uint8_t> &vec, long start, long len);
    bool saveDataToFile(string path);

};

#endif // HEXEDITOR_H
