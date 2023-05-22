#ifndef FUNCTIONS__H
#define FUNCTIONS__H

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

void die(const char *s);

void disableRawMode();

void enableRawMode();

int ispart(char s[],char s1[]);

char *concat(char string[]);

char *concopy(char string[]);

char *takeusername();

char *takesystemname();

char * removebefore(char string[]);

char * removeafter(char string[]);

int countslash(char string[]);

void display(char username[],char systemname[]);

void backgroundcompleted(int signum);

void handlingcntrlc(int signum);

void handlingcntrlz(int signum);

void addtohistory(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void Command();

void COMMANDcd(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDecho(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDpwd(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void implementls(char dorf[],int a,int l);

void implementlsf(char dorf[],int a,int l,char string[]);

void COMMANDls(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

char * format(char string[1000]);

void COMMANDpinfo(int parts,int numberofparts[],char commandparts[][50][50],int whichpart,int previousnumberback);

void implementdiscover(int d,int f,/*char path[]*/DIR * dir,char find[],char path[]);

void COMMANDdiscover(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDhistory(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

int compare(const void * a, const void * b);

void COMMANDjobs(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDsig(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDfg(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

void COMMANDbg(int parts,int numberofparts[],char commandparts[][50][50],int whichpart);

#endif 