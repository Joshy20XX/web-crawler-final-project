#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

//create a function which takes the page and then assigns it a queue of all of the article links present within it.

//Create the parser function which then outputs each additional wikipedia page onto a txt file.  
int parseHTML(FILE*inputFile,FILE*outputFile,char*target_name); //Prototype

//Main function. 
int main(int argc, char *argv[]) {
    CURL *curl = curl_easy_init();
    FILE *fullDataFile;
    //Declarations here. 
    FILE *links;
    //Open the files
    fullDataFile = fopen("wikipediaDownload.txt","w+");
    links = fopen("allWikipediaLinksinArticle.txt","w+"); //File for all of the links.

    if(fullDataFile == NULL)
    {
        fprintf(stderr, "error opening the right file, dummy\n");
        return EXIT_FAILURE;
    }
    
    //Handles case where the url isn't good.  
    if (!curl)
    {
        fprintf(stderr,"init failed\n");
        return EXIT_FAILURE;
    }
    
    if (argc == 1) {
        printf("Usage: %s <starting-url> <ending-url> <depth>\n", argv[0]);
        return 1;
    }

    else if (argc > 4) {
        printf("There's too many arguments! Abort!\n");
        return 1;
    }

    else if (argc == 2) {
        printf("A starting URL was provided without an ending URL or a depth value. Abort!\n");
        return 1;
    }

    else if (argc == 3) {
        printf("No depth value was provided. Abort!\n");
        return 1;
    }

    //set options
    curl_easy_setopt(curl,CURLOPT_URL,argv[1]);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RutgersUniGroupDJZ");
    curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJhdWQiOiJlZjg4NDQxMjk5Yzg0NWU5NTBjN2YwNDc5OGQ2NGQwMyIsImp0aSI6IjkwYmMyOTVhNTNiMzU1MmJmMjNlNmIzYmVlNGE5MWEyY2Q2ZGIzYTdkNTFiZThhZTRlMjExZWFmMWU5NjNjODAwMzcyOTMwMDcyYzNkM2JjIiwiaWF0IjoxNzYzMTYwNzA3LjQ2MTc2OCwibmJmIjoxNzYzMTYwNzA3LjQ2MTc3MSwiZXhwIjozMzMyMDA2OTUwNy40NTg5NTQsInN1YiI6IjgwNTE0NDg3IiwiaXNzIjoiaHR0cHM6Ly9tZXRhLndpa2ltZWRpYS5vcmciLCJyYXRlbGltaXQiOnsicmVxdWVzdHNfcGVyX3VuaXQiOjUwMDAsInVuaXQiOiJIT1VSIn0sInNjb3BlcyI6WyJiYXNpYyJdfQ.pkjZM6VqH9eHWJO4E54X1J2W2N95Se3kkYAPoXiJiROUakyl4TRPO7KAFTd5_CvYrkNtRv2RMh0jZyi-Bdc8MOTjvMZLPUIfo-pd9yNzrCDzrFTovJJ9MkW5qCRVXOD4APcXrrkjQbT_Rnt5y3LcnlqKZJBAFJiOVNz71-SyyMrsqO0o0kV5g6rIrP2WyhO9mT-L7XcitAy58mFFfzrcKBkwDAsj5FA7eJeTvPrAkZUB4l-SR_vfQzWfnGovsCzyATZbK6Z6QxPIC2sVdrQ2vNrxgRVe0d4x4lMwWPDUuaDbW3-8KGRcQ3Qns_BYm2ZiAx4yJIODHqY5ZLwH9-0hFhNadolWqMNBqsbMD7nNXDhrLNO71pih-qaCKjibvFUgADekZ0D-VoDBRtIBAmw4NQvvEgOGxh4z0rJ4tlwM61J-RJj9TOJLNwb7Bi-8bWWJANdQraC7YzELLF3Umh5iYBRNdDSwFkzHF0g2zq_UO6mj1ZbOY6_xL0joOP0m9sf_hO1pZ7OCcZAVM5Gks_7t91GJgME7Ui0iLHCsyF0ieQvqmJ7EmoNv9AvAzNcA2hmRnpNoFLKLpFYcg3qs4LGIAd_GBX9C_sMvdizwUNbQYHaQwMydOhrSVRwih-EmapDR8A26HimnbYOcC30zjvBFxIsusI-HJtf_yExTuIZc6Y");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fullDataFile);

    //Set argv[2] or the target article as a variable, then get rid of the string before it.  
    char *target = (char *)malloc(strlen(argv[2])+1); //Allocates memory to fit length of article name
    target=argv[2];
    target[strlen(argv[2])]='\0';//stores it with delimiter character.  
    size_t target_len = strlen(target); //Grabs size.  

    //Defines prefix, aka, what would be at the front of the article.  
    const char *prefix = "https://en.wikipedia.org/wiki/";
    size_t prefix_len = strlen(prefix); //Grabs size so it isn't called 5 million times.  

    //Eliminates article https prefix from the given name and returns that as the target variable.  
    if(target_len>=prefix_len && strncmp(target,prefix,prefix_len)==0)
    {
        memmove(target, target + prefix_len, target_len-prefix_len+1); //Moves the modified,cmped string to the initial str var.  
    }
    //ERGO, target is now just the target articlename and prepped for comparison in the HTML Parser.  
    //printf("%s\n",target);

    //perform output actions:
    CURLcode result = curl_easy_perform(curl);
    if(result!=CURLE_OK)
    {
        fprintf(stderr, "download issue: %s\n",curl_easy_strerror(result));

    }
    //DO NOT DELETE.  REWINDS POINTER TO THE BEGINNING, BC IT IS AT THE END AFTER WRITING.  
    //2 HOURS WERE WASTED PRIOR TO ITS DISCOVERY.  YOU HAVE BEEN WARNED.  
    rewind(fullDataFile);

    //Put file into parseHTML with the fullDataFile, the links, and the target.  
    parseHTML(fullDataFile,links,target);

    //cleans up the whole shabang when done.
    curl_easy_cleanup(curl);
    fclose(fullDataFile);
    fclose(links);
    return EXIT_SUCCESS;
}

int parseHTML(FILE *fileptr,FILE *outFileptr,char *target_name)
{
    //For the line in the file and file search position. 
    char line[1024];
    char *current_search_position;
    //For the opening and closing strings and pointers to hold the indexes of those strings.  
    const char *opening="<a href=\"/wiki/";
    const char *closing="\"";
    char *opening_ptr, *closing_ptr; //Opening and closing pointers for the location of the string opening and closing.
    //Opens the file for writing
    //Checks if the entered file has nothing to error out.  
    if(fileptr == NULL){printf("Error: You didn't give a file dummy\n");return EXIT_FAILURE;}
    //Loop through the file to get the text just after the marker text: 
    //<a href="/wiki/American_Revolutionary_War" title="American Revolutionary War">Revolutionary War</a>
    //Marker text (opening) is present at the head of the article, then grab the section between then and the ".
    while(fgets(line, sizeof(line), fileptr)!=NULL)
    {
        //moves the search position in the file to the beginning of the line.  
        current_search_position = line;

        //Make a second while loop to comb through the whole line as to not miss any. 
        while((opening_ptr = strstr(current_search_position,opening))!=NULL)
        {
            current_search_position = opening_ptr +strlen(opening);
            if((closing_ptr = strstr(current_search_position,closing))!=NULL)
            {
                int articleNameLength = closing_ptr-current_search_position; //Finds the nameLength
                char *articleName = (char *)malloc(articleNameLength+1); //Allocates memory to fit length of article name

                strncpy(articleName,current_search_position,articleNameLength); //Copies the string value of name to ArticleName.
                articleName[articleNameLength]='\0';//Adds null character to the name.  

                current_search_position=closing_ptr+strlen(closing); //Moves the position of the linereader
                fprintf(outFileptr,"https://en.wikipedia.org/wiki/%s\n",articleName); //prints article name to the file

                //Check if the article name matches the article name of the target using strcmp.  Exits if success, should 
                //work well as a general condition.   
                if(strcmp(articleName,target_name)==0)
                {
                    printf("FOUND YOU!!!!\n");
                    return EXIT_SUCCESS;
                }

                free(articleName); //then frees the memory space.  
            }  
        }
    }
}
