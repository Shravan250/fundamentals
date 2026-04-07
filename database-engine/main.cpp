#include<iostream>
#include<fstream>
#include<string>
#include <nlohmann/json.hpp>
#include <filesystem>
using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

const string filename = "database.txt";
const string metadata = "metadata.txt";
const string tempfile = "temp-file.txt";


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
}

void copyDatabase(){
    ifstream inFile(filename);
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

}

void writeFile(string data){
    int id = getLastId() + 1;

    ofstream metaFile(metadata);
    ofstream outFile(filename, ios::app);
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
    ifstream inFile(filename);
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
        fs::remove(filename);

        fs::rename(tempfile , filename);
        cout << "Update Complete!" << endl;

    }catch(const fs::filesystem_error& e){
        cout << "File error: " << e.what() << endl;
    }
}


int main() {

    int choice;

    while (true) {
        cout << "\n1. Write to file\n2. Read from file\n3. Update entry\n4. Exit\nChoice: ";
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
            break;
        }
    }

    
    return 0;
}
