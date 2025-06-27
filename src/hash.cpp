#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <iomanip>
using namespace std;

struct Mahasiswa {
    string nrp, nama, jurusan;
    int angkatan;
};

vector<Mahasiswa> loadData(const string &filename) {
    vector<Mahasiswa> data;
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        size_t pos1 = line.find(',');
        size_t posLast = line.rfind(',');
        size_t posSecondLast = line.rfind(',', posLast - 1);
        if (pos1 == string::npos || posLast == string::npos || posSecondLast == string::npos) continue;

        string nrp = line.substr(0, pos1);
        string nama = line.substr(pos1 + 1, posSecondLast - pos1 - 1);
        string jurusan = line.substr(posSecondLast + 1, posLast - posSecondLast - 1);
        string angkatan_str = line.substr(posLast + 1);

        try {
            int angkatan = stoi(angkatan_str);
            data.push_back({nrp, nama, jurusan, angkatan});
        } catch (...) {
            continue;
        }
    }
    return data;
}

const int TABLE_SIZE = 2003;

struct HashTable {
    vector<Mahasiswa> table[TABLE_SIZE];

    int hashFunction(const string &nrp) {
        int hash = 0;
        for (char c : nrp) hash += c;
        return hash % TABLE_SIZE;
    }

    void insert(const Mahasiswa &mhs) {
        int idx = hashFunction(mhs.nrp);
        table[idx].push_back(mhs);
    }

    Mahasiswa* search(const string &nrp) {
        int idx = hashFunction(nrp);
        for (auto &mhs : table[idx]) {
            if (mhs.nrp == nrp) return &mhs;
        }
        return nullptr;
    }

    bool update(const string &nrp, const string &namaBaru) {
        Mahasiswa *found = search(nrp);
        if (found) {
            found->nama = namaBaru;
            return true;
        }
        return false;
    }

    bool remove(const string &nrp) {
        int idx = hashFunction(nrp);
        auto &bucket = table[idx];
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->nrp == nrp) {
                bucket.erase(it);
                return true;
            }
        }
        return false;
    }

    size_t memoryUsage() {
        size_t total = TABLE_SIZE * sizeof(vector<Mahasiswa>);
        for (int i = 0; i < TABLE_SIZE; ++i) {
            total += table[i].size() * sizeof(Mahasiswa);
        }
        return total;
    }
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "Usage: ./hash <input_file.txt>\n";
        return 1;
    }

    cout << fixed << setprecision(2);
    string filename = argv[1];
    vector<Mahasiswa> data = loadData(filename);
    HashTable ht;

    cout << "\n=====  Mulai Proses Hash Table  =====\n";
    cout << "File Input     : " << filename << endl;
    cout << "Jumlah Data    : " << data.size() << "\n\n";

    auto start = chrono::high_resolution_clock::now();
    for (const auto &mhs : data) ht.insert(mhs);
    auto end = chrono::high_resolution_clock::now();
    double insertTime = chrono::duration<double, milli>(end - start).count();
    cout << "FASE INSERT\n";
    cout << "Waktu Insert   : " << insertTime << " ms\n\n";

    int langkahRead = 0;
    start = chrono::high_resolution_clock::now();
    for (int i = 0; i < TABLE_SIZE; ++i) {
        for (const auto& mhs : ht.table[i]) {
            volatile string dummy = mhs.nrp;
            langkahRead += 2;
        }
    }
    end = chrono::high_resolution_clock::now();
    double readTotal = chrono::duration<double, milli>(end - start).count();
    int totalRead = data.size();
    cout << "[READ x" << totalRead << "]\n";
    cout << "Waktu total     : " << readTotal << " ms\n";
    cout << "Waktu rata-rata : " << readTotal / totalRead << " ms\n\n";

    int iter = data.size();
    int langkahSearch = 0;
    start = chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        ht.search(data[i].nrp);
        langkahSearch += 2;
    }
    end = chrono::high_resolution_clock::now();
    double searchTotal = chrono::duration<double, milli>(end - start).count();
    cout << "[SEARCH x" << iter << "]\n";
    cout << "Waktu total     : " << searchTotal << " ms\n";
    cout << "Waktu rata-rata : " << searchTotal / iter << " ms\n\n";

    int langkahUpdate = 0;
    start = chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        ht.update(data[i].nrp, data[i].nama + "_upd");
        langkahUpdate += 3;
    }
    end = chrono::high_resolution_clock::now();
    double updateTotal = chrono::duration<double, milli>(end - start).count();
    cout << "[UPDATE x" << iter << "]\n";
    cout << "Waktu total     : " << updateTotal << " ms\n";
    cout << "Waktu rata-rata : " << updateTotal / iter << " ms\n\n";

    int langkahDelete = 0;
    start = chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        ht.remove(data[i].nrp);
        langkahDelete += 2;
    }
    end = chrono::high_resolution_clock::now();
    double deleteTotal = chrono::duration<double, milli>(end - start).count();
    cout << "[DELETE x" << iter << "]\n";
    cout << "Waktu total     : " << deleteTotal << " ms\n";
    cout << "Waktu rata-rata : " << deleteTotal / iter << " ms\n\n";

    cout << "[MEMORI]\n";
    cout << "Perkiraan penggunaan memori: " << ht.memoryUsage() << " byte\n\n";
    cout << "=====  Program Selesai  =====\n";
    return 0;
}