#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <cstring>
#include <regex>

using namespace std;

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
            string secureUrl;
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
    string securelUrl;
    uploadImage("testImage.png", securelUrl);

    return 0;
}
   

// int main()
// {
//     string api_key;
//     string api_secret;
//     string cloud_name;
//     string image_path = R"(/mnt/c/Users/ragha/projects/quail/IMG_20240813_213153.jpg)";

//     find_env_value("CLOUDINARY_API_KEY", api_key);
//     find_env_value("CLOUDINARY_API_SECRET", api_secret);
//     find_env_value("CLOUDINARY_CLOUD_NAME", cloud_name);
    
//     time_t timestamp  = time(0);
//     cout << api_secret << endl;
//     ostringstream string_to_sign;
//     string_to_sign << "timestamp=" << timestamp << api_secret;
//     cout << timestamp << endl;
//     string signature = find_hash(string_to_sign.str());
//     cout << signature << endl;
// 	CURL *curl = curl_easy_init();
//     CURLcode res;
// 	if (curl)
// 	{       
//         string url = "https://api.cloudinary.com/v1_1/" + cloud_name + "/image/upload";

//         struct curl_httppost *form = NULL;
//         struct curl_httppost *lastptr = NULL;

//         curl_formadd(&form, &lastptr,
//                      CURLFORM_COPYNAME, "file",
//                      CURLFORM_FILE, image_path.c_str(),
//                      CURLFORM_END);
        
//         curl_formadd(&form, &lastptr,
//                      CURLFORM_COPYNAME, "api_key",
//                      CURLFORM_COPYCONTENTS, api_key.c_str(),
//                      CURLFORM_END);

//         curl_formadd(&form, &lastptr,
//                      CURLFORM_COPYNAME, "timestamp",
//                      CURLFORM_COPYCONTENTS, to_string(timestamp).c_str(),
//                      CURLFORM_END);

//         curl_formadd(&form, &lastptr,
//                      CURLFORM_COPYNAME, "signature",
//                      CURLFORM_COPYCONTENTS, signature.c_str(),
//                      CURLFORM_END);

//         curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//         curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

//         res = curl_easy_perform(curl);

//         if (res != CURLE_OK) {
//             cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
//         } else {
//             cout << "Image uploaded successfully!" << endl;
//         }

//         curl_easy_cleanup(curl);
//         curl_formfree(form);

// 	}

// 	return 0;

// }
