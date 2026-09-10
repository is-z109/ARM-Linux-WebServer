#pragma once
#include<fstream>
#include<iostream>
#include<mutex>
#include<string>
using namespace std;
class GPIO{
    string pin;
    static string direction_path;
    static string value_path;
public:
    GPIO(string p):pin(p){};
    void GPIO_Init();
    void GPIO_Write(char value);
    void GPIO_direction(string de);
    char GPIO_GetInput();
    ~GPIO();
};