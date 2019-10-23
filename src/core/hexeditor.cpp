#include "hexeditor.h"

HexEditor::HexEditor()
{
    this->fileSize = 0;
    string path = string(getenv("HOME")) + "/fhex/config.json";
    this->patternMatching = new PatternMatching(path);
}

HexEditor::HexEditor(string path)
{
    HexEditor();
    this->loadFileAsync(path);
}

HexEditor::~HexEditor()
{
    delete this->patternMatching;
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

bool HexEditor::loadFile(string path) {
    std::ifstream ifs(path, std::ios::binary);

    if (!ifs.good()) {
        cerr << "The file " << path << " is not accessible." << endl;
        return false;
    }

    //clear current data
    this->current_data.clear();
    this->current_data.shrink_to_fit();

    // copies all data into buffer
    this->current_data.insert(this->current_data.begin(), std::istreambuf_iterator<char>(ifs), {});

    this->fileSize = this->current_data.size();

    this->current_path = path;
    return true;
}


vector<uint8_t> &HexEditor::getCurrentData() {
    return this->current_data;
}

string HexEditor::getCurrentDataAsString(unsigned long start, unsigned long len) {
    std::string str(this->current_data.begin() + start, this->current_data.begin() + start + len);
    return str;
}

wstring HexEditor::getCurrentDataAsWString(unsigned long start, unsigned long len) {
    wstring str;
    for (auto it = this->getCurrentData().begin() + start; it != this->getCurrentData().begin() + start + len; ++it) {
        str.push_back(*it);
    }
    return str;
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

vector<Match *> HexEditor::findPatternsInChunk(unsigned long start, unsigned long len) {
    wstring str = getCurrentDataAsWString(start, len);

    vector<Match *> matches = this->patternMatching->hasMatches(move(str));
    // Update the relative match positions to the absolute position
    for (Match *m : matches) {
        m->index += start;
    }
    return matches;
}

string HexEditor::fromUintVectorToPrintableString(vector<uint8_t> &vec, long start, long len) {
    stringstream ss;
    vector<uint8_t>::const_iterator it;

    for (it = vec.begin() + start; it != (vec.begin() + start + len); it++) {
        char c = static_cast<char>(*it);
        if (isprint(c)) {
            ss << c;
        } else {
            ss << ".";
        }
    }

    return ss.str();
}

vector<Match *> HexEditor::findPatterns() {
    unsigned long chunkSize = fileSize / this->task_num;
    unsigned long lastChunkSize = fileSize % this->task_num;

    int i;
    int b_offset = 0;
    for (i = 0; i < this->task_num; i++) {
        if ((i*chunkSize) > 512)
            b_offset = 512;
        pattern_tasks.push_back(async([this, chunkSize, i, b_offset](){ return this->findPatternsInChunk(chunkSize * i - b_offset, chunkSize + b_offset); }));
    }

    // Load the last chunk
    if (lastChunkSize > 0) {
        pattern_tasks.push_back(async([this, chunkSize, lastChunkSize, i, b_offset](){ return this->findPatternsInChunk(chunkSize * i - b_offset, lastChunkSize + b_offset); }));
    }

    vector<Match*> res;
    for (auto& ptask : pattern_tasks) {
        vector<Match*> data = ptask.get();
        res.insert(res.end(), data.begin(), data.end());
    }

    this->pattern_tasks.clear();
    this->pattern_tasks.shrink_to_fit();

    return res;
}

vector<pair<unsigned long, uint8_t>>  HexEditor::compareTo(HexEditor &hexEditor) {
    vector<pair<unsigned long, uint8_t>> diff_bytes;

    for (unsigned long i = 0; i < this->fileSize; i++) {
        uint8_t byte_new = hexEditor.getCurrentData()[i];
        if (this->getCurrentData()[i] != byte_new) {
            diff_bytes.push_back(pair<unsigned long, uint8_t>(i, byte_new));
        }
        this->bytesRead = i;
    }

    return diff_bytes;
}
