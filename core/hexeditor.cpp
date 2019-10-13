#include "hexeditor.h"

HexEditor::HexEditor()
{
}

HexEditor::HexEditor(string path)
{
    this->loadFileAsync(path);
}

vector<uint8_t> HexEditor::loadFilePart(string path, unsigned long start, unsigned long len) {
    vector<uint8_t> data(len);
    ifstream ifs(path, ios::binary|ios::ate);
    ifs.seekg(static_cast<long>(start), ios::beg);
    ifs.read(reinterpret_cast<char*>(data.data()), static_cast<long>(len));
    return data;
}

bool HexEditor::loadFileAsync(string path) {
    ifstream ifs(path, ios::binary|ios::ate);

    if (!ifs.good()) {
        cerr << "The file " << path << " is not accessible." << endl;
        return false;
    }

    ifstream::pos_type pos = ifs.tellg(); //file length
    unsigned long fileSize = static_cast<unsigned long>(pos);

    // Parallel async file read
    vector<future<vector<uint8_t>>> tasks;

    unsigned long chunkSize = fileSize / TASK_NUM;
    unsigned long lastChunkSize = fileSize % TASK_NUM;

    int i;
    for (i = 0; i < TASK_NUM; i++) {
        tasks.push_back(async([this, &path, &chunkSize, i](){ return this->loadFilePart(path, chunkSize * i, chunkSize); }));
    }

    // Load the last chunk
    if (lastChunkSize > 0)
        tasks.push_back(async([this, &path, &chunkSize, &lastChunkSize, i](){ return this->loadFilePart(path, chunkSize * i, lastChunkSize); }));

    for (auto& task : tasks) {
        vector<uint8_t> data = task.get();
        this->current_data.insert(this->current_data.end(), data.begin(), data.end());
    }

    this->current_path = path;
    return true;
}
/*
bool HexEditor::loadFile(string path) {
    ifstream ifs(path, ios::binary|ios::ate);

    if (!ifs.good()) {
        cerr << "The file " << path << " is not accessible." << endl;
        return false;
    }

    ifstream::pos_type pos = ifs.tellg(); //file length
    this->current_data.resize(static_cast<unsigned long>(pos));
    ifs.seekg(0, ios::beg);
    ifs.read(reinterpret_cast<char*>(this->current_data.data()), pos);

    this->current_path = path;
    return true;
}
*/

vector<uint8_t> HexEditor::getCurrentData() {
    return this->current_data;
}

void HexEditor::setCurrentData(vector<uint8_t> &data) {
    this->current_data = data;
}

string HexEditor::getCurrentPath() {
    return this->current_path;
}

void HexEditor::updateByte(uint8_t new_byte, unsigned long file_offset) {
    this->current_data[file_offset] = new_byte;
    fstream out(this->current_path, ios::binary | ios::out | ios::in);
    out.seekp(static_cast<long>(file_offset), ios::beg);
    const char *b = reinterpret_cast<const char*>(&new_byte);
    out.write(b, 1);
}

bool HexEditor::saveDataToFile(string path) {
    ofstream fout(path, ios::out | ios::binary);
    if (!fout.good()) {
        cerr << "The file " << path << " is not accessible." << endl;
        return false;
    }
    fout.write(reinterpret_cast<char*>(this->current_data.data()), static_cast<long>(this->current_data.size()));
    fout.close();
    return true;
}
