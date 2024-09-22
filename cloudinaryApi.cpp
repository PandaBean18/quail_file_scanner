#include <iostream>
#include <curl/curl.h>

using namespace std;

int main()
{
	//curl_global_init(CURL_GLOBAL_DEFAULT);
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

	//curl_global_cleanup();
	return 0;
}
