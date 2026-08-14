
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
    unsigned int hash = 2166136261u;
    for (char c : s) {
        hash ^= (unsigned char)c;
        hash *= 16777619u;
    }
    return hash % BUCKETS;
}

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void init_files() {
    if (!file_exists(INDEX_FILE)) {
        FILE* idx = fopen(INDEX_FILE, "wb");
        int val = -1;
        for (int i = 0; i < BUCKETS; ++i) {
            fwrite(&val, sizeof(int), 1, idx);
        }
        fclose(idx);
    }
    if (!file_exists(DATA_FILE)) {
        FILE* dat = fopen(DATA_FILE, "wb");
        fclose(dat);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    init_files();

    int n;
    if (!(cin >> n)) return 0;

    FILE* idx_fp = fopen(INDEX_FILE, "rb+");
    FILE* dat_fp = fopen(DATA_FILE, "rb+");

    for (int i = 0; i < n; ++i) {
        string cmd;
        cin >> cmd;
        if (cmd == "insert") {
            string index;
            int value;
            cin >> index >> value;
            unsigned int h = hash_fn(index);

            int head;
            fseek(idx_fp, h * sizeof(int), SEEK_SET);
            fread(&head, sizeof(int), 1, idx_fp);

            bool exists = false;
            int current_offset = head;
            while (current_offset != -1) {
                fseek(dat_fp, current_offset, SEEK_SET);
                Entry e;
                fread(&e, sizeof(Entry), 1, dat_fp);
                if (!e.deleted && strcmp(e.index, index.c_str()) == 0 && e.value == value) {
                    exists = true;
                    break;
                }
                current_offset = e.next_offset;
            }

            if (!exists) {
                fseek(dat_fp, 0, SEEK_END);
                int new_offset = ftell(dat_fp);
                Entry new_entry;
                memset(new_entry.index, 0, 64);
                strncpy(new_entry.index, index.c_str(), 63);
                new_entry.value = value;
                new_entry.next_offset = head;
                new_entry.deleted = false;
                fwrite(&new_entry, sizeof(Entry), 1, dat_fp);

                fseek(idx_fp, h * sizeof(int), SEEK_SET);
                fwrite(&new_offset, sizeof(int), 1, idx_fp);
                fflush(idx_fp);
            }
        } else if (cmd == "delete") {
            string index;
            int value;
            cin >> index >> value;
            unsigned int h = hash_fn(index);

            int head;
            fseek(idx_fp, h * sizeof(int), SEEK_SET);
            fread(&head, sizeof(int), 1, idx_fp);

            int current_offset = head;
            int prev_offset = -1;
            while (current_offset != -1) {
                fseek(dat_fp, current_offset, SEEK_SET);
                Entry e;
                fread(&e, sizeof(Entry), 1, dat_fp);
                if (!e.deleted && strcmp(e.index, index.c_str()) == 0 && e.value == value) {
                    // Remove from chain
                    if (prev_offset == -1) {
                        int next_head = e.next_offset;
                        fseek(idx_fp, h * sizeof(int), SEEK_SET);
                        fwrite(&next_head, sizeof(int), 1, idx_fp);
                        fflush(idx_fp);
                    } else {
                        Entry prev_e;
                        fseek(dat_fp, prev_offset, SEEK_SET);
                        fread(&prev_e, sizeof(Entry), 1, dat_fp);
                        prev_e.next_offset = e.next_offset;
                        fseek(dat_fp, prev_offset, SEEK_SET);
                        fwrite(&prev_e, sizeof(Entry), 1, dat_fp);
                        fflush(dat_fp);
                    }
                    e.deleted = true;
                    fseek(dat_fp, current_offset, SEEK_SET);
                    fwrite(&e, sizeof(Entry), 1, dat_fp);
                    fflush(dat_fp);
                    break;
                }
                prev_offset = current_offset;
                current_offset = e.next_offset;
            }
        } else if (cmd == "find") {
            string index;
            cin >> index;
            unsigned int h = hash_fn(index);

            int head;
            fseek(idx_fp, h * sizeof(int), SEEK_SET);
            fread(&head, sizeof(int), 1, idx_fp);

            vector<int> results;
            int current_offset = head;
            while (current_offset != -1) {
                fseek(dat_fp, current_offset, SEEK_SET);
                Entry e;
                fread(&e, sizeof(Entry), 1, dat_fp);
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

    fclose(idx_fp);
    fclose(dat_fp);
    return 0;
}
