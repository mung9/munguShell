#ifndef __SHELL_CMD_LINE__
#define __SHELL_CMD_LINE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

#include <termio.h>
#include <pthread.h>
#include <sys/ioctl.h>

#define MAX_WORD	20
#define MAX_FILE_NAME	50
#define MAX_FILE_NUM	100

#define NEW_PROCESS	1
#define CURRENT_PROCESS	0

#define EMPTY		100

#define TRUE		1
#define FALSE		0

#define VK_FNC1		27
#define VK_FNC2		91
#define VK_UP		65
#define VK_DOWN		66
#define VK_LEFT		68
#define VK_RIGHT	67
#define VK_DELETE	51

#define VK_BACKSPACE	127


#define TEXT_DEFAULT		"\033[0m"
#define TEXT_BOLD		"\033[1m"
#define TEXT_COLOR_GREEN	"\033[32m"
#define TEXT_COLOR_CYAN		"\033[36m"
#define TEXT_COLOR_RED		"\033[31m"
#define TEXT_COLOR_BLUE		"\033[34m"
#define TEXT_COLOR_YELLOW	"\033[33m"
#define TEXT_COLOR_MAGENTA	"\033[35m"


// back-up STDOUT_FILENO when call function "init()"
// std_output = dup(STDOUT_FILENO);
static int std_output;
static pid_t root_process;

static const char* cmd_need_color[] = {"ls","grep"};

static char user_name[20];
static char cwd[200];
void set_cwd(const char* new_cwd);

void set_text(char* value);
void init();

/***************** run command ***********************/


char* wordStack[MAX_WORD];
int wordCount;

void print_shell_input_line();

void pushAWord(char* word);
int isEmpty();
void cleanStack();
void print_shell_input_line();
// deal with cmd line string
int execute(const char* cmd, int in_fd, int out_fd, int final_destination);

// execute actual command
void exec(char* args[], int in_fd, int out_fd, int bColor);

/***************** process variables and functions ****************/
#define MAX_PROCESS 50
pid_t processes[MAX_PROCESS]; // array stores background process
int nproc; // the number of background process

void push_procid(pid_t new_pid);
void pop_procid(pid_t target_pid);

void clean_process(int signo);
void* zombie_cleaner(void* arg); // function to be called by "reaper thread"

/***************** functions control cursor ****************/

// move cursor //
// return value : value of current_cursor after move cursor;
int move_cursor_left(const char* cmd, int* current_cursor);
int move_cursor_right(const char* cmd, int* current_cursor);

// erase a character //
// return value : length of the cmd string after erase a character
int erase_a_character(char* cmd, int* current_cursor);

// replace dest_cmd with src_cmd //
// return value : value of current_cursor after replace cursor : last index + 1
int replace_cmd(char* dest_cmd, char* src_cmd, int* current_cursor);

//
int insert_character( char c, char* cmd, int* current_cursor );

//
int is_cursor_end( const char* cmd, const int* current_cursor );


/***************** cmd log ****************/


#define MAX_CMD_LENGTH	100
#define MAX_CMD_LOG	20

typedef struct _cmd_log{
	char cmds[MAX_CMD_LOG+1][MAX_CMD_LENGTH];
	int the_latest;
	int nlogs;
	int cursor;
}cmd_log;

void init_log(cmd_log* log);
void push_cmd(const char* cmd, cmd_log* log);

void reset_log_cursor(cmd_log* log);

char* prev_cmd_or_null(cmd_log* log);
char* next_cmd_or_null(cmd_log* log);

int prev_index(int src, int n);
int next_index(int src, int n);


/***************** functions control files list ****************/

int __is_start_with( const char* smaller, const char* large );
int __is_ok( const char* hello, const char* world );

// return value : the number of files found in directory
int files_start_with( const char* str, const char* directory ,char* dest[], int max_number );
int parse_path(const char* path, char parent_dir[], char file_name[]);
void print_file_list(char* const list[], int n);
void auto_completion(char* target_list[], char dest[], int n);

// getch
int getch(int milisec);


/************* redefined cmd ******************/
void ls();
void cd(char* const path);

/************* util ******************/
void print_pretty(char* files[], int n);
//int width_height_list( char* list[], int length_of_each_col[], int n);
int is_samefd(int fd1, int fd2);
void sort_string(char* str[], int n);
int _is_in_filename(char c);


#endif
