#include<iostream>
#include<fstream>
#include<string>
using namespace std;

const string filename = "database.txt";

void writeFile(string data){
    ofstream outFile(filename, ios::app);
    if(outFile.is_open()){
        outFile << data <<"\n";
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
        cerr << "Error opening file for reading!" << endl;
    };
};

int main() {

    int choice;
    string userInput;

    while (true) {
        cout << "\n1. Write to file\n2. Read from file\n3. Exit\nChoice: ";
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            cout << "Enter text to save: ";
            getline(cin, userInput);
            writeFile(userInput);
        } else if (choice == 2) {
            readFile();
        } else if (choice == 3) {
            break;
        } else {
            cout << "Invalid choice!" << endl;
        }
    }

    
    return 0;
}
