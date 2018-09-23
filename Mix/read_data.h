#pragma once

#include <iostream>  
#include <fstream>  
#include <string>  
#include <vector>  

using namespace std;

class read_data
{
public:
	read_data();
	~read_data();

public:

	void csvline_populate(vector<string> &record, const string& line, char delimiter);



};

