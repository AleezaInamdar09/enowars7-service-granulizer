#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "users.h"
#include "base64.h"
#include "granular.h"
#include "file_handler.h"

#define ARRSIZE(a) (sizeof(a)/sizeof(a[0]))

#define WAVE_HEADER_OFFSET 0x44

//Flag for debugging, forces the creation of a new clean setup when service is started
#define FORCE_NEW_SETUP false

char* ask(const char* prompt)
{
	printf("%s", prompt);

	static char buf[2048];
	char *tok;

	if (fgets(buf, ARRSIZE(buf), stdin)) {
		tok = strchr(buf, '\n');
		if (tok) *tok = '\0';
	} else {
		*buf = '\0';
	}

	return buf;
}

void init()
{
	srand(time(NULL)); //create random seed
}


/**
 * Perform setup of service if the service does not exist yet.
 *
 * Creates users-info.txt
 * Deletes users/ directory if it exist
 * Creates users/ directory
 *
 */
void setup_service()
{
	FILE* fp = fopen("users-info.txt", "r");
	
	if (fp != NULL && !FORCE_NEW_SETUP)
	{ //file already exist, do not perform setup
		fclose(fp);
		return;
	}
	//file doesn't exist, fclose is not necessary then
	
	//create empty file
	fp = fopen("users-info.txt", "w");
	fclose(fp);

	//delete users/ directory if it exist
	system("rm -rf users/");

	//create users/ directory
	int res = mkdir("users", 0777);

}

void login()
{
	char* username  = ask("Username: ");
	char* password	= ask("Password: ");
	printf("Checking %s with %s\n", username, password);

	load_user_file(); //load current user file
	if (exist_username_with_password(username, password))
	{
		printf("Welcome %s!\n", username);
	} else {
		printf("Wrong password\n");
		exit(0);
	}
	return;
}

void reg()
{
	char* username  = ask("Username: ");
	char* password	= ask("Password: ");
	char* details 	= ask("Please share some details about yourself (will be privately stored in your account): ");

	//check if username does not exist
	load_user_file(); //load current user file
	bool exist = exist_username(username);
	if (exist)
	{
		printf("user already exist!\n");
	} else {
		add_user(username, password, details);
		printf("ok\n");
	}
}


void synth_file_call()
{
	printf("Enter a file name: ");
	char file_name[1024];
	fgets(file_name, 1024, stdin);
	char* tok = strchr(file_name, '\n');
	if (tok) *tok = '\0';

	//check that file name is valid
	char *dot = strrchr(file_name, '.');
	if ( !dot || strcmp(dot, ".wav"))
	{
		printf("file name has to end with .wav\n");
		return;
	}
	//and "file is in correct folder" (fake check)

	//open file
	char* fn = strdup(file_name);
	if (!fn)
	{
		return;
	}

	#define MAXBUFLEN 100000

	char file_content[MAXBUFLEN + 1];
	FILE* fp = fopen(fn, "rb");
	if (!fp)
	{
		printf("error opening file\n");
		return;
	}
	int len = fread(file_content, sizeof(char), MAXBUFLEN, fp);
	if (len <= 0)
	{
		printf("error reading file\n");
		return;
	}
	printf("%s", file_content);
	fclose(fp);

	granular_info* info = granulize(file_content, len);
	print_granular_info(info);
}

void upload_wav_file_call()
{
	char* file_name = ask("Enter file name for new .wav file: ");
	if (!file_ends_with(file_name, ".wav"))
	{
		printf("File has to end with .wav\n");
		return;
	}
	if (path_contains_illegal_chars(file_name))
	{
		printf("File name contains illegal characters\n");
		return;
	}

	printf("Enter base64 encoded wave file, maximum size 1024 bytes:\n");
	char base64encoded[1024];
	fgets(base64encoded, 1024, stdin);

	char input[1024];
	//decode and write to file:
	int len = Base64decode(input, base64encoded);
	printf("%s\n", input);

	if (len <= 0)
	{
		printf("Error parsing the b64\n");
		return 1;
	}

	FILE* fp = fopen("output.wav", "wb");
	if (!fp)
	{
		perror("fopen");
		return 1;
	}

	fwrite(input, 1, len, fp);
	fclose(fp);
}




int main()
{
	setup_service();
	
	
	char* in = ask("do you want to login (l) or register (r)?\n >");
	if (!strcmp(in, "register") || !strcmp(in, "r"))
	{
		reg();
	} else if (!strcmp(in, "login") || !strcmp(in, "l"))
	{
		login();
	} else {
		printf("Please enter login or register\n");
		exit(0);
	}

	init();

	struct {
		const char *name;
		void (*func)();
	} cmds[] = {
		{ "login\n", login },
		{ "register\n", reg },
		{ "upload wav\n", upload_wav_file_call },
		{ "synth\n", synth_file_call }
		/*{ "users", api_list_users },
		{ "info", api_user_info },
		{ "login", api_login },
		{ "post", api_create_post },
		{ "posts", api_list_posts },
		{ "help", api_help },
		*/
	};

	printf("What do you wanna do?\n > ");
	
	char cmd[32];
	
	fgets(cmd, 32, stdin);


	int i;
	for (i = 0; i < ARRSIZE(cmds); i++) {
		if (!strcmp(cmd, cmds[i].name)) {
			cmds[i].func();
			break;
		}
	}

	if (i == ARRSIZE(cmds))
		printf("Unknown command: %s\n", cmd);

}
