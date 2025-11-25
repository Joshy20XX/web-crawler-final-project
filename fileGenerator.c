#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_THREADS 4

//Declare prototype of file name generator.
char *generateFiles(const char *fprefix,const char *fsuffix, int fnumber);

int main()
{
    //Main function shows how to open it and demonstrates how file names can be generated.  In the same vein, 
    // a loop can be made which closes the files which contain that particular indentifying number, also in the range of 0 and 
    // MAX_THREADS -1, using strcmp to determine that it ought to be closed from the file pool, as once the program has completed
    // everything ought to be closed.  

    const char *prefix = "bungity_ungity";
    const char *suffix = ".txt";
    char *fileName;

    //Create the file pool, allowing 20 characters per.  Array of strings which can then be opened and closed at the user's
    //leisure.  
    char filePool[MAX_THREADS][20];

    //For loop which creates a fileName from the given i and then copies it into its repsective position in the file pool.  
    for(int i=0;i<MAX_THREADS;i++)
    {
        //Create all file names, then construct the filePool
        fileName = generateFiles(prefix,suffix,i);
        //printf("%s\n",fileName);
        strcpy(filePool[i],fileName);
    }

    //Sees what's in the file pool, using an array of strings.  Mostly for proof of concept.  
    for(int j=0;j<MAX_THREADS;j++)
    {
        printf("%s\n",filePool[j]);
    }

}

//Actual file generator function.  
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
    //concats the suffix on.
    strcat(fileName,file_suffix);
    //Assign the null pointer at the end of it.  
    fileName[totalLenFileName]='\0';
    //return the name as the result of the function and free afterwards.  
    return fileName;
    free(fileName);
}
