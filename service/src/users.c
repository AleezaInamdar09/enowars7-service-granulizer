#include "users.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//Contains complete user file
char user_file_content[MAX_LEN_USER_FILE];

/*
 *
 * Read users-info.txt into user_file_content array
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
	char delimiter[] = ";";
	char delimiter_details[] = ":";
	char *save_ptr_1, *save_ptr_2;

	//USER:PASSWORD:PERSONAL_INFO
	
	//if (!user_file_content) return false; //if no users exist return false

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
	char command[64] = "mkdir users\0";
    system(command);
}

void add_user_folder(const char* username)
{
    //remove user folder for clean beginning
    char command[64] = "rm -rf users/";
    strcat(command, username);
    strcat(command, "/");
    system(command);
    
    //create users folder
    char command2[64] = "mkdir users/";
    strcat(command2, username);
    strcat(command2, "/");
    system(command2);
}

/**
 * Register the given user.
 * Adds users info to users-info.txt, creates user folder
 *
 */
int add_user(const char* username, const char* pwd, const char* details)
{	
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
	
    add_user_folder(username);

    return 0;
}