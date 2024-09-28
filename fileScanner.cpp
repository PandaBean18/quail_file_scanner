#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <curl/curl.h>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <ctime>
#include <cstring>

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

int findImgSrc(string line, string &src)
{
    regex pattern(R"(<img\b[^>]*\bsrc\s*=\s*['"]([^'"]+)['"])");

	smatch match;

    if (regex_search(line, match, pattern))
    {
        string s = match[1].str();

        for (string::iterator it = s.begin(); it != s.end(); it++)
        {
            src.push_back(*it);
        }

        return match.position();
    }

    return 0;
}

void replaceImgSrc(string line, string src, string newSrc, string &newLine)
{

    regex replacementPattern(src);

    string newString = regex_replace(line, replacementPattern, newSrc);
    for(string::iterator it = newString.begin(); it != newString.end(); it++)
        newLine.push_back(*it);
	
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

void find_env_value(string key, string &value)
{
    ifstream file = ifstream(".env");

    string line;

    while(getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue;
        size_t pos = line.find('=');

        if (pos == string::npos) continue;
        
        string k = line.substr(0, pos);
        
        if (key == k)
        {
            value = line.substr(pos+1);
            
            return;
        }
    }
}

void find_secure_url(string jsonResponse, string &secureUrl)
{
    regex pattern = regex(R"(secure_url"\s*:\s*"([^"]+))");
    smatch match;

    if (regex_search(jsonResponse, match, pattern))
    {
        string url = match[1].str();

        for(string::iterator it = url.begin(); it != url.end(); it++)
        {
            secureUrl.push_back(*it);
        }
        return;
    }
}

string find_hash(string input)
{
    unsigned char hash[SHA_DIGEST_LENGTH];

    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);

    std::ostringstream hexStream;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        hexStream << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }

    return hexStream.str(); 
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

void uploadImage(const char *path, string &secureUrl)
{
    string api_key;
    string api_secret;
    string cloud_name;

    cout << "Uploading " << path << "..." << endl;

    find_env_value("CLOUDINARY_API_KEY", api_key);
    find_env_value("CLOUDINARY_API_SECRET", api_secret);
    find_env_value("CLOUDINARY_CLOUD_NAME", cloud_name);

    time_t timestamp = time(0);
    ostringstream string_to_sign;
    string_to_sign << "timestamp=" << timestamp << api_secret;

    string signature = find_hash(string_to_sign.str());

    CURL *curl = curl_easy_init();
    CURLcode res;

    if (curl)
    {
        string url = "https://api.cloudinary.com/v1_1/" + cloud_name + "/image/upload";

        struct curl_httppost *form = NULL;
        struct curl_httppost *lastptr = NULL;
        string jsonResponse;

        curl_formadd(&form, &lastptr,
                     CURLFORM_COPYNAME, "file",
                     CURLFORM_FILE, path,
                     CURLFORM_END);
        
        curl_formadd(&form, &lastptr,
                     CURLFORM_COPYNAME, "api_key",
                     CURLFORM_COPYCONTENTS, api_key.c_str(),
                     CURLFORM_END);

        curl_formadd(&form, &lastptr,
                     CURLFORM_COPYNAME, "timestamp",
                     CURLFORM_COPYCONTENTS, to_string(timestamp).c_str(),
                     CURLFORM_END);

        curl_formadd(&form, &lastptr,
                     CURLFORM_COPYNAME, "signature",
                     CURLFORM_COPYCONTENTS, signature.c_str(),
                     CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &jsonResponse);

        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        } else {
            find_secure_url(jsonResponse, secureUrl);

            cout << endl << endl << "Secure url: " << secureUrl << endl;
            cout << "Image uploaded successfully!" << endl;
        }

        curl_easy_cleanup(curl);
        curl_formfree(form);
        cout << "Uploaded!";
    } else {
        cout << "Something went wrong." << endl;
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
                    string src;
                    string secureUrl;
                    
                    int shift = findImgSrc(line, src);

                    uploadImage(src.c_str(), secureUrl);

		    	    replaceImgSrc(line, src, secureUrl, newLine);

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