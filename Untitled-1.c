#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
char mian_qotation[10000];
char *user_name(char str[])
{
    int start;
    int end;
    for (int i = 0; i < strlen(str); i++)
    {
        if (strcmp(str[i], "\"") == 0)
        {
            start = i;
            break;
        }
        end = strlen(str) - 1;
        char dastoor[60];
        for (int j = start; j <= end; j++)
        {
            int y = 0;
            dastoor[y] = str[j];
            y++;
        }
    }
    return dastoor;
}
int main()
{
    char str[200];
    while (1)
    {
        fgets(str, 200, stdin);
        if (strstr(neogit config –global user.name, str) != NULL)
        {
        }
        else
        {
        }
        if (strstr(neogit config –global user.name, str) = NULL)
        {
            continue;
        }
        if (strstr(neogit config –global user.email, str) = NULL)
        {
        }
        else
        {
        }
        if (strstr(neogit config –global user.email, str) = NULL)
        {
            continue;
        }
    }
}