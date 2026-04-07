#include<iostream>
#include<fstream>
#include<string>
using namespace std;

const string filename = "database.txt";
const string metadata = "metadata.txt";

//  TODO : use metadata.txt for this
int getLastId(){
    ifstream inFile(filename);
    int lineCount = 0; 
    string line;

    if(inFile.is_open()){  
        while(getline(inFile , line)){
            lineCount++;
        }
    }else {
        cerr << "Error opening file for reading!" << endl;
    };
    inFile.close();

    return lineCount;
}

void writeFile(string data){
    int id = getLastId() + 1;

    ofstream outFile(filename, ios::app);
    if(outFile.is_open()){
        // creating json format 
        string jsonLine = "{\"id\": " + to_string(id) + ", \"data\": \"" + data + "\", \"timestamp\": " + to_string(time(0)) + "}";
        
        outFile << jsonLine << endl;
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
