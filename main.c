/*
 * I used this resource to create the qsort in sorting and compare functions:
 * https://www.tutorialspoint.com/c_standard_library/c_function_qsort.htm
 * I used this source to help with open, read and write files:
 * https://www.tutorialspoint.com/cprogramming/c_file_io.htm
 * */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *sorter(void *params); /* thread that performs basic sorting algorithm */
void *merger(void *params); /* thread that performs merging of results */
int compare(const void* a, const void* b); /* compare function to compare two inputs if one is bigger than the other*/

int originalList[501]; /*global array to store the list from the text file*/
int mergedList[501]; /*global array to store the merged list*/

/* structure for passing data to threads */
typedef struct
{
    int starting_index;
    int ending_index;
} parameters;

int main(int argc, const char * argv[])
{
    FILE *file = fopen("IntegerList.txt","r");
    char fileString[2000]; //string to save all the line of the input tex file in one sentence
    if (file == NULL){
        printf("error in opening file!");
        return 1;
    }
    if(fgets(fileString, sizeof(fileString), file)==NULL){
        printf("Not able to read from the text file or the file is empty!");
        return 1;
    }

    fclose(file);

    char *str_tokenized = strtok(fileString, ",");
    int listLength = 0;//to access the array and store the length of it
    while (str_tokenized != NULL){
        //converting string to integer
        int number = atoi(str_tokenized);
        originalList[listLength] = number;
        listLength++;
        str_tokenized = strtok(NULL, ",");
    }

    /* create worker threads */
    parameters *thread1data = (parameters *) malloc(sizeof(parameters));
    parameters *thread2data = (parameters *) malloc(sizeof(parameters));
    int midItemIndex = listLength/2;
    int thread1_N1 = 0; //starting index in thread1
    int thread1_N2 = midItemIndex-1; //ending index in thread1
    int thread2_N1 = midItemIndex; //starting index in thread2
    int thread2_N2 = listLength-1; //ending index in thread2

    //starting and ending	index number for thread 1
    thread1data->starting_index= thread1_N1;
    thread1data->ending_index = thread1_N2;
    //starting and ending	index number for thread 2
    thread2data->starting_index= thread2_N1;
    thread2data->ending_index = thread2_N2;

    pthread_t tid1, tid2;
    /* create the first sorting thread */
    pthread_create(&tid1, NULL, sorter, thread1data);
    /* create the second sorting thread */
    pthread_create(&tid2, NULL, sorter, thread2data);

    /* now wait for the 2 sorting threads to finish */
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    free(thread1data);
    free(thread2data);
    /* create the merge thread */
    parameters *mergeThreadData = (parameters *) malloc(sizeof(parameters));
    //starting and ending	index number for the merge thread
    mergeThreadData->starting_index= 0;
    mergeThreadData->ending_index = listLength-1;
    pthread_t mergeThread;
    pthread_create(&mergeThread, NULL, merger, mergeThreadData);


    /* wait for the merge thread to finish */
    pthread_join(mergeThread, NULL);
    free(mergeThreadData);


    /* output the sorted array */
    FILE *outputFile = fopen("SortedIntegerList.txt","w");

    if (outputFile == NULL){
        printf("Output file has error opening!");
        return 1;
    }

    for (int i = 0; i < listLength; ++i) {
        if (i < listLength - 1){
            fprintf(outputFile, "%d,", mergedList[i]);
            printf("%d,", mergedList[i]);
        } else{
            fprintf(outputFile, "%d", mergedList[i]); // to avoid the comma at the end
            printf("%d", mergedList[i]);
        }
    }

    fclose(outputFile);

}

//The sorting function qsort() in glibc is used to	sort integers
void *sorter(void *params)
{
    /* sorting algorithm implementation */
    parameters *data = params;

    /*base: the sum operation is to ensure we work in both cases if we are in first half so we are in element 0 in the second half in pointer to starting index
     *length: will be the comparison between ending and starting index to ensure we only traverse through the half we want
     * */
    qsort(originalList + data->starting_index, data->ending_index - data->starting_index + 1, sizeof(int), compare);
    pthread_exit(0);
}

//helper function for the qsort function to compare two elements
int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

// I used the two-pointer merge algorithm
void *merger(void *params)
{
    /* merging algorithm implementation */
    parameters *data = params;

    int firstHalfStarting = 0; /* starting point of the first half array always 0 */
    int secondHalfStarting = (data->ending_index+1)/2; /* starting point of the second half always middle */

    int i = firstHalfStarting;
    int j = secondHalfStarting;

    int index = 0; /*index for the merged list starting from zero (this helps accessing the merged list elements) */
    /*Merging algorithm looping through the two halfs of array until one of them reach the end of its half*/
    while (i < secondHalfStarting && j <= data->ending_index){

        //in case first half is smaller we store it in the merged list if not we store the second half eleemnt to the merged list
        if (originalList[i] < originalList[j]){
            mergedList[index] = originalList[i];
            i++;
        } else{
            mergedList[index] = originalList[j];
            j++;
        }

        index++;
    }
    //in case there is remaining elements from one of the halfs we add the remaining of it to the merged list
    if(i < secondHalfStarting){
        while (i < secondHalfStarting){
            mergedList[index] = originalList[i];
            i++;
            index++;
        }
    }
    else if(j <= data->ending_index){
        while (j <= data->ending_index){
            mergedList[index] = originalList[j];
            j++;
            index++;
        }
    }

    pthread_exit(0);
}
