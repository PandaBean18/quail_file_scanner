#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <iomanip>
#include <ctime>
#include <fstream>

using namespace std;

void find_env_value(string &key, string &value)
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

string create_signature(string &string_to_sign, string &api_secret)
{
    return string_to_sign;
}

int main()
{
    string key = "CLOUDINARY_API_KEY";
    string value;

    find_env_value(key, value);

    cout << value << endl;

    return 0;

	CURL *curl = curl_easy_init();

	if (curl)
	{       
        cout << "Inside curl" << endl;
	    CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.6:3000/");
		res = curl_easy_perform(curl);
        cout << curl_easy_strerror(res) << endl;
        cout << CURLOPT_URL;
		curl_easy_cleanup(curl);
	}

	return 0;
}
