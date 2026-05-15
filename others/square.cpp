#include<iostream>
using namespace std;


int square(int x){
    return x*x;
}

int main(){
    int n;
    cout << "Enter number: " << endl;
    cin >> n;

    cout << "Squared: " << square(n) << endl;

    return 0;
}
