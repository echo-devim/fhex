#include "hexeditor.h"

HexEditor::HexEditor()
{
    this->fileSize = 0;
}

HexEditor::HexEditor(string path)
{
    this->fileSize = 0;
    this->loadFileAsync(path);
}

vector<uint8_t>* HexEditor::loadFilePart(string path, unsigned long start, unsigned long len) {
    vector<uint8_t> *data = new vector<uint8_t>(len);
    ifstream ifs(path, ios::binary|ios::ate);
    ifs.seekg(static_cast<long>(start), ios::beg);
    ifs.read(reinterpret_cast<char*>(data->data()), static_cast<long>(len));
    this->bytesRead += len;
    return data;
}

// Parallel async file read
bool HexEditor::loadFileAsync(string path) {
    ifstream ifs(path, ios::binary|ios::ate);

    if (!ifs.good()) {
        cerr << "The file " << path << " is not accessible." << endl;
        return false;
    }

    //clear current data
    this->current_data.clear();
    this->current_data.shrink_to_fit();
    this->tasks.clear();

    ifstream::pos_type pos = ifs.tellg(); //file length
    this->fileSize = static_cast<unsigned long>(pos);

    unsigned long chunkSize = fileSize / this->task_num;
    unsigned long lastChunkSize = fileSize % this->task_num;

    this->bytesRead = 0;
    int i;
    for (i = 0; i < this->task_num; i++) {
        tasks.push_back(async([this, path, chunkSize, i](){ return this->loadFilePart(path, chunkSize * i, chunkSize); }));
    }

    // Load the last chunk
    if (lastChunkSize > 0) {
      tasks.push_back(async([this, path, chunkSize, lastChunkSize, i](){ return this->loadFilePart(path, chunkSize * i, lastChunkSize); }));
    }

    this->current_path = path;
    return true;
}

bool HexEditor::isFileLoaded() {
    if (this->bytesRead < this->fileSize)
        return false;

    for (auto& task : tasks) {
        vector<uint8_t> *data = task.get();
        this->current_data.insert(this->current_data.end(), data->begin(), data->end());
        delete data;
    }

    tasks.clear();
    tasks.shrink_to_fit();

    return true;
}

/* Legacy function */
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


vector<uint8_t> &HexEditor::getCurrentData() {
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
    fout.write(reinterpret_cast<char*>(this->current_data.data()), fileSize);
    fout.close();
    return true;
}

void HexEditor::saveDataToFileAsync(string path) {
    async(&HexEditor::saveDataToFile, this, path);
}
