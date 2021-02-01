#include "hexeditor.h"

/*
 * REMOVED HARDCODED PATTERNS FILE PATH
 */

HexEditor::HexEditor()
{
    this->fileSize = 0;
    this->patternMatching = NULL;
}

HexEditor::HexEditor(string patternsFile)
{
    HexEditor();
    this->patternMatching = new PatternMatching(patternsFile);
}

/*
 * ADDED patternsFile TO MAKE SIGNATURE DIFFERENT
 */

HexEditor::HexEditor(string path, string patternsFile)
{
    (HexEditor(patternsFile));
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
    this->current_data.reserve(this->fileSize);

    unsigned long chunkSize = fileSize / this->task_num;
    unsigned long lastChunkSize = fileSize % this->task_num;

    this->bytesRead = 0;
    unsigned int i;
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

    this->current_path = path;

    //clear current data
    this->current_data.clear();
    this->current_data.shrink_to_fit();

    ifs.seekg(0, std::ios::end);
    this->fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    // copies all data into buffer
    this->current_data.insert(this->current_data.begin(), std::istreambuf_iterator<char>(ifs), {});

    this->bytesRead = this->fileSize;

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
    return this->patternMatching->hasMatches(this->current_data);
}

vector<pair<unsigned long, uint8_t>>  HexEditor::compareTo(HexEditor &hexEditor) {
    vector<pair<unsigned long, uint8_t>> diff_bytes;

    //Find who has the smaller file size
    unsigned long filesize = this->fileSize;
    if (hexEditor.fileSize < filesize)
        filesize = hexEditor.fileSize;

    for (unsigned long i = 0; i < filesize; i++) {
        uint8_t byte_new = hexEditor.getCurrentData()[i];
        if (this->getCurrentData()[i] != byte_new) {
            diff_bytes.push_back(pair<unsigned long, uint8_t>(i, byte_new));
        }
        this->bytesRead = i;
    }

    return diff_bytes;
}
