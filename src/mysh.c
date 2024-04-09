#define BUFFER_SIZE 200
#define DEFAULT_MODE 0
#define PIPE_MODE 1
#define PIPED_MODE 2
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

static int exit_status = EXIT_SUCCESS;
static int base_pipe[2];

/// @brief Searches for given file name in sample directories, and returns correct filepath if found
char *find_file(char *file_name){

    char *test = malloc(200);

    strcpy(test, "/usr/local/bin/");
    strcat(test,file_name);
    if(access(test, F_OK)==0) return test;

    strcpy(test, "/usr/bin/");
    strcat(test,file_name);
    if(access(test, F_OK)==0) return test;

    strcpy(test, "/bin/");
    strcat(test,file_name);
    if(access(test, F_OK)==0) return test;

    // Also checks the working directory
    char path_buffer[BUFFER_SIZE];
    strcpy(test, getcwd(path_buffer, BUFFER_SIZE));
    strcat(test,"/");
    strcat(test,file_name);
    if(access(test, F_OK)==0) return test;

    strcpy(test, "Failed to find target file!");
    return(test);

}

typedef struct arraylist{
    
    char **string_array;
    int length;
    int capacity;

} arraylist_t;

    void arraylist_init(arraylist_t *new, int size){
        new->capacity=size;
        new->length=0;
        new->string_array = malloc(size * sizeof(char*));
    }

    void arraylist_add(arraylist_t *list, char* add){

        if(list->length == list->capacity){
            list->capacity = list->capacity*2;
            char **temp = realloc(list->string_array, list->capacity * sizeof(char*));
            if(temp==NULL){
                fprintf(stderr, "Out of memory!\n");
                exit(EXIT_FAILURE);
            }
            list->string_array = temp;
        }

        if(add==NULL){
            list->string_array[list->length] = NULL;
        } else {
            list->string_array[list->length] = strdup(add);
        }
        list->length=list->length+1;

    }

    void arraylist_format_array(arraylist_t *list){
        list->capacity=list->length;
        list->string_array = realloc(list->string_array, list->capacity *sizeof(char*));
    }

    void arraylist_destroy(arraylist_t *list){
        for(int i=0; i<list->length; i++){
            free(list->string_array[i]);
        }
        free(list->string_array);
    }

    void arraylist_print(arraylist_t *list){
        for(int i=0; i<list->length; i++){
            printf("%s\n", list->string_array[i]);
        }
    }

    /// @brief Returns the array after reallocating
    char **arraylist_get_array(arraylist_t *list){
        arraylist_format_array(list);
        return list->string_array;
    }

typedef struct program{
    
    char **argv;
    int argc;
    char *input;
    bool input_set;
    char *output;
    bool output_set;
    struct program *pipe;
    bool pipe_set;

} program_t;

/// @brief Tokenizes given command line
arraylist_t tokenize_command(char *command_line){

    arraylist_t token_list;
    arraylist_init(&token_list, 2);

    char temp_string[50];
	int temp_string_index = 0;

    int command_line_index = 0;

    char ch=command_line[command_line_index];
    bool prev_whitespace = true;

    while(1){

        if(ch==' '){
            if(!prev_whitespace){
                arraylist_add(&token_list, temp_string);
                temp_string_index=0;
            }
            
            prev_whitespace=true;
            command_line_index++;
            ch = command_line[command_line_index];

            continue;
        }

        if(ch=='<' || ch=='>' || ch=='|'){
            
            if(!prev_whitespace){
                arraylist_add(&token_list, temp_string);
                temp_string_index=0;
            }

            char str[2] = "\0";
            str[0]=ch;
            arraylist_add(&token_list, str);
            prev_whitespace = true;

            command_line_index++;
            ch = command_line[command_line_index];
            
            continue;

        }

        if(ch=='\n'){
            if(!prev_whitespace){
                arraylist_add(&token_list, temp_string);
                temp_string_index=0;
            }
            break;
        }

        temp_string[temp_string_index] = ch;
        temp_string[temp_string_index+1] = '\0';
        temp_string_index++;
        command_line_index++;
        ch = command_line[command_line_index];
        prev_whitespace=false;

    }

    return token_list;
    
}

/// @brief Interprets given program
void interpret_program(program_t process, int mode, int fd[]){

    int pid = fork();

    if(pid==-1) return;
    if(pid==0){

        int in_fd = STDIN_FILENO;
        int out_fd = STDOUT_FILENO;

        if(mode==PIPE_MODE){
            dup2(fd[1], 1);
		    close(fd[0]);
        }

        if(mode==PIPED_MODE){
            dup2(fd[0], 0);
            close(fd[1]);
        }

        if(mode==DEFAULT_MODE || mode==PIPE_MODE){
            if(process.input_set){
                in_fd = open(process.input, O_RDONLY);
                if(in_fd==-1){
                    printf("Could not find input file: %s\n", process.input);
                    exit_status=EXIT_FAILURE;
                    return;
                }
                dup2(in_fd, STDIN_FILENO);
            }
        }

        if(mode==DEFAULT_MODE || mode==PIPED_MODE){
            if(process.output_set){
                out_fd = open(process.output, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                dup2(out_fd, STDOUT_FILENO);
            }
        }

    } else {
        int wait_status;
        pid = wait(&wait_status);
        return;
    }

    char *program_name = process.argv[0];

    if(strcmp(program_name, "cd")==0){

        if(process.argc<2){
            printf("cd: Please specify target directory!\n");
            exit_status=EXIT_FAILURE;
            return;
        }
        if(process.argc>2){
            printf("cd: Too many arguments!\n");
            exit_status=EXIT_FAILURE;
            return;
        }
        char *new_path = process.argv[1];
        if(chdir(new_path)==-1) {
            printf("cd: Cannot find target directory!\n");
            exit_status=EXIT_FAILURE;
        }
        exit_status=EXIT_SUCCESS;
        return;

    } else if(strcmp(program_name, "pwd")==0){

        if(process.argc>1){
            printf("pwd: Too many arguments!\n");
            exit_status=EXIT_FAILURE;
            return;
        }
        char path_buffer[BUFFER_SIZE];
        getcwd(path_buffer, BUFFER_SIZE);
        printf("%s\n", path_buffer);
        return;

    } else if(strcmp(program_name, "which")==0){

        if(process.argc<2){
            printf("which: Please specify target file!\n");
            exit_status=EXIT_FAILURE;
            return;
        }
        if(process.argc>2){
            printf("which: Too many arguments!\n");
            exit_status=EXIT_FAILURE;
            return;
        }
        char *potential_file = process.argv[1];
        potential_file = (find_file(potential_file));
        printf("%s\n", potential_file);
        free(potential_file);
        return;

    } else if(strcmp(program_name, "exit")==0){

        for(int i=1; i<process.argc; i++){
            printf("%s ", process.argv[i]);
            if(i==process.argc-1) printf("\n");
        }
        printf("Exiting my shell.\n");
        exit_status=EXIT_SUCCESS;
        exit(EXIT_SUCCESS);

    }

    if(!(program_name[0]=='/')){
        program_name = (find_file(program_name));
        if(strcmp(program_name, "Failed to find target file!") == 0){
            printf("%s\n", program_name);
            free(program_name);
            exit_status=EXIT_FAILURE;
            return;
        }
        free(process.argv[0]);
        process.argv[0]=program_name;
    }


    execv(program_name, process.argv);
    exit(EXIT_FAILURE);


    int wait_status;
    pid = wait(&wait_status);

}

/// @brief Used to make sure pipes are piped correctly and using two separate processes, and waits for them
void pipe_helper(program_t process){

    int pid, status;
	int fd[2];

	pipe(fd);

    interpret_program(process, PIPE_MODE, fd);
    interpret_program(*(process.pipe), PIPED_MODE, fd);

    while ((pid = wait(&status)) != -1);
		
	return;

}

/// @brief Interprets command line, making it into a program struct
void interpret_command(char *command_line){

    arraylist_t token_list = tokenize_command(command_line);

    program_t process;    

    program_t *process_pointer = &process;
    (*process_pointer).input_set=false;
    (*process_pointer).output_set=false;
    (*process_pointer).pipe_set=false;

    program_t process2;
    process2.input_set=false;
    process2.output_set=false;
    process2.pipe_set=false;

    (*process_pointer).pipe=&process2;

    arraylist_t argument_list;
    arraylist_init(&argument_list, 2);

    for(int i=0; i<token_list.length; i++){

        // User wants to change the input of the process
        if(strcmp(token_list.string_array[i], "<")==0){

            if(!(*process_pointer).input_set){
                if(i+1<token_list.length){
                    (*process_pointer).input = token_list.string_array[i+1];
                    i++;
                    (*process_pointer).input_set=true;
                    continue;
                } else {
                    printf("Missing input file!\n");
                    exit_status=EXIT_FAILURE;
                    return;
                }
            } else {
                printf("Can only set one input!\n");
                exit_status=EXIT_FAILURE;
                return;
            }

        }

        // User wants to change the output of the process
        if(strcmp(token_list.string_array[i], ">")==0){

            if(!(*process_pointer).output_set){
                if(i+1<token_list.length){
                    (*process_pointer).output = token_list.string_array[i+1];
                    i++;
                    (*process_pointer).output_set=true;
                    continue;
                } else {
                    printf("Missing output file!\n");
                    exit_status=EXIT_FAILURE;
                    return;
                }
            } else {
                printf("Can only set one output!\n");
                exit_status=EXIT_FAILURE;
                return;
            }

        }

        // User wants to pipe to the next process
        // User wants to change the input of the process
        if(strcmp(token_list.string_array[i], "|")==0){

            (*process_pointer).pipe_set = true;

            (*process_pointer).argc = argument_list.length;
            arraylist_add(&argument_list, NULL);
            (*process_pointer).argv = arraylist_get_array(&argument_list);

            arraylist_init(&argument_list, 2);
            
            process_pointer=&process2;

            continue;


        }

        arraylist_add(&argument_list, token_list.string_array[i]);

    }

    (*process_pointer).argc = argument_list.length;
    arraylist_add(&argument_list, NULL);
    (*process_pointer).argv = arraylist_get_array(&argument_list);

    if(process.pipe_set){
        pipe_helper(process);
        return;
    }
    
    int fd[2];
    pipe(fd);
    interpret_program(process, DEFAULT_MODE, fd);

}

/// @brief Launches interactive mode
void run_interactive()
{
    printf("Welcome to my shell!\n");
    printf("mysh> ");
    fflush(stdout);

    char term_buffer[BUFFER_SIZE];

    while(read(0, term_buffer, BUFFER_SIZE)){
        interpret_command(term_buffer);
        printf("mysh> ");
        fflush(stdout);
    }

}

/// @brief Launches batch mode, with supplied filepath argument
void run_batch(const char* filepath)
{

    int fd = open(filepath, O_RDONLY);

}

int main(int argc, const char* argv[])
{

    pipe(base_pipe);

    // if(isatty(p[0])==1){
    if(argc==1){
        run_interactive();
    } else if (argc==2){
        run_batch(argv[1]);
    } else {
        printf("Too many arguments supplied!");
    }

}