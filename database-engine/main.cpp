#include "Database.hpp"
#include <iostream>
#include <string>

using namespace std;

int main() {
    LocalDB db("example");
    int choice;

    while (true) {
        cout << "\n1. Write\n2. Read All\n3. Update\n4. Read By ID\n5. Delete\n6. Compact\n7. Exit\nChoice: ";
        if (!(cin >> choice)) break;
        cin.ignore();

        if (choice == 1) {
            string data;
            cout << "Data: ";
            getline(cin, data);
            db.write(data);
        } else if (choice == 2) {
            db.readAll();
        } else if (choice == 3) {
            int id;
            string data;
            cout << "ID: "; cin >> id; cin.ignore();
            cout << "New Data: "; getline(cin, data);
            db.update(id, data);
        } else if (choice == 4) {
            int id;
            cout << "ID: "; cin >> id;
            db.readById(id);
        } else if (choice == 5) {
            int id;
            cout << "ID: "; cin >> id;
            db.deleteById(id);
        } else if (choice == 6) {
            db.compact();
        } else if (choice == 7) {
            break;
        }
    }
    return 0;
}
