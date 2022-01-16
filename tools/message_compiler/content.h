#ifndef CONTENT_H
#define CONTENT_H

#include <string>
#include <vector>
#include <set>
#include <memory>

struct Content
{
    Content(const std::string& fileName);

    char next() { return *++cur_; }
    char cur() const { return *cur_; }
    const char* ptr() const { return cur_; }
    const char* move(int offset) { cur_ += offset; return cur_; }

    const std::string& fileName() const { return fileName_; }

    std::string readString();
    std::string readStringUtil(const std::string& escape);
    std::string readIdentifier();

    template <typename T>
    T readUInt();

    template <typename T>
    T readHex();

    void skipInvalidWords();
    void abortForParseError(const std::string& what) const;


    std::vector<std::string> namespaceDef;
    std::set<std::string> includeList;
    std::vector<std::string> includeString;
    std::vector<std::string> messageNames;
    std::string result;

private:
    std::string fileName_;
    std::string data_;

    const char* cur_;
    const char* lineBegin_;
    int line_ = 1;
};


template <typename T>
T Content::readUInt()
{
    T num = *cur_++ - '0';
    while (isdigit(*cur_))
    {
        num = num * 10 + (*cur_++ - '0');
    }
    return num;
}

template <typename T>
T Content::readHex()
{
    T num = *cur_++ - '0';
    while (isdigit(*cur_))
    {
        num = num * 16 + (*cur_++ - '0');
    }
    return num;
}

class TypeParser;

struct Message
{
    std::string name;
    std::vector<std::shared_ptr<TypeParser>> typeParsers;

    std::set<std::string> variableNames;
    std::set<int> fieldNumbers;
    int level = 0;

    std::string subTypeDef;
    std::string result;

    std::string tag;
};

std::string addPadding(const std::string& str, int padding);

#endif // CONTENT_H
