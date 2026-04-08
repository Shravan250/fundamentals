#include<iostream>
#include<fstream>
#include<string>
#include <nlohmann/json.hpp>
#include <filesystem>
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

const string database = "database.txt";
const string metadata = "metadata.txt";
const string tempfile = "temp-file.txt";
const string indexfile = "index.txt";
map<int, streampos> indexmap;

int getLastId(){
    ifstream inFile(metadata);
    int lastId;
    string line;

    if(inFile.is_open()){  
        while(getline(inFile , line)){
            json j = json::parse(line);
            lastId = j["id"];  
        }
    }else {
        cerr << "Error opening file for reading!" << endl;
    };
    inFile.close();

    return lastId;
};

void copyDatabase(){
    ifstream inFile(database);
    ofstream outFile(tempfile);
    string line;
    

    if(outFile.is_open() && inFile.is_open()){
        while(getline(inFile , line)){
            outFile << line << endl;
        }
        inFile.close();
        outFile.close();
    }else {
        cerr << "Error copying the database!" << endl;
    };
};

void readDatabaseBinary(){
    ifstream inFile(database, ios::binary);
    string line;
    int idCount = 1;

    if(inFile.is_open()){
        
        while(true){
            streampos pos = inFile.tellg();

            if(!getline(inFile, line)) break;

            indexmap[idCount] = pos;
            idCount++;
        }
        inFile.close();
    }else {
        cerr << "Error opening file for reading!" << endl;
    };
;}

void updateIndex(){
    ofstream outFile(indexfile);

    if(outFile.is_open()){
        for (const auto& [id, pos] : indexmap){
            json entry = {
                {"id", id},
                {"startpos",static_cast<long long>(pos)},
            };
            outFile << entry.dump() << endl;
        }
        
        cout << "Index updated!!" << endl;
        outFile.close();
    }else {
        cerr << "Error opening file for writing!" << endl;
    };
};

void writeFile(string data){
    int id = getLastId() + 1;

    ofstream metaFile(metadata);
    ofstream outFile(database, ios::app);
    if(outFile.is_open() && metaFile.is_open()){
        // creating json format 
        // string jsonLine = "{\"id\": " + to_string(id) + ", \"data\": \"" + data + "\", \"timestamp\": " + to_string(time(0)) + "}";
        // outFile << jsonLine << endl;
        json entry = {
            {"id", id},
            {"data", data},
            {"timestamp", time(0)}
        };
        outFile << entry.dump() << endl;

        // update metadata
        json meta = {
            {"id", id},
        };
        metaFile << meta.dump() << endl;

        metaFile.close();
        outFile.close();
    }else {
        cerr << "Error opening file for writing!" << endl;
    };
};

void readFile(){
    ifstream inFile(database);
    string line;

    if(inFile.is_open()){
        while(getline(inFile , line)){
            cout << line << endl;
        }
        inFile.close();
    }else {
        cerr << "Database is empty or doesn't exist." << endl;
    };
};

// TODO : refactor the function
void updateEntry(int targetId, string newData){
    copyDatabase();

    ifstream inFile(tempfile);
    string line;
    vector<json> entries;
    bool found = false;

    if(inFile.is_open()){
        while(getline(inFile , line)){
            if (line.empty()) continue;
            
            // updating the entry
            json j = json::parse(line);
            if (j["id"] == targetId) {
                j["data"] = newData;
                j["timestamp"] = time(0);
                found = true;
            }
            entries.push_back(j);       
        }
        inFile.close();
    }else {
        cerr << "Error opening file for reading!" << endl;
    };

    if (!found) {
        cout << "ID not found!" << endl;
        return;
    }


    //write to the temp-file
    ofstream outFile(tempfile);

    if(outFile.is_open()){
        // creating json format 
        for (const auto& entry : entries) {
            outFile << entry.dump() << "\n";
        }      
        outFile.close();
    }else {
        cerr << "Error opening temp-file for writing!" << endl;
    };

    // cout << "SIMULATING CRASH NOW..." << endl;
    // exit(0); // The program stops here

    //delete original database and make temp-file new original
    try {
        fs::remove(database);

        fs::rename(tempfile , database);
        cout << "Update Complete!" << endl;

    }catch(const fs::filesystem_error& e){
        cout << "File error: " << e.what() << endl;
    }
};


int main() {

    int choice;

    while (true) {
        cout << "\n1. Write to file\n2. Read from file\n3. Update entry\n4. Update index\n5. Exit\nChoice: ";
        if (!(cin >> choice)) break;
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
        }else if (choice == 5) {
            break;
        }
    }

    
    return 0;
};
