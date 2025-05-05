#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_WORDS 100
#define MAX_NAME 100
#define MAX_ATTEMPTS 5
#define LEADERBOARD_FILE "leaderboard.txt"
#define WORDS_FILE "words.txt"

typedef struct {
    char name[MAX_NAME];
    int score;
} Player;

typedef struct {
    Player players[MAX_WORDS];
    int player_count;
} Leaderboard;

typedef struct {
    char word[MAX_NAME];
} WordList;

void clear_screen() {
    printf("\033[H\033[J");
}

void load_words(WordList *words, int *word_count) {
    FILE *file = fopen(WORDS_FILE, "r");
    if (!file) {
        printf("Error loading words!\n");
        return;
    }
    *word_count = 0;
    while (fscanf(file, "%s", words[*word_count].word) != EOF && *word_count < MAX_WORDS) {
        (*word_count)++;
    }
    fclose(file);
}

void save_leaderboard(Leaderboard *lb) {
    FILE *file = fopen(LEADERBOARD_FILE, "w");
    if (!file) {
        printf("Error saving leaderboard!\n");
        return;
    }
    for (int i = 0; i < lb->player_count; i++) {
        fprintf(file, "%s %d\n", lb->players[i].name, lb->players[i].score);
    }
    fclose(file);
}

void load_leaderboard(Leaderboard *lb) {
    FILE *file = fopen(LEADERBOARD_FILE, "r");
    if (!file) return;
    lb->player_count = 0;
    while (fscanf(file, "%s %d", lb->players[lb->player_count].name, &lb->players[lb->player_count].score) == 2) {
        lb->player_count++;
    }
    fclose(file);
}

void display_leaderboard(Leaderboard *lb) {
    clear_screen();
    printf("Leaderboard:\n");
    for (int i = 0; i < lb->player_count; i++) {
        printf("%s: %d\n", lb->players[i].name, lb->players[i].score);
    }
    printf("Press Enter to return to the menu...");
    getchar();
    getchar();
}

void mask_word(const char *original, char *masked) {
    size_t len = strlen(original);
    masked[0] = original[0];
    for (size_t i = 1; i < len - 1; i++) {
        masked[i] = (original[i] == original[0] || original[i] == original[len - 1]) ? original[i] : '_';
    }
    masked[len - 1] = original[len - 1];
    masked[len] = '\0';
}

void play_game(Leaderboard *lb, WordList *words, int word_count) {
    if (word_count == 0) {
        printf("No words available!\n");
        return;
    }
    char name[MAX_NAME];
    printf("Enter your name: ");
    scanf("%s", name);
    int score = 0;
    srand(time(NULL));
    while (1) {
        int index = rand() % word_count;
        char word[MAX_NAME], masked[MAX_NAME];
        strcpy(word, words[index].word);
        mask_word(word, masked);
        int mistakes = 0;
        char guessed_letters[MAX_NAME] = "";
        while (mistakes < MAX_ATTEMPTS) {
            clear_screen();
            printf("Word: %s\n", masked);
            printf("Incorrect attempts: %d/%d\n", mistakes, MAX_ATTEMPTS);
            printf("Enter a letter: ");
            char letter;
            char input[MAX_NAME];
            scanf("%s", input);
            if (strlen(input) != 1) {
                printf("Please enter only one letter!\n");
                printf("Press Enter to continue...");
                getchar();
                char choice_game = getchar();
                break;
            }
            letter = input[0];
            if (strchr(guessed_letters, letter)) {
                printf("You already guessed that letter!\n");
                continue;
            }
            strncat(guessed_letters, &letter, 1);
            bool found = false;
            for (size_t i = 0; i < strlen(word); i++) {
                if (word[i] == letter) {
                    masked[i] = letter;
                    found = true;
                }
            }
            if (!found) {
                mistakes++;
            }
            if (strcmp(word, masked) == 0) {
                printf("You guessed the word!\n");
                score += 1;
                printf("Press Enter to continue or X to exit...");
                getchar();
                char choice_game = getchar();
                if (choice_game == 'X' || choice_game == 'x'){
                    break;
                }
                break;
            }
        }
        if (mistakes >= MAX_ATTEMPTS) {
            printf("Game Over! The word was: %s\n", word);
            printf("Would you like to try again? (Y/N): ");
            getchar();
            char retry = getchar();
            if (retry == 'Y' || retry == 'y') {
                continue;
            } else {
                break;
            }
        }
    }
    int found = 0;
    for (int i = 0; i < lb->player_count; i++) {
        if (strcmp(lb->players[i].name, name) == 0) {
            lb->players[i].score += score;
            found = 1;
            break;
        }
    }
    if (!found && lb->player_count < MAX_WORDS) {
        strcpy(lb->players[lb->player_count].name, name);
        lb->players[lb->player_count].score = score;
        lb->player_count++;
    }
    save_leaderboard(lb);
}

void display_menu() {
    printf("Hangman Game\n");
    printf("1. Play Game\n");
    printf("2. View Leaderboard\n");
    printf("3. Add a new word\n");
    printf("4. Exit\n");
    printf("Choose an option: ");
}

void add_new_word(WordList *words, int *word_count) {
    while (1) {
        if (*word_count >= MAX_WORDS) {
            printf("Maximum number of words reached. Cannot add more.\n");
            printf("Press Enter to continue...");
            getchar();
            getchar();
            return;
        }
        printf("Enter a new word (or X to return): ");
        char new_word[MAX_NAME];
        scanf("%s", new_word);
        if (strcmp(new_word, "X") == 0 || strcmp(new_word, "x") == 0) {
            return;
        }
        int duplicate = 0;
        for (int i = 0; i < *word_count; i++) {
            if (strcmp(words[i].word, new_word) == 0) {
                printf("This word already exists.\n");
                printf("Press Enter to continue...");
                getchar();
                getchar();
                duplicate = 1;
                break;
            }
        }
        if (duplicate) {
            continue;
        }
        FILE *file = fopen(WORDS_FILE, "a");
        if (!file) {
            printf("Error opening file to add the new word.\n");
            printf("Press Enter to continue...");
            getchar();
            getchar();
            return;
        }
        fprintf(file, "%s\n", new_word); // Ensures a new line
        fclose(file);
        strcpy(words[*word_count].word, new_word);
        (*word_count)++;
        printf("New word \"%s\" added successfully!\n", new_word);
        printf("Press Enter to add another word or X to exit...");
        getchar();
        char c = getchar();
        if (c == 'X' || c == 'x') {
            return;
        }
    }
}

int main() {
    Leaderboard lb = {0};
    WordList words[MAX_WORDS];
    int word_count = 0;
    load_words(words, &word_count);
    load_leaderboard(&lb);
    int choice;
    do {
        clear_screen();
        display_menu();
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                play_game(&lb, words, word_count);
                break;
            case 2:
                display_leaderboard(&lb);
                break;
            case 3:
                add_new_word(words, &word_count);
                break;
            case 4:
                printf("Exiting application...\n");
                break;
            default:
                printf("Invalid option. Try again.\n");
        }
    } while (choice != 4);
    return 0;
}
