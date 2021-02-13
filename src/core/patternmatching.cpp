#include "patternmatching.h"

PatternMatching::PatternMatching(string path)
{
    std::ifstream configFile(path);

    if (!configFile.good()) {
        cerr << "The file '" << path << "' is not accessible." << endl;
        return;
    }

    try {
        configFile >> this->jconfig;
    } catch (exception &e) {
        cerr << "Exception occurred while parsing json configuration file:" << endl << e.what() << endl;
    }
}

vector<Match*> PatternMatching::hasMatches(vector<uint8_t> &content) {
    vector<Match*> res;
    vector<vector<uint8_t>> patterns;
    vector<string> colors;
    vector<string> messages;
    for (auto& e : this->jconfig["PatternMatching"]) {
        if (e.contains("string")) {
            vector<uint8_t> bytes;
            string str = e["string"].get<string>();
            for(uint8_t byte : str) {
                bytes.push_back(byte);
            }
            patterns.push_back(bytes);
            colors.push_back(e["color"].get<string>());
            messages.push_back(e["message"].get<string>());
        } else if (e.contains("bytes")) {
            string strbytes = e["bytes"].get<string>();
            stringstream converter;
            vector<uint8_t> bytes;
            for(size_t i = 0; i < strbytes.length(); i+=2) {
                converter << std::hex << strbytes.substr(i,2);
                short byte;
                converter >> byte;
                bytes.push_back(byte & 0xFF);
                converter.str(string());
                converter.clear();
            }
            patterns.push_back(bytes);
            colors.push_back(e["color"].get<string>());
            messages.push_back(e["message"].get<string>());
        }
    }
    //Search patterns in content
    unsigned long pos = 0;
    uint8_t *content_ptr = content.data(); //Use pointers to be faster
    for (pos = 0; pos < content.size(); pos++) {
        for (size_t p = 0; p < patterns.size(); p++) {
            uint8_t *pattern_ptr = patterns[p].data();
            size_t size = patterns[p].size();
            if (content.size() - pos < size)
                continue;
            size_t i;
            for (i = 0; i < size; i++) {
                if (content_ptr[pos+i] != pattern_ptr[i])
                    break;
            }
            bool found = (i == size);
            if ((found) && (res.size() < MAX_PATTERN_RESULTS)) {
                res.push_back(new Match(colors[p], messages[p], pos, size));
                pos+=size;
            }
        }
    }
    return res;
}

