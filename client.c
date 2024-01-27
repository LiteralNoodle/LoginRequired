// This code was written by Matthew Mahan for Cyberstorm.
// Other team members include Trey Burkhalter, Christian Brunning, and Aaron Earp. 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <openssl/sha.h>

struct MemoryStruct {
  char *memory;
  size_t size;
};

int main (void) {

	// hash password
	char* password = "this is my password. It is very secure!";
	char hashed[1024] = "";
	hash_str(password, hashed);

	printf("%s\n", hashed);

	send_user_login("cats", "myhash");

	

	// hash_string(hashed, unhashed);
	// printf("\n\n%s\n", hashed);

	return 0;
}
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, 
                    void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) 
  {
    printf("not enough memory (realloc returned NULL)\n");
    return CURL_WRITEFUNC_ERROR;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

int send_user_login(char* username, char* hash, char* response) {
	
	response = NULL;

	CURL *curl;
	CURLcode result;
	char url[2048];

	struct MemoryStruct chunk;
	chunk.memory = malloc(1);
	chunk.size = 0;

	curl = curl_easy_init();

	if (curl == NULL) {
		fprintf(stderr, "HTTP request failed/n");
		return -1;
	}

	char* url_encoded_username = curl_easy_escape(curl, username, 0);
	char* url_encoded_hash = curl_easy_escape(curl, hash, 0);

	sprintf(url, "https://opossum.fans/%s", url_encoded_username);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	result = curl_easy_perform(curl);

	if (result != CURLE_OK) {
		fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
		return -1;
	}

	printf("Reply from remote host: %s\n", chunk.memory);
	
	curl_free(url_encoded_username);
	curl_free(url_encoded_hash);
	curl_easy_cleanup(curl);

	free(chunk.memory);

	return 0; 
}

void hash_str(char* inbuf, char* hashbuf) {

	char outbuf[20];

	SHA1(inbuf, strlen(inbuf), outbuf);

	for (int i = 0; i < 20; i++) {
		char tmp[16];
		sprintf(tmp, "%x", outbuf[i]);
		strcat(hashbuf, tmp);
	}
}
