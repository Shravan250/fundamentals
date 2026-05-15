#include "Database.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ctime>

using namespace std;

// CONSTRUCTOR: Runs automatically  do : 'LocalDB db("myDB");
LocalDB::LocalDB(string name) {
    dbDir = "databases/" + name;
    dbFile = dbDir + "/" + name + ".txt";
    metaFile = dbDir + "/" + name + "_metadata.txt";
    idxFile = dbDir + "/" + name + "_index.txt";
    schemaFile = dbDir + "/" + name + "_schema.txt";

    if (!fs::exists(dbDir)) {
        fs::create_directories(dbDir);
    }

    if (!fs::exists(dbFile)) {
        ofstream create(dbFile);
        create.close();
    }
    
    if (!fs::exists(metaFile)) {
        ofstream create(metaFile);
        create.close();
    }

    if (!fs::exists(schemaFile)) {
        ofstream create(schemaFile);
        create.close();
    }
  // create index mao on system load
  loadIndexMap();
}

void LocalDB::loadIndexMap() {
  ifstream inFile(dbFile, ios::binary);
  string line;

  if (inFile.is_open()) {

    while (true) {
      streampos pos = inFile.tellg();

      if (!getline(inFile, line))
        break;
      if (line.empty()) continue;

      try {
        json j = json::parse(line);
        if (!j.contains("id")) continue;

        int actualId = j["id"];

        if (j.contains("deleted") && j["deleted"] == true) {
          indexmap.erase(actualId);
        } else {
          indexmap[actualId] = pos;
        }
      } catch (const json::exception & e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else {
    cerr << "Error opening file for reading!" << endl;
  };
}

// BUG : creating race conditoin in multiple write need to create local variable
int LocalDB::getLastId() {
  ifstream inFile(metaFile);
  int lastId = 0;
  string line;

  if (inFile.is_open()) {
    while (getline(inFile, line)) {
      try {
        json j = json::parse(line);
        lastId = j["id"];
      } catch (const json::exception & e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else {
    cerr << "Error opening file for reading!" << endl;
  };

  return lastId;
}

void LocalDB::appendIndex(int id, streampos pos) {
  ofstream outFile(idxFile, ios::app);

  if (outFile.is_open()) {
    try {
      json entry = {
        {
          "id",
          id
        },
        {
          "startpos",
          static_cast < long long > (pos)
        },
      };
      outFile << entry.dump() << endl;

      indexmap[id] = pos;

      cout << "Index updated!!" << endl;
      outFile.close();
    } catch (const exception & e) {
      cerr << "Error writing index: " << e.what() << endl;
    }
  } else {
    cerr << "Error opening file for writing!" << endl;
  };
}

bool LocalDB::hasSchema(){
    return fs::exists(schemaFile) && fs::file_size(schemaFile) > 0;
}

void LocalDB::setSchema(const vector<string>& keys){
    ofstream outFile(schemaFile);

    if(outFile.is_open()){
        json schema = json::object();

        for(const auto& key : keys){
            schema[key] = "";
        }

        outFile << schema.dump() << endl;
        outFile.close();
    }else{
        cerr << "Error opening schema file for writing!" << endl;
    }
};

vector<string> LocalDB::getSchema(){
    ifstream inFile(schemaFile);
    vector<string> keys;
    string line;

    if (inFile.is_open() && getline(inFile, line)) {
      try {
        json j = json::parse(line);
        
        if(j.is_object()){
            for(auto& it : j.items()){
                keys.push_back(it.key());
            }
        }

        inFile.close();
      } catch (const json::exception & e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    } else {
    cerr << "Error opening schema file for reading!" << endl;
  };

    return keys;
};

void LocalDB::write(json data) {
  if (data.empty()) {
    cerr << "Error: data cannot be empty!" << endl;
    return;
  }
  int id = getLastId() + 1;

  ofstream inFile(metaFile);
  ofstream outFile(dbFile, ios::app | ios::binary);

  if (!outFile.is_open() || !inFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try {

    // get the eof pos before write
    streampos startPos = outFile.tellp();

    data["id"] = id;
    outFile << data.dump() << endl;

    // update metaFile
    inFile << json({{"id", id}}).dump() << endl;

    // appendIndex
    appendIndex(id, startPos);

    inFile.close();
    outFile.close();
  } catch (const exception & e) {
    cerr << "Error writing file: " << e.what() << endl;
  }
}

void LocalDB::readAll() {
  ifstream inFile(dbFile);
  string line;

  if (inFile.is_open()) {
    int count = 0;
    vector<string> keys = getSchema();

    while (getline(inFile, line)) {
      try {
        json j = json::parse(line);

        if (!(j.contains("deleted") && j["deleted"] == true)) {
          cout << "ID: " << j["id"];

          for (const auto& key : keys) {
            if (j.contains(key)) {
              cout << " | " << key << ": " << j[key];
            }
          }

          cout << endl;
          count++;
        }
      } catch (const json::exception & e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }

    cout << "Total records: " << count << endl;
    inFile.close();
  } else {
    cerr << "Database is empty or doesn't exist." << endl;
  }
}

void LocalDB::readById(int id) {
  if (id <= 0) {
    cout << "Invalid ID!" << endl;
    return;
  }

  auto it = indexmap.find(id);
  string line;

  if (it != indexmap.end()) {
    streampos address = it -> second;
    ifstream inFile(dbFile, ios::binary);

    if (!inFile.is_open()) {
      cerr << "Error opening dbFile file!" << endl;
      return;
    }

    inFile.seekg(address);

    if (getline(inFile, line)) {
      try {
        json j = json::parse(line);
        if (j.contains("deleted") && j["deleted"] == true) {
          cout << "Record has been deleted!" << endl;
        } else {
          cout << "Found Record: " << line << endl;
        }
      } catch (const json::exception & e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else {
    cout << "Record Not Found!" << endl;
  }
}

// TODO : want to try to use slot method
// currently using 'stale' method
void LocalDB::update(int targetId, json newData) {
  if (newData.empty()) {
    cerr << "Error: new data cannot be empty!" << endl;
    return;
  }

  auto it = indexmap.find(targetId);

  if (it == indexmap.end()) {
    cout << "ID not found!" << endl;
    return;
  }

  ofstream outFile(dbFile, ios::app | ios::binary);
  if (!outFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try {

    // get the eof pos before write
    streampos startPos = outFile.tellp();

    newData["id"] = targetId;
    
    outFile << newData.dump() << endl;

    // appendIndex
    appendIndex(targetId, startPos);
    outFile.close();
  } catch (const exception & e) {
    cerr << "Error updating entry: " << e.what() << endl;
  }
}

void LocalDB::updateIndex() {
  ofstream outFile(idxFile);

  if (outFile.is_open()) {
    try {
      for (const auto & [id, pos]: indexmap) {
        json entry = {
          {
            "id",
            id
          },
          {
            "startpos",
            static_cast < long long > (pos)
          },
        };
        outFile << entry.dump() << endl;
      }

      outFile.close();
      cout << "Index updated!!" << endl;
    } catch (const exception & e) {
      cerr << "Error writing index: " << e.what() << endl;
    }
  } else {
    cerr << "Error opening file for writing!" << endl;
  };
}

void LocalDB::deleteById(int id) {
  if (id <= 0) {
    cout << "Invalid ID!" << endl;
    return;
  }

  auto it = indexmap.find(id);
  if (it == indexmap.end()) {
    cout << "Record not found!" << endl;
    return;
  }

  ofstream outFile(dbFile, ios::app);
  if (!outFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try {
    json tombstone = {
      {
        "id",
        id
      },
      {
        "deleted",
        true
      }
    };
    outFile << tombstone.dump() << endl;
    outFile.close();
    indexmap.erase(it -> first);
    cout << "Record deleted successfully!" << endl;
  } catch (const exception & e) {
    cerr << "Error deleting record: " << e.what() << endl;
  };
}

void LocalDB::compact() {
  const string tempfile = "tempfile.txt";
  ofstream outFile(tempfile, ios::app | ios::binary);
  ifstream inFile(dbFile, ios::binary);

  string line;

  if (!outFile.is_open() || !inFile.is_open()) {
    cerr << "Error opening files!" << endl;
    return;
  }

  try {

    for (const auto & [id, pos]: indexmap) {
      streampos startPos = outFile.tellp();

      inFile.seekg(pos);
      if (getline(inFile, line)) {
        outFile << line << "\n";
        indexmap[id] = startPos;
      }
    }

    outFile.close();
    inFile.close();

    // delete original dbFile and make temp-file new original
    fs::remove(dbFile);
    fs::rename(tempfile, dbFile);
    cout << "Update Complete!" << endl;

    updateIndex();

  } catch (const fs::filesystem_error & e) {
    cout << "File error: " << e.what() << endl;
  } catch (const exception & e) {
    cout << "Error: " << e.what() << endl;
  };
}

vector<json> query(string key, string value){
    
}
