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

void clear_screen(void){printf("\033[H\033[J");}
void pause_enter(void){printf("Press Enter to continue.");while(getchar()!='\n');}

int read_line(char* d,int m){
    if(!fgets(d,m,stdin))return 0;
    size_t l=strlen(d);
    if(l&&d[l-1]=='\n')d[l-1]='\0';
    return 1;
}

void sort_leaderboard(Leaderboard* lb){
    for(int i=0;i<lb->player_count-1;i++)
        for(int j=i+1;j<lb->player_count;j++)
            if(lb->players[j].score>lb->players[i].score){
                Player t=lb->players[i];
                lb->players[i]=lb->players[j];
                lb->players[j]=t;
            }
}

int read_menu_choice(void){
    char b[16];int c=-1;
    while(1){
        printf("%sMenu choice%s (1-4): ",C_YELLOW,C_RESET);
        if(!read_line(b,16))continue;
        char* e;long v=strtol(b,&e,10);
        if(*e=='\0'&&v>=1&&v<=4){c=(int)v;break;}
        printf("%sInvalid option. Choose 1-4.%s\n",C_RED,C_RESET);
    }
    return c;
}

void load_words(WordList*w,int*c){
    FILE*f=fopen(WORDS_FILE,"r");if(!f){*c=0;return;}
    *c=0;while(*c<MAX_WORDS&&fscanf(f,"%99s",w[*c].word)==1)(*c)++;
    fclose(f);
}

void save_leaderboard(Leaderboard*lb){
    sort_leaderboard(lb);
    FILE*f=fopen(LEADERBOARD_FILE,"w");if(!f)return;
    for(int i=0;i<lb->player_count;i++)fprintf(f,"%s %d\n",lb->players[i].name,lb->players[i].score);
    fclose(f);
}

void load_leaderboard(Leaderboard*lb){
    FILE*f=fopen(LEADERBOARD_FILE,"r");lb->player_count=0;if(!f)return;
    while(lb->player_count<MAX_WORDS&&fscanf(f,"%99s %d",lb->players[lb->player_count].name,&lb->players[lb->player_count].score)==2)lb->player_count++;
    fclose(f);
    sort_leaderboard(lb);
}

void display_leaderboard(Leaderboard*lb){
    clear_screen();
    printf("%sLeaderboard%s\n",C_CYAN,C_RESET);
    for(int i=0;i<lb->player_count;i++)
        printf("%s%s%s : %s%d%s\n",C_GREEN,lb->players[i].name,C_RESET,C_YELLOW,lb->players[i].score,C_RESET);
    pause_enter();
}

void mask_word(const char*o,char*m){
    size_t l=strlen(o);m[0]=o[0];
    for(size_t i=1;i<l-1;i++)m[i]=(o[i]==o[0]||o[i]==o[l-1])?o[i]:'_';
    m[l-1]=o[l-1];m[l]='\0';
}

void record_score_prompt(Leaderboard*lb,int score){
    if(score==0){printf("No points scored – nothing to record.\n");pause_enter();return;}
    printf("Your final score: %s%d point%s%s\n",C_YELLOW,score,(score==1?"":"s"),C_RESET);
    char name[MAX_NAME];
    while(1){
        printf("Enter your %sname%s to record %s%d point%s%s, or %sN%s to skip: ",
               C_CYAN,C_RESET,C_YELLOW,score,(score==1?"":"s"),C_RESET,C_RED,C_RESET);
        if(!read_line(name,MAX_NAME))continue;
        if((name[0]=='N'||name[0]=='n')&&name[1]=='\0'){printf("Score not recorded.\n");pause_enter();return;}
        int v=1;for(size_t i=0;i<strlen(name);i++)if(!isalpha((unsigned char)name[i])){v=0;break;}
        if(v&&strlen(name)>0)break;
        printf("%sInvalid name - letters only (A-Z).%s\n",C_RED,C_RESET);
    }
    int f=0;
    for(int i=0;i<lb->player_count;i++)
        if(strcmp(lb->players[i].name,name)==0){lb->players[i].score+=score;f=1;break;}
    if(!f&&lb->player_count<MAX_WORDS){
        strcpy(lb->players[lb->player_count].name,name);
        lb->players[lb->player_count].score=score;
        lb->player_count++;
    }
    save_leaderboard(lb);
    printf("%sScore recorded!%s\n",C_GREEN,C_RESET);
    pause_enter();
}

void play_game(Leaderboard*lb,WordList*words,int wc){
    if(wc==0){printf("%sNo words available!%s\n",C_RED,C_RESET);pause_enter();return;}
    int score=0;srand((unsigned)time(NULL));bool quit_all=false;
    while(!quit_all){
        int idx=rand()%wc;char word[MAX_NAME],masked[MAX_NAME];
        strcpy(word,words[idx].word);mask_word(word,masked);
        int mistakes=0;char guessed[MAX_NAME]="";
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
            size_t glen=strlen(guessed);guessed[glen]=letter;guessed[glen+1]='\0';
            bool found=false;
            for(size_t i=0;i<strlen(word);i++)if(tolower(word[i])==letter){masked[i]=word[i];found=true;}
            if(!found)mistakes++;
            if(strcmp(word,masked)==0){
                printf("%sYou guessed the word!%s\n",C_GREEN,C_RESET);
                score+=strlen(word);                         /* score equals word length */
                printf("Press Enter for next round or X to stop.");
                int c=getchar();if(c!='\n')while(getchar()!='\n');
                if(c=='X'||c=='x')quit_all=true;
                break;
            }
        }
        if(quit_all)break;
        if(mistakes>=MAX_ATTEMPTS){
            printf("%sGame Over%s - the word was %s%s%s\n",C_RED,C_RESET,C_CYAN,word,C_RESET);
            printf("Try again? (Y/N): ");
            int c=getchar();if(c!='\n')while(getchar()!='\n');
            if(c!='Y'&&c!='y')break;
        }
    }
    clear_screen();
    record_score_prompt(lb,score);
}

int is_valid_new_word(const char*w){
    if(!w[0])return 0;
    for(size_t i=0;i<strlen(w);i++)if(!islower((unsigned char)w[i]))return 0;
    return 1;
}

void add_new_word(WordList*w,int*wc){
    while(1){
        if(*wc>=MAX_WORDS){printf("%sMaximum number of words reached.%s\n",C_RED,C_RESET);pause_enter();return;}
        char nw[MAX_NAME];
        printf("%sNew word%s (lowercase, no spaces, X to return): ",C_YELLOW,C_RESET);
        if(!read_line(nw,MAX_NAME))continue;
        if((nw[0]=='X'||nw[0]=='x')&&nw[1]=='\0')return;
        if(!is_valid_new_word(nw)){printf("%sWord must be lowercase letters only.%s\n",C_RED,C_RESET);continue;}
        int dup=0;for(int i=0;i<*wc;i++)if(strcmp(w[i].word,nw)==0){dup=1;break;}
        if(dup){printf("%sWord already exists.%s\n",C_RED,C_RESET);continue;}
        FILE*f=fopen(WORDS_FILE,"a");
        if(!f){printf("%sError opening file.%s\n",C_RED,C_RESET);pause_enter();return;}
        fprintf(f,"%s\n",nw);fclose(f);
        strcpy(w[*wc].word,nw);(*wc)++;
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
    Leaderboard lb={0};WordList words[MAX_WORDS];int wc=0;
    load_words(words,&wc);load_leaderboard(&lb);
    while(1){
        clear_screen();display_menu();
        int ch=read_menu_choice();
        if(ch==1)play_game(&lb,words,wc);
        else if(ch==2)display_leaderboard(&lb);
        else if(ch==3)add_new_word(words,&wc);
        else if(ch==4){printf("%sExiting application.%s\n",C_CYAN,C_RESET);break;}
    }
    return 0;
}
