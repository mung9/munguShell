#include "shell.h"


void set_text(char* value){
	printf("%s",value);
}
void init(){
	std_output = dup(STDOUT_FILENO);
	root_process = getpid();

	set_text(TEXT_COLOR_RED);
	set_text(TEXT_BOLD);
	printf("** Welcome! **\n");
	set_text(TEXT_DEFAULT);
	uid_t uid = getuid();
	strcpy(user_name,getpwuid(uid)->pw_name);
	getcwd(cwd,200);
}
/***************** run command ***********************/
/***************** run command ***********************/
/***************** run command ***********************/
/***************** run command ***********************/

void set_cwd(const char* new_cwd){
	strcpy(cwd, new_cwd);
}

void print_shell_input_line(){
	set_text(TEXT_BOLD);
	set_text(TEXT_COLOR_YELLOW);
	printf("%s",user_name);
	set_text(TEXT_DEFAULT);
	printf(":");
	set_text(TEXT_BOLD);
	set_text(TEXT_COLOR_BLUE);
	printf("%s",cwd);
	set_text(TEXT_DEFAULT);
	printf("$ ");
}

void pushAWord(char* word){
	int length = strlen(word);
	if( length<1 || word[0]=='\0' ) return;

	wordStack[wordCount] = (char*)malloc(length*sizeof(char)+1);
	strcpy(wordStack[wordCount], word);
	++wordCount;
	word[0]='\0';
}
int isEmpty(){
	return wordCount==0;
}
void cleanStack(){
	int i;
	for(i=0;i<wordCount;++i){
		free(wordStack[i]);
		wordStack[i] = NULL;
	}
	wordCount = 0;
}


void print_shell_input_line();
	
// execute cmd
int execute(const char* cmd, int in_fd, int out_fd, int final_destination){
	const char *c = cmd;
	char pre_cmd[MAX_CMD_LENGTH] = {0,};
	char aWord[15] = {0,};
	char wordLen = 0;
	int new_fd[2];
	char parenthesis_cmd[50] = {0.};
	int parenthesis_length = 0;
	int parenthesis_count = 0;
	char srcfile[MAX_FILE_NAME] = {0,};
	char destfile[MAX_FILE_NAME] = {0,};
	int bColor = FALSE;
	if( is_samefd(final_destination, std_output) ){
		bColor = TRUE;
	}
	else 	bColor = FALSE;
	for( c=cmd; *c!='\0';c+=sizeof(char)){
		if(*c == '>'){
			bColor = FALSE;
		}
		else if(*c == '&'){
			pid_t pid;
			strncpy(pre_cmd,cmd,c-cmd);
			pre_cmd[c-cmd] = '\0';
			if((pid=fork())==0){
				execute(pre_cmd,in_fd,out_fd,final_destination);
				exit(0);
			}
			push_procid(pid);
			fprintf(stderr,"[%d] : running\n",nproc-1	);
			execute(c+1, in_fd, out_fd, final_destination);
			return TRUE;
		}
	}
	pipe(new_fd);
	cleanStack();
	for(c=cmd; *c!='\0'; c+=sizeof(char)){
		if( parenthesis_count > 0 ){
			if( *c == ')' ){ 
				if( --parenthesis_count == 0 )
					continue;
			}
			else if(*c=='('){
				++parenthesis_count;
			}
			parenthesis_cmd[parenthesis_length] = *c;
			++parenthesis_length;
			continue;
		}
		switch( *c ){
			int i=0;
		case '(':
			parenthesis_count = 1; break;
		// redirection
		case '<':
			memset(srcfile, 0, MAX_FILE_NAME);
			i=0;
			++c;
			while( _is_in_filename(*c) || srcfile[0]=='\0'){
				if( *c != ' '){
					srcfile[i] = *c;
					++i;
				}
				++c;
			}
			--c;
			in_fd = open(srcfile, O_RDONLY);
			break;
		// redirection
		case '>':
			memcpy( pre_cmd, cmd, strlen(cmd)-strlen(c));
			memset(destfile, 0, MAX_FILE_NAME);
			i=0;
			++c;
			while( _is_in_filename(*c) || destfile[0]=='\0'){
				if( *c != ' '){
					destfile[i] = *c;
					++i;
				}
				++c;
			}
			--c;
			bColor = FALSE;
			int tmp = open(destfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
			if( is_samefd(final_destination, out_fd) ){
				final_destination = dup(tmp);
			}
			out_fd = dup(tmp);
			break;
		case '|': 
			pushAWord(aWord);
			pid_t pid = fork();
			if(pid==0){
				close(new_fd[1]);
				execute( c+sizeof(char), new_fd[0], out_fd, final_destination);
				exit(0);
			}
			close(new_fd[0]);
			exec(wordStack, in_fd, new_fd[1],bColor);
			close(new_fd[1]);
			waitpid(pid,NULL,0);
			return TRUE;
		case ' ':
			aWord[wordLen] = '\0';
			wordLen = 0;
			pushAWord(aWord);
			break;
		default: 
			aWord[wordLen] = *c;
			aWord[wordLen+1] = 0;
			++wordLen;
		}
	}
	pushAWord(aWord);
	//
	if( parenthesis_cmd[0] != 0 ){
		execute(parenthesis_cmd, in_fd, out_fd, final_destination);
		return 0;
	}
	if( wordStack[0] == NULL ){
	       	return EMPTY;
	}
	exec(wordStack, in_fd, final_destination, bColor);
	return 0;
}


void exec(char* args[], int in_fd, int out_fd, int bColor){
	if( args[0] == NULL ) return;
	pid_t pid = fork();
	if(pid==0){
		if( in_fd != STDIN_FILENO){
			dup2(in_fd, STDIN_FILENO);
			close(in_fd);
		}
		if( out_fd != STDOUT_FILENO){
			dup2(out_fd, STDOUT_FILENO);
			close(out_fd);
		}
		// if the command is "ls" with no option, call my function.	
		if( (args[1]==NULL) && (strcmp(args[0],"ls")==0) )
			ls();
		// or execvp
		else{
			int i;
			if(bColor==TRUE){
				for( i=0; i<2; ++i ){
					if( strcmp(args[0], cmd_need_color[i])==0 ){
						int j;
						for( j=0; j<MAX_WORD; ++j ){
							if(args[j] == NULL){
								args[j] = "--color=always";
								args[j+1] = NULL;
								i=4;
								break;
				}}}} // for-loop end
			}
			execvp(args[0], args);
		}
		fprintf(stderr,"%s : invalid syntax\n",args[0]);
		exit(1);
	}
	waitpid(pid, NULL, 0);
	fflush(stdout);
}

/***************** process variables and functions ****************/
/***************** process variables and functions ****************/
/***************** process variables and functions ****************/
/***************** process variables and functions ****************/

// I don't care about index out of bound
void push_procid(pid_t new_pid){
	int cursor;
	for(cursor=0; cursor<nproc; ++cursor){
		if( processes[cursor] == new_pid ){
			perror("you've pushed the pid already");
			return;
		}
	}
	processes[nproc++] = new_pid;
}

void pop_procid(pid_t target_pid){
	int cursor;
	for(cursor=0; cursor<nproc; ++cursor){
		if( processes[cursor] == target_pid ){
			processes[cursor] = processes[nproc-1];
		}
	}
	--nproc;
}

void clean_process(int signo){
	pid_t pid;
	while(1){
		pid = waitpid(-1, NULL, WNOHANG);
		if( pid <= 0 ) return;
		pop_procid(pid);
	}
}
void* zombie_cleaner(void* arg){
	pid_t pid;
	struct sigaction sa;
	sa.sa_handler = clean_process;
	sa.sa_flags = SA_NODEFER | SA_NOCLDWAIT;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGCHLD, &sa, NULL);
	while(1){
		pause();
	}
}

/*************** functions control the cursor *******************/
/*************** functions control the cursor *******************/
/*************** functions control the cursor *******************/
/*************** functions control the cursor *******************/

// move cursor //
// return value : value of current_cursor after move cursor;
int move_cursor_left(const char* cmd, int* current_cursor){
	if( (cmd == NULL) || (current_cursor == NULL)) return -1;
	int cursor = *current_cursor;

	if( cursor > 0 ){
		fputc('\b',stdout);
		*current_cursor = --cursor;
	}
	return cursor;
}
int move_cursor_right(const char* cmd, int* current_cursor){
	if( (cmd == NULL) || (current_cursor == NULL)) return -1;
	int cmd_length = strlen(cmd);
	int cursor = *current_cursor;

	if( cursor < cmd_length ){
		fputc(cmd[cursor],stdout);
		*current_cursor = ++cursor;
	}
	return cursor;
}

// erase a character at the cursor-1//
// return value : length of the cmd string after erase a character
int erase_a_character(char* cmd, int* current_cursor){
	if( (cmd == NULL) || (current_cursor == NULL)) return -1;
	if( *current_cursor < 1 ) return strlen(cmd);
	int cmd_length = strlen(cmd);
	int cursor;

	fputc('\b',stdout);
	for( cursor=(*current_cursor-1); cursor<cmd_length-1; ++cursor ){
		cmd[cursor] = cmd[cursor+1];
		fputc(cmd[cursor], stdout);
	}
	fputs(" \b",stdout);
	cmd[cursor] = '\0';
	for(;cursor!=(*current_cursor-1); --cursor){
		fputc('\b',stdout);
	}
	*current_cursor -= 1;
	return --cmd_length;
}

// replace old command string with new one //
// return value : value of current_cursor after replace cursor : last index + 1
int replace_cmd(char* dest_cmd, char* src_cmd, int* current_cursor){
	if( (src_cmd == NULL) || (dest_cmd == NULL) || (current_cursor == NULL)){
		//fprintf(stderr,"no cmd to replace!\n");
		return strlen(dest_cmd);
	}
	int old_cmd_length = strlen(dest_cmd);
	int new_cmd_length = strlen(src_cmd);
	int cursor;
	
	strcpy(dest_cmd, src_cmd);
	for( cursor=*current_cursor; cursor>0; --cursor ){
		fputc('\b',stdout);
	}
	fputs(src_cmd, stdout);
	for( cursor=new_cmd_length; cursor<old_cmd_length; ++cursor ){
		fputc(' ',stdout);
	}
	for( cursor=old_cmd_length; cursor>new_cmd_length; --cursor ){
		fputc('\b',stdout);
	}
	*current_cursor = new_cmd_length;
	return new_cmd_length;
}

int insert_character( char c, char* cmd, int* current_cursor ){
	if( (cmd == NULL) || (current_cursor == NULL)){
		perror("insert_character : NULL parameter");
		return -1;
	}
	int cmd_length = strlen(cmd);
	int cursor;
	fputc(c,stdout);
	for( cursor=(*current_cursor); cursor<cmd_length; ++cursor ){
		fputc(cmd[cursor], stdout);
	}
	for( cursor=cmd_length; cursor>(*current_cursor); --cursor ){
		cmd[cursor] = cmd[cursor-1];
		fputc('\b',stdout);
	}
	cmd_length += 1;
	cmd[cursor] = c;
	cmd[cmd_length] = '\0';
	*current_cursor += 1;
	return cmd_length;
}

int is_cursor_end( const char* cmd, const int* current_cursor ){
	if( (cmd == NULL) || (current_cursor == NULL)){
		perror("is_cursor_end : NULL parameter");
		return -1;
	}
	return (strlen(cmd) == *current_cursor);
}

int getch(int milisec){
	int ch;
	struct termios buf, save;
	tcgetattr(0,&save);
	buf=save;
	buf.c_lflag &= ~(ICANON|ECHO);
	if( milisec == 0)
		buf.c_cc[VMIN]=1;
	else
		buf.c_cc[VMIN]=0;
	buf.c_cc[VTIME] = milisec/100;
	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);

	
	return ch;
}

/*************** cmd log functions *******************/
/*************** cmd log functions *******************/
/*************** cmd log functions *******************/
/*************** cmd log functions *******************/

void print(cmd_log* log){
	for(int i=0; i<MAX_CMD_LOG+1; ++i){
		fprintf(stderr,"[%s] ",log->cmds[i]);
	}
	fprintf(stderr,"\n");
}


void init_log(cmd_log* log){
	log->the_latest = prev_index(0,1);
	log->nlogs = 0;
	log->cursor = next_index(0,1);
}
void reset_log_cursor(cmd_log* log){
	if( log->nlogs == 0 )
		log->cursor = next_index(log->the_latest,2);
	else
		log->cursor = next_index(log->the_latest,1);
}

void push_cmd(const char* cmd, cmd_log* log){
	int i,j, index;
	for( i=0; i<log->nlogs; ++i ){
		index = prev_index(log->the_latest,i);
		if( strcmp(log->cmds[prev_index(log->the_latest,i)],cmd)==0 ){
			for( j=i; j>0; --j ){
				strcpy(log->cmds[prev_index(log->the_latest,j)],log->cmds[prev_index(log->the_latest,j-1)]);
			}
			strcpy(log->cmds[log->the_latest],cmd);
			return;
		}
	}

	log->the_latest = next_index(log->the_latest,1);
	strcpy(log->cmds[log->the_latest], cmd);
	if( log->nlogs < MAX_CMD_LOG )
		++log->nlogs;
}

// if there's no previous command, return NULL
char* prev_cmd_or_null(cmd_log* log){
	if( log->nlogs == 0 ) return NULL;
	if( log->cursor == prev_index(log->the_latest,log->nlogs-1) ) return NULL;
	log->cursor = prev_index(log->cursor, 1);
	return log->cmds[log->cursor];
}
char* next_cmd_or_null(cmd_log* log){
	if( log->nlogs == 0 ) return NULL;
	if( log->cursor == log->the_latest ) return NULL;
	log->cursor = next_index(log->cursor, 1);
	return log->cmds[log->cursor];
}

int prev_index(int src, int n){
	int answer = src;
	n = n%MAX_CMD_LOG;
	answer-=n;
	if( answer<0 )
		answer+=MAX_CMD_LOG;
	return answer;
}
int next_index(int src, int n){
	int answer = src;
	n = n%MAX_CMD_LOG;
	answer+=n;
	if( answer>=MAX_CMD_LOG )
		answer-=MAX_CMD_LOG;
	return answer;
}

/*************** functions control the list of files *******************/
/*************** functions control the list of files *******************/
/*************** functions control the list of files *******************/
/*************** functions control the list of files *******************/

int __is_start_with( const char* smaller, const char* large ){
	if( smaller == NULL || large == NULL ) return FALSE;
	int length = strlen(smaller);
	int cmp_ret = strncmp(smaller, large, length);
	return cmp_ret==0;
}
int __is_ok( const char* hello, const char* world ){
	return TRUE;
}

int files_start_with( const char* str, const char* directory ,char* dest[], int max_number ){
	if( (dest == NULL) || max_number < 0){
		perror("files_start_with : weird parameter");
		return -1;
	}
	struct dirent *direntp = NULL;
	DIR *dirp = NULL;
	int nfiles = 0;
	int (*condition)(const char*,const char*) = NULL;
	char path[MAX_FILE_NAME*2] = {0,};
	struct stat info;

	memset(dest,0,max_number*sizeof(char*));
	if( directory == NULL || *directory == '\0' )
		dirp = opendir("./");
	else
		dirp = opendir(directory);

	if( str == NULL ) condition = __is_ok;		
	else condition = __is_start_with;

	// insert the name of files satisfying the condition
	if( dirp != (DIR*)NULL ){
		while((direntp = readdir(dirp)) != NULL){
			sprintf(path,"%s%s",directory,direntp->d_name);
			stat(path, &info);
			if(condition(str, direntp->d_name)){
				dest[nfiles] = (char*)malloc(
					(strlen(direntp->d_name)+sizeof(char))*sizeof(char));
				strcpy(dest[nfiles],direntp->d_name);
				if( S_ISDIR(info.st_mode) )
					strcat(dest[nfiles], "/");
				++nfiles;
				strcpy(dest[nfiles-1],direntp->d_name);
			}
		}
		closedir(dirp);
	}
	int i;
	return nfiles;
}

int parse_path(const char* path, char parent_dir[], char file_name[]){
	if( path == NULL || path[0] == '\0' ){
		parent_dir = "./";
		file_name = NULL;
		return 0;
	}
	int cursor = 0, index=0;
	int path_length = strlen(path);;

	for( cursor=0; cursor<path_length; ++cursor){
		file_name[index] = path[cursor];
		++index;
		if(path[cursor] == '/'){
			strcat(parent_dir, file_name);
			memset(file_name, 0, index);
			index = 0;
		}
	}
	file_name[strlen(file_name)] = '\0';
	if(file_name[0] == '\0') file_name = NULL;
	parent_dir[strlen(parent_dir)] = '\0';
}

void print_file_name(const char* filename){
	struct stat info;
	char postfix = 0;
	int bColor = FALSE;
	if(is_samefd(std_output,STDOUT_FILENO))
		bColor = TRUE;	

	if(bColor){
		postfix = '\t';
		stat(filename, &info);
		if( S_ISREG(info.st_mode) ){
			set_text(TEXT_DEFAULT);
		}
		else if( S_ISDIR(info.st_mode) ){
			set_text(TEXT_COLOR_BLUE);
			set_text(TEXT_BOLD);
		}
		else{
			//fprintf(stderr,"%s is nothing\n",filename);
			/*set_text(TEXT_COLOR_GREEN);
			set_text(TEXT_BOLD);*/
		}	
	}
	else{
		postfix = '\n';
	}

	printf("%s%c",filename,postfix);
	if(bColor)
		set_text(TEXT_DEFAULT);
}
void print_file_list(char* const list[], int n){
	int i;
	int ac=0;
	char env[10];
	int width = 0;
	for(i=0;i<n;++i){
		print_file_name(list[i]);
	}
	printf("\n");
}

// get common word
void auto_completion(char* target_list[], char dest[], int n){
	if( target_list[0]==NULL ){
		dest = NULL;
		return;
	}
	int i,j;
	char common[MAX_FILE_NAME] = {0,};
	int ncontaining=0;
	int len = strlen(target_list[0]);
	int first_diff_index = 0;

	memcpy(common,target_list[0], MAX_FILE_NAME);
	for(i=0;i<n;++i){
		len = strlen(target_list[i]);
		for(j=0; j<len; ++j){
			common[j] &= target_list[i][j];
			if( common[j] != target_list[i][j] )
				len = j;
		}
	}
	for( i=0; i<MAX_FILE_NAME; ++i ){
		if( common[i] != target_list[0][i] ){
			first_diff_index = i;
			break;
		}
	}
	strncpy(dest, target_list[0], len);

}


void print_pretty(char* files[], int n){
	int i,j;	
	int dx = (n/5)+1;//3
	int tmp = 0;
	for(j=0; j<dx; ++j){
		for( i=j; i<n; i+=dx ){
			print_file_name(files[i]);
			if(i+dx >= n){
				break;				
			}
		}
		printf("\n");
	}
}
/* ls */
void ls(){
	struct dirent *direntp = NULL;
	DIR *dirp = NULL;
	int count = 0,i,j;
	char* files[MAX_FILE_NUM];
	char* res[MAX_FILE_NUM][MAX_FILE_NAME] = {0,};
	int max_length[5] = {0,};

	dirp = opendir("./");
	while((direntp = readdir(dirp)) != NULL){
		if( direntp->d_name[0] == '.' )
			continue;
		files[count] = (char*)malloc(MAX_FILE_NAME*sizeof(char));	
		strcpy(files[count], direntp->d_name);
		++count;
	}	
	sort_string(files, count);
	if( is_samefd(STDOUT_FILENO, std_output)){
		print_pretty(files,count);
	}
	else{
		for( i=0; i<count; ++i ){
			print_file_name(files[i]);
			if( is_samefd(STDOUT_FILENO, std_output)){
				print_file_name(files[i]);
				if( i%5 == 4 ) printf("\n");
			}
		}
	}
	
	for(i=0; i<count; ++i)
		free(files[i]);
	exit(0);
}


/* cd */
void cd(char* const path){
	char* dir = path;
	int i;
	while( *dir!='\0' ){
		if( *dir == ' ' ) ++dir;
		else break;
	}
	struct stat file;
	if( stat(dir, &file)<0 ) {
		fprintf(stderr,"no dir %s\n",dir);
		return;
	}
	chdir(dir);
	getcwd(cwd,200);
}


int is_samefd(int fd1, int fd2){
	struct stat stat1, stat2;
	if(fstat(fd1, &stat1)<0) return -1;
	if(fstat(fd2, &stat2)<0) return -1;
	return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}

void sort_string(char* str[], int n){
	int i,j;
	int max;
	//char tmp[MAX_FILE_NAME] = {0,};
	char* tmp = NULL;
	for(i=0;i<n-1;++i){	
		max = i;
		for(j=i+1; j<n; ++j){
			if( strcmp(str[max], str[j]) > 0 ){
				max = j;
			}
		}
		if( i==max ) continue;
		tmp = str[max];
		str[max] = str[i];
		str[i] = tmp;
	}
}

int _is_in_filename(char c){
	switch(c){
	case' ':case'>': case'<':case'/':case'&':case'|':case'\0': return FALSE;
	default : return TRUE;
	}
}








