#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

//create a function which takes the page and then assigns it a queue of all of the article links present within it.


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

    //cleans up the whole shabang when done.
    curl_easy_cleanup(curl);
    fclose(outfile);
    return EXIT_SUCCESS;
}
