#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

using namespace std;
using namespace std::filesystem;

int includes(string str, char *c)
{
    for(string::iterator it = str.begin(); it != str.end(); it++)
    {
	    if (*it == *c) 
	    {
                return 1;
	    }
    }
    return 0;
}

int isImgTag(string line)
{
    regex pattern(R"(<img\b[^>]*>)");
    return regex_search(line, pattern);
}

void findImgId(string line, string *emptyId)
{
    regex pattern(R"(<img\b[^>]*\bid\s*=\s*['"]([^'"]+)['"])");
    smatch match;
    if (regex_search(line, match, pattern))
    {
        string id = match[1].str();
        for(int i = 0; i < id.size(); i++)
        {
            emptyId->push_back(id[i]);
        }
    }

}

void replaceImgSrc(string line, string newSrc, string &newLine)
{
	regex pattern(R"(<img\b[^>]*\bsrc\s*=\s*['"]([^'"]+)['"])");

	smatch match;

	if(regex_search(line, match, pattern)) {
		regex replacementPattern(match[1].str());

		string newString = regex_replace(line, replacementPattern, newSrc);
		for(string::iterator it = newString.begin(); it != newString.end(); it++)
			newLine.push_back(*it);
	}
	return;	
}

int main()
{
    directory_iterator d = directory_iterator(current_path());
    directory_iterator e = end(d);
    cout << "Looking for image tags with the id `test` in the directory: " << current_path() << endl << endl;
    for(d; d != e; d++)
    {
	    string relativePath = relative(d->path()).string();
	    if ((includes(relativePath, ".") == 0) || relativePath[0] == *".")
	    {
	        continue;
	    }
	    cout<< "Checking file " << relativePath << endl;
	    string line;
	    string pathname = d->path().string();
	    ifstream file = ifstream(pathname);

	    while (getline(file, line))
	    {
            int currentPos = file.tellg();
            if (isImgTag(line))
	        {
                string imageId;
                findImgId(line, &imageId);
                if (imageId == "test") 
                {
                    file.close();
                    ofstream writeFile(pathname, ios_base::app);
                    writeFile.seekp(currentPos);
                    string newLine;
                    string replacement = "test2.jpg";
                    replaceImgSrc(line, replacement, newLine);
                    writeFile.write(newLine.c_str(), newLine.size());
                    writeFile.write("\n", 1);
                    int newPos = writeFile.tellp();
                    writeFile.close();
                    file = ifstream(pathname);
                    file.seekg(newPos);
                    cout << "Updated file " << pathname << endl;
                }
            }
        }
	    cout << endl;
    }
    return 0;
}