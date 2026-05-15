#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <string>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
using namespace std;

namespace fs = filesystem;
using json = nlohmann::ordered_json;

class LocalDB {
private:
    string dbDir;
    string dbFile;
    string metaFile;
    string idxFile;
    string schemaFile;
    unordered_map<int, streampos> indexmap;

    // Internal helper methods
    void loadIndexMap();
    int getLastId();
    void updateIndex();
    void appendIndex(int id, streampos pos);

public:
    // Constructor handles initialization
    LocalDB(string name);

    void write(json data);
    void readAll();
    void readById(int id);
    void update(int id, json newData);
    void deleteById(int id);
    void compact(); // cleanUpDatabase logic

    bool hasSchema();
    void setSchema(const vector<string>& keys);
    vector<string> getSchema();
    vector<json> query(string key, string value);
};


#endif
