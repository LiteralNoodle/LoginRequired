// This code was written by Matthew Mahan for Cyberstorm.
// Other team members include Trey Burkhalter, Christian Brunning, and Aaron Earp. 

#include <stdio.h>
#include <curl/curl.h>

int send_user_login();

int main (void) {
	
	send_user_login();

	return 0; 
}

int send_user_login() {
	
	CURL *curl;
	CURLcode result; 

	curl = curl_easy_init();

	if (curl == NULL) {
		fprintf(stderr, "HTTP request failed/n");
		return -1;
	}

	curl_easy_setopt(curl, CURLOPT_URL, "https://opossum.fans");

	result = curl_easy_perform(curl);

	if (result != CURLE_OK) {
		fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
		return -1;
	}

	curl_easy_cleanup(curl);
	
	return 0; 
}
