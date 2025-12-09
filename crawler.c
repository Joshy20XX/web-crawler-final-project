#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdbool.h>

#define NUM_THREADS 4 

//mutex locks keep more than 1 thread of execution from executing a given function at any point in time, until that function within the cur
//thread is finished. 

//URL Queue; has link to next and to parent, with an integer for the storage of depth.  
typedef struct URLQueueNode {
    char *url; //the current url
    struct URLQueueNode *parent; //for parent url
    int depth; //necessary for depth tracking
    struct URLQueueNode *next; //Points to the next node in the queue
} URLQueueNode;

// Define a structure for a thread-safe queue but keep it single-threaded until further notice
typedef struct {
    URLQueueNode *head, *tail;
    pthread_mutex_t lock;
    bool enqueueGood; //Control flag 
} URLQueue;

//For global var that says if it finds it:
int foundYou=0;
pthread_mutex_t foundMutex = PTHREAD_MUTEX_INITIALIZER; //initializes mutex.  

// Initialize a URL queue. Right now it's single-threaded until further notice
void initQueue(URLQueue *queue) {
    queue->head = queue->tail = NULL;
    pthread_mutex_init(&queue->lock, NULL);
    queue->enqueueGood = true;
}

// Add a URL to the queue.
bool enqueue(URLQueue *queue, const char *url, URLQueueNode *parent, int depth) {
    if(!queue->enqueueGood)
    {
        return false;
    }
    URLQueueNode *newNode = malloc(sizeof(URLQueueNode));
    newNode->url = strdup(url); //Adds for url data
    newNode->next = NULL; //Adds for the pointer to next.  
    newNode->parent = parent; //Adds parent funct.
    newNode->depth = depth; //Adds depth functionality.

    pthread_mutex_lock(&queue->lock);
    if (queue->tail) {
        queue->tail->next = newNode;
    } else {
        queue->head = newNode;
    }
    queue->tail = newNode;
    pthread_mutex_unlock(&queue->lock);
    return true;
}

//BEGIN MUTEXES
// Remove a URL from the queue.
URLQueueNode *dequeue(URLQueue *queue) {
    pthread_mutex_lock(&queue->lock); //Mutex Lock.  Keeps 2 threads from dequeueing at the same time.  
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    URLQueueNode *temp = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    pthread_mutex_unlock(&queue->lock); //Mutex Unlock
    return temp;
    free(temp);
}

//Destroy the queue and free its memory once everything is done.
void destroyQueue(URLQueue *queue) {    
    //printf("Destroying Queue!\n");      NOT NECESSARY TO SAY THIS.  
    pthread_mutex_lock(&queue->lock);         

    URLQueueNode *cur = queue->head;          
    while (cur) {                              
        URLQueueNode *next = cur->next;       
        free(cur->url);                       
        free(cur);           
        //printf("Node deleted");                 
        cur = next;                           
    }                                         

    queue->head = queue->tail = NULL;        
    pthread_mutex_unlock(&queue->lock);       
    pthread_mutex_destroy(&queue->lock);      
}    

//For debugging purposes, print what is in the URL queue.  PRINTING THIS PROVES THAT IT DOES STORE IT PROPERLY.
void printQueue(URLQueue *queue) {
    pthread_mutex_lock(&queue->lock);    
    URLQueueNode *cur = queue->head;

    printf("Printing Queue: \n");
    while (cur != NULL) {
        URLQueueNode *next = cur->next;  
        printf("%d %s %s", cur->depth, cur->url, cur->parent->url);
        cur = next;
        printf("\n");
    }
    pthread_mutex_unlock(&queue->lock);
}

//Custom empty function.  ret 1 if queue empty; 0 if not-empty.
int queueEmpty(URLQueue *queue)
{
    pthread_mutex_lock(&queue->lock); 
    int empty = (queue->head == NULL);
    pthread_mutex_unlock(&queue->lock);
    return empty;
}

//Gets the tail Node value from the queue for use in multithreaded mode.  
URLQueueNode *getTailValue(URLQueue *queue)
{
    URLQueueNode *temp = queue->tail;
    return temp;
    free(temp);
}

//closes the queue when necessary.  
void close_queue(URLQueue *queue)
{
    queue->enqueueGood = false;
}

//Create the parser function which then outputs each additional wikipedia page onto a txt file.  
int parseHTML(FILE *inputFile, FILE *outputFile, char *start_name, char *target_name); //Prototype

int grabURL(char *start_link, char *target_link,FILE *URL_outputFile, FILE *links_file); //Prototype

int addToQueue(FILE *links_file, URLQueue *queue, URLQueueNode *parent, int depth); //Just adds the contents of a file to the queue.  

//Prototype for reprinting the taken Path.
void printLineage(URLQueueNode *URLNode); //Prototype

//workerargs structure.  
typedef struct {
    URLQueue *queue;                                       
    char *targetURL;                                       
    int maxDepth;
} WorkerArgs;

//WorkerFunc handles most of the legwork of a given thread (Mimics loop for single thread.)
void *workerFunc(void *arg) {
    WorkerArgs *wargs = (WorkerArgs *)arg;                 
 
    // Each thread gets its own temp files                   
    FILE *htmlFile = tmpfile();                            
    FILE *linksFile = tmpfile();
    if (!htmlFile || !linksFile) {
        perror("tmpfile");                                 
        if (htmlFile) fclose(htmlFile);                    
        if (linksFile) fclose(linksFile);                  
        return NULL;                                       
    }                                                 
    
    //main loop, similar to the old loop for the single-threaded.  
    while (1) {                                            
        //def curpage as the one being evalled.  
        URLQueueNode *curPage = dequeue(wargs->queue);     
        if (!curPage) {                                    
            usleep(10000); //Picks next node in the queue if there isn't a value at that point                
            continue;                                      
        }                                    
        

        if (curPage->depth < wargs->maxDepth) {            
            printf("Thread %lu processing: %s (Parent: %s, depth=%d)\n", 
                    (unsigned long)pthread_self(),          
                    curPage->url,                           
                    curPage->parent ? curPage->parent->url : "None", 
                    curPage->depth);                        
  
            // Download + parse
            grabURL(curPage->url, wargs->targetURL, htmlFile, linksFile); 
            // Always enqueue children; target will be among them if found
            addToQueue(linksFile, wargs->queue, curPage, curPage->depth+1); 
            
            // Check if target was found by parseHTML
            pthread_mutex_lock(&foundMutex);               
            int found = foundYou;                          
            pthread_mutex_unlock(&foundMutex);             
            
            if (found) {
                //Find the node in the queue whose URL matches the target URL
                pthread_mutex_lock(&wargs->queue->lock);
                close_queue(wargs->queue); //Close the queue when a solution is found.
                
                URLQueueNode *n = getTailValue(wargs->queue);  //Queue head is the top of it.  Not the tail.  
                URLQueueNode *targetNode = NULL;
                                
                pthread_mutex_unlock(&wargs->queue->lock);
                
                if (targetNode) {
                    printf("Processing: %s (Parent: %s)\n",targetNode->url,targetNode->parent ? targetNode->parent->url : "None");
                }
                // Once one thread has printed the path, this thread can exit
                break;
            }
        }                                                  
    }                                                      
  
    fclose(htmlFile);                                      
    fclose(linksFile);                                     
    return NULL;                                           
}
// Run the original single-threaded crawl.  Mostly for show.  
void run_single_crawl(URLQueue *queue, char *targetURL, int maxDepth, FILE *fullDataFile, FILE *links) {
    URLQueueNode *curPage;

    // Initial dequeue + process (your original code)
    if ((curPage = dequeue(queue)) != NULL) {
        printf("Processing: %s (Parent: %s)\n",
               curPage->url,
               curPage->parent ? curPage->parent->url : "None");
        grabURL(curPage->url, targetURL, fullDataFile, links);
        addToQueue(links, queue, curPage, curPage->depth + 1);
    }

    // Original while loop
    while (foundYou != 1 && queueEmpty(queue) == 0) {
        if ((curPage = dequeue(queue)) != NULL) {
            if (curPage->depth < maxDepth) {
                printf("Processing: %s (Parent: %s)\n",
                       curPage->url,
                       curPage->parent ? curPage->parent->url : "None");
                grabURL(curPage->url, targetURL, fullDataFile, links);
                addToQueue(links, queue, curPage, curPage->depth + 1);
            } else {
                continue;
            }
        }
    }
    //Add condition for being found.  


}

// Run the multithreaded crawl using workerFunc + pthreads
void run_multi_crawl(URLQueue *queue, char *targetURL, int maxDepth) {
    WorkerArgs wargs;
    wargs.queue = queue;
    wargs.targetURL = targetURL;
    wargs.maxDepth = maxDepth;

    //creates threads.  
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, workerFunc, &wargs);
    }

    //joins threads when done
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

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
    
    //Handles case where the url isn't good.  Checks arguments.  
    if (!curl)
    {
        fprintf(stderr,"init failed\n");
        return EXIT_FAILURE;
    }
    
    if (argc == 1) {
        printf("Usage: %s <starting-url> <ending-url> <depth>\n" 
            "NOTE: If any URL contains parentheses, enclose them with quotation marks. Example: 'URL/()/'\n", argv[0]);
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

    if(strstr(argv[1],"https://en.wikipedia.org/wiki/")==NULL)
    {
        printf("Please make sure your starter URL is properly formatted.\n");
        return EXIT_FAILURE;
    }
    else if (strstr(argv[2],"https://en.wikipedia.org/wiki/")==NULL)
    {
        printf("Please make sure your target URL is properly formatted.\n");
        return EXIT_FAILURE;
    }
    
    //ALL QUEUEING STUFF GOES HERE
    URLQueue queue;

    int maxDepth = atoi(argv[3]);

    //Found an edge case. If the user specifies a depth <= 0, the program gets trapped in an infinite loop
    if (maxDepth <= 0) {
        printf("The specified max depth is too low. Try a depth of 1 or higher.\n");
        return EXIT_FAILURE;
    }

    int startingDepth = 0;
    
    //Initialize the Queue
    initQueue(&queue);

    //gets a char of the whole initial URL for the sense of adding it to the queue.  
    char *initialURL = (char *)malloc(strlen(argv[1])+1);
    initialURL=argv[1];
    initialURL[strlen(argv[1])]='\0';
    
    //Enqueue the url into the queue.  
    enqueue(&queue, initialURL, NULL, startingDepth);

    // Choose mode: 0 = single-threaded, 1 = multithreaded.  BOTH WORKING!
    int use_multithreading = 1;

    if (use_multithreading) {
        run_multi_crawl(&queue, argv[2], maxDepth);
    } else {
        run_single_crawl(&queue, argv[2], maxDepth, fullDataFile, links);
    }

    URLQueueNode *curPage;

    if(foundYou==1)
    {
        curPage=queue.tail;
        //printf("Processing: %s (Parent: %s)\n", curPage->url, curPage->parent ? curPage->parent->url : "None"); //For target found. 
        //Then create a function for reprinting the path taken.  ONLY WHEN FOUND.
        printLineage(curPage);
    }
    if(foundYou==0)
    {
        printf("Failed to find link at depth.  Suggested to increase depth allowance.\n");
        destroyQueue(&queue);
        fclose(links);
        fclose(fullDataFile);
        return EXIT_FAILURE;
    }

    //CLEANUP
    destroyQueue(&queue);
    fclose(links); //Closes the links file at the very end of the enqueue loop.  
    fclose(fullDataFile);
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;
}

//Parses HTML of a doc
int parseHTML(FILE *fileptr,FILE *outFileptr,char *current_name,char *target_name)
{
    //For the line in the file and file search position. 
    char line[1024];
    char *current_search_position;
    //For the opening and closing strings and pointers to hold the indexes of those strings.  
    const char *opening="<a href=\"/wiki/";
    const char *closing="\"";
    char *opening_ptr, *closing_ptr; //Opening and closing pointers for the location of the string opening and closing.

    //Rewrite so it returns an array

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


                //Contain in an if statement; make all failure points for common files; i.e. if it contains.  
                if(strstr(articleName,"File:")==NULL && strstr(articleName,"Category:")==NULL && strstr(articleName,"Help:")==NULL && strstr(articleName,"Wikipedia:")==NULL && strstr(articleName,"Talk:")==NULL && strstr(articleName,"Special:")==NULL && strstr(articleName,"Template:")==NULL && strstr(articleName,"Portal:")==NULL && strstr(articleName, "Main_Page")==NULL)
                {
                    //Check if the article contains itself before adding.  If even contains itself, more likely to loop i.e. bad news.  
                    //ALL of this is for the sake of making it a shit ton easier to traverse.  
                    if(strstr(articleName, current_name)==NULL)
                    {
                        fprintf(outFileptr,"https://en.wikipedia.org/wiki/%s\n",articleName); //prints article name to the file
                        
                        //Add them to the queue
                    }
                }

                //Check if the article name matches the article name of the target using strcmp.  Exits if success, should 
                //work well as a general condition.   
                if(strcmp(articleName,target_name)==0)
                {
                    printf("FOUND YOU!!!!\n");
                    pthread_mutex_lock(&foundMutex);
                    foundYou = 1;
                    pthread_mutex_unlock(&foundMutex);
                    return EXIT_SUCCESS;
                }

                free(articleName); //then frees the memory space.  
            }  
        }
    }
    return EXIT_SUCCESS;
}

//Grabs the URL; calls parseHTML
int grabURL(char *start_link, char *target_link, FILE *HTML_outputFile, FILE *links_file)
{
    //NOTE:  In a multithreaded environment, grabURL should be given thread-specific HTML output file and thread specific links
    //       file.  This is because each thread operates in serial, but all n threads work in parallel; if they share only one
    //       file pair, then they are not gonna work properly and likely will write over one another whenever convenient, which
    //       can be called an absolute and utter failure on my part to plan.  That being said, when done in serial and with a 
    //       queue for the links within the article, there should be no concern as to whether or not the file is overwritten,
    //       as the links data will be stored within the queue (with blocks made to fit specific lengths or something to that
    //       extent).  The point is, this function is made to be messed with, but I recommend the inputs remain the same.
    
    //Takes arguments of a starting link, a target link, and then an HTML output file and a file which will use parseHTML to 
    //output all links to other articles present within that article to that file.  Queueing can then be done from there
    CURL *curl = curl_easy_init();

    //Checks if curl isn't working, in which case it fails on initialization.  
    if (!curl)
    {
        fprintf(stderr,"init failed\n");
        return EXIT_FAILURE;
    }

    //Set options for CURL, specifically the url to use, the registered user agent name, the XOAUTH2 key, then allows it to
    //write the curl data to the HTML output file.  
    curl_easy_setopt(curl,CURLOPT_URL,start_link);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "RutgersUniGroupDJZ");
    curl_easy_setopt(curl, CURLOPT_XOAUTH2_BEARER, "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9.eyJhdWQiOiJlZjg4NDQxMjk5Yzg0NWU5NTBjN2YwNDc5OGQ2NGQwMyIsImp0aSI6IjkwYmMyOTVhNTNiMzU1MmJmMjNlNmIzYmVlNGE5MWEyY2Q2ZGIzYTdkNTFiZThhZTRlMjExZWFmMWU5NjNjODAwMzcyOTMwMDcyYzNkM2JjIiwiaWF0IjoxNzYzMTYwNzA3LjQ2MTc2OCwibmJmIjoxNzYzMTYwNzA3LjQ2MTc3MSwiZXhwIjozMzMyMDA2OTUwNy40NTg5NTQsInN1YiI6IjgwNTE0NDg3IiwiaXNzIjoiaHR0cHM6Ly9tZXRhLndpa2ltZWRpYS5vcmciLCJyYXRlbGltaXQiOnsicmVxdWVzdHNfcGVyX3VuaXQiOjUwMDAsInVuaXQiOiJIT1VSIn0sInNjb3BlcyI6WyJiYXNpYyJdfQ.pkjZM6VqH9eHWJO4E54X1J2W2N95Se3kkYAPoXiJiROUakyl4TRPO7KAFTd5_CvYrkNtRv2RMh0jZyi-Bdc8MOTjvMZLPUIfo-pd9yNzrCDzrFTovJJ9MkW5qCRVXOD4APcXrrkjQbT_Rnt5y3LcnlqKZJBAFJiOVNz71-SyyMrsqO0o0kV5g6rIrP2WyhO9mT-L7XcitAy58mFFfzrcKBkwDAsj5FA7eJeTvPrAkZUB4l-SR_vfQzWfnGovsCzyATZbK6Z6QxPIC2sVdrQ2vNrxgRVe0d4x4lMwWPDUuaDbW3-8KGRcQ3Qns_BYm2ZiAx4yJIODHqY5ZLwH9-0hFhNadolWqMNBqsbMD7nNXDhrLNO71pih-qaCKjibvFUgADekZ0D-VoDBRtIBAmw4NQvvEgOGxh4z0rJ4tlwM61J-RJj9TOJLNwb7Bi-8bWWJANdQraC7YzELLF3Umh5iYBRNdDSwFkzHF0g2zq_UO6mj1ZbOY6_xL0joOP0m9sf_hO1pZ7OCcZAVM5Gks_7t91GJgME7Ui0iLHCsyF0ieQvqmJ7EmoNv9AvAzNcA2hmRnpNoFLKLpFYcg3qs4LGIAd_GBX9C_sMvdizwUNbQYHaQwMydOhrSVRwih-EmapDR8A26HimnbYOcC30zjvBFxIsusI-HJtf_yExTuIZc6Y");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, HTML_outputFile);

    //For the target article, this first allocates the memory for the variable to fit the article name.  
    char *target = (char *)malloc(strlen(target_link)+1); //Allocates memory to fit length of article name
    target=target_link;
    target[strlen(target_link)]='\0';//stores it with delimiter character.  
    size_t target_len = strlen(target); //Grabs size.  

    //For the current article, this first allocates memory in the same way for the target article (helps to make prog more efficient)
    char *current = (char *)malloc(strlen(start_link)+1);
    current=start_link;
    current[strlen(start_link)]='\0';
    size_t current_len = strlen(current); //Grabs size of current article name

    //Universal between the two.
    const char *prefix = "https://en.wikipedia.org/wiki/";
    size_t prefix_len = strlen(prefix); //Grabs size so it isn't called like 5 million times.  

    //At this point, it checks if the target contains the prefix string, in which case it moves the memory such that 
    //target.  Should look into whether or not memory needs to be reallocked to prevent memory leaks/wastage.  
    if(target_len>=prefix_len && strncmp(target,prefix,prefix_len)==0)
    {
        memmove(target, target + prefix_len, target_len-prefix_len+1); //Moves the modified,cmped string to the initial str var.  
    }
    //Dpes same as above but for current instead.  
    if(current_len>=prefix_len && strncmp(current,prefix,prefix_len)==0)
    {
        memmove(current, current + prefix_len, current_len-prefix_len+1); //Moves the modified,cmped string to the initial str var.  
    }

    //Checks and verifies that everything went okay with the curling of the link.  If it fails, flags a download problem.  
    CURLcode result = curl_easy_perform(curl);
    if(result!=CURLE_OK)
    {
        fprintf(stderr, "download issue: %s\n",curl_easy_strerror(result));

    }
    //DO NOT DELETE.  REWINDS POINTER TO THE BEGINNING, BC IT IS AT THE END AFTER WRITING.  
    //2 HOURS WERE WASTED PRIOR TO ITS DISCOVERY.  YOU HAVE BEEN WARNED.  THE SESAME WILL DIE AND I HAVE TOLD YOU. 
    rewind(HTML_outputFile);

    //Put file into parseHTML with the fullDataFile, the links, and the target.  
    parseHTML(HTML_outputFile,links_file,current,target);

    if(links_file == NULL)
    {
        fprintf(stderr, "ur a dumbass, you didn't open the right skibidi file.\n");
        return EXIT_FAILURE;
    }
    rewind(links_file);
    //enqueue(&queue,line,result,curDepth);

    //erase the contents of the html file
    if(ftruncate(fileno(HTML_outputFile),0)==-1)
    {
        perror("Error truncating file");
        fclose(HTML_outputFile);
        return EXIT_FAILURE;
    }
    rewind(HTML_outputFile);

    //cleans up the whole shabang when done.
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS; //Ends the whole thing with a good note.
}

//Add to queue full function:  ALL it does is just go through a file and enqueue all of the contents. 
int addToQueue(FILE*links_file, URLQueue *queue, URLQueueNode *parent, int depth)
{
    char lineBuffer[128];    
    char targetChar = '\n';
    //Loop through all lines in the link file and enter them into the queue.  
    while(fgets(lineBuffer,128,links_file) != NULL)
    {
        //Change it so that it eliminates the newline from the buffer.  
        lineBuffer[strcspn(lineBuffer, "\n")] = '\0'; // Remove newline character using strcspn, which just for the sake
        //of brevity finds the number of chars before a specific char in a string, returns the index of that, then we set it to 
        //the null character so that it doesn't keep erroring out.  
        enqueue(queue,lineBuffer,parent,depth); //enqueues the shitshow.  
    }

    //Then erase the contents of the file to make room for the new data.  
    if(ftruncate(fileno(links_file),0)==-1)
    {
        perror("Error truncating file");
        fclose(links_file);
        return EXIT_FAILURE;
    }
    rewind(links_file);
    return EXIT_SUCCESS;
}

//Prints the lineage of the URL Queue when the path is found.  
void printLineage(URLQueueNode *URLNode)
{
    URLQueue pathQueue;
    URLQueueNode *current = URLNode;
    URLQueueNode *next;

    initQueue(&pathQueue);
    
    printf("\n");
    while(current!=NULL)
    {
        
        enqueue(&pathQueue,current->url,current->parent,current->depth);
        current = current->parent;
    }
    //Empties the queue.  
    while(queueEmpty(&pathQueue)==0)
    {
        next = dequeue(&pathQueue);
        if(strstr(next->url,"https://en.wikipedia.org/wiki/")!=NULL)
        {
            printf("\n%d %s\n",next->depth,next->url);
        }
        else{printf("\n%d https://en.wikipedia.org/wiki/%s\n",next->depth,next->url);}
    }
    //Print the last one.  
    //next = dequeue(&pathQueue);
    //printf("\n%d https://en.wikipedia.org/wiki/%s\n\n",next->depth,next->url);
    printf("\nLINK PATH TAKEN\nValue to the left is the depth of the link (starting from 0)\n\n");
    
    destroyQueue(&pathQueue);
    //Enqueue all of them into a fresh queue, then just print that queue from the head to the tail.  
}
