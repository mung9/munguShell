#ifndef __MAIN__
#define __MAIN__

#include "shell.h"

static	int input_cur=0;
static	int cmd_length=0;
static	char command[128]={0,};

void sigint_handler(int signo){
	//
	set_text(TEXT_BOLD);
	printf("\nEnter 'q' to quit\n");
	set_text(TEXT_DEFAULT);
	print_shell_input_line();
	input_cur = cmd_length;
	printf("%s",command);
	fflush(stdout);
}


//hangeuli an doi yo
/*********** main **************/
int main(){
	int c,i;
	int input_cur=0;
	int is_tabbed = FALSE;
	sigset_t set;
	cmd_log log;
	init_log(&log);
	
	init(); 
	pthread_t thread_reaper;
	pthread_create(&thread_reaper, NULL, zombie_cleaner, NULL); // defunct process reaper
	
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	pthread_sigmask(SIG_BLOCK, &set, NULL); // main thread will not receive SIGCHLD
	signal(SIGINT, sigint_handler); // prevent process from being terminated by signals (SIGINT, SIGQUIT)
	signal(SIGQUIT, sigint_handler);

		

	while(1){
		memset(command, '\0', cmd_length);
		cmd_length = 0;
		input_cur = 0;
		reset_log_cursor(&log);
		print_shell_input_line();
		while( (c=getch(0)) != '\n' ){
			// tab once
			if( c=='\t'){
				int cursor = cmd_length-1;
				char path[80];
				int index = 0;
				char srcdir[MAX_FILE_NAME] = {0,};
				char src[MAX_FILE_NAME] = {0,};
				char src_dir_tmp[30] = {0,};

				struct dirent* direntp = NULL;
				DIR *dirp = NULL;

				char* file_list[1000] = {0,};
				int nfile = 0;
				//
				if( command[0] == 0 ) continue;

				cursor = cmd_length-1;
				if(command[cursor] == ' ')
					parse_path("./", srcdir, src);
				else{
					while(1){
						if(command[cursor] == ' '){
							parse_path(&command[cursor+1], srcdir, src);
							break;
						}
						--cursor;
						if(cursor<0){
							parse_path(&command[0], srcdir, src);
							break;
						}
					}
				}
				nfile = files_start_with( src, srcdir, file_list, MAX_FILE_NUM);
				if( is_tabbed == FALSE){ // tab once -> auto completion
					is_tabbed = TRUE;
					char common[20] = {0,};
					auto_completion(file_list, common, nfile);
					if( common != NULL ){
						if( strlen(common) > strlen(src) ){
							char *addend = common+strlen(src);
							printf("%s",addend);
							strcat(command, addend);
							cmd_length += strlen(addend);
							input_cur += strlen(addend);
							command[cmd_length] = '\0';
						}
					}						
					else{
						fprintf(stderr,"no common\n");
					}
				}
				else{ // tab twice -> print file list
					char tmp[50];
					getcwd(tmp,50),
					chdir(srcdir);
					printf("\n");
					sort_string(file_list,nfile);
					print_pretty(file_list,nfile);
					//print_file_list(file_list,nfile);
					chdir(tmp);
					print_shell_input_line();
					input_cur = cmd_length;
					printf("%s", command);
				}

				for( i=0; i<nfile; ++i ){
					free(file_list[nfile]);
				}
				continue;
			}
			is_tabbed = FALSE;

			// backspace
			if( c == VK_BACKSPACE ){
				cmd_length = erase_a_character(command, &input_cur);
				continue;
			}
			// handling function key
			if( c == VK_FNC1 && getch(0) == VK_FNC2 ){
				c=getch(0);
				switch(c){
				case VK_UP: 
					//up
					
					cmd_length = replace_cmd(
							command,
							prev_cmd_or_null(&log),
							&input_cur);
					continue;
				case VK_DOWN: 
					//down
					cmd_length = replace_cmd(
							command,
							next_cmd_or_null(&log),
							&input_cur);
					continue;
				case VK_LEFT://left
					move_cursor_left(command,&input_cur);
					continue;
				case VK_RIGHT://right
					move_cursor_right(command,&input_cur);
					continue;
				case VK_DELETE://delete
					if(getch(0)!=126) break;
					if( is_cursor_end(command,&input_cur) ) continue;
					move_cursor_right(command,&input_cur);
					cmd_length = erase_a_character(command,&input_cur);
				default:
					continue;
				}
			}
			//
			cmd_length = insert_character(c,command,&input_cur);
		}
		fputs("\n",stdout);
		command[cmd_length]='\0';
		// quit command : "q"
		if((strcmp(command,"q")==0)||(strcmp(command,"Q")==0)){
			set_text(TEXT_COLOR_RED);
			set_text(TEXT_BOLD);
			printf("** Good night! **\n");
			fflush(stdout);
			break;
		}
		// my shell provides "cd" command
		else if(strncmp(command,"cd ",3)==0) cd(command+3);
		// calls the "execute" function that analyzes command and make it execute
		else if(execute(command,STDIN_FILENO, STDOUT_FILENO, STDOUT_FILENO) == EMPTY){
			continue;
		}
		// push execute command to stack that stores commands ran before
		push_cmd(command,&log);
		// repeat
	}
	return 0;
}





#endif
