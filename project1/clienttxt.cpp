#include <iostream>
#include <fstream>
#include <windows.h>
using namespace std;

struct Person {
	char name[25];
	int height;
	int weight;
}A;

int answer;
long size_pred;

int main()
{
	setlocale(LC_ALL, "rus");
	ofstream fR;
	ifstream fA;
	const char* nameR = "C:/REQUEST.txt";
	const char* nameA = "C:/ANSWER.txt";
	while (1) {
		cout << "Enter: name, height, weight" << endl;
		cin >> A.name >> A.height >> A.weight;
		fR.open(nameR, ios::app);
		fR << A.name << " " << A.height << " " << A.weight << "\n";
		fR.seekp(0, ios::end);
		fR.close();
		fA.open(nameA);
		fA.seekg(0, ios::end);
		size_pred = fA.tellg();
		while (size_pred >= fA.tellg())
		{
			Sleep(100);
			fA.seekg(0, ios::end);
		}
		fA.seekg(size_pred, ios::beg);
		fA >> answer;
		fA.close();
		switch (answer) {
		case 0: {cout << "Nedostatok vesa\n"; break; }
		case 1: {cout << "Norma vesa\n"; break; }
		case 2: {cout << "Izbitok vesa\n"; break; }
		}
	}
}