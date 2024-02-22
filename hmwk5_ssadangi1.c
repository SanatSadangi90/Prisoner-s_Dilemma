/*
hmwk5_ssadangi1.c

Sanat Sadangi
CSc 3320 Homework #5
Account: ssadangi1
Due date: 12/3/23

Description: This program plays the prisoner's dilemma.

Input: The input is the two players pick their spot in this game.

Output: The output is the printf stamtents that shows instruction and results about the game.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1
#define score1 0 // This is to store player 1 score.
#define score2 0 // This is to store player 2 score.
#define DEBUG 1

// These are calls to the process.
void parent(int p1_write, int p2_write, int p1_read, int p2_read, int rounds); // This calls the parent process.
void player(int read_pipe, int write_pipe, int player_number); // This calls the player process.

// This is the main function.
int main(int argc, char *argv[]) {

    printf("The game is about to begin.\n");
    if (argc != 2) { // This checks to see if the rounds or -h argurement is passed.
        fprintf(stderr, "Usage: %s Use [-h] or <rounds>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // This is the help information about the game.
    if (argc == 2 && strcmp(argv[1], "-h") == 0) {
        printf("Usage: program_name [-h] int\n");
        printf("Options:\n");
        printf("  -h : Display help information.\n");
        printf(" int : Sets the number of rounds.\n");
        exit(EXIT_SUCCESS);
        return 0;
    }
    
    // This to check if the game has negative rounds.
    int rounds = atoi(argv[1]);
    if (rounds <= 0) {
        fprintf(stderr, "Invalid number of rounds: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    // These variables are to store process details.
    int p1_to_parent[2];
    int p2_to_parent[2];
    int parent_to_p1[2];
    int parent_to_p2[2];

    // This checks if the pipe was sucessful.
    if (pipe(p1_to_parent) == -1 || pipe(p2_to_parent) == -1 ||
        pipe(parent_to_p1) == -1 || pipe(parent_to_p2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1, pid2; // This stores the pid of the child and the parent.

    // This creates the first child process. (player 1)
    if ((pid1 = fork()) == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // This is the first child process. (player 1)
        close(p1_to_parent[WRITE]);
        close(parent_to_p1[READ]);
        player(p1_to_parent[READ], parent_to_p1[WRITE], 1);
        exit(EXIT_SUCCESS);
    }

    // This create the second child process. (player 2)
    if ((pid2 = fork()) == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // This is the second child process. (player 2)
        close(p2_to_parent[WRITE]);
        close(parent_to_p2[READ]);
        player(p2_to_parent[READ], parent_to_p2[WRITE], 2);
        exit(EXIT_SUCCESS);
    }

    // This is the parent process.
    close(p1_to_parent[READ]);
    close(p2_to_parent[READ]);
    close(parent_to_p1[WRITE]);
    close(parent_to_p2[WRITE]);

    // This calls the parent process.
    parent(p1_to_parent[WRITE], p2_to_parent[WRITE], parent_to_p1[READ], parent_to_p2[READ], rounds);

    // This will wait for both players (child process) to exit.
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // This will close all the pipes and exit the program.
    close(p1_to_parent[WRITE]);
    close(p2_to_parent[WRITE]);
    close(parent_to_p1[READ]);
    close(parent_to_p2[READ]);

    return 0;
}

// This is the parent process function.
void parent(int p1_write, int p2_write, int p1_read, int p2_read, int rounds) {
    int score_p1 = 0, score_p2 = 0; // This keeps track of both players scores.

    // This is the round loop.
    for (int round = 1; round <= rounds; ++round) {
        // This will wait for both players to respond.
        char ready_p1, ready_p2;
        // Announce the start of the round
        // This should run only in the beginning of the program.
        printf("The game is beginning. Press Y to ready up.\n");
        printf("Are you ready player one?\n");
        printf("Are you ready player two?\n");

        // This passes the char to the process.
        read(p1_read, &ready_p1, 1);
        read(p2_read, &ready_p2, 1);

        // This is check if both players are ready.
        if (ready_p1 == 'Y' && ready_p2 == 'Y') {
            // This will announce the start of the play and write the data to the process.
            printf("The round is beginnning.\n");
            write(p1_write, "P", 1);
            write(p2_write, "P", 1);

            // This will get the moves from both players.
            char move_p1, move_p2;
            read(p1_read, &move_p1, 1);
            read(p2_read, &move_p2, 1);

            // This will determine the scores and update.
            int score_round_p1, score_round_p2;
            // This gives a prompt, so the user knows what to do.
            printf("Enter your move P1: \n");

            // This gives a prompt, so the user knows what to do.
            printf("Enter your move P2: \n");
            // The is the scoring mechanism for the game.
            if (move_p1 == 'C' && move_p2 == 'C') { // This is when the players work with each other.
                score_round_p1 = 3;
                score_round_p2 = 3;
                printf("It is a tie.\n");
            } else if (move_p1 == 'C' && move_p2 == 'D') { // Player 2 wants to not work with player 1, but player 1 wants to work with player 2.
                score_round_p1 = 0;
                score_round_p2 = 5;
                printf("Player 2 Wins the round.\n");
            } else if (move_p1 == 'D' && move_p2 == 'C') { // Player 1 wants to not work with player 2, but player 2 wants to work with player 1.
                score_round_p1 = 5;
                score_round_p2 = 0;
                printf("Player 1 Wins the round.\n");
            } else if (move_p1 == 'D' && move_p2 == 'D') {  // This is when the players do not work with each other.
                score_round_p1 = 1;
                score_round_p2 = 1;
                printf("It is a tie.\n");
            }

            // This will add the score to the final score.
            score_p1 += score_round_p1;
            score_p2 += score_round_p2;

            // This will print out the current round information.
            printf("Round %d: player 1 %c, player 2 %c. Player 1 scores %d, player 2 scores %d.\n",
                   round, move_p1, move_p2, score_round_p1, score_round_p2);

            // This will communicate scores back to players.
            char score_message_p1[2], score_message_p2[2];
            sprintf(score_message_p1, "%d", score_round_p1);
            sprintf(score_message_p2, "%d", score_round_p2);

            // This will write the score to the process.
            write(p1_write, score_message_p1, 2);
            write(p2_write, score_message_p2, 2);
        } else if (ready_p1 == 'Q' && ready_p2 == 'Q'){ // This will end the game.
            printf("The game is ending.\n");
            break;
        } else { // This is when the input is unknow.
            printf("Unknow Input\n"); 
            break;
        }
    }

    // This will announce the end of the tournament.
    write(p1_write, "Q", 1);
    write(p2_write, "Q", 1);

    // This will close the pipes.
    close(p1_write);
    close(p2_write);
    close(p1_read);
    close(p2_read);

    // This will print the final scores.
    printf("Final Scores - Player 1: %d, Player 2: %d\n", score_p1, score_p2);
}

void player(int read_pipe, int write_pipe, int player_number) {
    // These are the variables for the player (child) process.
    char response;
    int score = 0;

    // This will announce the readiness to the parent.
    write(write_pipe, "Y", 1);

    // This is the while loop for checking what char was passed from the parent process.
    while (1) {
        // This will read the commands from the parent.
        char command;
        char move; // This is the logic to determine the moves.
        read(read_pipe, &command, 1); // This will write the data to the parent process.

        switch (command) {
            case 'R':
                // This will announce that the player is ready.
                write(write_pipe, "Y", 1);
                break;
            case 'P':
                // This will implement the player's strategy for choosing either work together or not working together.
                write(write_pipe, &move, 1);

                // This will receive the score from the parent process.
                char score_message;
                read(read_pipe, &score_message, 1);
                score = score_message - '0'; // This convert the character to a integer.

                break;
            case 'Q':
                // This will run when the player quits.
                close(read_pipe);
                close(write_pipe);
                exit(EXIT_SUCCESS);
        }
    }
}
