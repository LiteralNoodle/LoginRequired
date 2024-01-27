// This code was written by Matthew Mahan for Cyberstorm.
// Other team members include Trey Burkhalter, Christian Brunning, and Aaron Earp. 

// Source for libcurl code can be found at: https://www.youtube.com/playlist?list=PLA1FTfKBAEX6p-lfk1l_Q2zh2E5wd-cup

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <stdbool.h>

#define ANSI_FOREGROUND_GREEN "\e[0;32m"
#define ANSI_FOREGROUND_RED "\e[0;31m"
#define ANSI_FOREGROUND_WHITE "\e[0;37m"

// function pointer for consistent question format
typedef bool (*questionCallback)(char*);

struct MemoryStruct {
  char *memory;
  size_t size;
};

typedef struct Question {
	char* message;
	questionCallback callback;
	struct Question* nextQuestion;
} tQuestion;
 
// callback for libcurl memory chunks 
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

int send_user_account(char* username, char* hash, char* server_ip, char* server_port) {

	printf("Sending user account creation request...\n");

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

	sprintf(url, "http://%s:%s?username=%s&hash=%s", server_ip, server_port, url_encoded_username, url_encoded_hash);

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

int test_connection(char* server_ip, char* server_port) {

	printf("Testing connection...\n");

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

	sprintf(url, "http://%s:%s", server_ip, server_port);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

	result = curl_easy_perform(curl);

	if (result != CURLE_OK) {
		fprintf(stderr, "Error: %s\n", curl_easy_strerror(result));
		return -1;
	}

	printf("Reply from remote host: %s\n", chunk.memory);
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

// only accepts a single word
// whitespace will not be included in response 
void get_input_with_message(char* message, char* userinput) {

	if (message) {
		printf("%s\n>>> ", message);
	}
	scanf("%s", userinput); 
	printf("\n");

	return;

}

void color_string(char* message, char* color, char* colorstr) {
	strcat(colorstr, color);
	strcat(colorstr, message);
	strcat(colorstr, ANSI_FOREGROUND_WHITE);
}

// This function prints the list but also is responsible for checking correctness 
bool print_question_list(tQuestion* q, int stop, char* password) {

	bool allCorrect = true;
	// printf("%d\n", stop);

	for (int i = 0; i < stop; i++) {

		char colorstr[1024] = "";

		// set the color according to if the question is fulfilled 
		if (q->callback(password)) {
			color_string(q->message, ANSI_FOREGROUND_GREEN, colorstr);
		} else {
			color_string(q->message, ANSI_FOREGROUND_RED, colorstr);
			allCorrect = false;
		}

		printf("%s\n", colorstr);

		if (!allCorrect) {
			return allCorrect;
		}

		if (q->nextQuestion == NULL) {
			return allCorrect;
		}

		q = q->nextQuestion;
	}

}

// manages server configuration on program start
// asks the user where the server is 
void opening_config(char* server_ip, char* server_port) {

	get_input_with_message("What is the ip of the server?", server_ip);
	get_input_with_message("What is the port of the server?", server_port);

}

int get_total_questions(tQuestion* q) {

	int total = 0;

	while (q != NULL) {
		total++;
		q = q->nextQuestion;
	}	

	return total;

}

// question functions 

bool example_function(char* message) {
	printf("%s\n", message);
	return true;
}

bool question1(char* message) {
	return true;
}

bool question2(char* message) {
	return true;
}

bool question3(char* message) {
	return true;
}

bool question4(char* message) {
	return true;
}

int main (void) {

	char server_ip[64];
	char server_port[8];

	printf("\n\nWelcome to the Secure Account Generator!\n\n");

	// opening_config(server_ip, server_port);
	// if (test_connection(server_ip, server_port)) {
	// 	printf("\nConnection failed! Aborting...\n");
	// 	exit(-1);
	// }

	// winning variables
	char username[64] = "";
	char password[2048] = "";

	// question struct instances
	// MUST be defined in reverse order so that list can be made
	// struct Question test = { "Hey this is a question!", example_function };
	tQuestion q4 = { "The length of your password must be a highly composite number.", question4, NULL }; 
	tQuestion q3 = { "Your password must contain a special character from this list: !@#$^&*()+=~", question3, &q4 };
	tQuestion q2 = { "Your password must contain a capital letter.", question2, &q3 };
	tQuestion q1 = { "Your password must contain a number.", question1, &q2 };
	
	// main game loop
	tQuestion* first = &q1;
	int questions_found = 0;
	int total_questions = get_total_questions(first);
	bool allCorrect = false;
	bool prevAllCorrect = false;
	while (true) {

		// check previous attempt and print all the questions
		allCorrect = print_question_list(first, questions_found, password);

		// prompt for user input 
		get_input_with_message("Please enter a new password.", password);

		prevAllCorrect = allCorrect;

		// check if no more questions (Win state, sbreak loop)
		if (allCorrect && questions_found == total_questions) {
			break;
		}

		// increment found questions 
		questions_found++;

	}

	// win state
	char hashed[64];
	hash_str(password, hashed);
	send_user_account(username, hashed, server_ip, server_port);

	return 0;
}