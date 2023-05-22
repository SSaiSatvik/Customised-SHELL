#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
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
#include <string.h>

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
    printf("Prompt>");
    memset(inp, '\0', 100);
    int pt = 0;
    while (read(STDIN_FILENO, &c, 1) == 1) 
    {
        if (iscntrl(c)) 
        {
            if (c == 10) 
                break;
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
                
                    printf("Prompt>");
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

int main() 
{
    char input[1000];
    strcat(input,readinput());
    
    return 0;
}