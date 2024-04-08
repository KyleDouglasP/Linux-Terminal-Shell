#define BUFFER_SIZE 200
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int exit_status = 1;

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
            list->capacity *= 2;
            char **temp = realloc(list->string_array, list->capacity * sizeof(char*));
            if(temp==NULL){
                fprintf(stderr, "Out of memory!\n");
                exit(EXIT_FAILURE);
            }
            list->string_array = temp;
        }

        list->string_array[list->length];
        list->length++;

    }

    void arraylist_format_array(arraylist_t *list){
        list->capacity=list->length;
        list->string_array = realloc(list->string_array, list->capacity *sizeof(char*));
    }

    void arraylist_destroy(arraylist_t *list){
        free(list->string_array);
    }

typedef struct program{
    
    char **argument_list;
    char *input;
    char *output;
    struct program *pipe;

} program_t;

/// @brief Tokenizes given command line
arraylist_t tokenize_command(char *command_line){

    arraylist_t token_list;
    arraylist_init(&token_list, 2);

    char temp_string[50];
	int temp_string_index = 0;

    char ch=command_line[temp_string_index];
    bool prev_whitespace = true;

    while(ch!='\n'){

        if(ch==' '){
            if(!prev_whitespace){
                arraylist_add(&token_list, temp_string);
                temp_string_index=0;
            }
            prev_whitespace=true;
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
            continue;

        } 

        temp_string[temp_string_index] = ch;
        temp_string[temp_string_index+1] = '\0';
        temp_string_index++;
        ch = command_line[temp_string_index];
        prev_whitespace=false;

    }

    return token_list;

    // program_t new_program;
    // arraylist_init(&(new_program.argument_list), 2);

    // char temp_string[50];
	// int temp_string_index = 0;

    // char ch=command_line[temp_string_index];

    // while(ch!='\n'){

    //     temp_string[temp_string_index] = ch;
    //     temp_string[temp_string_index+1] = '\0';
    //     temp_string_index++;
    //     ch = command_line[temp_string_index];
    // }

    // arraylist_add(&(new_program.argument_list), temp_string);
    
    
}

/// @brief Interprets command line, making it into a program struct
void interpret_command(char *command_line){
    arraylist_t token_list = tokenize_command(command_line);
    for(int i=0; i<token_list.length; i++){
        printf("%s\n", token_list.string_array[i]);
    }

}

/// @brief Interprets given program
void interpret_program(char *command_line){

    
    char *program_name = strtok(command_line, " \n");

    if(strcmp(program_name, "cd")==0){

        char *new_path = strtok(NULL, " \n");
        if(chdir(new_path)==-1) printf("Cannot find target directory!\n");
        return;

    } else if(strcmp(program_name, "pwd")==0){

        char path_buffer[BUFFER_SIZE];
        getcwd(path_buffer, BUFFER_SIZE);
        printf("%s\n", path_buffer);
        return;

    } else if(strcmp(program_name, "which")==0){

        char *potential_file = strtok(NULL, " \n");
        potential_file = (find_file(potential_file));
        printf("%s\n", potential_file);
        free(potential_file);
        return;

    } else if(strcmp(program_name, "exit")==0){

        printf("Exiting my shell.\n");
        exit(EXIT_SUCCESS);

    }

    if(program_name[0]=='/'){
        
    } else {
        program_name = (find_file(program_name));
        if(strcmp(program_name, "Failed to find target file!") == 0){
            printf("%s\n", program_name);
            free(program_name);
            return;
        }

    }

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

    int p[2];
    pipe(p);

    // if(isatty(p[0])==1){
    if(argc==1){
        run_interactive();
    } else if (argc==2){
        run_batch(argv[1]);
    } else {
        printf("Too many arguments supplied!");
    }

}