#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;
struct Person
{
    char name[25];
    int height;
    int weight;
} A;

int main()
{
    const char* nameR = "C:/REQUEST.bin";


    const char* nameA = "C:/ANSWER.bin";

    ofstream f_REQ;
    ifstream f_ANS;
    long pred_size;
    int answer;

    while (true) {

        cout << "Enter: name, height, weight" << endl;
        cin >> A.name >> A.height >> A.weight;


        f_REQ.open(nameR, ios::app | ios::binary);


        f_REQ.write((char*)&A, sizeof(A));


        f_REQ.close();


        f_ANS.open(nameA, ios::binary);


        f_ANS.seekg(0, ios::end);


        pred_size = f_ANS.tellg();
        while (pred_size >= f_ANS.tellg())
        {
            Sleep(100);


            f_ANS.seekg(0, ios::end);

        }

        f_ANS.seekg(pred_size, ios::beg);


        f_ANS.read((char*)&answer, sizeof(answer));


        f_ANS.close();


        switch (answer) {
        case 0: {cout << "Nedostak vesa\n"; break; }
        case 1: {cout << "Norma vesa\n"; break; }
        case 2: {cout << "Izbitok vesa\n"; break; }
        }
    }
}