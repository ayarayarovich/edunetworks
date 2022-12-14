#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;

struct Person {
    char name[25];
    int height;
    int weight;
}B;

int answer;
long size_pred;

int main()
{
    ifstream fR;
    ofstream fA;
    const char* nameR = "C:/REQUEST.txt";
    const char* nameA = "C:/ANSWER.txt";
    cout << "Server is running...\n";
    fR.open(nameR);
    fR.seekg(0, ios::end);
    size_pred = fR.tellg();
    while (true) {
        fR.seekg(0, ios::end);
        while (size_pred >= fR.tellg())
        {
            Sleep(100);
            fR.seekg(0, ios::end);
        }
        fR.seekg(size_pred, ios::beg);
        fR >> B.name >> B.height >> B.weight;
        fR.seekg(0, ios::end);
        size_pred = fR.tellg();
        cout << "Data is received: " << B.name << " " << B.height << " " << B.weight << endl;
        double IMT = B.weight / (0.01 * B.height) / (0.01 * B.height);
        if (18.5 <= IMT && IMT < 25) answer = 1;
        if (18.5 > IMT) answer = 0;
        if (IMT >= 25) answer = 2;
        cout << "Data is processed\n";
        fA.open(nameA, ios::app);
        fA << answer << "\n";
        fA.close();
    }
    fR.close();
}
