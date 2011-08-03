#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <files.h>

const char* readFile(const char* filename) {
    struct stat st;
    stat(filename, &st);
    int size = st.st_size;
    char* content = (char*) malloc((size+1)*sizeof(char));
    content[size] = 0;
	// we need to read as binary, not text, otherwise we are screwed on Windows
    FILE *file = fopen(filename, "rb"); 
    fread(content, 1, size, file);
    return content;
}
