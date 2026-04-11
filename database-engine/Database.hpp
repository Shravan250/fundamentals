#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
using namespace std;

namespace fs = filesystem;
using json = nlohmann::json;

class LocalDB {
private:
    string dbDir;
    string dbFile;
    string metaFile;
    string idxFile;
    unordered_map<int, streampos> indexmap;

    // Internal helper methods
    void loadIndexMap();
    int getLastId();
    void updateIndex();
    void appendIndex(int id, streampos pos);

public:
    // Constructor handles initialization
    LocalDB(string name);

    void write(string data);
    void readAll();
    void readById(int id);
    void update(int id, string newData);
    void deleteById(int id);
    void compact(); // cleanUpDatabase logic
};


#endif
