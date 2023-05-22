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

char allcommands[10000][50][50];
int numberofcommandspassed;
int numberofparametersinallcommands[10000]={0};

int timecalculated;

char presentdir[1000];

char previousdir[1000];

char negation[1000];
char home[1000];
char root[1000];

int background[1000];
int backpid[1000]={0};
int backnumber;

int countinback;

char inputfile[1000];
char outputfile[1000];
int append;

int replaceinput;
int replaceoutput;

int mainpid;
int forepid;

void handlingcntrlc(int signum)
{
    if(forepid!=mainpid)
    {
        kill(forepid,SIGINT);
    } 
    signal(SIGINT,handlingcntrlc);
}

void handlingcntrlz(int signum)
{
    // printf("%d %d\n",forepid,mainpid);
    printf("\n");
    if(forepid!=mainpid)
    {
        kill(forepid,SIGINT);
    }

    int i=0;

    while(numberofparametersinallcommands[i]!=0)
    {
        i++;
    }
    i--;
    // printf("%d\n",i);
    background[i]=1;
    backpid[i]=forepid;

    countinback++;
    char temp[1000];
    temp[0]='\0';

    for(int j=0;j<=numberofparametersinallcommands[i];j++)
    {
        strcat(temp,allcommands[i][j]);
        strcat(temp," ");
    }

    printf("[%d] stopped %s\n\n",countinback,temp);
}



void die(const char *s) {
    perror(s);
    exit(1);
}

struct termios orig_termios;

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

/**
 * Enable row mode for the terminal
 * The ECHO feature causes each key you type to be printed to the terminal, so you can see what you’re typing.
 * Terminal attributes can be read into a termios struct by tcgetattr().
 * After modifying them, you can then apply them to the terminal using tcsetattr().
 * The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any input that hasn’t been read.
 * The c_lflag field is for “local flags”
 */
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
    atexit(disableRawMode);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
/**
 * stdout and stdin are buffered we disable buffering on that
 * After entering in raw mode we read characters one by one
 * Up arrow keys and down arrow keys are represented by 3 byte escape codes
 * starting with ascii number 27 i.e. ESC key
 * This way we interpret arrow keys
 * Tabs are usually handled by the term, but here we are simulating tabs for the sake of simplicity
 * Backspace move the cursor one control character to the left
 * @return
 */



char *readinput(void)
{
    char *inp = malloc(sizeof(char) * 100);
    char c;
    int space=0;

    setbuf(stdout, NULL);
    enableRawMode();
    display(takeusername(),takesystemname());
    memset(inp, '\0', 100);
    int pt = 0;
    while (read(STDIN_FILENO, &c, 1) == 1) 
    {
        if (iscntrl(c)) 
        {
            if (c == 10)
            { 
                inp[pt++]='\n';
                printf("\n");
                break;
            }
            else if (c == 27) 
            {
                char buf[3];
                buf[2] = 0;
                if (read(STDIN_FILENO, buf, 2) == 2) 
                { // length of escape code
                    printf("\rarrow key: %s", buf);
                }
            } 
            else if (c == 127) 
            { // backspace
                if (pt > 0) 
                {
                    if (inp[pt-1] == 9) 
                    {
                        for (int i = 0; i < 7; i++) 
                        {
                            printf("\b");
                        }
                    }
                    inp[--pt] = '\0';
                    printf("\b \b");
                }
            } 
            else if (c == 9) 
            { // TAB character
                // inp[pt++] = c;
                // TABS should be 8 spaces
                char list[1000][1000];
                int flagmultiple=0;

                DIR *dir;
                struct dirent *d;
                dir=opendir(".");

                while(dir!=NULL)
                {
                    d=readdir(dir);
                    if(d==NULL)
                        break;
                    if(d->d_name[0]=='.')
                        continue;
                    
                    struct stat st;
                    stat(d->d_name,&st);

                    struct passwd *person1=getpwuid(st.st_uid);
                    struct group *group1=getgrgid(st.st_gid);
                    
                    if(!strncmp(&inp[space],d->d_name,pt-space))
                    {
                        flagmultiple++;
                        strcpy(list[flagmultiple-1],d->d_name);
                    }
                }
                closedir(dir);

                if (flagmultiple==1)
                {
                    printf("%s ",&list[0][pt-space]);
                    strcat(inp,&list[0][pt-space]);
                    pt += strlen(&list[0][pt-space]);
                    inp[pt]=' ';
                    pt++;
                    space=pt;
                }
                else if(flagmultiple>1)
                {
                    printf("\n");
                    for(int i=0;i<flagmultiple;i++)
                    {
                        printf("%s\n",list[i]);
                    }
                
                    display(takeusername(),takesystemname());
                    int common=0;
                    for(common=0;common<strlen(list[0]);common++)
                    {
                        int f=0;
                        for (int j=0;j<flagmultiple;j++)
                        {
                            if(strlen(list[j])<common)
                            {
                                break;
                            }
                            if(list[j][common]!=list[0][common])
                            {
                                f=1;
                                break;
                            }
                        }
                        if(f==1)
                        {
                            break;
                        }
                    }
                    // printf("%d",common);
                    strncat(inp,&list[0][pt-space],common-(pt-space));
                    pt += common-(pt-space);
                    printf("%s",inp);
                }
                
            } 
            else if (c == 4) 
            {
                printf("Bye!!!\n");
                exit(0);
            }
            else if (c == 3) 
            {
                printf("%d\n", c);
                signal(SIGINT,handlingcntrlc);
            } 
            else 
            {
                printf("%d\n", c);
            }
        } 
        else 
        {
            inp[pt++] = c;
            printf("%c", c);
            if(inp[pt-1]==' ')
            {
                space=pt;
            }
        }

        FILE * f=fopen("bax.txt","a+");
        fprintf(f,"\n pt:%d \n",pt);
    }
    disableRawMode();

    return inp;
}



int ispart(char s[],char s1[])
{
    if(strlen(s)<=strlen(s1))
    {
        for(int i=0;i<strlen(s);i++)
        {
            if(s[i]!=s1[i])
            {
                return 0;
            }
        }
        return strlen(s);
    }
    else
    {
        return 0;
    }
}

char *concat(char string[])
{
    char *s=malloc(1000);
    s[0]='~';

    int i;
    for(i=0;i<strlen(string);i++)
    {
        s[i+1]=string[i];
    }

    s[strlen(string)+1]='\0';

    return s;
}

char *concopy(char string[])
{
    char *s=malloc(1000);

    for(int i=1;i<strlen(string)-1;i++)
    {
        s[i-1]=string[i];
    }

    s[strlen(string)-1]='\0';

    return s;
}

char *takeusername()
{
    char *name=malloc(50);
    
    struct passwd *p;
    uid_t u;
    u=geteuid();
    p=getpwuid(u);
    strcpy(name,p->pw_name);

    return name;
}

char *takesystemname()
{
    char *name=malloc(50);
    
    gethostname(name,50);

    return name;
}

int countslash(char string[])
{
    int count=0;

    for(int i=0;i<strlen(string);i++)
    {
        if(string[i]=='/')
            count++;
    }
    return count;
}

char * removebefore(char string[])
{
    char *s=malloc(1000);
    int count=0;

    for(int i=0;i<strlen(string);i++)
    {
        if(string[i]=='/')
            count++;
    }

    int countp=0;
   
    int p;
    for(p=0;p<strlen(string);p++)
    {
        if(count==countp)
        {
            break;
        }

        if(string[p]=='/')
        {
            countp++;
        }
    }
    int a=0;
    while(p<strlen(string))
    {
        s[a++]=string[p++];
    }
    s[a]='\0';

    return s;
}

char * removeafter(char string[])
{
    char *s=malloc(1000);
    int count=0;

    for(int i=0;i<strlen(string);i++)
    {
        if(string[i]=='/')
            count++;
    }

    int countp=0;
    int i;
    for(i=0;i<strlen(string);i++)
    {
        s[i]=string[i];
        if(string[i]=='/')
        {
            countp++;
            if(countp==count)
                break;
        }
    }
    s[i]='\0';

    return s;
}

void display(char username[],char systemname[])
{
    if(timecalculated>=1)
        printf("\r<%s@%s:%s,,,time taken is %d>\n",username,systemname,presentdir,timecalculated);
    else
        printf("\r<%s@%s:%s>\n",username,systemname,presentdir);
}

void backgroundcompleted(int signum)
{
    int status;
    int returnvalue=waitpid(-1,&status, WNOHANG);

    if(returnvalue <= 0) 
        return;

    if(status==0)
    {
        printf("\rThe child with pid %d has been terminated\n",returnvalue);

    }

    kill(returnvalue, SIGKILL);
}

void Command()
{
    size_t size=50;
    char *command=malloc(1000);

    int previousnumberofcommands=numberofcommandspassed;

    strcpy(command,readinput());
    // printf("com %s",command);
    // printf("  len  %d  \n\n",strlen(command));
    // if(command[strlen(command)-1]=='\n')
    //     printf("   0\n\n");


    // printf("intial back numebr %d",backnumber);

    signal(SIGINT,handlingcntrlc);
    signal(SIGTSTP,handlingcntrlz);
    
    int previousnumberback=backnumber;

    char commandparts[1000][50][50];
    int i=0;
    int parts=0;
    int numberinpart[1000]={0};
    int count=0;

    int pipeflag=0;

    forepid=mainpid;

    int outputfd;
    int inputfd;

    while((command[i]!='\n') && (command[i]!='<') && (command[i]!='>'))
    {
        if((command[i]==' ' || command[i]=='\t'))
        {
            if(count!=0)
            {
                commandparts[parts][numberinpart[parts]][count]='\0';
                numberinpart[parts]++;
                count=0;
                //printf("a");
            }
        }

        if((command[i]!=' ' && command[i]!='\t') && (command[i]!='&' && command[i]!=';' && command[i]!='|'))
        {
            commandparts[parts][numberinpart[parts]][count]=command[i];
            count++;
            //printf("b");
        }

        if(command[i]=='&' || command[i]==';' ||command[i]=='|')
        {   
            commandparts[parts][numberinpart[parts]][count]='\0';
            if(command[i]=='|')
            {
                pipeflag=1;
            }
            //if & 1
            if(command[i]=='&')
            {
                background[previousnumberback+parts]=1;
                //printf("%d",backnumber);
            }
            parts++;
            numberinpart[parts]=0;
            count=0;
            //printf("c");
        }
        i++;
    }

    int flag;
    int flagi=0;
    int flago=0;
    i=0;
    
    while(command[i]!='\n')
    {
        flag=0;

        if(command[i]=='<')
        {
            flagi=1;
            flag=1;
            i++;
            while(command[i]==' ' || command[i]=='\t')
            {
                i++;
            }

            int inputcount=0;
            while(command[i]!=' ' && command[i]!='\t' && command[i]!='\n')
            {
                inputfile[inputcount]=command[i];
                inputcount++;
                i++;
            }
            inputfile[inputcount]='\0';

            inputfd = open(inputfile, O_RDONLY);
            if(inputfd<0)
            {
                printf("No such input file");
                exit(1);
            }

            // dup2(STDIN_FILENO, replaceinput);
            dup2(inputfd, STDIN_FILENO);
            close(inputfd);
        }

        if(command[i]=='>')
        {
            flago=1;
            flag=1;
            i++;
            if(command[i]=='>')
            {
                append=1;
                i++;
            }

            while(command[i]==' ' || command[i]=='\t')
            {
                i++;
            }

            int outputcount=0;
            while(command[i]!=' ' && command[i]!='\t' && command[i]!='\n')
            {
                outputfile[outputcount]=command[i];
                outputcount++;
                i++;
            }
            outputfile[outputcount]='\0';

            if(append==0)
            {
                outputfd = open(outputfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                // dup2(STDOUT_FILENO, replaceoutput);
                dup2(outputfd, STDOUT_FILENO);
                close(outputfd);
            }
            else
            {
                outputfd = open(outputfile, O_WRONLY | O_CREAT | O_APPEND, 0644);

                // dup2(STDOUT_FILENO, replaceoutput);
                dup2(outputfd, STDOUT_FILENO);
                close(outputfd);
            }
        }
        if(flag==0)
        {
            i++;
        }
    }

    backnumber += parts+1;
    
    commandparts[parts][numberinpart[parts]][count]='\0';
    
    if(numberinpart[parts]==0)
    {
        if(commandparts[parts][0][0]=='\0')
        {
            parts--;
            backnumber--;
        }
    }

    int pipefd[2];
    int outputsafety;
    int inputsafety;

    if(pipeflag==1)
    {
        outputsafety=dup(STDOUT_FILENO);
        inputsafety=dup(STDIN_FILENO);
    }

    for(int p=0;p<=parts;p++)
    {
        if(commandparts[p][numberinpart[p]][0]=='\0')
        {
            numberinpart[p]--;
        }
    }

    //printf("%d\n\n",parts);
    // for(int w=previousnumberback;w<=previousnumberback+parts;w++)
    // {
    //     printf("%d  %d num\t",background[w],w);
    // }

    numberofcommandspassed += (parts+1);
    
    //parts numberofparts whichpart all are indexed with 0.
    for(int number=0;number<=parts;number++)
    {
        addtohistory(parts,numberinpart,commandparts,number);

        if(pipeflag==1)
        {
            if(number>=0 && number<parts)
            {
                pipe(pipefd);
                dup2(pipefd[1],STDOUT_FILENO);
            }
            else
            {
                dup2(outputsafety, STDOUT_FILENO);
            }
        }

        // for (int i = 0; i <= numberinpart[number]; i++)
        // {
        //     printf("%s\n",commandparts[number][i]);
        // }
        // printf("%d\n",numberinpart[number]);
        numberofparametersinallcommands[number+previousnumberofcommands]=numberinpart[number];
        for (int i = 0; i <= numberinpart[number]; i++)
        {
            strcpy(allcommands[number+previousnumberofcommands][i],commandparts[number][i]);
        }

        if(background[number+previousnumberback]==0)
        {   
            struct timespec start,end;
            clock_gettime(CLOCK_REALTIME,&start);

            if(!strcmp(commandparts[number][0],"cd"))
            {
                //printf("1\n");
                COMMANDcd(parts,numberinpart,commandparts,number);
            }
            else if(!strcmp(commandparts[number][0],"echo"))
            {
                //printf("2\n");
                COMMANDecho(parts,numberinpart,commandparts,number);
            }
            else if(!strcmp(commandparts[number][0],"pwd"))
            {
                //printf("3\n");
                COMMANDpwd(parts,numberinpart,commandparts,number);
            }
            else if(!strcmp(commandparts[number][0],"ls"))
            {
                //printf("4\n");
                COMMANDls(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"pinfo"))
            {
                //printf("5\n");
                COMMANDpinfo(parts,numberinpart,commandparts,number,previousnumberback);
            }
            else if (!strcmp(commandparts[number][0],"discover"))
            {
                //printf("6\n");
                COMMANDdiscover(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"history"))
            {
                //printf("7\n");
                COMMANDhistory(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"jobs"))
            {
                //printf("7\n");
                COMMANDjobs(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"sig"))
            {
                //printf("7\n");
                COMMANDsig(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"fg"))
            {
                //printf("7\n");
                COMMANDfg(parts,numberinpart,commandparts,number);
            }
            else if (!strcmp(commandparts[number][0],"bg"))
            {
                //printf("7\n");
                COMMANDbg(parts,numberinpart,commandparts,number);
            }
            else
            {
                // printf("8\n");
                char *argument_listf[numberinpart[number]+2];
                for(int i=0;i<numberinpart[number]+2;i++)
                {
                    argument_listf[i]=malloc(1000);
                }
                int j=0,k;

                for(j=0;j<=numberinpart[number];j++)
                {
                    for(k=0;k<strlen(commandparts[number][j]);k++)
                    {
                        argument_listf[j][k]=commandparts[number][j][k];
                    }
                    argument_listf[j][k]='\0';
                    // printf("%s\n",argument_listf[j]);
                }
                argument_listf[numberinpart[number]+1]=NULL;

                // for(int i=0;i<j;i++)
                // {
                //     printf("%s",argument_listf[i]);
                // }
                // if(argument_listf[numberinpart[number]+1]==NULL)
                // {
                //     printf("f\n");
                // }

                int pid = fork();
                backpid[previousnumberback+number]=pid;
                forepid=pid;
                if(pid==0)
                {
                    
                    // printf("%d\n",forepid);
                    execvp(argument_listf[0],argument_listf);
                    
                    // printf("ishbu");
                    //exit(0);
                }
                else
                {
                    int status;
                    
                    waitpid(pid, &status, WUNTRACED);
                    forepid=mainpid;
                    // printf("%d\n",forepid);
                    // printf("n %d\n",numberofparametersinallcommands[previousnumberback+number]);
                    // printf("n %d\n",numberofparametersinallcommands[previousnumberback+number+1]);
                    // printf("%d",status);
                    // if(WIFEXITED(status))
                        // printf("done");
                    
                }
            }
            clock_gettime(CLOCK_REALTIME,&end);
            timecalculated=end.tv_sec-start.tv_sec;
            // printf("time %d",timecalculated);
        }
        else if(background[number+previousnumberback]==1)
        {
            // printf("9\n");
            char *argument_listb[numberinpart[number]+2];
            for(int i=0;i<numberinpart[number]+2;i++)
            {
                argument_listb[i]=malloc(1000);
            }
            int j=0,k;

            for(j=0;j<=numberinpart[number];j++)
            {
                for(k=0;k<strlen(commandparts[number][j]);k++)
                {
                    argument_listb[j][k]=commandparts[number][j][k];
                }
                argument_listb[j][k]='\0';
                //printf("%s\n",argument_listb[j]);
            }
            argument_listb[numberinpart[number]+1]=NULL;

            // for(int i=0;i<j;i++)
            // {
            //     printf("%s\n",argument_listb[i]);
            // }
            // if(argument_listb[numberinpart[number]+1]==NULL)
            // {
            //     printf("f\n");
            // }

            signal(SIGCHLD, backgroundcompleted);
            int pid=fork();
            backpid[previousnumberback+number]=pid;
            countinback++;
            if(pid==0)
            {
                backpid[previousnumberback+number]=getpid();
                // printf("The backpid[%d] is %d\n",previousnumberback+number,backpid[previousnumberback+number]);
                printf("\r[%d] - %-100d\n",countinback,backpid[previousnumberback+number]);
        
                execvp(argument_listb[0],argument_listb);
            }
        
        }
    
        if(pipeflag==1)
        {
            if(number>=0 && number<parts)
            {
                close(pipefd[1]);
                dup2(pipefd[0], STDIN_FILENO);
            }
            else
            {
                dup2(inputsafety,STDIN_FILENO);
            }
        }
    }
    if(flagi==1)
    {
        // printf("ishbu\n\n");
        dup2(replaceinput, STDIN_FILENO);
        // printf("ishbu\n\n");
        // fflush(stdout);
    }
    if(flago==1)
    {
        // printf("ishbu\n\n");
        dup2(replaceoutput, STDOUT_FILENO);
        // printf("ishbu\n\n");
        // fflush(stdout);
    }
}

void COMMANDcd(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    char present[1000];

    if(numberofparts[whichpart]==0)
    {
        getcwd(present,1000);
        strcpy(previousdir,present);
        chdir(home);
        strcpy(presentdir,home);
    }

    if(numberofparts[whichpart]==1)
    {
        if(!strcmp(commandparts[whichpart][1],"."))
        {
            printf("\n");
        }
        else if (!strcmp(commandparts[whichpart][1],".."))
        {
            getcwd(present,1000);
            strcpy(previousdir,present);
            chdir(commandparts[whichpart][1]);
            getcwd(present,1000);
            if(!ispart(negation,present))
            {
                strcpy(presentdir,present);
            }
            else
            {
                strcpy(presentdir,concat(&present[ispart(negation,present)]));
            }
        }
        else if (!strcmp(commandparts[whichpart][1],"~"))
        {   
            getcwd(present,1000);
            strcpy(previousdir,present);
            chdir(home);
            strcpy(presentdir,home);
        }
        else if (!strcmp(commandparts[whichpart][1],"-"))
        {
            char temp[1000];
            strcpy(temp,previousdir);
            getcwd(present,1000);
            strcpy(previousdir,present);
            strcpy(presentdir,temp);
            chdir(presentdir);

            getcwd(present,1000);

            if(!ispart(negation,present))
            {
                strcpy(presentdir,present);
            }
            else
            {
                strcpy(presentdir,concat(&present[ispart(negation,present)]));
            }

            printf("%s\n",presentdir);
        }
        else if (!strcmp(commandparts[whichpart][1],"/"))
        {
            getcwd(present,1000);
            strcpy(previousdir,present);
            chdir(root);
            strcpy(presentdir,root);
        }
        else if (commandparts[whichpart][1][0]=='"' && commandparts[whichpart][1][strlen(commandparts[whichpart][1])-1]=='"')
        {
            getcwd(present,1000);
            strcpy(previousdir,present);  
            
            char *string=concopy(commandparts[whichpart][1]);
            chdir(string);
            getcwd(present,1000);
            if(!ispart(negation,present))
            {
                strcpy(presentdir,present);
            }
            else
            {
                strcpy(presentdir,concat(&present[ispart(negation,present)]));
            }
        }
        else
        {
            getcwd(present,1000);
            strcpy(previousdir,present);
            chdir(commandparts[whichpart][1]);
            getcwd(present,1000);
            if(!ispart(negation,present))
            {
                strcpy(presentdir,present);
            }
            else
            {
                // char *s=malloc(1000);
                // s=&present[ispart(negation,present)];
                //sprintf(presentdir,"~%s",s);
                //strncat(presentdir,&present[ispart(negation,present)],1);
                strcpy(presentdir,concat(&present[ispart(negation,present)]));
            }
        }
    }  
}

void COMMANDecho(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    for(int i=1;i<=numberofparts[whichpart];i++)
    {
        printf("%s ",commandparts[whichpart][i]);
    }
    printf("\n");
}

void COMMANDpwd(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    if(numberofparts[parts]==0)
    {
        char present[1000];
        getcwd(present,1000);
        printf("%s\n",present);
    }
}

// void implementls(char dorf[],int a,int l)
// {
//     DIR *dir;
//     struct dirent *d;
//     dir=opendir(dorf);

//     while(dir!=NULL)
//     {
//         d=readdir(dir);
//         if(d==NULL)
//             break;
//         if(a==0)
//             if(d->d_name[0]=='.')
//                 continue;
        
//         struct stat st;
//         stat(d->d_name,&st);

//         struct passwd *person1=getpwuid(st.st_uid);
//         struct group *group1=getgrgid(st.st_gid);
        
//         if(l==0)
//         {
//             if(S_ISDIR(st.st_mode))
//                 printf("\e[36m""%s\n""\e[m",d->d_name);
//             else if((st.st_mode & S_IXUSR)==S_IXUSR)
//                 printf("\e[32m""%s\n""\e[m",d->d_name);
//             else 
//                 printf("\e[m""%s\n""\e[m",d->d_name);
//         }
//         else if(l==1)
//         {
//             if(S_ISDIR(st.st_mode))
//                 printf("d");
//             else
//                 printf("-");

//             if((st.st_mode & S_IRUSR)==S_IRUSR)
//                 printf("r");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IWUSR)==S_IWUSR)
//                 printf("w");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IXUSR)==S_IXUSR)
//                 printf("x");
//             else    
//                 printf("-");
//             printf("-");

//             if((st.st_mode & S_IRGRP)==S_IRGRP)
//                 printf("r");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IWGRP)==S_IWGRP)
//                 printf("w");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IXGRP)==S_IXGRP)
//                 printf("x");
//             else    
//                 printf("-");
//             printf("-");

//             if((st.st_mode & S_IROTH)==S_IROTH)
//                 printf("r");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IWOTH)==S_IWOTH)
//                 printf("w");
//             else    
//                 printf("-");
//             if((st.st_mode & S_IXOTH)==S_IXOTH)
//                 printf("x");
//             else    
//                 printf("-");
            
//             printf("\t");
//             printf("%d",st.st_nlink); 
//             printf("\t");
//             printf("%s",person1->pw_name);
//             printf("\t");
//             printf("%s",group1->gr_name);
//             printf("\t");
//             printf("%d",st.st_size);
//             printf("\t");
//             printf("%s",ctime(&st.st_mtime));
//             printf("\t");

//             if(S_ISDIR(st.st_mode))
//                 printf("\e[36m""%s\n""\e[m",d->d_name);
//             else if((st.st_mode & S_IXUSR)==S_IXUSR)
//                 printf("\e[32m""%s\n""\e[m",d->d_name);
//             else
//                 printf("\e[m""%s\n""\e[m",d->d_name);
//         }
//     }
//     closedir(dir);
// }

void implementls(char dorf[],int a,int l)
{
    struct dirent **subdirectories;
    chdir(dorf);
    int fd;
    char present[1000];
    getcwd(present,1000);
    fd = open(present, O_DIRECTORY);
    int n=scandirat(fd,".",&subdirectories,NULL,alphasort);

    for(int i=0;i<n;i++) 
    {
        struct stat st;
        stat(subdirectories[i]->d_name,&st);

        struct passwd *person1=getpwuid(st.st_uid);
        struct group *group1=getgrgid(st.st_gid);

        if(l==0)
        {
            if(a && subdirectories[i]->d_name[0]=='.')
            {
                if(S_ISDIR(st.st_mode))
                    printf("\e[36m""%s\n""\e[m",subdirectories[i]->d_name);
                else if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",subdirectories[i]->d_name);
                else
                    printf("\e[m""%s\n""\e[m",subdirectories[i]->d_name);
            }
            if(subdirectories[i]->d_name[0]!='.')
            {
                if(S_ISDIR(st.st_mode))
                    printf("\e[36m""%s\n""\e[m",subdirectories[i]->d_name);
                else if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",subdirectories[i]->d_name);
                else
                    printf("\e[m""%s\n""\e[m",subdirectories[i]->d_name);
            }
        }
        else
        {
            if(a && subdirectories[i]->d_name[0]=='.')
            {
                if(S_ISDIR(st.st_mode))
                    printf("d");
                else
                    printf("-");

                if((st.st_mode & S_IRUSR)==S_IRUSR)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWUSR)==S_IWUSR)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IRGRP)==S_IRGRP)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWGRP)==S_IWGRP)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXGRP)==S_IXGRP)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IROTH)==S_IROTH)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWOTH)==S_IWOTH)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXOTH)==S_IXOTH)
                    printf("x");
                else    
                    printf("-");
                
                printf("\t");
                printf("%d",st.st_nlink); 
                printf("\t");
                printf("%s",person1->pw_name);
                printf("\t");
                printf("%s",group1->gr_name);
                printf("\t");
                printf("%d",st.st_size);
                printf("\t");
                printf("%s",ctime(&st.st_mtime));
                printf("\t");

                if(S_ISDIR(st.st_mode))
                    printf("\e[36m""%s\n""\e[m",subdirectories[i]->d_name);
                else if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",subdirectories[i]->d_name);
                else
                    printf("\e[m""%s\n""\e[m",subdirectories[i]->d_name);
            }
            if(subdirectories[i]->d_name[0]!='.')
            {
                if(S_ISDIR(st.st_mode))
                    printf("d");
                else
                    printf("-");

                if((st.st_mode & S_IRUSR)==S_IRUSR)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWUSR)==S_IWUSR)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IRGRP)==S_IRGRP)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWGRP)==S_IWGRP)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXGRP)==S_IXGRP)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IROTH)==S_IROTH)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWOTH)==S_IWOTH)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXOTH)==S_IXOTH)
                    printf("x");
                else    
                    printf("-");
                
                printf("\t");
                printf("%d",st.st_nlink); 
                printf("\t");
                printf("%s",person1->pw_name);
                printf("\t");
                printf("%s",group1->gr_name);
                printf("\t");
                printf("%d",st.st_size);
                printf("\t");
                printf("%s",ctime(&st.st_mtime));
                printf("\t");

                if(S_ISDIR(st.st_mode))
                    printf("\e[36m""%s\n""\e[m",subdirectories[i]->d_name);
                else if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",subdirectories[i]->d_name);
                else
                    printf("\e[m""%s\n""\e[m",subdirectories[i]->d_name);
            }
        }
    }
}

void implementlsf(char dorf[],int a,int l,char string[])
{
    DIR *dir;
    struct dirent *d;
    dir=opendir(dorf);

    while(dir!=NULL)
    {
        d=readdir(dir);
        if(d==NULL)
            break;
        if(a==0)
            if(d->d_name[0]=='.')
                continue;
        
        struct stat st;
        stat(d->d_name,&st);

        struct passwd *person1=getpwuid(st.st_uid);
        struct group *group1=getgrgid(st.st_gid);

        if(!strcmp(string,d->d_name))
        {
            if(l==0)
            {
                if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",d->d_name);
                else 
                    printf("\e[m""%s\n""\e[m",d->d_name);
            }
            else if(l==1)
            {
                if(S_ISDIR(st.st_mode))
                    printf("d");
                else
                    printf("-");

                if((st.st_mode & S_IRUSR)==S_IRUSR)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWUSR)==S_IWUSR)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IRGRP)==S_IRGRP)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWGRP)==S_IWGRP)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXGRP)==S_IXGRP)
                    printf("x");
                else    
                    printf("-");
                printf("-");

                if((st.st_mode & S_IROTH)==S_IROTH)
                    printf("r");
                else    
                    printf("-");
                if((st.st_mode & S_IWOTH)==S_IWOTH)
                    printf("w");
                else    
                    printf("-");
                if((st.st_mode & S_IXOTH)==S_IXOTH)
                    printf("x");
                else    
                    printf("-");
                
                printf("\t");
                printf("%d",st.st_nlink); 
                printf("\t");
                printf("%s",person1->pw_name);
                printf("\t");
                printf("%s",group1->gr_name);
                printf("\t");
                printf("%d",st.st_size);
                printf("\t");
                printf("%s",ctime(&st.st_mtime));
                printf("\t");

                if((st.st_mode & S_IXUSR)==S_IXUSR)
                    printf("\e[32m""%s\n""\e[m",d->d_name);
                else
                    printf("\e[m""%s\n""\e[m",d->d_name);
            }
        }
    }
    closedir(dir);
}

void COMMANDls(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int i=0;
    int a=0;
    int l=0;
    int number=0;

    while (i<=numberofparts[whichpart])
    {
        if(commandparts[whichpart][i][0]=='-')
        {
            for(int j=1;j<strlen(commandparts[whichpart][i]);j++)
            {
                if(commandparts[whichpart][i][j]=='a')
                    a=1;
                else if (commandparts[whichpart][i][j]=='l')
                    l=1;
                else
                    printf("INVALID COMMAND");
            }
            number++;
        }
        i++;
    }

    int flag=0;
    i=0;

    char presentp[1000];
    char present[1000];
    getcwd(present,1000);

    while (i<=numberofparts[whichpart])
    {
        if(commandparts[whichpart][i][0]!='-' && strcmp(commandparts[whichpart][i],"ls"))
        {
            if(numberofparts[whichpart]-number-1>=2)
            {
                printf("%s:\n",commandparts[whichpart][i]);
            }
            if(strcmp(commandparts[whichpart][i],".") && commandparts[whichpart][i][0]!='~')
            {
                flag=1;
                DIR *dir;
                struct dirent *d;
                dir=opendir(commandparts[whichpart][i]);

                if(dir==NULL)
                {
                    if(errno==ENOENT)
                        printf("No such file or directory\n");
                    if(errno==ENOTDIR)
                    {   
                        if(countslash(commandparts[whichpart][i])==0)
                        {
                            getcwd(presentp,1000);
                            implementlsf(presentp,a,l,commandparts[whichpart][i]);
                            chdir(present);
                        }
                        else
                        {
                            chdir(removeafter(commandparts[whichpart][i]));
                            getcwd(presentp,1000);
                            implementlsf(presentp,a,l,removebefore(commandparts[whichpart][i]));
                            chdir(present);
                        }
                    }        
                }
                else
                {
                    chdir(commandparts[whichpart][i]);
                    getcwd(presentp,1000);
                    implementls(presentp,a,l);
                    chdir(present);
                }
            }
            else if (!strcmp(commandparts[whichpart][i],"."))
            {
                flag=1;
                getcwd(presentp,1000);
                implementls(presentp,a,l);
                chdir(present);
            }
            else
            {
                flag=1;
                chdir(negation);
                if(!(strcmp(commandparts[whichpart][i],"~")))
                {
                    getcwd(presentp,1000);
                    implementls(presentp,a,l);
                    chdir(present);
                }
                else
                {
                    chdir(&commandparts[whichpart][i][2]);
                    getcwd(presentp,1000);
                    implementls(presentp,a,l);
                    chdir(present);
                }
            }
        }
        i++;
    }

    if(flag==0)
    {
        char present[1000];
        implementls(getcwd(present,1000),a,l);
    }
}

char * format(char string[1000])
{
    int i=0;
    int point=0;
    while(1)
    {
        if(string[i]=='\t')
        {
            point=i;
            break;
        }
        i++;
    }

    char *s=malloc(1000);
    i=0;

    for(int j=0;point+j<1000;j++)
    {
        s[j]=string[point+1+j];
    }

    return s;
}

void COMMANDpinfo(int parts,int numberofparts[],char commandparts[][50][50],int whichpart,int previousnumberback)
{
    int pid=0;
    if(numberofparts[whichpart]==0)
    {
        pid=getpid();
        printf("pid : %d\n",pid);
    
        char path[1000];
        sprintf(path,"/proc/%d/status",pid);
        FILE *f=fopen(path,"r");

        char buffer[1000];
        
        fgets(buffer,1000,f);
        printf("executable path : %s",format(buffer));

        fgets(buffer,1000,f);
        fgets(buffer,1000,f);
        printf("process state :  %s+",format(buffer));

        int i=0;

        while (i++<14)
        {
            fgets(buffer,1000,f);
        }
        
        printf("memory : %s",format(buffer));
    }
    else if (numberofparts[whichpart]==1)
    {
        pid=atoi(commandparts[whichpart][1]);

        printf("pid : %d\n",pid);
    
        char path[1000];
        sprintf(path,"/proc/%d/status",pid);
        FILE *f=fopen(path,"r");

        char buffer[1000];
        
        fgets(buffer,1000,f);
        printf("executable path : %s",format(buffer));

        fgets(buffer,1000,f);
        fgets(buffer,1000,f);

        if(background[whichpart+previousnumberback]==0)
        {
            printf("process state :  %s",format(buffer));
        }
        else
        {
            printf("process state :  %s+",format(buffer));
        }

        int i=0;

        while (i++<14)
        {
            fgets(buffer,1000,f);
        }
        
        printf("memory : %s",format(buffer));
    }
}

void COMMANDdiscover(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int d=1;
    int f=1;
    int i=0;

    char find[1000];
    find[0]='\0';
    char path[1000];
    path[0]='\0';
    int j;

    while (i<=numberofparts[whichpart])
    {
        if(commandparts[whichpart][i][0]=='"')
        {
            for(j=1;j<strlen(commandparts[whichpart][i]);j++)
            {
                find[j-1]=commandparts[whichpart][i][j];
            }
            find[j-2]='\0';
        }
        if(commandparts[whichpart][i][0]=='-')
        {
            for(int j=1;j<strlen(commandparts[whichpart][i]);j++)
            {
                if(commandparts[whichpart][i][j]=='d')
                    d=0;
                else if (commandparts[whichpart][i][j]=='f')
                    f=0;
                else
                    printf("INVALID COMMAND");
            }
        }
        if(commandparts[whichpart][i][0]!='"' && strcmp(commandparts[whichpart][i],"discover") && commandparts[whichpart][i][0]!='-')
        {
            strcpy(path,commandparts[whichpart][i]);
        }
        i++;
    }

    char present[1000];
    getcwd(present,1000);

    //printf("%d %d\n",d,f);
    
    if(path==NULL)
    {
        DIR *dir;
        dir=opendir(present);
        if(!strcmp(find,"")&&!(f==0 &&d==1))
        {
            printf("%s\n",present);
        }
        implementdiscover(d,f,/*presentp*/dir,find,present);
    }
    else
    {
        //printf("%s\n\n\n",path);
        chdir(path);
        char presentp[1000];
        getcwd(presentp,1000);
        DIR* dir;
        dir=opendir(presentp);
        if(!strcmp(find,"")&&!(f==0 &&d==1))
        {
            printf("%s\n",presentp);
        }
        implementdiscover(d,f,/*presentp*/dir,find,presentp);
    }
    
    chdir(present);

    return;
}

void implementdiscover(int d,int f,/*char path[]*/DIR *parent,char find[],char path[])
{
    if(strcmp(find,""))//if find is there 
    {
        struct dirent *ent;
        DIR *child;
        int fd;

        while ((ent = readdir(parent)) != NULL) {
            if (ent->d_name[0]=='.') 
            {
                continue;
            }
            if (ent->d_type == DT_DIR) 
            {
                char present[1000];
                sprintf(present,"%s/%s",path,ent->d_name);
                if(!(d==1 && f==0))
                {
                    if(!strcmp(find,ent->d_name))
                    {
                        printf("%s\n",present);
                    }
                }
                fd = openat(dirfd(parent), ent->d_name, O_DIRECTORY);
                if (fd != -1) {
                    child = fdopendir(fd);
                    implementdiscover(d,f,child,find,present);
                    closedir(child);
                } else {
                    perror("open");
                }
            } 
            else
            {
                char present[1000];
                sprintf(present,"%s/%s",path,ent->d_name);
                if(!(f==1 && d==0))
                {
                    if(!strcmp(find,ent->d_name))
                    {
                        printf("%s\n",present);
                    }
                }
            }
        }
    }
    else//if no find
    {
        struct dirent *ent;
        DIR *child;
        int fd;

        while ((ent = readdir(parent)) != NULL) {
            if (ent->d_name[0]=='.') 
            {
                continue;
            }
            if (ent->d_type == DT_DIR) 
            {
                char present[1000];
                sprintf(present,"%s/%s",path,ent->d_name);
                if(!(d==1 && f==0))
                    printf("%s\n",present);
                fd = openat(dirfd(parent), ent->d_name,O_DIRECTORY);
                if (fd != -1) 
                {
                    child = fdopendir(fd);
                    implementdiscover(d,f,child,find,present);
                    closedir(child);
                } 
                else 
                {
                    perror("open");
                }
            } 
            else
            {
                char present[1000];
                sprintf(present,"%s/%s",path,ent->d_name);
                if(!(f==1 && d==0))
                    printf("%s\n",present);
            }
        }
    }
}

void COMMANDhistory(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    char path[1000];
    sprintf(path,"%s/history",negation);
    FILE *history=fopen(path,"r");

    char *buffer=malloc(1000);
    size_t size=1000;
    int count=0;

    while((getline(&buffer,&size,history))!=-1)
    {
        count++;
    }

    fclose(history);

    FILE *history1=fopen(path,"r");

    if(count<=10)
    {
        while(count--)
        {
            getline(&buffer,&size,history1);
            printf("%s",buffer);
        }
    }
    else
    {
        int val=count-10;
        int i=0;
        while(i<val)
        {
            getline(&buffer,&size,history1);
            i++;
        }
        while((getline(&buffer,&size,history1))!=-1)
        {
            printf("%s",buffer);
        }
    }

    fclose(history1);
}

void addtohistory(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int i=0;
    int c=0;
    char command[1000];
    
    while(i<=numberofparts[whichpart])
    {
        for(int j=0;j<strlen(commandparts[whichpart][i]);j++)
        {
            command[c++]=commandparts[whichpart][i][j];
        }
        if(i<numberofparts[whichpart])
            command[c++]=' ';
        i++;
    }
    command[c]='\0';

    char path[1000];
    sprintf(path,"%s/history",negation);
    FILE *history=fopen(path,"a+");

    char *buffer[20];
    for(int j=0;j<20;j++)
    {
        buffer[j]=malloc(1000);
    }
    size_t size=1000;
    int count=0;

    while((getline(&buffer[count],&size,history)!=-1) && strcmp(buffer[count],"\n"))
    {
        count++;
    }
    
    if(count==0 || (count!=0 && (strlen(command)!=strlen(buffer[count-1]-1) && strncmp(buffer[count-1],command,c))))
    {
        if(count<20)
        {
            fprintf(history,"%s\n",command);
            // printf("%s\n",command);
            fclose(history);
        }
        else
        {
            char commandsnew[20][1000];
            for(int j=0;j<19;j++)
            {
                buffer[j+1][strlen(buffer[j+1])-1]='\0';
                strcpy(commandsnew[j],buffer[j+1]);
                commandsnew[j][strlen(buffer[j+1])]='\0';
            }
            strcpy(commandsnew[19],command);
            commandsnew[19][strlen(command)]='\0';

            // for(int j=0;j<19;j++)
            // {
            //     printf("%s %s\n",commandsnew[j],buffer[j+1]);
            // }
            remove(path);
            FILE *history1=fopen(path,"w");
            for(int j=0;j<20;j++)
            {
                fprintf(history1,"%s\n",commandsnew[j]);
            }
            fclose(history1);
        }
    }
}

char * onlybraketpart(char buffer[])
{
    char *s=malloc(50);
    int a=0;
    int i=0;

    while(buffer[i]!='(')
    {
        i++;
    }
    i++;
    while(buffer[i]!=')')
    {
        s[a]=buffer[i];
        i++;
        a++;
    }

    s[a]='\0';

    return s;
}

typedef struct
{
    char name[100];
    int number;
    char state[50];
    int pid;
}forjob;

forjob getdatawithpid(int pid)
{
    forjob s;
    s.pid=pid;

    char path[1000];
    sprintf(path,"/proc/%d/status",pid);
    FILE *f=fopen(path,"r");
    if(f==NULL)
    {
        strcpy(s.state,"stopped");
    }

    else
    {
        char buffer[1000];

        fgets(buffer,1000,f);
        fgets(buffer,1000,f);
        fgets(buffer,1000,f);
        strcpy(s.state,onlybraketpart(buffer));
    }
    
    return s;
}

int compare(const void * a, const void * b)
{
    forjob *forjobA = (forjob *)a;
    forjob *forjobB = (forjob *)b;

    return strcmp( forjobA->name , forjobB->name );
}

void COMMANDjobs(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int list[1000];
    int listnumber[1000];
    int a=0;
    for(int i=0;i<backnumber;i++)
    {
        if(background[i]==1)
        {
            list[a]=backpid[i];
            listnumber[a]=i;
            a++;
        }
    }

    forjob job[a];
    // printf("%d  a\n",a);
    // for(int i=0;i<a;i++)
    // {
    //     printf("%d  list\n",listnumber[i]);
    // }

    for(int i=0;i<a;i++)
    {
        char temp[1000];
        job[i]=getdatawithpid(list[i]);
        job[i].number=i+1;
        strcpy(temp,"");
        for(int j=0;j<=numberofparametersinallcommands[listnumber[i]];j++)
        {
            // printf("%s\n",allcommands[listnumber[i]][j]);
            sprintf(temp,"%s  %s",temp,allcommands[listnumber[i]][j]);
        }
        
        strcpy(job[i].name,temp);
        job[i].name[strlen(temp)]='\0';
    }

    // for(int i=0;i<a;i++)
    // {
    //     printf("number: %d      state: %s        pid: %d",job[i].number,job[i].state,job[i].pid);
    //     printf("command: %s\n",job[i].name);
    // }

    qsort(job,a,sizeof(job[0]),compare);

    if(numberofparts[whichpart]==0)
    {
        for(int i=0;i<a;i++)
        {
            printf("[%d] %s\t%s\t-- %d\n",job[i].number,job[i].state,job[i].name,job[i].pid);
        }
    }

    else if(numberofparts[whichpart]==1)
    {
        for(int i=0;i<a;i++)
        {
            if(!strcmp(commandparts[whichpart][1],"-r"))
            {
                if(!strcmp(job[i].state,"running"))
                {
                    printf("[%d] %s\t%s\t-- %d\n",job[i].number,job[i].state,job[i].name,job[i].pid);
                }
            }
            else if (!strcmp(commandparts[whichpart][1],"-s"))
            {
                if(!strcmp(job[i].state,"stopped"))
                {
                    printf("[%d] %s\t%s\t-- %d\n",job[i].number,job[i].state,job[i].name,job[i].pid);
                }
            }
        }
    }
}

void COMMANDsig(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int parameter1=atoi(commandparts[whichpart][1]);
    int parameter2=atoi(commandparts[whichpart][2]);

    int w=0;
    int current=-1;
    for(int i=0;i<backnumber;i++)
    {
        if(background[i]==1)
        {
            w++;
            if(w==parameter1)
            {
                current=i;
                break;
            }
        }
    }

    if(current==-1)
    {
        printf("No such index\n");
        return;
    }
    else
    {
        kill(backpid[current],parameter2);
    }
}

void COMMANDfg(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int parameter1=atoi(commandparts[whichpart][1]);
    int w=0;
    
    for(int i=0;i<backnumber;i++)
    {
        if(background[i]==1)
        {
            w++;
            if(w==parameter1)
            {
                background[i]=0;

                int givenpid=backpid[i];
                backpid[i]=0;
                
                int status;

                struct timespec start,end;
                clock_gettime(CLOCK_REALTIME,&start);
                
                kill(givenpid,SIGCONT);
                waitpid(givenpid,&status,0);

                clock_gettime(CLOCK_REALTIME,&end);
                timecalculated=end.tv_sec-start.tv_sec;
            }
        }
    }
}

void COMMANDbg(int parts,int numberofparts[],char commandparts[][50][50],int whichpart)
{
    int parameter1=atoi(commandparts[whichpart][1]);

    int w=0;
    int current=-1;
    for(int i=0;i<backnumber;i++)
    {
        if(background[i]==1)
        {
            w++;
            if(w==parameter1)
            {
                current=i;
                printf("%d",current);
                break;
            }
        }
    }

    if(current<0)
    {
        printf("No such index\n");
        return;
    }
    printf("%d",current);
    kill(backpid[current],SIGCONT);
    return;
}