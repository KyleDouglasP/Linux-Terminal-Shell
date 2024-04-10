#include <stdio.h>

int main(int argc, const char* argv[]){
    if(argc!=2){
        printf("Please supply a name!\n");
    } else {
        printf("Hello, %s!\n", argv[1]);
    }
}