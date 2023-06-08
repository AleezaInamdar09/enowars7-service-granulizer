#include "users.h"
#include "log.c/log.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#define _XOPEN_SOURCE 500


//Contains complete user file
char user_file_content[MAX_LEN_USER_FILE];

/*
 *
 * Read users-info.txt into user_file_content array
 * Need to be called before reading contents of users-infos from user-file-content array.
 *
 * @return 	0 is success
			1 if file couldn't be opened
			2 error reading file
 */
int load_user_file()
{
	FILE* fp = fopen("users/users-info.txt", "r");
	if (!fp)
	{
		printf("couldn't open file\n");
		return 1; //error
	}
	int len = fread(user_file_content, MAX_LEN_USER_FILE, 1, fp);
	if (len == 0 && ferror(fp))
	{
		printf("error reading file\n");
		return 2;
	}

	fclose(fp);
	
	return 0;
}



bool exist_username_with_password(const char* username_in, const char* password_in)
{
	//check if user folder exists
	char path[128] = "users/";
	
    strcat(path, username_in);
    strcat(path, "/");
    struct stat sb;

    if (!(stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))) { //folder does not exist
        return false;
    }

	if (password_in == NULL) //only check if user exists
	{
		return true;
	}

	//check if content of passwd.txt is like the password_in
	strcat(path, "passwd.txt");
	
	FILE *fp = fopen(path, "r");
	if (!fp)
	{
		log_error("Error opening passwd.txt file, but the user exists.");
		return false;
	}
	char password_read[MAX_PWD_LEN];
	for (int i=0; i < MAX_PWD_LEN; i++)
	{
		password_read[i] = 0;
	}
	int len = fread(password_read, MAX_PWD_LEN, 1, fp);
	if (len == 1 && ferror(fp))
	{
		log_error("Error reading passwd.txt file content");
		return false;
	}
	log_trace("Read password: %s", password_read);
	if (!strcmp(password_read, password_in))
	{ //correct
		return true;
	} else {
		log_warn("Wrong provided password");
		return false;
	}

	/*
	char delimiter[] = ";";
	char delimiter_details[] = ":";
	char *save_ptr_1, *save_ptr_2;

	//USER:PASSWORD:PERSONAL_INFO

	load_user_file(); //always work with up-to-date user/pwd data

	char* split = strdup(user_file_content);
	
	//split
	char* data_row = strtok_r(split, delimiter, &save_ptr_1);

	while (data_row)
	{
		if (data_row[0] == '\n') return false;
		
		//split content
		char* username 	= strtok_r(data_row, delimiter_details, &save_ptr_2);
		assert(username);
		char* pwd 		= strtok_r(NULL, delimiter_details, &save_ptr_2);
		assert(pwd);
		char* details 	= strtok_r(NULL, delimiter_details, &save_ptr_2);
		assert(details);
		
		if (!strncmp(username, username_in, MAX_USER_NAME_LEN))
		{
			if (password_in)
			{ //only check password if one was specified
				if (!strncmp(pwd, password_in, MAX_PWD_LEN))
				{
					free(split);
					return true;
				}
			} else {
				free(split);
				return true;
			}
		}

		//get next data_row
		data_row = strtok_r(NULL, delimiter, &save_ptr_1);
	}

	free(split);
	*/
	return false;
}



bool exist_username(const char* username_in)
{
	return exist_username_with_password(username_in, NULL);
}


char* get_users_details()
{
	printf("TODO\n");
	return 0;
}

void add_user_base_folder()
{
	//adds users/ folder
	struct stat st = {0};

	if (stat("users", &st) == -1) {
		mkdir("users", 0700);
	}

}

static int unlink_cb(const char *fpath, 
	__attribute__ ((unused)) const struct stat *sb, 
	__attribute__ ((unused)) int typeflag, 
	__attribute__ ((unused)) struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
	{
        perror(fpath);
	}

    return rv;
}

static int rmrf(char *path)
{
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void add_user_folder_and_password(const char* username, const char* password)
{
    //remove user folder for clean beginning
    char path[128] = "users/";
	
    strcat(path, username);
    strcat(path, "/");
    
	rmrf(path);

    //create new users folder
	struct stat st = {0};
	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
	}

	//create password file and write it
	strcat(path, "passwd.txt");
	FILE *fp = fopen(path, "w");
	if (!fp)
	{
		return;
	}
	fwrite(password, strlen(password), 1, fp);
	fclose(fp);

}

/**
 * Register the given user.
 * Adds users info to users-info.txt, creates user folder
 *
 */
int add_user(const char* username, const char* pwd)
{	
	/*
	//open file and append user infos
	FILE* fp = fopen("users/users-info.txt", "a");
	if (!fp)
	{
		printf("couldnt open file\n");
		return 1; //error
	}
	int len = fwrite(username, strlen(username), 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	len = fwrite(":", 1, 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	len = fwrite(pwd, strlen(pwd), 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	len = fwrite(":", 1, 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	len = fwrite(details, strlen(details), 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	len = fwrite(";", 1, 1, fp);
	if (len < 1)
	{
		printf("error writing file: %i\n", len);
		return 2;
	}
	fclose(fp);
	*/

    add_user_folder_and_password(username, pwd);

    return 0;
}