//--------------------------------------------------------------------------------
// STRING UTILS
//--------------------------------------------------------------------------------
#pragma once

#include "../common.h"

#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <iomanip>


AMN_NAMESPACE_OPEN_SCOPE

// STRING END WITH
//---------------------------------------------------------------------------------------
static bool EndsWithString(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// STRING START WITH
//---------------------------------------------------------------------------------------
static bool StartsWithString(std::string const & value, std::string const & starting)
{
    if (starting.size() > value.size()) return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}

// SPLIT STRING
//---------------------------------------------------------------------------------------
static std::vector<std::string> SplitString(const std::string& s, const std::string& delim)
{
    std::vector<std::string> tokens;
    
    std::size_t start = s.find_first_not_of(delim), end = 0;
    if(start == 1)tokens.push_back("");

    while((end = s.find_first_of(delim, start)) != std::string::npos)
    {
        if(end-start == 1)tokens.push_back("");
        else tokens.push_back(s.substr(start, end - start));
        start = s.find_first_not_of(delim, end);
    }
    if(start != std::string::npos)
    tokens.push_back(s.substr(start));
    
    return tokens;
}


// JOIN STRING
//---------------------------------------------------------------------------------------
static std::string JoinString(const std::vector<std::string>& ss, const std::string& delim)
{
    std::string result;
    size_t last = ss.size() - 1;
    for(int i=0;i<last;++i)
      result += ss[i] + delim;
    
    return result + ss[last];
}

// COUNT STRING
//---------------------------------------------------------------------------------------
static int CountString(const std::string & str, const std::string & find)
{
    int count = 0;
    size_t nPos = str.find(find, 0); // fist occurrence
    while(nPos != std::string::npos)
    {
        count++;
        nPos = str.find(find, nPos+1);
    }
    return count;
}

// REPLACE STRING
//---------------------------------------------------------------------------------------
static std::string ReplaceString(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    
    if(start_pos == std::string::npos)
    	return str;
    std::string out_str = str;
    out_str.replace(start_pos, from.length(), to);
    	return out_str;
}

// REPLACE ALL STRING
//---------------------------------------------------------------------------------------
static std::string ReplaceAllString(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos;
    bool search = true;
    std::string out_str = str;
    
    while(search == true)
    {
        start_pos = out_str.find(from);
        if(start_pos != std::string::npos)
    		out_str.replace(start_pos, from.length(), to);
        else
        	search = false;
    }
    return out_str;
}

// FRAME TO STRING
//---------------------------------------------------------------------------------------
static std::string FrameToString(int frame, int padding)
{
	std::stringstream ss;
	ss << std::setw(padding) << std::setfill('0') << frame;
	std::string fs = ss.str();
	return fs;
}

// GET STRING PADDING
//---------------------------------------------------------------------------------------
static int GetPadding(const std::string& padding_string)
{
	char nb = padding_string[2];
	return  atoi(&nb);
}

/*
// RANDOM STRING
//---------------------------------------------------------------------------------------
std::string randomString(size_t length)
{
  const char* charmap = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  const size_t charmapLength = strlen(charmap);
  std::string result;
  auto generator = [&](){ return charmap[rand()%charmapLength]; };
  string result;
  result.reserve(length);
  std::generate_n(std::back_inserter(result), length, generator);
  return result;
}
*/

AMN_NAMESPACE_CLOSE_SCOPE

template <typename T> 
static std::string ToString(const T& t)
{
	std::ostringstream os;
	os << t;
	return os.str();
}
