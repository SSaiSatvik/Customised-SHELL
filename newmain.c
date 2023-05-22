#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <termios.h>
#include "func.h"

extern char allcommands[10000][50][50];
extern int numberofcommandspassed;
extern int numberofparametersinallcommands[10000];

extern int timecalculated;

extern char presentdir[1000];
extern char previousdir[1000];

extern char negation[1000];
extern char home[1000];
extern char root[1000];

extern int background[1000];
extern int backpid[1000];
extern int backnumber;

extern int countinback;
extern char inputfile[1000];
extern char outputfile[1000];
extern int append;

extern int replaceinput;
extern int replaceoutput;

extern int mainpid;
extern int forepid;

int main()
{
    char username[50];
    char systemname[50];
    strcpy(username,takeusername());
    strcpy(systemname,takesystemname());

    getcwd(negation,1000);

    strcpy(presentdir,"~");

    strcpy(previousdir,negation);

    strcpy(home,"/home/satvik100");
    
    strcpy(root,"/");

    backnumber=0;
    countinback=0;

    numberofcommandspassed=0;

    FILE * history=fopen("history","a+");
    fclose(history);

    replaceinput = dup(STDIN_FILENO);
    replaceoutput = dup(STDOUT_FILENO);

    mainpid=getpid();

    while (1)
    {
        inputfile[0]='\0';
        outputfile[0]='\0';
        append=0;
        timecalculated=0;
        Command();
    }

    return 0;
}