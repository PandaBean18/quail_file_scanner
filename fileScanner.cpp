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

int replaceImgSrc(string line, string newSrc, string &newLine)
{
	regex pattern(R"(<img\b[^>]*\bsrc\s*=\s*['"]([^'"]+)['"])");

	smatch match;

	if(regex_search(line, match, pattern)) {
		regex replacementPattern(match[1].str());

		string newString = regex_replace(line, replacementPattern, newSrc);
		for(string::iterator it = newString.begin(); it != newString.end(); it++)
			newLine.push_back(*it);
	}
	return match.position();	
}

// create a fucntion that takes an open file, the current line data, and the new line data as inputs 
// it should find the diff betweeen new line data and current line data and then add that many spaces at the end of the new file

void pushContent(const char *filename, int startPos, int shiftWidth, char *b)
{
    // start from the end of the file-shiftWidth
    // move the cursor back by shiftWidth 
    // copy the elements from current position to current+shiftWidth
    // write them at current+shiftWidth

    // set currentPosition to currentPosition-shiftWidth

    // continue till currentPosition <= startPos 

    ifstream inFile = ifstream(filename, ios_base::in | ios_base::out);
    ofstream outFile = ofstream(filename, ios_base::in | ios_base::out);
    
    inFile.seekg(0, inFile.end);

    int length = inFile.tellg();

    inFile.seekg(0, inFile.beg);

    outFile.seekp(-shiftWidth, outFile.end);
    inFile.seekg(-shiftWidth, inFile.end);

    int currentPos = outFile.tellp();
    int i = 2;

    while(currentPos > startPos)
    {
        int shift = shiftWidth;
        // if ((currentPos - shiftWidth) <= startPos)
        // {
        //     shift = currentPos-shiftWidth;
        // }
        inFile.seekg(-(shift*i), inFile.end);
        inFile.read(b, shift);
        // cursor GET position will be eq to currentPos;

        outFile.write(b, shift); 
        // cursor PUT will be eq ti currentPos+shiftWidth;
        currentPos -= shift;
        outFile.seekp(currentPos);
        inFile.seekg(currentPos);
        i++;
    }

    outFile.seekp(startPos);

    for (int j = 0; j < shiftWidth; j++)
    {
        outFile.write(" ", 1);
    }

    outFile.close();
    inFile.close();
}

void makeSpace(const char *filename, string currentLine, string newLine)
{
    ofstream file = ofstream(filename, ios_base::app);
    int diff = newLine.size() - currentLine.size();

    if (diff <= 0)
    {
        file.close();
        return;
    }

    for (int i = 0; i < diff; i++)
    {
        file.write(" ", 1);
    }

    file.close();
    return ;
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

		int currentPos = file.tellg();
        string prevLine;
	    while (getline(file, line))
	    {
            if (isImgTag(line))
	        {
                string imageId;
                findImgId(line, &imageId);
                if (imageId == "test") 
                {
                    cout << "Found image with id test" << endl;
                    file.close();
                    string newLine;
                    string replacement = "https://res.cloudinary.com/demoImage.jpg";
                    
		    	    int shift  = replaceImgSrc(line, replacement, newLine);

                    int shiftWidth = newLine.size() - line.size();

                    if (shiftWidth > 1)
                    {
                        char buffer[shiftWidth];

                        makeSpace(pathname.c_str(), line, newLine);
                        pushContent(pathname.c_str(), currentPos, shiftWidth, buffer);
                    }
                    ofstream writeFile = ofstream(pathname, ios_base::in | ios_base::out);
                    writeFile.seekp(currentPos);
                    writeFile.write(newLine.c_str(), newLine.size());
                    writeFile.write("\n", 1);
                    
		    	    int newPos = currentPos + newLine.size();
                                        
		    	    file = ifstream(pathname);
                    file.seekg(newPos);
		    	    currentPos = newPos;
                    
		    	    cout << "Updated file " << pathname << endl;
                }
            }
            prevLine = line;
	    	currentPos = file.tellg();
        }
	    cout << endl;
    }
    return 0;
}