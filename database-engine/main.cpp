#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

const string database = "database.txt";
const string metadata = "metadata.txt";
const string tempfile = "temp-file.txt";
const string indexfile = "index.txt";
unordered_map<int, streampos> indexmap;

int getLastId() {
  ifstream inFile(metadata);
  int lastId = 0;
  string line;

  if (inFile.is_open()) {
    while (getline(inFile, line)) {
      try {
        json j = json::parse(line);
        lastId = j["id"];
      } catch (const json::exception& e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else {
    cerr << "Error opening file for reading!" << endl;
  };

  return lastId;
};

void loadIndexMap() {
  ifstream inFile(database, ios::binary);
  string line;

  if (inFile.is_open()) {

    while (true) {
      streampos pos = inFile.tellg();

      if (!getline(inFile, line))
        break;
      if(line.empty()) continue;

      try{
        json j = json::parse(line);
        if (!j.contains("id")) continue;

        int actualId = j["id"];
        
        if (j.contains("deleted") && j["deleted"] == true){
            indexmap.erase(actualId);
        }else{
          indexmap[actualId] = pos;
        }
      } catch (const json::exception& e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else {
    cerr << "Error opening file for reading!" << endl;
  };
  ;
}

void updateIndex() {
  ofstream outFile(indexfile);

  if (outFile.is_open()) {
    try {
      for (const auto &[id, pos] : indexmap) {
        json entry = {
            {"id", id},
            {"startpos", static_cast<long long>(pos)},
        };
        outFile << entry.dump() << endl;
      }

      outFile.close();
      cout << "Index updated!!" << endl;
    } catch (const exception& e) {
      cerr << "Error writing index: " << e.what() << endl;
    }
  } else {
    cerr << "Error opening file for writing!" << endl;
  };
};

void appendIndex(int id, streampos pos) {
  ofstream outFile(indexfile, ios::app);

  if (outFile.is_open()) {
    try{
      json entry = {
          {"id", id},
          {"startpos", static_cast<long long>(pos)},
      };
      outFile << entry.dump() << endl;
      
      indexmap[id] = pos;

      cout << "Index updated!!" << endl;
      outFile.close();
    } catch (const exception& e) {
      cerr << "Error writing index: " << e.what() << endl;
    }
  } else {
    cerr << "Error opening file for writing!" << endl;
  };
}

void writeFile(string data) {
  if (data.empty()) {
    cerr << "Error: data cannot be empty!" << endl;
    return;
  }
  int id = getLastId() + 1;

  ofstream metaFile(metadata);
  ofstream outFile(database, ios::app | ios::binary);
  
  if (!outFile.is_open() || !metaFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try{

    // get the eof pos before write
    streampos startPos = outFile.tellp();

    // creating json format
    // string jsonLine = "{\"id\": " + to_string(id) + ", \"data\": \"" + data +
    // "\", \"timestamp\": " + to_string(time(0)) + "}"; outFile << jsonLine <<
    // endl;
    json entry = {{"id", id}, {"data", data}, {"timestamp", time(0)}};
    outFile << entry.dump() << endl;

    // update metadata
    json meta = {
        {"id", id},
    };
    metaFile << meta.dump() << endl;

    // appendIndex
    appendIndex(id, startPos);

    metaFile.close();
    outFile.close();
  } catch (const exception& e) {
    cerr << "Error writing file: " << e.what() << endl;
  }
};

void readFile() {
  ifstream inFile(database);
  string line;

  if (inFile.is_open()) {
    int count = 0;
    while (getline(inFile, line)) {
      try {
        json j = json::parse(line);
        if (!(j.contains("deleted") && j["deleted"] == true)) {
          cout << "ID: " << j["id"] << " | Data: " << j["data"] 
               << " | Timestamp: " << j["timestamp"] << endl;
          count++;
        }
      } catch (const json::exception& e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    cout << "Total records: " << count << endl;
    inFile.close();
  } else {
    cerr << "Database is empty or doesn't exist." << endl;
  };
};

void readById(int id) {
  if (id <= 0) {
    cout << "Invalid ID!" << endl;
    return;
  }

  auto it = indexmap.find(id);
  string line;

  if (it != indexmap.end()) {
    streampos address = it->second;
    ifstream inFile(database, ios::binary);

    if (!inFile.is_open()) {
      cerr << "Error opening database file!" << endl;
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
      } catch (const json::exception& e) {
        cerr << "JSON parse error: " << e.what() << endl;
      }
    }
    inFile.close();
  } else{
    cout << "Record Not Found!" << endl;
  }
};

// TODO : want to try to use slot method
// currently using 'stale' method
void updateEntry(int targetId, string newData) {
  if (newData.empty()) {
    cerr << "Error: new data cannot be empty!" << endl;
    return;
  }
    
    auto it = indexmap.find(targetId);

    if (it == indexmap.end()) {
        cout << "ID not found!" << endl;
        return;
    }

  ofstream outFile(database, ios::app | ios::binary);
  if (!outFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try {

    // get the eof pos before write
    streampos startPos = outFile.tellp();

    json entry = {{"id", targetId}, {"data", newData}, {"timestamp", time(0)}};
    outFile << entry.dump() << endl;

    // appendIndex
    appendIndex(targetId, startPos);
    outFile.close();
  } catch (const exception& e) {
    cerr << "Error updating entry: " << e.what() << endl;
  }
};

void cleanUpDatabase(){

    ofstream outFile(tempfile, ios::app | ios::binary);
    ifstream inFile(database, ios::binary);

    string line;

  if (!outFile.is_open() || !inFile.is_open()) {
    cerr << "Error opening files!" << endl;
    return;
  }

  try {
     
    for (const auto& [id, pos] : indexmap) {
        streampos startPos = outFile.tellp();

        inFile.seekg(pos);
      if (getline(inFile, line)) {
        outFile << line << "\n";
        indexmap[id] = startPos;
      }
     }

    outFile.close();
    inFile.close();


    // delete original database and make temp-file new original
    fs::remove(database);
    fs::rename(tempfile, database);
    cout << "Update Complete!" << endl;
    
      updateIndex();

  } catch (const fs::filesystem_error &e) {
    cout << "File error: " << e.what() << endl;
  } catch (const exception& e) {
    cout << "Error: " << e.what() << endl;
  }
    

};

void deleteById(int id){
  if (id <= 0) {
    cout << "Invalid ID!" << endl;
    return;
  }

    auto it = indexmap.find(id);
  if (it == indexmap.end()) {
    cout << "Record not found!" << endl;
    return;
  }

    ofstream outFile(database, ios::app);
  if (!outFile.is_open()) {
    cerr << "Error opening file for writing!" << endl;
    return;
  }

  try {
    json tombstone = {{"id", id}, {"deleted", true}};
    outFile << tombstone.dump() << endl;
    outFile.close();
    indexmap.erase(it->first);
    cout << "Record deleted successfully!" << endl;
  } catch (const exception& e) {
    cerr << "Error deleting record: " << e.what() << endl;
  }
}

int main() {

  int choice;
  loadIndexMap();

  while (true) {
    cout << "\n1. Write to file\n2. Read from file\n3. Update entry\n4. Update "
            "index\n5. Read by ID\n6. Delete by Id\n7. Clean database\n8. Exit\nChoice: ";
    if (!(cin >> choice))
      break;
    cin.ignore();

    if (choice == 1) {
      string userInput;

      cout << "Enter text: ";
      getline(cin, userInput);

      writeFile(userInput);
    } else if (choice == 2) {
      readFile();
    } else if (choice == 3) {
      int id;
      string newData;

      cout << "Enter ID to update: ";
      cin >> id;
      cin.ignore();

      cout << "Enter new data: ";
      getline(cin, newData);

      updateEntry(id, newData);
    } else if (choice == 4) {
      updateIndex();
    } else if (choice == 5) {
      int id;

      cout << "Enter ID of record: ";
      cin >> id;
      cin.ignore();

      readById(id);
    } else if (choice == 6) {
      int id;

      cout << "Enter ID of record: ";
      cin >> id;
      cin.ignore();

      deleteById(id);
    } else if (choice == 7) {
      cleanUpDatabase();
    } else if (choice == 8) {
      break;
    }
  }

  return 0;
};
