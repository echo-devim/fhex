#ifndef HEXEDITOR_H
#define HEXEDITOR_H

#include <vector>
#include <istream>
#include <fstream>
#include <iostream>
#include <future>

#define TASK_NUM 8

using namespace std;

class HexEditor
{
public:
    HexEditor();
    HexEditor(string path);
    bool loadFile(string path);
    bool loadFileAsync(string path);
    vector<uint8_t> getCurrentData();
    void setCurrentData(vector<uint8_t> &data);
    void updateByte(uint8_t new_byte, unsigned long file_offset);
    bool saveDataToFile(string path);
    string getCurrentPath();

private:
    string current_path;
    vector<uint8_t> current_data;
    vector<uint8_t> loadFilePart(string path, unsigned long start, unsigned long len);
};

#endif // HEXEDITOR_H
