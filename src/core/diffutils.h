#ifndef DIFFUTILS_H
#define DIFFUTILS_H

#include <iostream>
#include <vector>
#include <string>
using namespace std;

// define maximum possible length of vector<uint8_t> X and Y
#define M 1024
#define N 1024

class DiffByte {
public:
    enum Operation {
        ADDED,
        CHANGED,
        DELETED
    };

    unsigned long index;
    uint8_t data;
    Operation op;
    DiffByte(unsigned long index, uint8_t data, Operation op) {
        this->index = index;
        this->data = data;
        this->op = op;
    }
    DiffByte& operator=(const DiffByte &b) {
        this->index = b.index;
        this->data = b.data;
        this->op = b.op;
        return *this;
    }
};

class DiffUtils {
private:
    // lookup[i][j] stores the length of LCS of subvector<uint8_t> X[0..i-1], Y[0..j-1]
    int lookup[M][N];
public:
    vector<pair<unsigned long, uint8_t>> changed_bytes;
    vector<pair<unsigned long, uint8_t>> added_bytes;
    vector<pair<unsigned long, uint8_t>> deleted_bytes;
    void diff(vector<uint8_t> X, vector<uint8_t> Y, int m, int n);
    void lcs_length(vector<uint8_t> X, vector<uint8_t> Y, int m, int n);
    vector<uint8_t> getAddedBytes();
    vector<uint8_t> getDeletedBytes();

};

#endif // DIFFUTILS_H
