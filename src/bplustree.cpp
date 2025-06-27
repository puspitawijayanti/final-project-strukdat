#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <iomanip>
using namespace std;
using namespace chrono;

const int ORDER = 4;

struct Node {
    bool isLeaf;
    int n;
    vector<string> keys;
    vector<int> values;
    vector<Node*> children;

    Node(bool leaf) {
        isLeaf = leaf;
        n = 0;
        keys.reserve(ORDER);
        values.reserve(ORDER);
        children.reserve(ORDER + 1);
    }
};

Node* root = nullptr;

string trim(const string& str) {
    const auto strBegin = str.find_first_not_of(" \t\r\n");
    const auto strEnd = str.find_last_not_of(" \t\r\n");
    if (strBegin == string::npos) return "";
    return str.substr(strBegin, strEnd - strBegin + 1);
}

void splitLeaf(Node* parent, int index) {
    Node* leaf = parent->children[index];
    Node* newLeaf = new Node(true);

    int mid = ORDER / 2;
    for (int i = mid; i < ORDER; ++i) {
        newLeaf->keys.push_back(leaf->keys[i]);
        newLeaf->values.push_back(leaf->values[i]);
        newLeaf->n++;
    }

    leaf->keys.resize(mid);
    leaf->values.resize(mid);
    leaf->n = mid;

    parent->keys.insert(parent->keys.begin() + index, newLeaf->keys[0]);
    parent->children.insert(parent->children.begin() + index + 1, newLeaf);
    parent->n++;
}

void splitRoot() {
    Node* newRoot = new Node(false);
    Node* newLeaf = new Node(true);

    for (int i = ORDER / 2; i < ORDER; ++i) {
        newLeaf->keys.push_back(root->keys[i]);
        newLeaf->values.push_back(root->values[i]);
        newLeaf->n++;
    }

    root->keys.resize(ORDER / 2);
    root->values.resize(ORDER / 2);
    root->n = ORDER / 2;

    newRoot->keys.push_back(newLeaf->keys[0]);
    newRoot->children.push_back(root);
    newRoot->children.push_back(newLeaf);
    newRoot->n = 1;

    root = newRoot;
}

void insertToTree(const string& NRP, int pk) {
    if (!root) {
        root = new Node(true);
        root->keys.push_back(NRP);
        root->values.push_back(pk);
        root->n = 1;
        return;
    }

    if (root->isLeaf && root->n < ORDER) {
        auto it = lower_bound(root->keys.begin(), root->keys.end(), NRP);
        int pos = it - root->keys.begin();
        root->keys.insert(it, NRP);
        root->values.insert(root->values.begin() + pos, pk);
        root->n++;
        return;
    }

    if (root->isLeaf && root->n == ORDER) {
        splitRoot();
    }

    Node* current = root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->n && NRP >= current->keys[i]) i++;

        Node* child = current->children[i];

        if (child->isLeaf && child->n == ORDER) {
            splitLeaf(current, i);
            if (NRP >= current->keys[i]) i++;
        }
        current = current->children[i];
    }

    auto it = lower_bound(current->keys.begin(), current->keys.end(), NRP);
    int pos = it - current->keys.begin();
    current->keys.insert(it, NRP);
    current->values.insert(current->values.begin() + pos, pk);
    current->n++;
}

int search(Node* node, const string& NRP) {
    if (!node) return -1;
    if (node->isLeaf) {
        auto it = lower_bound(node->keys.begin(), node->keys.begin() + node->n, NRP);
        if (it != node->keys.begin() + node->n && *it == NRP)
            return node->values[it - node->keys.begin()];
        return -1;
    }

    int i = 0;
    while (i < node->n && NRP >= node->keys[i]) i++;
    return search(node->children[i], NRP);
}

bool updateInTree(Node* node, const string& NRP, int newPK) {
    if (!node) return false;
    if (node->isLeaf) {
        auto it = lower_bound(node->keys.begin(), node->keys.begin() + node->n, NRP);
        if (it != node->keys.begin() + node->n && *it == NRP) {
            node->values[it - node->keys.begin()] = newPK;
            return true;
        }
        return false;
    }
    int i = 0;
    while (i < node->n && NRP >= node->keys[i]) i++;
    return updateInTree(node->children[i], NRP, newPK);
}

bool deleteFromTree(Node* node, const string& NRP) {
    if (!node) return false;
    if (node->isLeaf) {
        auto it = lower_bound(node->keys.begin(), node->keys.begin() + node->n, NRP);
        if (it != node->keys.begin() + node->n && *it == NRP) {
            int index = it - node->keys.begin();
            node->keys.erase(it);
            node->values.erase(node->values.begin() + index);
            node->n--;
            return true;
        }
        return false;
    }
    int i = 0;
    while (i < node->n && NRP >= node->keys[i]) i++;
    return deleteFromTree(node->children[i], NRP);
}

size_t calculateMemory(Node* node) {
    if (!node) return 0;
    size_t size = sizeof(Node);
    size += node->keys.capacity() * sizeof(string);
    size += node->values.capacity() * sizeof(int);
    size += node->children.capacity() * sizeof(Node*);
    for (const string& key : node->keys) size += key.capacity();
    for (Node* child : node->children) size += calculateMemory(child);
    return size;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./bplustree <input_file.txt>\n";
        return 1;
    }

    ifstream file(argv[1]);
    if (!file.is_open()) {
        cerr << "Gagal membuka file.\n";
        return 1;
    }

    cout << fixed << setprecision(2);
    string line, filename = argv[1];
    int counter = 0;
    vector<string> allNRPs;

    cout << "\n=====  Mulai Proses B+ Tree  =====\n";
    cout << "File Input     : " << filename << endl;

    auto startInsert = high_resolution_clock::now();
    while (getline(file, line)) {
        stringstream ss(line);
        string NRP, nama, jurusan, angkatan;
        getline(ss, NRP, ',');
        getline(ss, nama, ',');
        getline(ss, jurusan, ',');
        getline(ss, angkatan, ',');
        NRP = trim(NRP);
        allNRPs.push_back(NRP);
        insertToTree(NRP, counter++);
    }
    auto endInsert = high_resolution_clock::now();
    file.close();

    cout << "Jumlah Data    : " << counter << "\n\n";
    double insertTime = duration_cast<duration<double, milli>>(endInsert - startInsert).count();
    cout << "FASE INSERT\n";
    cout << "Waktu Insert   : " << insertTime << " ms\n\n";

    auto startRead = high_resolution_clock::now();
    for (const string& nrp : allNRPs) {
        volatile int dummy = search(root, nrp);
    }
    auto endRead = high_resolution_clock::now();
    double readTotal = duration_cast<duration<double, milli>>(endRead - startRead).count();
    cout << "[READ x" << counter << "]\n";
    cout << "Waktu total     : " << readTotal << " ms\n";
    cout << "Waktu rata-rata : " << readTotal / counter << " ms\n\n";

    auto startSearch = high_resolution_clock::now();
    for (const string& nrp : allNRPs) {
        search(root, nrp);
    }
    auto endSearch = high_resolution_clock::now();
    double searchTotal = duration_cast<duration<double, milli>>(endSearch - startSearch).count();
    cout << "[SEARCH x" << counter << "]\n";
    cout << "Waktu total     : " << searchTotal << " ms\n";
    cout << "Waktu rata-rata : " << searchTotal / counter << " ms\n\n";

    auto startUpdate = high_resolution_clock::now();
    for (const string& nrp : allNRPs) {
        updateInTree(root, nrp, 9999);
    }
    auto endUpdate = high_resolution_clock::now();
    double updateTotal = duration_cast<duration<double, milli>>(endUpdate - startUpdate).count();
    cout << "[UPDATE x" << counter << "]\n";
    cout << "Waktu total     : " << updateTotal << " ms\n";
    cout << "Waktu rata-rata : " << updateTotal / counter << " ms\n\n";

    auto startDelete = high_resolution_clock::now();
    for (const string& nrp : allNRPs) {
        deleteFromTree(root, nrp);
    }
    auto endDelete = high_resolution_clock::now();
    double deleteTotal = duration_cast<duration<double, milli>>(endDelete - startDelete).count();
    cout << "[DELETE x" << counter << "]\n";
    cout << "Waktu total     : " << deleteTotal << " ms\n";
    cout << "Waktu rata-rata : " << deleteTotal / counter << " ms\n\n";

    cout << "[MEMORI]\n";
    size_t memBytes = calculateMemory(root);
    cout << "Perkiraan penggunaan memori: " << memBytes << " byte\n\n";

    cout << "=====  Program Selesai  =====\n";
    return 0;
}