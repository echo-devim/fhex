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

pair<vector<DiffByte>, vector<DiffByte>> HexEditor::compareTo(HexEditor &hexEditor) {
    vector<DiffByte> deleted_bytes;
    vector<DiffByte> added_bytes;
    unsigned long j = 0;
    int step = 1024;
    //Check who has more data (i.e. bytes added)
    vector<uint8_t>& firstVec = this->getCurrentData();
    vector<uint8_t>& secondVec = hexEditor.getCurrentData();

    for (unsigned long i = 0; i < this->fileSize; i += (i+step > this->fileSize ? i+step-this->fileSize : step)) {

        //Extract subvectors to compare
        vector<uint8_t> vec1;
        vector<uint8_t> vec2;

        if (firstVec.begin() + i + step < firstVec.end())
            vec1.insert(vec1.begin(), firstVec.begin() + i, firstVec.begin() + i + step);
        else
            vec1.insert(vec1.begin(), firstVec.begin() + i, firstVec.end());

        if (secondVec.begin() + i + step < secondVec.end())
            vec2.insert(vec2.begin(), secondVec.begin() + i, secondVec.begin() + i + step);
        else
            vec2.insert(vec2.begin(), secondVec.begin() + i, secondVec.end());

        DiffUtils du;
        // fill lookup table
        du.lcs_length(vec1, vec2, vec1.size(), vec2.size());

        // find difference by reading lookup table in top-down manner
        du.diff(vec1, vec2, vec1.size(), vec2.size());


        for (pair<unsigned long, uint8_t> p : du.deleted_bytes) {
            cout << "Deleted char " << (char)p.first << " at index " << p.first << " + " << i << "\n";
            deleted_bytes.push_back(DiffByte(p.first + i, p.second, DiffByte::Operation::DELETED));
        }
        for (pair<unsigned long, uint8_t> p : du.added_bytes) {
            cout << "Added char " << (char)p.first << " at index " << p.first << " + " << i << "\n";
            added_bytes.push_back(DiffByte(p.first + i, p.second, DiffByte::Operation::ADDED));
        }

        this->bytesRead = i;
    }

/*
    for (unsigned long i = 0; i < this->fileSize; i++) {
        uint8_t byte_new = hexEditor.getCurrentData()[j];
        cout << "j = " << j << " , i = " << i << "\n" << flush;
        cout << "check if " << this->getCurrentData()[i] << " != " << byte_new << "\n" << flush;
        if (this->getCurrentData()[i] != byte_new) {
            cout << "check if " << (char)hexEditor.getCurrentData()[first_diff_byte_index] << " != " << (char)this->getCurrentData()[i] << "\n" << flush;
            if (hexEditor.getCurrentData()[first_diff_byte_index] != this->getCurrentData()[i]) {
                diff_bytes.push_back(DiffByte(i, this->getCurrentData()[i], DiffByte::Operation::CHANGED));
                cout << "Pushing back: " << (char)this->getCurrentData()[i] << " at offset " << i << "\n" << flush;
            } else { //An insertion was made
                j = first_diff_byte_index;
                cout << "Insertion detected, j = " << first_diff_byte_index << "\n" << flush;
                DiffByte b = diff_bytes.at(diff_bytes.size() - 1);
                diff_bytes.pop_back();
                b.op = DiffByte::Operation::ADDED;
                diff_bytes.push_back(b);
            }
            cout << "next check if " << (char)this->getCurrentData()[j+1] << " != " << (char)hexEditor.getCurrentData()[i+1] << "\n" << flush;
        } else {
            cout << "first_diff_byte_index = " << j+1 << "\n" << flush;
            first_diff_byte_index = j + 1;
        }
        j++;
        this->bytesRead = i;
    }
*/
    return pair<vector<DiffByte>, vector<DiffByte>>(deleted_bytes, added_bytes);
}
