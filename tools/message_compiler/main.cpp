
#include <iostream>
#include <fstream>
#include <sstream>

#include "content.h"
#include "type_parser.h"

using namespace std;


string generateCodeForTypedef(Content& text, const Message& message)
{
    ostringstream code, extra;
    int padding = 4 * message.level;

    for (const auto& x : message.typeParsers)
    {
        for (const auto& inc : x->includeList())
        {
            auto res = text.includeList.insert(inc);
            if (res.second)
            {
                text.includeString.push_back(*res.first);
            }
        }
        code << string(padding + 4, ' ') << x->realType() << " " << x->variableName() << ";\n";
        if (!x->defaultValue().empty()) code << " = " << x->defaultValue();

        if (x->useCachedSize())
        {
            extra << string(padding + 4, ' ') << "mutable int " << x->variableName() << "_cachedSize_;\n";
        }
    }
    string extraStr = extra.str();
    if (!extraStr.empty())
    {
        code << string(padding, ' ') << "private:\n" << extraStr
             << string(padding, ' ') << "public:\n";
    }
    return code.str();
}


string generateCodeForByteSize(const Message& message)
{
    ostringstream code, extra;
    int padding = 4 * (message.level + 1);

    int count = message.typeParsers.size();
    int i = 0;
    for (const auto& x : message.typeParsers)
    {
        if (!x->useCachedSize())
        {
            code << addPadding(x->codeForByteSize(), 12);
        }
        else
        {
            code << addPadding(x->codeForCachedSize(), 12);
            extra << addPadding(x->codeForByteSize() + "\n", 4);
        }

        if (++i != count)
        {
            code << " + \n";
        }
        else
        {
            code << ";";
        }
    }

    return addPadding(
               "int byteSize() const override\n{\n" + extra.str() +
               "    return // byte size\n" + code.str() + "\n}\n", padding);
}


string generateCodeForWriter(const Message& message)
{
    ostringstream code;
    int padding = 4 * (message.level + 1);

    code << addPadding(
             "int encodeToBytes(char* buf) const override\n"
             "{\n"
             "    net::CodedStreamWriter out(buf);\n", padding);

    for (const auto& x : message.typeParsers)
    {
        code << addPadding(x->codeForWriter(), padding + 4) << "\n";
    }
    code << addPadding(
             "    return out.size();\n"
             "}\n", padding);

    return code.str();
}

string generateCodeForReader(const Message& message)
{
    ostringstream code;
    int padding = 4 * (message.level + 1);

    code << addPadding(
             "bool decodeFromBytes(const char* buf, int len) override\n"
             "{\n"
             "    net::CodedStreamReader in(buf, buf + len);\n"
             "    while (in)\n"
             "    {\n"
             "        int tag = in.readTag();\n"
             "        switch (net::fieldNumberOfTag(tag))\n"
             "        {\n", padding);
    int casePadding = padding + 12;
    for (const auto& x : message.typeParsers)
    {
        code << addPadding("case " + to_string(x->fieldNumber()) + ":\n{\n", casePadding)
             << addPadding(x->codeForReader(), casePadding + 4)
             << addPadding("\n    break;\n}\n", casePadding);
    }
    code << addPadding(
             "default:\n"
             "    if (!in.readUnknownField(tag)) return false;\n"
             "    break;\n", casePadding);
    code << addPadding(
             "        }\n"
             "    }\n"
             "    return true;\n"
             "}\n", padding);
    return code.str();
}


void parseMessage(Content& text, Message* parent = nullptr);


void parseItem(Content& text, Message* message)
{
    string firstWord = text.readIdentifier();
    text.skipInvalidWords();

    string typeName = firstWord;
    vector<std::shared_ptr<TypeParser>> innerTypes;
    if (firstWord == "optional")
    {
        typeName = text.readIdentifier();
    }
    else if (firstWord == "repeated")
    {
        typeName = "vector";
        innerTypes.push_back(TypeParserFactory::instance().create(text.readIdentifier()));
    }
    else if (firstWord == "required")
    {
        typeName = text.readIdentifier();
    }
    else if (firstWord == "message")
    {
        parseMessage(text, message);
        return;
    }
    text.skipInvalidWords();

    auto parser = TypeParserFactory::instance().create(typeName);
    parser->setTypeName(typeName);
    parser->setInnerTypes(innerTypes);
    parser->parseItem(text, message);

    message->typeParsers.push_back(std::move(parser));
}


void parseMessage(Content& text, Message* parent)
{
    Message message;
    if (parent) message.level = parent->level + 1;
    message.name = text.readIdentifier();
    text.skipInvalidWords();

    if (text.cur() != '{') text.abortForParseError("lose '{'");
    text.next();

    text.skipInvalidWords();

    if (parent != nullptr)
    {
        message.tag = parent->tag + "_";
    }
    else
    {
        for (const auto& x : text.namespaceDef) message.tag += x + "_";
    }
    message.tag += message.name;

    while (text.cur() && text.cur() != '}')
    {
        parseItem(text, &message);
        text.skipInvalidWords();
    }
    if (text.cur() == '\0') text.abortForParseError("lose '}'");
    text.next();

    ostringstream oss;
    int padding = message.level * 4;
    oss << "\n" << string(padding, ' ') << "struct " << message.name
        << " : net::Message\n" << string(padding, ' ') << "{\n";

    oss << string(padding + 4, ' ') << "MESSAGE_TAG(" << message.tag << ")\n";

    oss << message.subTypeDef << "\n"
        << generateCodeForTypedef(text, message) << endl
        << generateCodeForWriter(message) << endl
        << generateCodeForReader(message) << endl
        << string(padding, ' ') << "protected:\n"
        << generateCodeForByteSize(message) << endl
        << string(padding, ' ') << "};\n";
    if (parent == nullptr)
        text.result += oss.str();
    else
        parent->subTypeDef = oss.str();
}

void parseContent(const string& fileName)
{
    Content text(fileName);
    text.includeString.push_back("\"net_ext/message/coded_stream.h\"");

    while (true)
    {
        text.skipInvalidWords();
        if (text.cur() == '\0') break;
        string keyword = text.readIdentifier();
        text.skipInvalidWords();
        if (keyword == "message")
        {
            parseMessage(text);
        }
        else if (keyword == "import")
        {
            text.includeString.push_back(text.readString());
        }
        else if (keyword == "package")
        {
            text.namespaceDef.push_back(text.readIdentifier());
            while (text.cur() == '.')
            {
                text.next();
                text.namespaceDef.push_back(text.readIdentifier());
            }
            text.skipInvalidWords();
            if (text.cur() != ';') text.abortForParseError("lose ;");
            text.next();
        }
        else
        {
            text.abortForParseError("unknown keyword");
        }
    }

    string headFileName = fileName + ".h";

    string realFileName = headFileName;
    size_t pos = headFileName.find_last_of('/');
    if (pos != std::string::npos)
    {
        realFileName = headFileName.substr(pos + 1, -1);
    }
    else
    {
        pos = headFileName.find_last_of('\\');
        if (pos != std::string::npos)
        {
            realFileName = headFileName.substr(pos + 1, -1);
        }
    }

    string protectedMacro;
    for (const auto& x : text.namespaceDef) protectedMacro += x + "_";
    protectedMacro += realFileName;

    for (auto& x : protectedMacro)
    {
        if (isalpha(x))
            x = toupper(x);
        else if (!isdigit(x))
            x = '_';
    }

    ostringstream oss;
    oss << "// auto-generated head file\n";
    oss << "#ifndef " << protectedMacro << "\n"
        << "#define " << protectedMacro << "\n\n";

    for (const auto& x : text.includeString)
    {
        oss << "#include " << x << "\n";
    }

    for (const auto& x : text.namespaceDef)
        oss << "\nnamespace " << x << "\n{\n";

    oss << text.result;

    for (auto it = text.namespaceDef.rbegin(); it != text.namespaceDef.rend(); ++it)
        oss << "\n} // " << "namespace " << *it << "\n";

    oss << "\n#endif // " << protectedMacro << "\n";


    ofstream out(headFileName);
    if (!out.is_open())
    {
        cout << "open failed: " << headFileName << endl;
        return;
    }
    out << oss.str() << endl;
    cout << "create: " << headFileName << endl;
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s fileName\n", argv[0]);
        return -1;
    }

    vector<string> fileNames;
    for (int i = 1; i < argc; ++i)
    {
        fileNames.push_back(argv[i]);
    }
    for (const auto& x : fileNames)
    {
        parseContent(x);
    }
    return 0;
}
