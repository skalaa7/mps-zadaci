#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;
#define NUM 10000
#define SEED 1

int main()
{
    ofstream MyFile("baza10000.txt");

    srand(SEED);
    for(int i=0;i<NUM;i++)
    {
        for(int j=0;j<NUM;j++)
        {
            MyFile<<((double)((rand()%100)*pow(-1.0,(double)(rand()%2))))/10;
            MyFile<<" ";
        }
        MyFile<<endl;
    }
    for(int i =0;i<NUM;i++)
    {
        MyFile<<(((double)(rand()%200))/10)*(-1);
        MyFile<<" ";
    }
    MyFile<<endl;
    for(int i =0;i<NUM;i++)
    {
        MyFile<<(200+rand()%9800);
        MyFile<<" ";
    }

    return 0;
}
