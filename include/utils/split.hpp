#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

vector<string> split(const string& str, char delimeter) {
    vector<string> result;
    stringstream ss(str);
    string item;

    while (getline(ss, item, delimeter)) {
        result.push_back(item);
    }

    return result;
}

vector<string> splitToChars(const string& str) {
    vector<string> result;
    for (char c : str) {
        string s;
        s += c;
        result.push_back(s);
    }
    return result;
}