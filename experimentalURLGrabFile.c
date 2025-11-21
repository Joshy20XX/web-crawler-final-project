#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

//create a function which takes the page and then assigns it a queue of all of the article links present within it.

//Create the parser function which then outputs each additional wikipedia page onto a txt file.  
int parseHTML(FILE*); //Prototype

//Main function. 
int main(int argc, char *argv[]) {
    CURL *curl = curl_easy_init();
    FILE *outfile;
    //Open the file
    outfile = fopen("wikipediaDownload.txt","wb");
    if(outfile == NULL)
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
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);


    //perform output actions:
    CURLcode result = curl_easy_perform(curl);
    if(result!=CURLE_OK)
    {
        fprintf(stderr, "download issue: %s\n",curl_easy_strerror(result));

    }
    //DO NOT DELETE.  REWINDS POINTER TO THE BEGINNING, BC IT IS AT THE END AFTER WRITING.  
    //2 HOURS WERE WASTED PRIOR TO ITS DISCOVERY.  YOU HAVE BEEN WARNED.  
    rewind(outfile);

    //Put file into parseHTML
    parseHTML(outfile);

    //cleans up the whole shabang when done.
    curl_easy_cleanup(curl);
    fclose(outfile);
    return EXIT_SUCCESS;
}

int parseHTML(FILE *fileptr)
{
    FILE *links;
    char line[256];
    char *line_ptr = line;
    int capturing =0;

    const char *opening="<a href=\"/wiki/";
    const char *closing="\"";

    links = fopen("allWikipediaLinksinArticle.txt","w+"); //File for all of the links.

    if(fileptr == NULL){printf("Error: You didn't give a file dummy\n");}

    //Loop through the file to get the text just after the marker text: 
    //<a href="/wiki/American_Revolutionary_War" title="American Revolutionary War">Revolutionary War</a>
    //Get it to the before portion of the end.  
    
    while(fgets(line, sizeof(line), fileptr)!=NULL)
    {
        //JUST PROVES YOU CAN ACCESS THE FILE AND RUN THE FIRST LINE.
        //What we first need to do is parse the individual line for what could be in it, then add it to the document.  
        //For example, we need to find the <a href=", then extract what is between the set of quotation marks to get the next url.
        char *opening_ptr = strstr(line,opening);
        if(opening_ptr!=NULL) 
        {
            char *articleNameStart=opening_ptr+strlen(opening); //Creates a pointer to the start of the article name
            char *closing_ptr = strstr(articleNameStart,closing); //Creates a pointer towards where the statement closes
            
            if(closing_ptr!=NULL)
            {
                int articleNameLength = closing_ptr-articleNameStart;
                char *articleName = (char *)malloc(articleNameLength+1);
                
                if(articleName==NULL) //FAIL CONDITION
                {
                    perror("Failed Memory Allocation");
                    fclose(links);
                    return EXIT_FAILURE;
                }
                strncpy(articleName,articleNameStart,articleNameLength);
                articleName[articleNameLength]='\0';

                fprintf(links,"https://en.wikipedia.org/wiki/%s\n",articleName);
                free(articleName);
            }
            //Write it so that it might be able to do multiple links in one line instead of one link per.  
            //Likely would need to create a function to house it.  Probably just keep calling it before the pointer
            //reaches the null termination or when the line is over.  Might work best.
        }
    }
    fclose(links);
}
