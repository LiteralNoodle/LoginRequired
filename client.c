// This code was written by Matthew Mahan for Cyberstorm.
// Other team members include Trey Burkhalter, Christian Brunning, and Aaron Earp. 

// Source for libcurl code can be found at: https://www.youtube.com/playlist?list=PLA1FTfKBAEX6p-lfk1l_Q2zh2E5wd-cup
// Source for sha256 code can be found at: https://stackoverflow.com/questions/2262386/generate-sha256-with-openssl-and-c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <stdbool.h>
#include <regex.h>

#define ANSI_FOREGROUND_GREEN "\e[0;32m"
#define ANSI_FOREGROUND_RED "\e[0;31m"
#define ANSI_FOREGROUND_WHITE "\e[0;37m"

#define SKIP_CONNECTION_TEST false
#define DEBUG_NETWORK false

#define PASSWORD_MAX_LENGTH 512

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

typedef struct History {
	struct History* previous;
	char* message;
} tHistory; 
 
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
    return CURLE_WRITE_ERROR;
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

	sprintf(url, "http://%s:%s/createaccount?username=%s&hash=%s", server_ip, server_port, url_encoded_username, url_encoded_hash);

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

	sprintf(url, "http://%s:%s/connectiontest", server_ip, server_port);

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

// hash function sha256 which will also be used on the server side
void sha256_string(char *string, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

// push onto history list
void push_history(tHistory** head, char* message) {

	// don't add to history if it's the same as the last
	if (*head != NULL) {
		tHistory* p = *head;
		if (!strcmp(p->message, message)) {
			return;
		}
	}

	char* message_copy = (char*)malloc(sizeof(char) * PASSWORD_MAX_LENGTH);
	strcpy(message_copy, message);

	tHistory* h;
	h = (tHistory*)malloc(sizeof(tHistory));

	h->message = message_copy;
	h->previous = *head;
	*head = h; 

	// limit history to 10
	// since this is built-up one at a time, just have to delete the last one
	tHistory* cursor = *head;
	int limit = 10;
	for (int i = 0; i < limit; i++) {
		
		if (i == limit-1) {
			if (cursor->previous != NULL) {
				free(cursor->previous);
				cursor->previous = NULL;
			}
			return;
		}
				
		if (cursor->previous == NULL) {
			break;
		}

		cursor = cursor->previous;
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

// check the validity of the proposed password
void check_question_list(tQuestion* q, int stop, char* password, bool* correctness) {

	for (int i = 0; i < stop; i++) {

		// set the truth according to if the question is fulfilled 
		if (q->callback(password)) {
			correctness[i] = true;
		} else {
			correctness[i] = false;
		}

		if (q->nextQuestion == NULL) {
			break;
		}

		q = q->nextQuestion;
	}

}

bool all_correct(bool* answers, int length) {

	bool allCorrect = true;

	for (int i = 0; i < length; i++) {
		if (!answers[i]) {
			allCorrect = false;
		}
	}

	return allCorrect;

} 

// This function prints the list but also is responsible for checking correctness 
void print_question_list(tQuestion* q, int stop, bool* correctness) {

	// early break for when there's 0 questions shown
	if (!stop) {
		return;
	}

	for (int i = 0; i < stop; i++) {

		char colorstr[1024] = "";

		// set the color according to if the question is fulfilled 
		if (correctness[i]) {
			color_string(q->message, ANSI_FOREGROUND_GREEN, colorstr);
		} else {
			color_string(q->message, ANSI_FOREGROUND_RED, colorstr);
		}

		printf("%s\n", colorstr);

		if (q->nextQuestion == NULL) {
			break;
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

// prints out the error from regex compilation 
void print_regex_error(int compilation_code, regex_t* reg) {

	printf("Regex compilation error code: %d\n", compilation_code);
	size_t length = regerror(compilation_code, reg, NULL, 0);
	char* err_buffer = malloc(length);
	regerror(compilation_code, reg, err_buffer, length);
	printf("%s\n", err_buffer);
	free(err_buffer);

}

// question functions 

bool example_function(char* message) {
	printf("%s\n", message);
	return true;
}

// "Your password must contain a number."
bool question1(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, ".*[0-9].*", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "Your password must contain a capital letter."
bool question2(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, ".*[A-Z].*", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "Your password must contain a special character from this list: !@#$^&*()+=~"
bool question3(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, ".*[!@#$^&*()+=~].*", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "The length of your password must be a prime number."
bool question4(char* password) {

	// length of array is 54
	// only need up to 256 since that's half of the allowed length and 2*256 would fill up the whole thing 
	int primes[] = {   2,   3,   5,   7,   11,   13,   17,   19,   23,   29,   31,   37,   41,   43,   47,   53,   59,   61,   67,   71,   73,   79,   83,   89,   97,   101,   103,   107,   109,   113,   127,   131,   137,   139,   149,   151,   157,   163,   167,   173,   179,   181,   191,   193,   197,   199,   211,   223,   227,   229,   233,   239,   241,   251 };
	int length = strlen(password);
	
	// I don't really expect them to make a password past this, but just in case:
	if (length > 256) {

		for (int i = 0; i < 54; i++) {
			if ((length % primes[i]) == 0) {	// if divisible by anything in the array, then it's composite.
				return false;
			}
		}

	} else {
		for (int i = 0; i < 54; i++) {
			if (length == primes[i]) {
				return true;
			}
		}
	}

	// fail
	return false;
}

// "Your password must contain one of our sponsors: Pepsi Walmart Lowes LEGO Autozone Build-A-Bear"
bool question5(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*Pepsi|Walmart|Lowes|LEGO|Autozone|Build-A-Bear.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "Your password must contain one word of university spirit: Anky Timo Bulldogs LATech Cyberstorm"
bool question6(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*Anky|Timo|Bulldogs|LATech|Cyberstorm.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "Who lives in a pineapple under the sea?"
bool question7(char* password) {
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*spongebob|Spongebob|spongebobsquarepants|Spongebobsquarepants|SpongebobSquarepants.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}


// "Onions have layers, who else has layers?"
bool question8(char* password) {
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*shrek|Shrek|ogres|Ogres.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}


// "Your password must contain a cartoon character whose slogan is 'Good Mornin'"
bool question9(char* password) {
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*unclegrandpa|UncleGrandpa|Unclegrandpa.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "What's 9+10?"
bool question10(char* password) {
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*21|twentyone.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}

// "Your password must contain the name of the main robot from Futurama"
bool question11(char* password) {
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*bender|Bender.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}




// "Your password must contain a generation 4 starter Pokemon." (piplup, turtwig, chimchar)
bool question12(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*piplup|turtwig|chimchar|Piplup|Turtwig|Chimchar.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the base power of extremespeed in Pokemon." (80)
bool question13(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*80.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain a type that is supereffective against dragon type." (dragon, fairy)
bool question14(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*dragon|fairy|Dragon|Fairy.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain a type that is supereffective against flying type." (ice, electric, rock)
bool question15(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*ice|electric|rock|Ice|Electric|Rock.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the name of the Sinnoh League Champion." (Cynthia, cynthia)
bool question16(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*Cynthia|cynthia.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the name of the Sinnoh League Champion's Ace." (garchomp, Garchomp)
bool question17(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*garchomp|Garchomp.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain your trainer level in Pokemon Go." (1-50)
bool question18(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*[0-50].*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the name of a grass-fire dual type Pokemon." (scovillain, ogerpon)
bool question19(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*scovillain|Scovillain|ogerpon|Ogerpon.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the name of the Greek god of the sea." (Poseidon)
bool question20(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*poseidon|Poseidon.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}
// "Your password must contain the name of the Roman god of the sea." (Neptune)
bool question21(char* password) {
	regex_t reg;
	int compilation_code;

	compilation_code = regcomp(&reg, "(.*neptune|Neptune.*)", REG_EXTENDED);

	// error handling for regex compilation
	if (compilation_code) {
		print_regex_error(compilation_code, &reg);
		return true; // return true just to be kind if the mistake is not on the player's part
	}

	int match_code;
	match_code = regexec(&reg, password, 0, NULL, 0);

	// match was not found. password fails this rule.
	if (match_code == REG_NOMATCH){
		return false;
	}

	// regex ran out of memory. give question for free since it's not their mistake.
	if (match_code == REG_ESPACE) {
		return true;
	}

	// success
	return true;
}



// Entry point 
int main (void) {

	char server_ip[64];
	char server_port[8];

	printf("\n\nWelcome to the Secure Account Generator!\n\n");

	if (!SKIP_CONNECTION_TEST) {
		opening_config(server_ip, server_port);
		if (test_connection(server_ip, server_port)) {
			printf("\nConnection failed! Aborting...\n");
			exit(-1);
		}
	}

	// winning variables
	char username[64] = "";
	char password[PASSWORD_MAX_LENGTH] = "";

	// begin history 
	tHistory* history_head = NULL;

	// ask for username 
	get_input_with_message("Please enter the username you wish to use. (Max 64 characters)", username);

	if (DEBUG_NETWORK) {
		get_input_with_message("Please enter a new password.", password);

		// win state
		char hashed[64];
		sha256_string(password, hashed);
		printf("Hash digest: %s\n", hashed);
		send_user_account(username, hashed, server_ip, server_port);
	}

	// question struct instances
	// MUST be defined in reverse order so that list can be made
	// struct Question test = { "Hey this is a question!", example_function };
	tQuestion q21 = { "Your password must contain the name of the Roman god of the sea.", question21, NULL };
	tQuestion q20 = { "Your password must contain the name of the Greek god of the sea.", question20, &q21 };
	tQuestion q19 = { "Your password must contain the name of a grass-fire dual type Pokemon.", question19, &q20 };
	tQuestion q18 = { "Your password must contain your trainer level in Pokemon Go.", question18, &q19 };
	tQuestion q17 = { "Your password must contain the name of the Sinnoh League Champion's Ace.", question17, &q18 };
	tQuestion q16 = { "Your password must contain the name of the Sinnoh League Champion.", question16, &q17 };
	tQuestion q15 = { "Your password must contain a type that is supereffective against flying type.", question15, &q16 };
	tQuestion q14 = { "Your password must contain a type that is supereffective against dragon type.", question14, &q15 };
	tQuestion q13 = { "Your password must contain the base power of extremespeed in Pokemon.", question13, &q14 };
	tQuestion q12 = { "Your password must contain a generation 4 starter Pokemon.", question12, &q13 };
	tQuestion q11 = { "Your password must contain the name of the main robot from Futurama", question11, &q12 };
	tQuestion q10 = { "What's 9+10?", question10, &q11 };
	tQuestion q9 = { "Your password must contain a cartoon character whose slogan is 'Good Mornin'", question9, &q10 };
	tQuestion q8 = { "Onions have layers, who else has layers?", question8, &q9 };
	tQuestion q7 = { "Who lives in a pineapple under the sea?", question7, &q8 };
	tQuestion q6 = { "Your password must contain one word of university spirit: Anky Timo Bulldogs LATech Cyberstorm", question6, &q7 };
	tQuestion q5 = { "Your password must contain one of our sponsors: Pepsi Walmart Lowes LEGO Autozone Build-A-Bear", question5, &q6 };
	tQuestion q4 = { "The length of your password must be a prime number.", question4, &q5 }; 
	tQuestion q3 = { "Your password must contain a special character from this list: !@#$^&*()+=~", question3, &q4 };
	tQuestion q2 = { "Your password must contain a capital letter.", question2, &q3 };
	tQuestion q1 = { "Your password must contain a number.", question1, &q2 };
	
	// main game loop
	tQuestion* first = &q1;
	int questions_found = 0; 
	int total_questions = get_total_questions(first);
	bool allCorrect = false;
	bool correctness[total_questions];
	// init the correctness
	for (int i = 0; i < total_questions; i++) {
		correctness[i] = false;
	}
	while (true) {

		// print all the questions
		print_question_list(first, questions_found, correctness);

		// prompt for user input 
		get_input_with_message("Please enter a new password.", password);

		// add to history
		push_history(&history_head, password);

		// check attempt 
		check_question_list(first, questions_found, password, correctness);
		allCorrect = all_correct(correctness, questions_found);

		// check if no more questions (Win state, sbreak loop)
		if (allCorrect && questions_found == total_questions) {
			break;
		}

		// increment if correct
		if (allCorrect) {
			questions_found++;
		}

	}

	// win state
	char hashed[64];
	sha256_string(password, hashed);
	send_user_account(username, hashed, server_ip, server_port);

	return 0;
}
