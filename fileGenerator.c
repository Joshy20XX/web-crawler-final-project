#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_THREADS 4


char *generateFiles(const char *file_prefix, const char *file_suffix, int file_num)
{
    //get the size of the total file string including the prefix and the suffix.  
    size_t totalLenFileName = (strlen(file_prefix)+(sizeof(char)*sizeof(int))+strlen(file_suffix)+1);

    //allocate memory for the string.  
    char *fileName = (char*)malloc(totalLenFileName*sizeof(char));

    //Copies the prefix into fileName
    strcpy(fileName,file_prefix);

    //uses snprintf to concatenate the integer fileNum into the existing filename
    snprintf(fileName+strlen(fileName), sizeof(fileName)-strlen(fileName),"%d",file_num);

    strcat(fileName,file_suffix);

    //Assign the null pointer at the end of it.  
    //printf("%i\n",totalLenFileName);
    return fileName;
    free(fileName);
}

int main()
{
    //Main function shows how to open it
    
    const char *prefix = "bungity_ungity";
    const char *suffix = ".txt";
    char *fileName;
    //fileName = generateFiles(prefix,suffix,MAX_THREADS);


    for(int i=0;i<MAX_THREADS;i++)
    {
        fileName = generateFiles(prefix,suffix,i);
        printf("%s\n",fileName);
    }
}
