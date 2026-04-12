#include "Database.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

int main() {

    string dbName;
    int startChoice;

    cout << "1. Create New Database\n2. Use Existing Database\nChoice: ";
    cin >> startChoice;
    cin.ignore();

    if(startChoice == 1){
        cout << "Enter new database name: ";
        getline(cin, dbName);
    } 
    else if(startChoice == 2){

        vector<string> dbs;

        if(fs::exists("databases")){
            for(const auto& entry : fs::directory_iterator("databases")){
                if(entry.is_directory()){
                    dbs.push_back(entry.path().filename().string());
                }
            }
        }

        if(dbs.empty()){
            cout << "No databases found. Creating new one.\n";
            cout << "Enter database name: ";
            getline(cin, dbName);
        }else{
            cout << "Existing Databases:\n";
            for(size_t i=0;i<dbs.size();i++){
                cout << i+1 << ". " << dbs[i] << endl;
            }

            int choice;
            cout << "Select: ";
            cin >> choice;
            cin.ignore();

            dbName = dbs[choice-1];
        }
    }

    LocalDB db(dbName);

    // schema setup
    if(!db.hasSchema()){
        int n;
        cout << "No schema found. Enter number of fields: ";
        cin >> n;
        cin.ignore();

        vector<string> keys;
        for(int i = 0; i < n; i++){
            string key;
            cout << "Field " << i+1 << ": ";
            getline(cin, key);
            keys.push_back(key);
        }

        db.setSchema(keys);
    }

    int choice;

    while (true) {
        cout << "\n1. Write\n2. Read All\n3. Update\n4. Read By ID\n5. Delete\n6. Compact\n7. Exit\nChoice: ";
        if (!(cin >> choice)) break;
        cin.ignore();

        if (choice == 1) {
            json data;
            vector<string> keys = db.getSchema();

            for(const auto& key : keys){
                string value;
                cout << key << ": ";
                getline(cin, value);
                data[key] = value;
            }

            db.write(data);

        } else if (choice == 2) {
            db.readAll();

        } else if (choice == 3) {
            int id;
            cout << "ID: ";
            cin >> id;
            cin.ignore();

            json data;
            vector<string> keys = db.getSchema();

            for(const auto& key : keys){
                string value;
                cout << key << ": ";
                getline(cin, value);
                data[key] = value;
            }

            db.update(id, data);

        } else if (choice == 4) {
            int id;
            cout << "ID: ";
            cin >> id;
            db.readById(id);

        } else if (choice == 5) {
            int id;
            cout << "ID: ";
            cin >> id;
            db.deleteById(id);

        } else if (choice == 6) {
            db.compact();

        } else if (choice == 7) {
            break;
        }
    }
    return 0;
}
