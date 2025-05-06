#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#define MAX_WORDS 100
#define MAX_NAME 100
#define MAX_ATTEMPTS 5
#define LEADERBOARD_FILE "leaderboard.txt"
#define WORDS_FILE "words.txt"

#define C_RESET  "\033[0m"
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_CYAN   "\033[36m"

typedef struct { char name[MAX_NAME]; int score; } Player;
typedef struct { Player players[MAX_WORDS]; int player_count; } Leaderboard;
typedef struct { char word[MAX_NAME]; } WordList;

void clear_screen(void) { printf("\033[H\033[J"); }
void pause_enter(void) { printf("Press Enter to continue."); while (getchar()!='\n'); }

int read_line(char *dest,int max){
    if(!fgets(dest,max,stdin))return 0;
    size_t len=strlen(dest);
    if(len&&dest[len-1]=='\n')dest[len-1]='\0';
    return 1;
}

int read_menu_choice(void){
    char buf[16]; int choice=-1;
    while(1){
        printf("%sMenu choice%s (1-4): ",C_YELLOW,C_RESET);
        if(!read_line(buf,16))continue;
        char *end; long val=strtol(buf,&end,10);
        if(*end=='\0'&&val>=1&&val<=4){choice=(int)val;break;}
        printf("%sInvalid option. Choose 1-4.%s\n",C_RED,C_RESET);
    }
    return choice;
}

void load_words(WordList *words,int *word_count){
    FILE *file=fopen(WORDS_FILE,"r");
    if(!file){*word_count=0;return;}
    *word_count=0;
    while(*word_count<MAX_WORDS&&fscanf(file,"%99s",words[*word_count].word)==1)(*word_count)++;
    fclose(file);
}

void save_leaderboard(Leaderboard *lb){
    FILE *file=fopen(LEADERBOARD_FILE,"w"); if(!file)return;
    for(int i=0;i<lb->player_count;i++)fprintf(file,"%s %d\n",lb->players[i].name,lb->players[i].score);
    fclose(file);
}

void load_leaderboard(Leaderboard *lb){
    FILE *file=fopen(LEADERBOARD_FILE,"r"); lb->player_count=0; if(!file)return;
    while(lb->player_count<MAX_WORDS&&fscanf(file,"%99s %d",lb->players[lb->player_count].name,&lb->players[lb->player_count].score)==2)lb->player_count++;
    fclose(file);
}

void display_leaderboard(Leaderboard *lb){
    clear_screen();
    printf("%sLeaderboard%s\n",C_CYAN,C_RESET);
    for(int i=0;i<lb->player_count;i++)
        printf("%s%s%s : %s%d%s\n",C_GREEN,lb->players[i].name,C_RESET,C_YELLOW,lb->players[i].score,C_RESET);
    pause_enter();
}

void mask_word(const char *o,char *m){
    size_t len=strlen(o); m[0]=o[0];
    for(size_t i=1;i<len-1;i++)m[i]=(o[i]==o[0]||o[i]==o[len-1])?o[i]:'_';
    m[len-1]=o[len-1]; m[len]='\0';
}

void play_game(Leaderboard *lb,WordList *words,int word_count){
    if(word_count==0){printf("%sNo words available!%s\n",C_RED,C_RESET);pause_enter();return;}
    char name[MAX_NAME];
    while(1){
        printf("%sPlayer name%s (letters only, X to cancel): ",C_YELLOW,C_RESET);
        if(!read_line(name,MAX_NAME))continue;
        if((name[0]=='X'||name[0]=='x')&&name[1]=='\0')return;
        int valid=1;
        for(size_t i=0;i<strlen(name);i++)if(!isalpha((unsigned char)name[i])){valid=0;break;}
        if(valid&&strlen(name)>0)break;
        printf("%sInvalid name - letters only (A-Z).%s\n",C_RED,C_RESET);
    }

    int score=0; srand((unsigned)time(NULL)); bool quit_all=false;

    while(!quit_all){
        int index=rand()%word_count; char word[MAX_NAME],masked[MAX_NAME];
        strcpy(word,words[index].word); mask_word(word,masked);
        int mistakes=0; char guessed[MAX_NAME]="";

        while(mistakes<MAX_ATTEMPTS){
            clear_screen();
            printf("%sWord:%s %s\n",C_CYAN,C_RESET,masked);
            printf("%sIncorrect attempts:%s %d/%d\n",C_RED,C_RESET,mistakes,MAX_ATTEMPTS);
            printf("%sSingle letter guess%s (a-z, ! to quit): ",C_YELLOW,C_RESET);
            char buf[16];
            if(!read_line(buf,16))continue;
            if(strlen(buf)==1&&buf[0]=='!'){quit_all=true;break;}
            if(strlen(buf)!=1||!isalpha((unsigned char)buf[0])){
                printf("%sEnter exactly one letter (a-z).%s\n",C_RED,C_RESET);pause_enter();continue;
            }
            char letter=tolower(buf[0]);
            if(strchr(guessed,letter)){printf("%sLetter already guessed.%s\n",C_RED,C_RESET);pause_enter();continue;}
            size_t glen=strlen(guessed); guessed[glen]=letter; guessed[glen+1]='\0';

            bool found=false;
            for(size_t i=0;i<strlen(word);i++)if(tolower(word[i])==letter){masked[i]=word[i];found=true;}
            if(!found)mistakes++;

            if(strcmp(word,masked)==0){
                printf("%sYou guessed the word!%s\n",C_GREEN,C_RESET);
                score++;
                printf("Press Enter to play next round or X to stop.");
                int c=getchar(); if(c!='\n')while(getchar()!='\n');
                if(c=='X'||c=='x')quit_all=true;
                break;
            }
        }

        if(quit_all)break;
        if(mistakes>=MAX_ATTEMPTS){
            printf("%sGame Over%s - the word was %s%s%s\n",C_RED,C_RESET,C_CYAN,word,C_RESET);
            printf("Try again? (Y/N): ");
            int c=getchar(); if(c!='\n')while(getchar()!='\n');
            if(c!='Y'&&c!='y')break;
        }
    }

    int found=0;
    for(int i=0;i<lb->player_count;i++)
        if(strcmp(lb->players[i].name,name)==0){lb->players[i].score+=score;found=1;break;}
    if(!found&&lb->player_count<MAX_WORDS){
        strcpy(lb->players[lb->player_count].name,name);
        lb->players[lb->player_count].score=score;
        lb->player_count++;
    }
    save_leaderboard(lb);
}

int is_valid_new_word(const char *w){
    if(!w[0])return 0;
    for(size_t i=0;i<strlen(w);i++)if(!islower((unsigned char)w[i]))return 0;
    return 1;
}

void add_new_word(WordList *words,int *word_count){
    while(1){
        if(*word_count>=MAX_WORDS){printf("%sMaximum number of words reached.%s\n",C_RED,C_RESET);pause_enter();return;}
        char new_word[MAX_NAME];
        printf("%sNew word%s (lowercase, no spaces, X to return): ",C_YELLOW,C_RESET);
        if(!read_line(new_word,MAX_NAME))continue;
        if((new_word[0]=='X'||new_word[0]=='x')&&new_word[1]=='\0')return;
        if(!is_valid_new_word(new_word)){printf("%sWord must be lowercase letters only.%s\n",C_RED,C_RESET);continue;}
        int duplicate=0;
        for(int i=0;i<*word_count;i++)if(strcmp(words[i].word,new_word)==0){duplicate=1;break;}
        if(duplicate){printf("%sWord already exists.%s\n",C_RED,C_RESET);continue;}

        FILE *file=fopen(WORDS_FILE,"a");
        if(!file){printf("%sError opening file.%s\n",C_RED,C_RESET);pause_enter();return;}
        fprintf(file,"%s\n",new_word); fclose(file);
        strcpy(words[*word_count].word,new_word); (*word_count)++;
        printf("%sNew word added successfully!%s\n",C_GREEN,C_RESET);
    }
}

void display_menu(void){
    printf("%sHangman Game%s\n",C_CYAN,C_RESET);
    printf("%s1%s Play Game\n",C_YELLOW,C_RESET);
    printf("%s2%s View Leaderboard\n",C_YELLOW,C_RESET);
    printf("%s3%s Add a new word\n",C_YELLOW,C_RESET);
    printf("%s4%s Exit\n",C_YELLOW,C_RESET);
}

int main(void){
    Leaderboard lb={0}; WordList words[MAX_WORDS]; int word_count=0;
    load_words(words,&word_count); load_leaderboard(&lb);

    while(1){
        clear_screen(); display_menu();
        int choice=read_menu_choice();
        if(choice==1)play_game(&lb,words,word_count);
        else if(choice==2)display_leaderboard(&lb);
        else if(choice==3)add_new_word(words,&word_count);
        else if(choice==4){printf("%sExiting application.%s\n",C_CYAN,C_RESET);break;}
    }
    return 0;
}
