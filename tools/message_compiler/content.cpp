
#include "content.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;


Content::Content(const string& fileName)
    : fileName_(fileName)
{
    ifstream msgFile(fileName);
    if (!msgFile.is_open())
    {
        cout << "open fail: " << fileName << endl;
        abort();
    }
    stringstream buf;
    buf << msgFile.rdbuf();
    data_ = buf.str();
    msgFile.close();

    lineBegin_ = cur_ = data_.data();
}
    
    
void Content::abortForParseError(const std::string& what) const
{
    std::cout << "parse error: " << what << " line:" << line_
              << " pos:" << cur_ - lineBegin_ << std::endl;
    abort();;
}

void Content::skipInvalidWords()
{
    while (*cur_ == ' ' || *cur_ == '\t' || *cur_ == '\n' || *cur_ == '\r' || *cur_ == '/')
    {
        if (*cur_ == '\n')
        {
            ++line_;
            lineBegin_ = cur_ + 1;
        }
        ++cur_;
        if (*(cur_ - 1) == '/')
        {
            if (*cur_ == '/')
            {
                while (*cur_ && *cur_ != '\n') ++cur_;
            }
            else if (*cur_ == '*')
            {
                while (true)
                {
                    ++cur_;
                    if (*cur_ == '*' && *++cur_ == '/')
                    {
                        ++cur_;
                        break;
                    }
                    else if (*cur_ == '\n')
                    {
                        ++line_;
                        lineBegin_ = cur_ + 1;
                    }
                    else if (*cur_ == '\0')
                    {
                        abortForParseError("no match */");
                    }
                }
            }
            else
            {
                --cur_;
                return;
            }
        }
    }
}


string Content::readString()
{
    string res;
    while (*cur_ && *cur_ != ' ' &&
            *cur_ != '\n' && *cur_ != '\r')
    {
        res.push_back(*cur_++);
    }
    return res;
}

string Content::readStringUtil(const std::string& escape)
{
    string res;
    string escapeAll = escape + " \t\n\r";
    while (*cur_ && std::find(escapeAll.begin(), escapeAll.end(), *cur_) == escapeAll.end())
    {
        res.push_back(*cur_++);
    }
    return res;
}

string Content::readIdentifier()
{
    string res;
    if (isalpha(*cur_) || (*cur_) == '_')
    {
        res.push_back(*cur_++);
        while (isalnum(*cur_) || (*cur_) == '_')
        {
            res.push_back(*cur_++);
        }
    }
    return res;
}


std::string addPadding(const std::string& str, int padding)
{
    const char* start = &*str.cbegin();
    const char* end = &*str.cend();
    string res;
    while (true)
    {
        const char* endline = std::find(start, end, '\n');
        res += string(padding, ' ') + string(start, endline);
        if (endline == end) return res;
        if (endline == end - 1) return res + "\n";
        res += "\n";
        start = endline + 1;
    }
}

