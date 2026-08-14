
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <sys/stat.h>

using namespace std;

const int BUCKETS = 1000003;
const char* INDEX_FILE = "index.bin";
const char* DATA_FILE = "data.bin";

struct Entry {
    char index[64];
    int value;
    int next_offset;
    bool deleted;
};

unsigned int hash_fn(const string& s) {
    unsigned int hash = 5381;
    for (char c : s) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % BUCKETS;
}

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void init_files() {
    if (!file_exists(INDEX_FILE)) {
        ofstream idx(INDEX_FILE, ios::binary);
        int val = -1;
        for (int i = 0; i < BUCKETS; ++i) {
            idx.write(reinterpret_cast<const char*>(&val), sizeof(int));
        }
        idx.close();
    }
    if (!file_exists(DATA_FILE)) {
        ofstream dat(DATA_FILE, ios::binary);
        dat.close();
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    init_files();

    int n;
    if (!(cin >> n)) return 0;

    fstream idx_file(INDEX_FILE, ios::binary | ios::in | ios::out);
    fstream dat_file(DATA_FILE, ios::binary | ios::in | ios::out);

    for (int i = 0; i < n; ++i) {
        string cmd;
        cin >> cmd;
        if (cmd == "insert") {
            string index;
            int value;
            cin >> index >> value;
            unsigned int h = hash_fn(index);

            int head;
            idx_file.seekg(h * sizeof(int));
            idx_file.read(reinterpret_cast<char*>(&head), sizeof(int));

            bool exists = false;
            int current_offset = head;
            while (current_offset != -1) {
                dat_file.seekg(current_offset);
                Entry e;
                dat_file.read(reinterpret_cast<char*>(&e), sizeof(Entry));
                if (!e.deleted && strcmp(e.index, index.c_str()) == 0 && e.value == value) {
                    exists = true;
                    break;
                }
                current_offset = e.next_offset;
            }

            if (!exists) {
                dat_file.seekp(0, ios::end);
                int new_offset = dat_file.tellp();
                Entry new_entry;
                memset(new_entry.index, 0, 64);
                strncpy(new_entry.index, index.c_str(), 63);
                new_entry.value = value;
                new_entry.next_offset = head;
                new_entry.deleted = false;
                dat_file.write(reinterpret_cast<const char*>(&new_entry), sizeof(Entry));

                idx_file.seekp(h * sizeof(int));
                idx_file.write(reinterpret_cast<const char*>(&new_offset), sizeof(int));
                idx_file.flush();
            }
        } else if (cmd == "delete") {
            string index;
            int value;
            cin >> index >> value;
            unsigned int h = hash_fn(index);

            int head;
            idx_file.seekg(h * sizeof(int));
            idx_file.read(reinterpret_cast<char*>(&head), sizeof(int));

            int current_offset = head;
            while (current_offset != -1) {
                dat_file.seekg(current_offset);
                Entry e;
                dat_file.read(reinterpret_cast<char*>(&e), sizeof(Entry));
                if (!e.deleted && strcmp(e.index, index.c_str()) == 0 && e.value == value) {
                    e.deleted = true;
                    dat_file.seekp(current_offset);
                    dat_file.write(reinterpret_cast<const char*>(&e), sizeof(Entry));
                    dat_file.flush();
                    break;
                }
                current_offset = e.next_offset;
            }
        } else if (cmd == "find") {
            string index;
            cin >> index;
            unsigned int h = hash_fn(index);

            int head;
            idx_file.seekg(h * sizeof(int));
            idx_file.read(reinterpret_cast<char*>(&head), sizeof(int));

            vector<int> results;
            int current_offset = head;
            while (current_offset != -1) {
                dat_file.seekg(current_offset);
                Entry e;
                dat_file.read(reinterpret_cast<char*>(&e), sizeof(Entry));
                if (!e.deleted && strcmp(e.index, index.c_str()) == 0) {
                    results.push_back(e.value);
                }
                current_offset = e.next_offset;
            }

            if (results.empty()) {
                cout << "null" << "\n";
            } else {
                sort(results.begin(), results.end());
                for (size_t j = 0; j < results.size(); ++j) {
                    cout << results[j] << (j == results.size() - 1 ? "" : " ");
                }
                cout << "\n";
            }
        }
    }

    return 0;
}
