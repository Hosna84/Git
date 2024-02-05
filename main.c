#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <windows.h>
#include <errno.h>
#include <libgen.h>
#include <time.h>

void replaceDotWithNull(char *str) // removing characters after . and also .
{
    while (*str != '\0')
    {
        if (*str == '.')
        {
            *str = '\0';
            return; // Stop after the first occurrence is replaced
        }
        str++;
    }
}

void saparator(char user[200], char saved[200]) // saparate parts that are in ""
{
    int start = 0;
    int end = 0;
    int len1 = strlen(user);
    for (int i = 0; i < len1; i++)
    {
        if (user[i] == '\"')
        {
            start = i;
            break;
        }
    }
    end = len1 - 1;
    int i = 0;
    for (int j = start + 1; j < end; j++)
    {
        saved[i] = user[j];
        i++;
    }
    saved[i] = '\0';
}

bool directoryExists(char *path) // if we have a dir or not
{
    struct stat info;
    if (stat(path, &info) != 0)
    {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

int match(char *pattern, char *text) // for wild card if the name match or not
{
    // for null characters.
    if (*pattern == '\0' && *text == '\0')
    {
        return 1;
    }
    // for *a;
    if (*pattern == '*' && *(pattern + 1) != '\0' && *text == '\0')
    {
        return 0;
    }
    if (*pattern == *text)
    {
        return match(pattern + 1, text + 1);
    }
    if (*pattern == '*')
    {
        return match(pattern + 1, text) || match(pattern, text + 1);
    }
    return 0;
}

int token_del(char *name_of_files[1000], char user[100]) // tokenizing parts between < Hey! >
{
    int count = -1;
    int input = 0;
    char del[100] = "<>";
    char *ptr = strtok(user, del);
    while (ptr != NULL)
    {
        count++;
        if (count != 0)
        {
            name_of_files[input] = ptr;
            input++;
        }
        ptr = strtok(NULL, del);
    }
    return input;
}

void addBackslashes(char *str) // for example if i have hi/bye it replaces it with hi//bye
{
    int length = strlen(str);

    for (int i = 0; i < length; i++)
    {
        if (str[i] == '\\')
        {
            memmove(&str[i + 2], &str[i + 1], (length - i));
            str[i + 1] = '\\';
            length++;
            i++; // Skip the next character to avoid an infinite loop
        }
    }
}

void reset_for_directory(const char *filename, char *name) // a part of reset command that is for dir
{
    struct stat path_stat_d;
    DIR *dir;
    char line_d[1000];
    struct dirent *entry;
    dir = opendir(name);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error: cannot open the staging file.\n");
        return 1;
    }
    FILE *temp_file = fopen("temp_for_dir.txt", "a");
    if (temp_file == NULL)
    {
        printf("Error: cannot create a temporary file.\n");
        fclose(file);
        return 1;
    }
    FILE *unstaged = fopen("unstage.txt", "a");
    if (unstaged == NULL)
    {
        printf("Error: cannot create unstaged file.\n");
        fclose(file);
        fclose(temp_file);
        return 1;
    }
    fclose(file);
    fclose(temp_file);
    fclose(unstaged);
    file = fopen(filename, "r");
    temp_file = fopen("temp_for_dir.txt", "a");
    unstaged = fopen("unstage.txt", "a");
    // int match = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
        {
            fprintf(temp_file, "%s\n", entry->d_name);
        }
    }
    fprintf(temp_file, "%s\n", basename(name));
    char line1[1000];
    char line2[1000];
    fclose(temp_file);
    fclose(file);
    fclose(unstaged);
    closedir(dir);
    FILE *file1, *file2, *file3;
    file1 = fopen("temp_for_dir.txt", "r");
    file2 = fopen("staging.txt", "r");
    unstaged = fopen("unstage.txt", "a");
    while (fgets(line1, sizeof(line1), file1) != NULL)
    {
        line1[strcspn(line1, "\n")] = '\0';

        rewind(file2);
        while (fgets(line2, sizeof(line2), file2) != NULL)
        {
            line2[strcspn(line2, "\n")] = '\0';
            if (strcmp(line1, line2) == 0)
            {
                fprintf(unstaged, "%s\n", line2);
            }
        }
    }

    fclose(file1);
    fclose(file2);
    fclose(unstaged);
    // remove(file2);
    remove("temp_for_dir.txt");
    fclose(file2);
    fclose(temp_file);
    FILE *fp1 = fopen("staging.txt", "r");
    FILE *fp2 = fopen("unstage.txt", "r");
    FILE *temp = fopen("t.txt", "a");

    char line11[100];
    char line12[100];
    int match;

    while (fgets(line11, sizeof(line11), fp1))
    {
        line11[strcspn(line11, "\n")] = '\0';
        match = 0;
        rewind(fp2);

        while (fgets(line12, sizeof(line12), fp2))
        {
            line12[strcspn(line12, "\n")] = '\0';
            if (strcmp(line11, line12) == 0)
            {
                match = 1;
                break;
            }
        }

        if (!match)
        {
            fputs(line11, temp);
        }
    }

    fclose(fp1);
    fclose(fp2);
    fclose(temp);
    remove("staging.txt");
    rename("t.txt", "staging.txt");
    fclose(temp);
    fclose(fp2);
}

void extractText(char *str, char userName[]) // actually it is a compeleter version of search func and if our "" is not at the end also it works
{
    int start = -1;
    int end = -1;
    int len = strlen(str);

    for (int i = 0; i < len; i++)
    {
        if (str[i] == '\"')
        {
            if (start == -1)
            {
                start = i + 1;
            }
            else
            {
                end = i;
                break;
            }
        }
    }
    userName[end - start + 1];
    strncpy(userName, str + start, end - start);
    userName[end - start] = '\0';
}

int main()
{
    char input[200]; // command saver
    char first_name_cond[100][100];
    char shortcut[256];
    char shortcut_message[256];
    char input_saver_config_name[200];  // for username
    char input_saver_config_email[200]; // for email
    char file_address[256][256];
    char copy_one_cu[260];
    int exists_config_name = 0;
    int exists_config_email = 0;
    int couunt = 0;
    int ii = 0;
    int commit_id = 1;
    struct stat directoryStat;
    char currentDirectory[FILENAME_MAX];
    char copy_currentDirectory[FILENAME_MAX];
    _fullpath(currentDirectory, ".", FILENAME_MAX); // getting addres of current code path
    addBackslashes(currentDirectory);
    strcpy(copy_currentDirectory, currentDirectory);
    strcat(copy_currentDirectory, "\\global_config.txt");
    char *directoryPath = copy_currentDirectory;
    strcpy(copy_one_cu, currentDirectory);
    addBackslashes(copy_one_cu);
    strcat(copy_one_cu, "\\.shoombiz");
    FILE *global_config_file;
    FILE *config_file;
    FILE *staging = fopen("staging.txt", "a");
    fclose(staging);
    FILE *temp = fopen("temp.txt", "a");
    fclose(temp);
    FILE *commit = fopen("commit.txt", "a");
    fclose(commit);
    DIR *dir_1;
    struct dirent *entry_1;

    // Open the directory
    dir_1 = opendir(copy_one_cu);

    // Read each entry in the directory
    while ((entry_1 = readdir(dir_1)) != NULL)
    {
        if (strcmp(entry_1->d_name, ".") != 0 && strcmp(entry_1->d_name, "..") != 0) // ignoring . & ..
        {
            strcpy(first_name_cond[couunt], entry_1->d_name);
            couunt++;
        }
    }
    closedir(dir_1);

    struct stat filestat;
    time_t lastModifiedTime[256]; // for status is needed(time modification)

    for (int i = 0; i < couunt; i++) // making address of each file for having time modification
    {
        snprintf(file_address[i], sizeof(file_address[i]), "%s\\%s", copy_one_cu, first_name_cond[i]); // making address.
        stat(file_address[i], &filestat);
        lastModifiedTime[i] = filestat.st_mtime; // putting their time modification in an array
    }

    while (1) // while for giving input from user!
    {
        gets(input);

        if (strncmp(input, "shoombiz init", 13) == 0)
        {
            DIR *dir = opendir("."); // opening hide dir for .shoombiz
            int flagg = 0;
            char tmp_cwd[2000];
            char cwd[2000];
            bool exists = false;
            if (dir)
            {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL)
                {
                    if (strcmp(entry->d_name, ".shoombiz") == 0)
                    {
                        printf("The directory 'shoombiz' exists in the root folder.\n"); // can not make a .shoombiz twice
                        flagg = 1;
                        exists = true;
                        break;
                    }
                }
                if (flagg == 0)
                {
                    system("mkdir .shoombiz");
                    printf(".shoombiz made! Be happy!:)");
                    continue;
                }
                closedir(dir);
                if (getcwd(cwd, sizeof(cwd)) == NULL) // if this making .shoombiz was OK or not!
                    continue;
                if (getcwd(tmp_cwd, sizeof(tmp_cwd)) == NULL) // having mohkam kari:)
                    continue;
            }
            if (strcmp(tmp_cwd, "/") != 0) // if your code is around of your .shoomabiz:)
            {
                if (chdir("..") != 0)
                    continue;
            }
            while (strcmp(tmp_cwd, "/") != 0)
            {
                if (chdir(cwd) != 0)
                {
                    continue;
                }
            }
        }

        else if (strncmp(input, "shoombiz config global -username", 31) == 0)
        {
            if (exists_config_name == 1)
            {
                printf("You can't change your name.");
            }
            else
            {
                if (access(directoryPath, F_OK) != -1) // if golbal config file exists
                {
                    printf("You've already set your email and name.");
                }
                else
                {
                    saparator(input, input_saver_config_name); // just save username in ana array
                }
            }
        }

        else if (strncmp(input, "shoombiz config global -email", 29) == 0)
        {

            if (exists_config_email == 1)
            {
                printf("You can't set a new email again.");
            }
            else
            {

                if (access(directoryPath, F_OK) != -1)
                {
                    printf("You've already set your email and name."); // if golbal config file exists
                }
                else
                {
                    saparator(input, input_saver_config_email);
                    config_file = fopen("global_config.txt", "w");
                    fprintf(config_file, "Username : %s \n", input_saver_config_name);
                    fprintf(config_file, "Useremail : %s \n", input_saver_config_email);
                    // extra info for other inputs.
                    fprintf(config_file, "last_commit_id : %d\n", 0);
                    fprintf(config_file, "current_commit_id : %d\n", 0);
                    fprintf(config_file, "branch : %s\n", "master");
                    fclose(config_file);
                    printf("You set your global name and global email succsefully.");
                    exists_config_name = 1;
                }
                exists_config_email = 1;
            }
        }

        else if (strncmp(input, "shoombiz config -username", 25) == 0)
        {
            if (access(directoryPath, F_OK) != -1) // if you have global config file you can not set config file!
            {
                printf("You've already set your email.");
            }
            else
            {
                saparator(input, input_saver_config_name); // saving name in an array
            }
        }

        else if (strncmp(input, "shoombiz config -email", 22) == 0)
        {

            if (access(directoryPath, F_OK) != -1)
            {
                printf("You'v already set your email."); // if you have global config file you can not set normal config
            }
            else
            {
                saparator(input, input_saver_config_email);
                config_file = fopen("config.txt", "a");
                fprintf(config_file, "Username : %s \n", input_saver_config_name);
                fprintf(config_file, "Useremail : %s \n", input_saver_config_email);
                // extra info for other inputs.
                fprintf(config_file, "last_commit_id : %d\n", 0);
                fprintf(config_file, "current_commit_id : %d\n", 0);
                fprintf(config_file, "branch : %s\n", "master");
                fclose(config_file);
                printf("you set your name and email succsefully.");
                FILE *info_saver = fopen("user_saver.txt", "w");
                fprintf(info_saver, "Username : %s\n", input_saver_config_name);
                fprintf(info_saver, "Useremail : %s\n", input_saver_config_email);
                fclose(info_saver);
            }
        }

        else if (strncmp(input, "shoombiz add", 12) == 0 && strstr(input, "*") == NULL && strncmp(input, "shoombiz add -f", 15) != 0 && strncmp(input, "shoombiz add depth", 18) != 0 && strncmp(input, "shoombiz add -redo", 18) != 0) // because of having a lot commands in add:)))))))))
        {
            char path[100];
            int fouuund = 0;
            int fouuunddd = 0;
            saparator(input, path);
            if (access(path, F_OK) == 0) // if that file exists & becareful that code get address
            {
                if (directoryExists(path)) // if it is dir
                {
                    DIR *dir;
                    struct dirent *entry;

                    char *path_temp = basename(path); // just the name of dir

                    // Open the directory
                    dir = opendir(path);
                    if (dir == NULL) // the address of dir does not exist
                    {
                        printf("Aw! Something wrong happend. please try again.");
                        continue;
                    }
                    while ((entry = (readdir(dir))) != NULL) // reading name of each file that is in the dir
                    {
                        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) // ignoring things like .vscode .hi
                        {
                            char *dir_name = basename(entry->d_name);
                            staging = fopen("staging.txt", "r");
                            if (staging == NULL)
                                printf("Something wrong happend. plaese try again...\n"); // for sure.
                            int len = strlen(dir_name);
                            dir_name[len] = '\0';
                            char line[100];
                            while (fgets(line, (sizeof(line)), staging))
                            {
                                if (strstr(line, dir_name) != NULL)
                                {
                                    fouuunddd = 1; // we had that file at staging before do not add it
                                    break;
                                }
                            }
                            if (fouuunddd == 0)
                            {
                                staging = fopen("staging.txt", "a");
                                fprintf(staging, "%s\n", dir_name); // printing file that exists in dir in staging
                                fclose(staging);
                            }
                        }
                    }
                    if (fouuunddd == 1)
                    {
                        printf("This directory has been already on stage mood.");
                    }
                    else
                    {
                        staging = fopen("staging.txt", "a");
                        fprintf(staging, "%s\n", path_temp); // printing name of dir in staging
                        fclose(staging);
                        printf("Your directory is on stage mode now.");
                    }
                }
                else
                {
                    // first we check that we have this address in our file or not.
                    staging = fopen("staging.txt", "r");
                    if (staging == NULL)
                        printf("Something wrong happend. plaese try again.");
                    int len = strlen(path);
                    path[len] = '\0';
                    char line[100];
                    char *temp_path = basename(path);
                    while (fgets(line, sizeof(line), staging))
                    {
                        if (strstr(line, temp_path) != NULL)
                        {
                            fouuund = 1;
                            break;
                        }
                    }
                    fclose(staging);
                    if (fouuund == 1)
                    {
                        printf("This file has been already on stage mood.");
                    }
                    else
                    {
                        staging = fopen("staging.txt", "a");
                        if (staging == NULL)
                            printf("Something wrong happend. olease try again.");
                        fprintf(staging, "%s\n", temp_path);
                        fclose(staging);
                        printf("Your file is on stage mood now.");
                    }
                }
            }
            else
            {
                printf("We dont have such a file or directory");
            }
        }

        else if (strstr(input, "*") != NULL) // wild card
        {
            char file_star[1000];
            saparator(input, file_star);
            int counter = 0;
            char names_of_files[100][100];
            char names_of_dir[100][100];
            DIR *dir;
            struct dirent *entry;
            dir = opendir(currentDirectory);
            // Check if the directory was opened successfully
            if (dir == NULL)
            {
                printf("Unable to open the directory.\n");
            }
            // Read each entry in the directory
            while ((entry = readdir(dir)) != NULL)
            {
                // Ignore the "." and ".." entries
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    strcpy(names_of_files[counter], entry->d_name);
                    counter++;
                }
            }
            closedir(dir);

            for (int i = 0; i < counter; i++)
            {
                if (strstr(names_of_files[i], ".") == NULL) // is dir
                {

                    if (match(file_star, names_of_files[i]))
                    {
                        char string[150];
                        strcpy(string, currentDirectory);
                        strcat(string, "\\\\");
                        strcat(string, names_of_files[i]); // making addrss of each file
                        // printf("%s\n", string);

                        DIR *dir;
                        int counter_dir = 0;
                        struct dirent *entry;
                        dir = opendir(string);
                        // Check if the directory was opened successfully
                        if (dir == NULL)
                        {
                            printf("Unable to open the directory.\n");
                        }
                        // Read each entry in the directory
                        while ((entry = readdir(dir)) != NULL)
                        {
                            // Ignore the "." and ".." entries
                            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                            {
                                strcpy(names_of_dir[counter_dir], entry->d_name);
                                counter_dir++;
                            }
                        }
                        closedir(dir);
                        for (int j = 0; j < counter_dir; j++)
                        {
                            staging = fopen("staging.txt", "a");
                            if (staging == NULL)
                                printf("something wrong happend. please try again.");
                            char *temp = basename(names_of_dir[j]);
                            fprintf(staging, "%s\n", temp);
                            fclose(staging);
                        }
                        staging = fopen("staging.txt", "a");
                        char *temp1 = basename(string);
                        fprintf(staging, "%s\n", temp1);
                        fclose(staging);
                        printf("This directory is now in stage mood!\n");
                    }
                }
                else if (strstr(names_of_files[i], ".") != NULL) // is file
                {
                    char m[100];
                    strcpy(m, names_of_files[i]);
                    replaceDotWithNull(names_of_files[i]);
                    // printf("%s+++%s\n", file_star, names_of_files[i]);
                    if (match(file_star, names_of_files[i]))
                    {
                        int len = strlen(names_of_files[i]);
                        names_of_files[i][len] = '\0';
                        bool flagg = false;
                        for (int i = 0; i <= counter; i++)
                        {
                            if (match(file_star, names_of_files[i]))
                            {
                                staging = fopen("staging.txt", "a");
                                if (staging == NULL)
                                    printf("something wrong happend. please try again.");
                                fprintf(staging, "%s\n", m);
                                fclose(staging);
                                flagg = true;
                                // break;
                            }
                        }
                        if (flagg)
                        {
                            printf("your file is on stage mood..\n");
                        }
                        else
                        {
                            printf("we dont have such a file.");
                        }
                    }
                }
            }
        }

        else if (strncmp(input, "shoombiz add -f", 15) == 0)
        {
            char *name[1000];
            int flagg_for_f = 0;
            int y = token_del(name, input);
            for (int d = 0; d < y; d++)
            {
                if (strstr(name[d], ".") != NULL)
                {
                    flagg_for_f = 0;
                    char *temp_name;
                    if (access(name[d], F_OK) == 0)
                    {
                        staging = fopen("staging.txt", "r");
                        if (staging == NULL)
                        {
                            printf("something wrong happend. plaese try again.");
                        }
                        int len = strlen(name[d]);
                        name[len] = '\0';
                        temp_name = basename(name[d]);
                        char line[100];
                        while (fgets(line, sizeof(line), staging))
                        {
                            if (strstr(line, temp_name) != NULL)
                            {
                                flagg_for_f = 1;
                                break;
                            }
                        }
                        fclose(staging);
                        if (flagg_for_f == 1)
                        {
                            printf("%s has been already on stage mood.\n", temp_name);
                        }
                        else
                        {
                            staging = fopen("staging.txt", "a");
                            if (staging == NULL)
                                printf("something wrong happend. olease try again.");
                            fprintf(staging, "%s\n", temp_name);
                            fclose(staging);
                        }
                    }
                    else
                    {
                        printf("file %s is not exists", temp_name);
                    }
                }
                else if (strstr(name[d], ".") == NULL)
                {
                    int flagg_for_2 = 0;
                    if (access(name[d], F_OK) == 0)
                    {
                        DIR *dir;
                        struct dirent *entry;

                        // Open the directory
                        dir = opendir(name[d]);
                        if (dir == NULL)
                        {
                            printf("something wrong happend. please try again.");
                            continue;
                        }
                        while ((entry = (readdir(dir))) != NULL)
                        {
                            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                            {
                                staging = fopen("staging.txt", "r");
                                if (staging == NULL)
                                    printf("something wrong happend. plaese try again."); // for sure.
                                char *temp_name = basename(entry->d_name);
                                int len = strlen(temp_name);
                                temp_name[len] = '\0';
                                char line[100];
                                while (fgets(line, (sizeof(line)), staging))
                                {
                                    if (strstr(line, temp_name) != NULL)
                                    {
                                        flagg_for_2 = 1;
                                        break;
                                    }
                                }
                                if (flagg_for_2 == 0)
                                {
                                    staging = fopen("staging.txt", "a");
                                    if (staging == NULL)
                                        printf("something wrong happend. please try again.");
                                    fprintf(staging, "%s\n", temp_name);
                                    fclose(staging);
                                }
                            }
                        }
                        if (flagg_for_2 == 1)
                        {
                            printf("this directory has been already on stage mood.\n");
                        }
                        else if (flagg_for_2 == 0)
                        {
                            char *temp_dir = basename(name[d]);
                            staging = fopen("staging.txt", "a");
                            fprintf(staging, "%s\n", temp_dir);
                            fclose(staging);
                        }
                    }

                    else
                    {
                        char *temp = basename(name[d]);
                        printf("directory %s is not exists.\n", temp);
                    }
                }
            }
        }

        else if (strncmp(input, "shoombiz add depth", 18) == 0)
        {
            FILE *file;
            char line[100];
            DIR *dir;
            struct dirent *entry;
            file = fopen("staging.txt", "r");
            if (file == NULL)
                printf("Error opening file.\n");
            dir = opendir(copy_one_cu);
            if (dir == NULL)
                printf("Error opening directory.\n");

            while (fgets(line, sizeof(line), file))
            {
                line[strcspn(line, "\n")] = '\0';
                while ((entry = readdir(dir)) != NULL)
                {
                    if (strcmp(line, entry->d_name) == 0)
                    {
                        printf("Alert:%s is on stage mode\n", entry->d_name);
                        break;
                    }
                }
                rewinddir(dir);
            }
            fclose(file);
            FILE *tem = fopen("depth.txt", "w");
            while ((entry = readdir(dir)) != NULL)
            {
                if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0))
                {
                    fprintf(tem, "%s\n", entry->d_name);
                }
            }
            fclose(tem);
            closedir(dir); // for moving the pointer of pc.
            FILE *file1, *file2;
            char line1[100], line2[100];
            int match = 0;

            file1 = fopen("depth.txt", "r");
            file2 = fopen("staging.txt", "r");
            while (fgets(line1, sizeof(line1), file1))
            {
                rewind(file2); // for moving the pointer of pc.
                match = 0;
                while (fgets(line2, sizeof(line2), file2))
                {
                    if (strcmp(line1, line2) == 0)
                    {
                        match = 1;
                        break;
                    }
                }

                if (match == 0)
                {
                    printf("Alert: %s is not on stage mode.\n", line1);
                }
            }

            fclose(file1);
            fclose(file2);
            remove(file1);
            remove(tem);
        }

        else if (strncmp(input, "shoombiz reset", 14) == 0)
        {
            char reset[200];
            saparator(input, reset);
            const char *filename = "staging.txt";
            struct stat path_stat;
            struct dirent *entry;
            if (stat(reset, &path_stat) == 0)
            {
                // printf("944\n");
                if (S_ISDIR(path_stat.st_mode))
                {
                    // printf("947\n");
                    reset_for_directory(filename, reset);
                }
                else if (S_ISREG(path_stat.st_mode))
                {
                    // printf("952\n");
                    char *name = basename(reset);
                    // printf("954\n");
                    FILE *file = fopen("staging.txt", "r");
                    if (file == NULL)
                    {
                        printf("Error opening file.\n");
                        continue;
                    }
                    FILE *tempFile = fopen("temp_for_file.txt", "w");
                    if (tempFile == NULL)
                    {
                        printf("Error creating temporary file.\n");
                        fclose(file);
                        continue;
                    }
                    FILE *unstaged = fopen("unstage.txt", "a");
                    if (unstaged == NULL)
                    {
                        printf("Error: cannot create unstaged file.\n");
                        fclose(tempFile);
                        fclose(file);
                        continue;
                    }

                    char line[1000];
                    while (fgets(line, sizeof(line), file) != NULL)
                    {
                        line[strcspn(line, "\n")] = '\0';
                        // printf("%s", line);
                        if (strstr(line, name) == NULL)
                        {

                            fprintf(tempFile, "%s\n", line);
                        }
                        else
                        {
                            fprintf(unstaged, "%s\n", line);
                        }
                    }
                    fclose(unstaged);
                    fclose(file);
                    fclose(tempFile);
                    remove("staging.txt");
                    rename("temp_for_file.txt", "staging.txt");
                    fclose(tempFile);
                }
            }
        }

        else if (strncmp(input, "shoombiz add -redo", 18) == 0)
        {
            FILE *sourceFile, *destinationFile;
            char ch;
            sourceFile = fopen("unstage.txt", "r");
            if (sourceFile == NULL)
            {
                printf("Unable to open source file.\n");
                continue;
            }

            destinationFile = fopen("staging.txt", "a");
            if (destinationFile == NULL)
            {
                printf("Unable to open destination file.\n");
                fclose(sourceFile);
                continue;
            }

            while ((ch = fgetc(sourceFile)) != EOF)
            {
                fputc(ch, destinationFile);
            }

            fclose(sourceFile);
            fclose(destinationFile);

            printf("The mission is done.\n");
            remove("unstage.txt");
        }

        else if (strncmp(input, "shoombiz status", 15) == 0)
        {
            DIR *dir_s;
            struct dirent *entry_s;
            char second_con[100][100];
            char temp_cond[100][100];
            int count_s = 0;
            dir_s = opendir(copy_one_cu);

            // Read each entry in the directory
            while ((entry_s = readdir(dir_s)) != NULL)
            {
                // Ignore "." and ".." entries
                if (strcmp(entry_s->d_name, ".") != 0 && strcmp(entry_s->d_name, "..") != 0)
                {
                    strcpy(second_con[count_s], entry_s->d_name);
                    count_s++;
                }
            }
            closedir(dir_s);
            int flagg_for_stat_d = 0;
            for (ii = 0; ii < couunt; ii++)
            {
                flagg_for_stat_d = 0;
                for (int j = 0; j < count_s; j++)
                {
                    if (flagg_for_stat_d == 0)
                    {
                        if (strcmp(first_name_cond[ii], second_con[j]) == 0)
                        {
                            flagg_for_stat_d = 1;
                        }
                    }
                }
                if (flagg_for_stat_d == 0)
                {
                    staging = fopen("staging.txt", "r");
                    char line[100];
                    int flagg = 0;
                    while (fgets(line, sizeof(line), staging) != NULL)
                    {
                        line[strcspn(line, "\n")] = '\0';
                        if (strcmp(line, first_name_cond[ii]) != 0)
                        {
                            flagg = 1;
                            strcpy(temp_cond[ii], first_name_cond[ii]);
                            printf("%s || +D\n", first_name_cond[ii]);
                            break;
                        }
                    }
                    fclose(staging);
                    if (flagg == 0)
                    {
                        strcpy(temp_cond[ii], first_name_cond[ii]);
                        printf("%s || -D\n", first_name_cond[ii]);
                    }
                }
            }
            int flagg_for_stat_a = 0;
            for (int k = 0; k < count_s; k++)
            {
                flagg_for_stat_a = 0;
                for (int l = 0; l < couunt; l++)
                {
                    if (flagg_for_stat_a == 0)
                    {
                        if (strcmp(second_con[k], first_name_cond[l]) == 0)
                        {
                            flagg_for_stat_a = 1;
                        }
                    }
                }
                if (flagg_for_stat_a == 0)
                {
                    staging = fopen("staging.txt", "r");
                    char line[100];
                    int flagg = 0;
                    while (fgets(line, sizeof(line), staging) != NULL)
                    {
                        line[strcspn(line, "\n")] = '\0';
                        if (strcmp(line, second_con[k]) != 0)
                        {
                            flagg = 1;
                            printf("%s || +A\n", second_con[k]);
                            break;
                        }
                    }
                    fclose(staging);
                    if (flagg == 0)
                    {
                        printf("%s || -A\n", second_con[k]);
                    }
                }
            }
            struct stat updatedFileStat;
            time_t updatedLastModifiedTime[256];
            for (int k = 0; k < couunt; k++)
            {
                stat(file_address[k], &updatedFileStat);
                updatedLastModifiedTime[k] = updatedFileStat.st_mtime;
                if ((lastModifiedTime[k] != updatedLastModifiedTime[k] && strcmp(temp_cond[k], first_name_cond[k]) != 0))
                {
                    staging = fopen("staging.txt", "r");
                    char line[100];
                    int flagg = 0;
                    while (fgets(line, sizeof(line), staging) != NULL)
                    {
                        line[strcspn(line, "\n")] = '\0';
                        if (strcmp(line, first_name_cond[k]) != 0)
                        {
                            printf("%s || +M\n", first_name_cond[k]);
                            flagg = 1;
                            break;
                        }
                    }
                    if (flagg == 0)
                    {
                        printf("%s || -M\n", first_name_cond[k]);
                    }
                }
            }
        }

        else if (strncmp(input, "shoombiz commit -m", 18) == 0)
        {
            if (access(directoryPath, F_OK) != -1)
            {
                char commit_message[1000];
                if (strstr(input, "\"") != NULL)
                {
                    saparator(input, commit_message);
                    int len = strlen(commit_message);
                    if (len >= 72)
                    {
                        printf("plaese enter a shorter commit.");
                        continue;
                    }
                    staging = fopen("staging.txt", "r");
                    char copy_address[500];
                    strcpy(copy_address, currentDirectory);
                    strcat(copy_address, "\\\\staging.txt");
                    fseek(staging, 0, SEEK_END);
                    long size = ftell(staging);
                    if (size == 0 || staging == NULL)
                    {
                        printf("there is no file on stage mood. first please add some file.");
                        fclose(staging);
                    }
                    else
                    {

                        char line[300];
                        FILE *message = fopen("message.txt", "a");
                        // Get the current time
                        time_t currentTime = time(NULL);
                        char *timeString;
                        commit = fopen("commit.txt", "a");
                        rewind(staging);
                        int count = 0; // number of files that goes to staging.
                        while (fgets(line, sizeof(line), staging) != NULL)
                        {
                            int len = strlen(line);
                            line[len] = '\0';
                            fprintf(commit, "%s\n", line);
                        }
                        fclose(staging);
                        char line1[100];
                        char *ptr;
                        global_config_file = fopen("global_config.txt", "r");
                        if (global_config_file != NULL)
                        {
                            while (fgets(line1, sizeof(line1), global_config_file) != NULL)
                            {
                                if (strstr(line1, "last") != NULL)
                                {
                                    char *token;
                                    token = strtok(line1, ":");
                                    token = strtok(NULL, " ");
                                    commit_id = atoi(token);
                                    commit_id++;
                                    ptr = malloc(sizeof(char) * 2);
                                    sprintf(ptr, "%d", commit_id);
                                }
                            }
                        }
                        fclose(global_config_file);
                        global_config_file = fopen("global_config.txt", "r");
                        char line2[100];
                        FILE *fcommit = fopen("temp_c.txt", "w");
                        while (fgets(line2, sizeof(line2), global_config_file) != NULL)
                        {
                            int len = strlen(line2);
                            line2[len] = '\0';

                            if (strstr(line2, "last") != NULL)
                            {
                                fprintf(fcommit, "last_commit_id : %s\n", ptr);
                            }
                            else
                            {
                                fprintf(fcommit, "%s", line2);
                            }
                        }
                        fclose(fcommit);
                        fclose(global_config_file);
                        remove("global_config.txt");
                        rename("temp_c.txt", "global_config.txt");
                        fclose(fcommit);
                        char line3[100];
                        staging = fopen("staging.txt", "r");
                        rewind(staging);
                        while (fgets(line3, sizeof(line3), staging) != NULL)
                        {
                            fprintf(message, "file : %s", line3);
                            fprintf(message, "commit id : %d\n ", commit_id);
                            fprintf(message, "message is : %s\n", commit_message);
                            timeString = ctime(&currentTime);
                            fprintf(message, "time : %s", timeString);
                            count++;
                        }
                        fprintf(message, "number of files is : %d\n", count);
                        fclose(commit);
                        fclose(message);
                        fclose(staging);
                        staging = fopen("staging.txt", "w");
                        fclose(staging);
                        printf("your commit id is : %d\n", commit_id);
                        printf("your commit message is :%s\n", commit_message);
                        printf("your commit time : %s", timeString);
                    }
                }
                else
                {
                    char saved[100];
                    char *extract[1000];
                    int index = 0;
                    char delimeter[] = " ";
                    char *ptr = strtok(input, delimeter);
                    while (ptr != NULL)
                    {
                        extract[index] = ptr;
                        index++;
                        ptr = strtok(NULL, delimeter);
                    }
                    int len = strlen(commit_message);
                    if (strcmp(extract[index - 1], "-m") == 0)
                    {
                        printf("please enter a message.");
                        continue;
                    }
                    if (len >= 72)
                    {
                        printf("plaese enter a shorter commit.");
                        continue;
                    }
                    staging = fopen("staging.txt", "r");
                    char copy_address[500];
                    strcpy(copy_address, currentDirectory);
                    strcat(copy_address, "\\\\staging.txt");
                    fseek(staging, 0, SEEK_END);
                    long size = ftell(staging);
                    if (size == 0 || staging == NULL)
                    {
                        printf("there is no file on stage mood. first please add some file.");
                        fclose(staging);
                        continue;
                    }
                    else
                    {
                        char line[300];
                        FILE *message = fopen("message.txt", "a");
                        // Get the current time
                        time_t currentTime = time(NULL);
                        char *timeString;
                        commit = fopen("commit.txt", "a");
                        rewind(staging);
                        int count = 0; // number of files that goes to staging.
                        while (fgets(line, sizeof(line), staging) != NULL)
                        {
                            // int len = strlen(line);
                            // line[len] = '\0';
                            fprintf(commit, "%s", line);
                        }
                        fclose(staging);
                        char line1[100];
                        char *ptr;
                        global_config_file = fopen("global_config.txt", "r");
                        if (global_config_file != NULL)
                        {
                            while (fgets(line1, sizeof(line1), global_config_file) != NULL)
                            {
                                if (strstr(line1, "last") != NULL)
                                {
                                    char *token;
                                    token = strtok(line1, ":");
                                    token = strtok(NULL, " ");
                                    commit_id = atoi(token);
                                    commit_id++;
                                    ptr = malloc(sizeof(char) * 2);
                                    sprintf(ptr, "%d", commit_id);
                                }
                            }
                        }
                        fclose(global_config_file);
                        global_config_file = fopen("global_config.txt", "r");
                        char line2[100];
                        FILE *fcommit = fopen("temp_c.txt", "w");
                        while (fgets(line2, sizeof(line2), global_config_file) != NULL)
                        {
                            // int len = strlen(line2);
                            // line2[len] = '\0';
                            if (strstr(line2, "last") != NULL)
                            {
                                fprintf(fcommit, "last_commit_id : %s\n", ptr);
                            }
                            else
                            {
                                fprintf(fcommit, "%s", line2);
                            }
                        }
                        fclose(fcommit);
                        fclose(global_config_file);
                        remove("global_config.txt");
                        rename("temp_c.txt", "global_config.txt");
                        fclose(fcommit);
                        char line3[100];
                        staging = fopen("staging.txt", "r");
                        rewind(staging);
                        while (fgets(line3, sizeof(line3), staging) != NULL)
                        {
                            fprintf(message, "file : %s", line3);
                            fprintf(message, "commit id : %d\n ", commit_id);
                            fprintf(message, "message is : %s\n", extract[index - 1]);
                            timeString = ctime(&currentTime);
                            fprintf(message, "time : %s", timeString);
                            count++;
                        }
                        fprintf(message, "number of files is : %d\n", count);
                        fclose(commit);
                        fclose(message);
                        fclose(staging);
                        staging = fopen("staging.txt", "w");
                        fclose(staging);
                        printf("your commit id is : %d\n", commit_id);
                        printf("your commit message is :%s\n", extract[index - 1]);
                        printf("your commit time : %s", timeString);
                    }
                }
            }

            else
            {
                char commit_message[1000];
                if (strstr(input, "\"") != NULL)
                {
                    saparator(input, commit_message);
                    int len = strlen(commit_message);
                    if (len >= 72)
                    {
                        printf("plaese enter a shorter commit.");
                        continue;
                    }
                    staging = fopen("staging.txt", "r");
                    char copy_address[500];
                    strcpy(copy_address, currentDirectory);
                    strcat(copy_address, "\\\\staging.txt");
                    fseek(staging, 0, SEEK_END);
                    long size = ftell(staging);
                    if (size == 0 || staging == NULL)
                    {
                        printf("there is no file on stage mood. first please add some file.");
                        fclose(staging);
                        continue;
                    }
                    else
                    {
                        char line[300];
                        FILE *message = fopen("message.txt", "a");
                        // Get the current time
                        time_t currentTime = time(NULL);
                        char *timeString;
                        commit = fopen("commit.txt", "a");
                        rewind(staging);
                        int count = 0; // number of files that goes to staging.
                        while (fgets(line, sizeof(line), staging) != NULL)
                        {
                            fputs(line, commit);
                        }
                        fclose(staging);
                        fclose(commit);
                        char line_user[100];
                        char *ptr;
                        char line_con[100];
                        FILE *config = fopen("config.txt", "r");
                        FILE *user = fopen("user_saver.txt", "r");
                        char *token2;
                        rewind(user);
                        rewind(config);
                        while (fgets(line_user, sizeof(line_user), user) != NULL)
                        {
                            line_user[strcspn(line_user, "\n")] = '\0';
                            if (strstr(line_user, "Username") != NULL)
                            {
                                token2 = strtok(line_user, ":");
                                token2 = strtok(NULL, " ");
                                break;
                            }
                        }
                        int count_l = 1;
                        rewind(config);
                        int saver = 0;
                        rewind(config);
                        while (fgets(line_con, sizeof(line_con), config) != NULL)
                        {
                            line_con[strlen(line_con) - 1] = '\0';
                            if ((count_l % 5 == 1))
                            {
                                char *extract;
                                extract = strtok(line_con, ":");
                                extract = strtok(NULL, " ");
                                if (strcmp(extract, token2) == 0)
                                {
                                    saver = count_l + 2;
                                }
                            }
                            if (count_l == saver)
                            {
                                char *token;
                                token = strtok(line_con, ":");
                                token = strtok(NULL, " ");
                                // printf("%s", token);
                                commit_id = atoi(token);
                                commit_id++;
                                //  printf("%d\n", commit_id);
                                ptr = malloc(sizeof(char) * 2);
                                sprintf(ptr, "%d", commit_id);
                                break;
                            }
                            count_l++;
                        }
                        fclose(config);
                        fclose(user);
                        config = fopen("config.txt", "r");
                        char line2[100];
                        FILE *fcommit = fopen("temp_c.txt", "w");
                        int flagg_for_print = 0;
                        int counter_c = 0;
                        while (fgets(line2, sizeof(line2), config) != NULL)
                        {
                            counter_c++;
                            if (counter_c == saver)
                            {
                                fprintf(fcommit, "last_commit_id : %s\n", ptr);
                            }
                            else
                            {
                                fprintf(fcommit, "%s", line2);
                            }
                        }
                        fclose(fcommit);
                        fclose(config);
                        remove("config.txt");
                        rename("temp_c.txt", "config.txt");
                        fclose(fcommit);
                        char line3[100];
                        staging = fopen("staging.txt", "r");
                        rewind(staging);
                        while (fgets(line3, sizeof(line3), staging) != NULL)
                        {
                            fprintf(message, "file : %s", line3);
                            fprintf(message, "username : %s\n", token2);
                            fprintf(message, "commit id : %d\n ", commit_id);
                            fprintf(message, "message is : %s\n", commit_message);
                            timeString = ctime(&currentTime);
                            fprintf(message, "time : %s", timeString);
                            count++;
                        }
                        fprintf(message, "number of files is : %d", count);
                        fclose(commit);
                        fclose(message);
                        fclose(staging);
                        staging = fopen("staging.txt", "w");
                        fclose(staging);
                        printf("the user committing project is: %s\n", token2);
                        printf("your commit id is : %d\n", commit_id);
                        printf("your commit message is :%s\n", commit_message);
                        printf("your commit time : %s", timeString);
                    }
                }
                else
                {

                    char saved[100];
                    char *extract[1000];
                    int index = 0;
                    char delimeter[] = " ";
                    char *ptr = strtok(input, delimeter);
                    while (ptr != NULL)
                    {
                        extract[index] = ptr;
                        index++;
                        ptr = strtok(NULL, delimeter);
                    }
                    strcpy(commit_message, extract[index - 1]);
                    int len = strlen(commit_message);
                    if (strcmp(extract[index - 1], "-m") == 0)
                    {
                        printf("please enter a message.");
                        continue;
                    }
                    if (len >= 72)
                    {
                        printf("plaese enter a shorter commit.");
                        continue;
                    }
                    staging = fopen("staging.txt", "r");
                    char copy_address[500];
                    strcpy(copy_address, currentDirectory);
                    strcat(copy_address, "\\\\staging.txt");
                    fseek(staging, 0, SEEK_END);
                    long size = ftell(staging);
                    if (size == 0 || staging == NULL)
                    {
                        printf("there is no file on stage mood. first please add some file.");
                        fclose(staging);
                        continue;
                    }
                    else
                    {
                        char line[300];
                        FILE *message = fopen("message.txt", "a");
                        // Get the current time
                        time_t currentTime = time(NULL);
                        char *timeString;
                        commit = fopen("commit.txt", "a");
                        rewind(staging);
                        int count = 0; // number of files that goes to staging.
                        while (fgets(line, sizeof(line), staging) != NULL)
                        {
                            fputs(line, commit);
                        }
                        fclose(staging);
                        fclose(commit);
                        char line_user[100];
                        char *ptr;
                        char line_con[100];
                        FILE *config = fopen("config.txt", "r");
                        FILE *user = fopen("user_saver.txt", "r");
                        char *token2;
                        rewind(user);
                        rewind(config);
                        while (fgets(line_user, sizeof(line_user), user) != NULL)
                        {
                            line_user[strcspn(line_user, "\n")] = '\0';
                            if (strstr(line_user, "Username") != NULL)
                            {
                                token2 = strtok(line_user, ":");
                                token2 = strtok(NULL, " ");
                                break;
                            }
                        }
                        int count_l = 1;
                        rewind(config);
                        int saver = 0;
                        rewind(config);
                        while (fgets(line_con, sizeof(line_con), config) != NULL)
                        {
                            line_con[strlen(line_con) - 1] = '\0';
                            if ((count_l % 5 == 1))
                            {
                                char *extract;
                                extract = strtok(line_con, ":");
                                extract = strtok(NULL, " ");
                                if (strcmp(extract, token2) == 0)
                                {
                                    saver = count_l + 2;
                                }
                            }
                            if (count_l == saver)
                            {
                                char *token;
                                token = strtok(line_con, ":");
                                token = strtok(NULL, " ");
                                // printf("%s", token);
                                commit_id = atoi(token);
                                commit_id++;
                                //  printf("%d\n", commit_id);
                                ptr = malloc(sizeof(char) * 2);
                                sprintf(ptr, "%d", commit_id);
                                break;
                            }
                            count_l++;
                        }
                        fclose(config);
                        fclose(user);
                        config = fopen("config.txt", "r");
                        char line2[100];
                        FILE *fcommit = fopen("temp_c.txt", "w");
                        int flagg_for_print = 0;
                        int counter_c = 0;
                        while (fgets(line2, sizeof(line2), config) != NULL)
                        {
                            counter_c++;
                            if (counter_c == saver)
                            {
                                fprintf(fcommit, "last_commit_id : %s\n", ptr);
                            }
                            else
                            {
                                fprintf(fcommit, "%s", line2);
                            }
                        }
                        fclose(fcommit);
                        fclose(config);
                        remove("config.txt");
                        rename("temp_c.txt", "config.txt");
                        fclose(fcommit);
                        char line3[100];
                        staging = fopen("staging.txt", "r");
                        rewind(staging);
                        while (fgets(line3, sizeof(line3), staging) != NULL)
                        {
                            fprintf(message, "file : %s", line3);
                            fprintf(message, "username : %s\n", token2);
                            fprintf(message, "commit id : %d\n ", commit_id);
                            fprintf(message, "message is : %s\n", commit_message);
                            timeString = ctime(&currentTime);
                            fprintf(message, "time : %s", timeString);
                            count++;
                        }
                        fprintf(message, "number of files is : %d", count);
                        fclose(commit);
                        fclose(message);
                        fclose(staging);
                        staging = fopen("staging.txt", "w");
                        fclose(staging);
                        printf("the user committing project is: %s\n", token2);
                        printf("your commit id is : %d\n", commit_id);
                        printf("your commit message is :%s\n", commit_message);
                        printf("your commit time : %s", timeString);
                    }
                }
            }
        }

        else if (strncmp(input, "shoombiz set -m", 15) == 0)
        {
            FILE *shortcuts = fopen("shortcut.txt", "a");
            extractText(input, shortcut_message);
            // printf("%s",shortcut_message);
            char *extract[1000];
            int index = 0;
            char delimeter[] = " ";
            char *ptr = strtok(input, delimeter);
            while (ptr != NULL)
            {
                extract[index] = ptr;
                index++;
                ptr = strtok(NULL, delimeter);
            }
            strcpy(shortcut, extract[index - 1]);

            fprintf(shortcuts, "%s\n", shortcut);
            fprintf(shortcuts, "%s\n", shortcut_message);
            fclose(shortcuts);
        }

        else if (strncmp(input, "shoombiz commit -s", 18) == 0)
        {
            char *extract_1[1000];
            int index_1 = 0;
            char delimeter[] = " ";
            char *ptr = strtok(input, delimeter);
            while (ptr != NULL)
            {
                extract_1[index_1] = ptr;
                index_1++;
                ptr = strtok(NULL, delimeter);
            }
            char line[100];
            int flagg_for_commit = 0;
            FILE *shorts = fopen("shortcut.txt", "r");
            rewind(shorts);
            while (fgets(line, sizeof(line), shorts) != NULL)
            {
                line[strcspn(line, "\n")] = '\0';
                if (strcmp(line, extract_1[index_1 - 1]) == 0)
                {
                    flagg_for_commit = 1;
                    if (access(directoryPath, F_OK) != -1)
                    {
                        char commit_message[1000];
                        strcpy(commit_message, extract_1[index_1 - 1]);
                        int len = strlen(extract_1[index_1 - 1]);
                        if (strcmp(extract_1[index_1 - 1], "-m") == 0)
                        {
                            printf("please enter a message.");
                            continue;
                        }
                        if (len >= 72)
                        {
                            printf("plaese enter a shorter commit.");
                            continue;
                        }
                        staging = fopen("staging.txt", "r");
                        char copy_address[500];
                        strcpy(copy_address, currentDirectory);
                        strcat(copy_address, "\\\\staging.txt");
                        fseek(staging, 0, SEEK_END);
                        long size = ftell(staging);
                        if (size == 0 || staging == NULL)
                        {
                            printf("there is no file on stage mood. first please add some file.");
                            fclose(staging);
                            continue;
                        }
                        else
                        {
                            strcpy(commit_message, extract_1[index_1 - 1]);
                            int len = strlen(commit_message);
                            if (strcmp(extract_1[index_1 - 1], "-m") == 0)
                            {
                                printf("please enter a message.");
                                continue;
                            }
                            if (len >= 72)
                            {
                                printf("plaese enter a shorter commit.");
                                continue;
                            }
                            staging = fopen("staging.txt", "r");
                            char copy_address[500];
                            strcpy(copy_address, currentDirectory);
                            strcat(copy_address, "\\\\staging.txt");
                            fseek(staging, 0, SEEK_END);
                            long size = ftell(staging);
                            if (size == 0 || staging == NULL)
                            {
                                printf("there is no file on stage mood. first please add some file.");
                                fclose(staging);
                                continue;
                            }
                            else
                            {
                                char line[300];
                                FILE *message = fopen("message.txt", "a");
                                // Get the current time
                                time_t currentTime = time(NULL);
                                char *timeString;
                                commit = fopen("commit.txt", "a");
                                rewind(staging);
                                int count = 0; // number of files that goes to staging.
                                while (fgets(line, sizeof(line), staging) != NULL)
                                {
                                    // int len = strlen(line);
                                    // line[len] = '\0';
                                    fprintf(commit, "%s", line);
                                }
                                fclose(staging);
                                char line1[100];
                                char *ptr;
                                global_config_file = fopen("global_config.txt", "r");
                                if (global_config_file != NULL)
                                {
                                    while (fgets(line1, sizeof(line1), global_config_file) != NULL)
                                    {
                                        if (strstr(line1, "last") != NULL)
                                        {
                                            char *token;
                                            token = strtok(line1, ":");
                                            token = strtok(NULL, " ");
                                            commit_id = atoi(token);
                                            commit_id++;
                                            ptr = malloc(sizeof(char) * 2);
                                            sprintf(ptr, "%d", commit_id);
                                        }
                                    }
                                }
                                fclose(global_config_file);
                                global_config_file = fopen("global_config.txt", "r");
                                char line2[100];
                                FILE *fcommit = fopen("temp_c.txt", "w");
                                while (fgets(line2, sizeof(line2), global_config_file) != NULL)
                                {
                                    // int len = strlen(line2);
                                    // line2[len] = '\0';
                                    if (strstr(line2, "last") != NULL)
                                    {
                                        fprintf(fcommit, "last_commit_id : %s\n", ptr);
                                    }
                                    else
                                    {
                                        fprintf(fcommit, "%s", line2);
                                    }
                                }
                                fclose(fcommit);
                                fclose(global_config_file);
                                remove("global_config.txt");
                                rename("temp_c.txt", "global_config.txt");
                                fclose(fcommit);
                                char line3[100];
                                staging = fopen("staging.txt", "r");
                                rewind(staging);
                                int co = 0;
                                int check = 0;
                                char line18[100];
                                char commit_n[100];
                                FILE *sh = fopen("shortcut.txt", "r");
                                while (fgets(line18, sizeof(line18), sh) != NULL)
                                {
                                    co++;
                                    line18[strcspn(line18, "\n")] = '\0';
                                    if (strcmp(line18, commit_message) == 0)
                                    {
                                        check = co + 1;
                                    }
                                    if (check == co)
                                    {
                                        strcpy(commit_n, line18);
                                        break;
                                    }
                                }
                                fclose(sh);
                                while (fgets(line3, sizeof(line3), staging) != NULL)
                                {
                                    fprintf(message, "file : %s", line3);
                                    fprintf(message, "commit id : %d\n ", commit_id);
                                    fprintf(message, "message is : %s\n", commit_n);
                                    timeString = ctime(&currentTime);
                                    fprintf(message, "time : %s", timeString);
                                    count++;
                                }
                                fprintf(message, "number of files is : %d\n", count);
                                fclose(commit);
                                fclose(message);
                                fclose(staging);
                                staging = fopen("staging.txt", "w");
                                fclose(staging);
                                printf("your commit id is : %d\n", commit_id);
                                printf("your commit message is :%s\n", commit_n);
                                printf("your commit time : %s", timeString);
                            }
                        }
                    }

                    else
                    {
                        char commit_message[1000];
                        strcpy(commit_message, extract_1[index_1 - 1]);
                        int len = strlen(commit_message);
                        if (strcmp(extract_1[index_1 - 1], "-m") == 0)
                        {
                            printf("please enter a message.");
                            continue;
                        }
                        if (len >= 72)
                        {
                            printf("plaese enter a shorter commit.");
                            continue;
                        }
                        staging = fopen("staging.txt", "r");
                        char copy_address[500];
                        strcpy(copy_address, currentDirectory);
                        strcat(copy_address, "\\\\staging.txt");
                        fseek(staging, 0, SEEK_END);
                        long size = ftell(staging);
                        if (size == 0 || staging == NULL)
                        {
                            printf("there is no file on stage mood. first please add some file.");
                            fclose(staging);
                            continue;
                        }
                        else
                        {
                            char line[300];
                            FILE *message = fopen("message.txt", "a");
                            // Get the current time
                            time_t currentTime = time(NULL);
                            char *timeString;
                            commit = fopen("commit.txt", "a");
                            rewind(staging);
                            int count = 0; // number of files that goes to staging.
                            while (fgets(line, sizeof(line), staging) != NULL)
                            {
                                fputs(line, commit);
                            }
                            fclose(staging);
                            fclose(commit);
                            char line_user[100];
                            char *ptr;
                            char line_con[100];
                            FILE *config = fopen("config.txt", "r");
                            FILE *user = fopen("user_saver.txt", "r");
                            char *token2;
                            rewind(user);
                            rewind(config);
                            while (fgets(line_user, sizeof(line_user), user) != NULL)
                            {
                                line_user[strcspn(line_user, "\n")] = '\0';
                                if (strstr(line_user, "Username") != NULL)
                                {
                                    token2 = strtok(line_user, ":");
                                    token2 = strtok(NULL, " ");
                                    break;
                                }
                            }
                            int count_l = 1;
                            rewind(config);
                            int saver = 0;
                            rewind(config);
                            while (fgets(line_con, sizeof(line_con), config) != NULL)
                            {
                                line_con[strlen(line_con) - 1] = '\0';
                                if ((count_l % 5 == 1))
                                {
                                    char *extract;
                                    extract = strtok(line_con, ":");
                                    extract = strtok(NULL, " ");
                                    if (strcmp(extract, token2) == 0)
                                    {
                                        saver = count_l + 2;
                                    }
                                }
                                if (count_l == saver)
                                {
                                    char *token;
                                    token = strtok(line_con, ":");
                                    token = strtok(NULL, " ");
                                    // printf("%s", token);
                                    commit_id = atoi(token);
                                    commit_id++;
                                    //  printf("%d\n", commit_id);
                                    ptr = malloc(sizeof(char) * 2);
                                    sprintf(ptr, "%d", commit_id);
                                    break;
                                }
                                count_l++;
                            }
                            fclose(config);
                            fclose(user);
                            config = fopen("config.txt", "r");
                            char line2[100];
                            FILE *fcommit = fopen("temp_c.txt", "w");
                            int counter_c = 0;
                            while (fgets(line2, sizeof(line2), config) != NULL)
                            {
                                counter_c++;
                                if (counter_c == saver)
                                {
                                    fprintf(fcommit, "last_commit_id : %s\n", ptr);
                                }
                                else
                                {
                                    fprintf(fcommit, "%s", line2);
                                }
                            }
                            fclose(fcommit);
                            fclose(config);
                            remove("config.txt");
                            rename("temp_c.txt", "config.txt");
                            fclose(fcommit);
                            char line3[100];
                            int co = 0;
                            int check = 0;
                            char line18[100];
                            char commit_n[100];
                            FILE *sh = fopen("shortcut.txt", "r");
                            while (fgets(line18, sizeof(line18), sh) != NULL)
                            {
                                co++;
                                line18[strcspn(line18, "\n")] = '\0';
                                if (strcmp(line18, commit_message) == 0)
                                {
                                    check = co + 1;
                                }
                                if (check == co)
                                {
                                    strcpy(commit_n, line18);
                                    break;
                                }
                            }
                            fclose(sh);
                            staging = fopen("staging.txt", "r");
                            rewind(staging);
                            while (fgets(line3, sizeof(line3), staging) != NULL)
                            {
                                fprintf(message, "file : %s", line3);
                                fprintf(message, "username : %s\n", token2);
                                fprintf(message, "commit id : %d\n ", commit_id);
                                fprintf(message, "message is : %s\n", commit_n);
                                timeString = ctime(&currentTime);
                                fprintf(message, "time : %s", timeString);
                                count++;
                            }
                            fprintf(message, "number of files is : %d", count);
                            fclose(commit);
                            fclose(message);
                            fclose(staging);
                            staging = fopen("staging.txt", "w");
                            fclose(staging);
                            printf("the user committing project is: %s\n", token2);
                            printf("your commit id is : %d\n", commit_id);
                            printf("your commit message is :%s\n", commit_n);
                            printf("your commit time : %s", timeString);
                        }
                    }
                }
            }
            if (flagg_for_commit == 0)
            {
                printf("you enterd invalid shortcut.");
            }
            fclose(shorts);
        }

        else if (strncmp(input, "shoombiz replace -m", 19) == 0)
        {
            char saved[100];
            extractText(input, saved);
            char *extract[1000];
            int index = 0;
            char delimeter[] = " ";
            char *ptr = strtok(input, delimeter);
            while (ptr != NULL)
            {
                extract[index] = ptr;
                index++;
                ptr = strtok(NULL, delimeter);
            }
            FILE *cf = fopen("temp_commit.txt", "w");
            FILE *ct = fopen("shortcut.txt", "r");
            int number = 1;
            char line[100];
            int flagg_for_replace = 0;
            while (fgets(line, sizeof(line), ct) != NULL)
            {
                // printf("%d\n",number);
                number++;
                line[strcspn(line, "\n")] = '\0';
                if (strcmp(line, extract[index - 1]) == 0)
                {
                    flagg_for_replace = 1;
                    break;
                }
            }
            if (flagg_for_replace == 0)
            {
                printf("please enter valid shortcut.");
                continue;
            }
            // printf("%d",number);
            fclose(ct);
            fclose(cf);
            ct = fopen("shortcut.txt", "r");
            cf = fopen("temp_commit.txt", "w");
            int number2 = 1;
            while (fgets(line, sizeof(line), ct) != NULL)
            {
                line[strcspn(line, "\n")] = '\0';
                if (number2 == number)
                {
                    fprintf(cf, "%s\n", saved);
                }
                else
                {
                    fprintf(cf, "%s\n", line);
                }
                number2++;
            }
            fclose(ct);
            fclose(cf);
            remove("shortcut.txt");
            rename("temp_commit.txt", "shortcut.txt");
            fclose(cf);
        }

        else if (strncmp(input, "shoombiz remove -s", 18) == 0)
        {
            char *extract[1000];
            int index = 0;
            char delimeter[] = " ";
            char *ptr = strtok(input, delimeter);
            while (ptr != NULL)
            {
                extract[index] = ptr;
                index++;
                ptr = strtok(NULL, delimeter);
            }
            FILE *sh = fopen("shortcut.txt", "r");
            FILE *delet = fopen("delet.txt", "w");
            char line[100];
            int flagg_for_delet = 0;
            int number = 1;
            while (fgets(line, sizeof(line), sh) != NULL)
            {
                line[strcspn(line, "\n")] = '\0';
                if (strcmp(line, extract[index - 1]) == 0)
                {
                    flagg_for_delet = 1;
                    break;
                }
                number++;
            }
            if (flagg_for_delet == 0)
            {
                printf("please enter valid shortcut.");
                continue;
            }
            // printf("%d",number);
            fclose(sh);
            sh = fopen("shortcut.txt", "r");
            int flagg_n = 0;
            int number2 = 1;
            while (fgets(line, sizeof(line), sh) != NULL)
            {
                line[strcspn(line, "\n")] = '\0';
                if (flagg_n == 1)
                {
                    flagg_n = 0;
                    continue;
                }
                if (number2 == number)
                {
                    flagg_n = 1;
                    continue;
                }
                else
                {
                    fprintf(delet, "%s\n", line);
                }
                number2++;
            }
            fclose(delet);
            fclose(sh);
            remove("shortcut.txt");
            rename("delet.txt", "shortcut.txt");
            fclose(delet);
        }
    }
}
