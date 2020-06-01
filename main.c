/*
 * Author: Liam O'Shea
 * Date: November 18 2019
 * File: main.c
 * Description: This file reads a text file containing tasks and schedules them according to scheduling algorithms
 * learned in class including FCFS, RR, NSJF, and PSJF.
 *
 * Note to marker: Apologies for somewhat messy code. I did not have time to refactor code into functions.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXCHARS 15
#define MAXTASKS 50

typedef struct
{
    char name[3];
    int arvTime;
    int burstTime;
    int lastInCPU;
    int waiting;
    int isFirstPass;

} task;

void swap(task *t1, task *t2)
{
    task temp = *t1;
    *t1 = *t2;
    *t2 = temp;
}

char** tokenizeInput(char * in)
{
    int numTok = 0;           //Number of separate tokens in command
    char ** args = malloc(20 * sizeof(char*));
    char* tok = "nil";
    while ( tok != NULL )
    {
        if ( numTok == 0 )
        {
            tok = strtok( in, "," );
        }
        else
        {
            tok = strtok( NULL, "," );
        }
        args[numTok] = tok;
        if ( tok != NULL )
        {
            numTok++;
        }
    }

    return args;
}

int main()
{
    // Opening task list file
    FILE *taskFile;
    char line[MAXCHARS];

    // Create output file
    FILE *outFile;
    //outFile = fopen("C:\\Users\\Liam\\Desktop\\OSA3\\Output.txt", "w");
    outFile = fopen("Output.txt", "w");

    char ** taskData;   // Will store tokenized task attributes
    task * taskList = (task*)malloc(sizeof(task) * MAXTASKS);   // Will store tasks read from file
    task * tmp;         // Used when reallocating

    //taskFile = fopen("C:\\Users\\Liam\\Desktop\\OSA3\\Tests\\TaskSpec6.txt", "r");
    taskFile = fopen("TaskSpec.txt", "r");

    if(taskFile == NULL)
    {
        printf("Could not open file.");
        return 1;
    }

    // Create tasks and place them in list
    int numTasks = 0;
    while(fgets(line, sizeof(line), taskFile))
    {
        taskData = tokenizeInput(line);
        // Exit file read if we encounter new line as first token
        if(strcmp(taskData[0], "\n") == 0) break;

        task *tempTask = (task *)malloc(sizeof(task));
        // Initialize task struct
        strcpy(tempTask->name, taskData[0]);
        tempTask->arvTime = atoi(taskData[1]);
        tempTask->burstTime = atoi(taskData[2]);
        tempTask->lastInCPU = 0;
        tempTask->waiting = 0;
        tempTask->isFirstPass = 1;

        // Place task in list
        taskList[numTasks] = *tempTask;
        numTasks++;
    }

    fclose(taskFile);

    // Free up unused space
    if((tmp = realloc(taskList, sizeof(task) * numTasks)) != NULL)
    {
        taskList = tmp;
    } else{
        perror("Realloc");
    }


    // *********************************************
    // Schedule tasks using FCFS, RR, NSJF, and PSJF
    // *********************************************


    // FCFS - Process the requests CPU first is allocated CPU first
    // ************************************************************

    int t = 0;                  // Timestep
    int waitingNumerator = 0;   // Used when calculating average waiting time
    int remBurst = 0;           // Remaining CPU burst length left in task
    int done = 0;               // For exit condition
    int numReady = 0;           // Number of items in ready queue
    int taskListIndex = 0;      // Index for list of all tasks
    int readyIndex = 0;         // Index for use in ready queue

    task * selectedTask;
    selectedTask = NULL;
    task * readyQueue = (task*)malloc(sizeof(task) * numTasks);


    // Point to first task before loop begins
    task * nextTask = &taskList[taskListIndex];

    printf("FCFS:\n");
    fprintf(outFile, "FCFS:\n");
    //Program loop
    while(done == 0)
    {
        // Add arriving items to queue
        while(nextTask->arvTime == t  && taskListIndex != numTasks)
        {
            // If tasks is arriving at this time step add it to the arrival queue
            readyQueue[taskListIndex] = taskList[taskListIndex];
            numReady ++;
            taskListIndex ++;

            // Get the next task to check for arrival
            nextTask = &taskList[taskListIndex];
        }

        // If there is no current task being worked on, give CPU to next item in arrival queue
        if(numReady > 0 && selectedTask == NULL)
        {
            selectedTask = &readyQueue[readyIndex];
            readyIndex++;

            // Keep track of factors to calculate average waiting
            remBurst = selectedTask->burstTime;
            waitingNumerator += (t - selectedTask->arvTime);

            numReady --;

            // Output
            printf("%s\t%d\t", selectedTask->name, t);
            fprintf(outFile, "%s\t%d\t", selectedTask->name, t);

        }

        // If we are currently working on a task we decrement one
        // unit of time from CPU burst length
        if(selectedTask != NULL)
        {
            remBurst--;
            // If task completes remove task from CPU
            if(remBurst == 0)
            {
                printf("%d\n", t+1);
                fprintf(outFile, "%d\n", t+1);
                selectedTask = NULL;
            }
        }

        t++;

        //Exit condition
        if(numReady == 0 && taskListIndex == numTasks && selectedTask == NULL) done = 1;

    }
    printf("Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);
    fprintf(outFile, "Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);

    /// Round Robin
    /// ***********

    printf("RR:\n");
    fprintf(outFile, "RR:\n");

    // Clear ready queue after last algorithm
    free(readyQueue);
    readyQueue = (task*)malloc(sizeof(task) * numTasks);

    // Resets
    done = 0;
    t = 0;
    waitingNumerator = 0;
    numReady = 0;
    taskListIndex = 0;
    int q = 4;              // Time quantam
    int timer = 0;          // Quantam timer
    int taskInCPU = 0;      // Boolean to indicate if task is currently in CPU
    task currentTask;       // Object to place current task in

    // We will create a copy of our task list to avoid polluting the original list. This is because
    // we must keep track of CPU burst and other factors, and we do not wish to modify the original
    // list as we will use it again later for our other scheduling algorithms

    task * allTasks = (task*)malloc(sizeof(task) * numTasks);
    for (int i = 0; i < numTasks; i++){
        allTasks[i]=taskList[i];
    }

    // Point to first task before loop begins
    nextTask = &allTasks[taskListIndex];

    // Add arriving items to ready queue
    while(done == 0){

        while(nextTask->arvTime == t && taskListIndex != numTasks)
        {
            // If tasks is arriving at this time step add it to the arrival queue
            readyQueue[numReady] = allTasks[taskListIndex];
            numReady ++;
            taskListIndex ++;
            // Get the next task to check for arrival
            nextTask = &allTasks[taskListIndex];
        }

        //If there is no current task, assign first available task from ready queue as current task
        if(!taskInCPU && numReady > 0)
        {
            currentTask = readyQueue[0];
            taskInCPU = 1;

            // Output
            printf("%s\t%d\t", currentTask.name, t);
            fprintf(outFile, "%s\t%d\t", currentTask.name, t);

            // Rotate queue left by 1
            for(int i = 0; i < numReady-1; i++){
                swap(&readyQueue[i], &readyQueue[i+1]);
            }

            numReady--;


            // Keep track of factors for use in calculating waiting time.
            if(currentTask.isFirstPass){
                currentTask.isFirstPass = 0;
                currentTask.waiting = t - currentTask.arvTime;
            } else{
                currentTask.waiting += t - currentTask.lastInCPU;
            }
        }

        // If we have a current task, decrement burst length by 1
        if(taskInCPU)
        {
            currentTask.burstTime--;
            timer++; // Increment timer to compare against quantam

            if (currentTask.burstTime == 0){
                taskInCPU = 0;
                timer = 0;
                printf("%d\n", t+1);
                fprintf(outFile,"%d\n", t+1);

                // When task is finished add its waiting time to the waiting numerator.
                waitingNumerator += currentTask.waiting;
            }
        }

        // If task is not complete and quantam is up, place task back in ready queue.
        if(taskInCPU && timer == q){

            if(numReady > 0) {
                // Keep track of when task leaves CPU for waiting time
                currentTask.lastInCPU = t + 1;

                readyQueue[numReady] = currentTask;
                numReady++;

                taskInCPU = 0;
                timer = 0;

                // Output
                printf("%d\n", t + 1);
                fprintf(outFile, "%d\n", t + 1);
            }
            else
            {
                timer = 0;
            }
        }

        t++;

        // Exit condition
        if(numReady == 0 && taskListIndex == numTasks && taskInCPU == 0){
            done = 1;
        }
    }

    printf("Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);
    fprintf(outFile, "Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);



    /// NSJF - (Non-Preemptive Shortest Job First)
    /// ******************************************

    // Clear ready queue after last algorithm
    free(readyQueue);
    readyQueue = (task*)malloc(sizeof(task) * numTasks);

    // Resets
    t = 0;
    done = 0;
    selectedTask = NULL;
    numReady = 0;
    taskListIndex = 0;
    remBurst = 0;
    waitingNumerator = 0;
    taskInCPU = 0;

    // Point to first task before loop begins
    nextTask = &taskList[taskListIndex];

    printf("NSJF:\n");
    fprintf(outFile, "NSJF:\n");
    //Program loop
    while(done == 0)
    {
        // Add arriving items to queue
        while(nextTask->arvTime == t && taskListIndex != numTasks)
        {
            // If tasks is arriving at this time step add it to the arrival queue
            readyQueue[numReady] = taskList[taskListIndex];
            numReady ++;
            taskListIndex ++;

            // Get the next task to check for arrival
            nextTask = &taskList[taskListIndex];
        }

        // Sort arrival queue so task with shortest CPU burst is first.
        // Implemented by bubble sort in this case. Bubble sort will preserve
        // FCFS ordering in the event two items have the same CPU burst length.

        for(int i = 0; i < numReady-1; i++)
        {
            for(int j = 0; j < numReady-1; j++)
            {
                if(readyQueue[j].burstTime > readyQueue[j+1].burstTime)
                {
                    swap(&readyQueue[j], &readyQueue[j+1]);
                }
            }
        }


        // If there is no current task being worked on, give CPU to first
        // item in arrival queue
        if(numReady > 0 && taskInCPU == 0)
        {
            currentTask = readyQueue[0];
            taskInCPU = 1;

            // Rotate ready queue left by 1

            for(int i = 0; i < numReady-1; i++)
            {
                swap(&readyQueue[i], &readyQueue[i+1]);
            }

            // Keep track of factors to calculate average waiting
            remBurst = currentTask.burstTime;
            waitingNumerator += (t - currentTask.arvTime);

            numReady --;

            // Output
            printf("%s\t%d\t", currentTask.name, t);
            fprintf(outFile, "%s\t%d\t", currentTask.name, t);

        }

        // If we are currently working on a task we decrement one
        // unit of time from CPU burst length
        if(taskInCPU == 1)
        {
            remBurst--;
            // If task completes remove task from CPU
            if(remBurst == 0)
            {
                printf("%d\n", t+1);
                fprintf(outFile, "%d\n", t+1);
                taskInCPU = 0;
            }
        }

        t++;

        //Exit condition
        if(numReady == 0 && taskListIndex == numTasks && taskInCPU == 0) done = 1;

    }
    printf("Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);
    fprintf(outFile, "Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);

    /// PSJF (Preemptive SJF)
    /// *********************

    t = 0;
    done = 0;
    taskListIndex = 0;
    taskInCPU = 0;
    t = 0;
    done = 0;
    numReady = 0;
    waitingNumerator = 0;

    // Clear ready queue from last algorithm
    free(readyQueue);
    readyQueue = (task*)malloc(sizeof(task) * numTasks);


    // Point to first task before loop begins
    nextTask = &taskList[taskListIndex];

    printf("PSJF:\n");
    fprintf(outFile, "PSJF:\n");

    //Program loop
    while(done == 0)
    {
        // Add arriving items to queue
        while(nextTask->arvTime == t && taskListIndex != numTasks)
        {

            // If arriving item has shorter CPU burst, place arriving item in CPU and old item in ready queue.
            if(taskInCPU)
            {
                if(nextTask->burstTime < currentTask.burstTime && taskInCPU)
                {
                    // Output
                    printf("%d\n", t);
                    fprintf(outFile, "%d\n", t);

                    // Waiting time factor
                    currentTask.lastInCPU = t;

                    // Place current task in ready queue
                    readyQueue[numReady] = currentTask;
                    // Choose next task as current task
                    currentTask = *nextTask;
                    taskListIndex++;
                    numReady++;

                    // Output
                    printf("%s\t%d\t", currentTask.name, t);
                    fprintf(outFile, "%s\t%d\t", currentTask.name, t);

                    // Waiting time factors
                    currentTask.isFirstPass = 0;
                    currentTask.waiting = t - currentTask.arvTime;
                }
                else
                {
                    // Place new task in ready queue.
                    readyQueue[numReady] = allTasks[taskListIndex];
                    numReady++;
                    taskListIndex++;
                }
            }
            else
            {
                // Place new task in ready queue.
                readyQueue[numReady] = allTasks[taskListIndex];
                numReady++;
                taskListIndex++;
            }

            // Get the next task to check for arrival
            nextTask = &allTasks[taskListIndex];
        }


        // Sort ready queue, make shortest items on left and longest items on right
        for(int i = 0; i < numReady-1; i++)
        {
            for(int j = 0; j < numReady-1; j++)
            {
                if(readyQueue[j].burstTime > readyQueue[j+1].burstTime)
                {
                    swap(&readyQueue[j], &readyQueue[j+1]);
                }
            }
        }


        // If there is no current task, place shortest task in CPU
        if(!taskInCPU && numReady > 0)
        {
            currentTask = readyQueue[0];

            // Rotate ready queue left by 1
            for(int i = 0; i < numReady-1; i++)
            {
                swap(&readyQueue[i], &readyQueue[i+1]);
            }

            numReady --;
            taskInCPU = 1;

            // Output
            printf("%s\t%d\t", currentTask.name, t);
            fprintf(outFile, "%s\t%d\t", currentTask.name, t);

            // Keep track of waiting time factors.
            if(currentTask.isFirstPass)
            {
                currentTask.isFirstPass = 0;
                currentTask.waiting = t - currentTask.arvTime;
            } else
            {
                currentTask.waiting += t - currentTask.lastInCPU;
            }

        }


        // Do work on current task
        if(taskInCPU){
            currentTask.burstTime--;

            // If task is finished, remove it
            if (currentTask.burstTime == 0)
            {
                taskInCPU = 0;
                printf("%d\n", t+1);
                fprintf(outFile, "%d\n", t+1);
                /// Calculate final wait time and add to waiting numerator
                waitingNumerator += currentTask.waiting;
            }
        }

        // Increment time step
        t++;

        // Exit condition
        if(numReady == 0 && taskListIndex == numTasks && taskInCPU == 0) done = 1;
    }
    printf("Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);
    fprintf(outFile, "Average Waiting Time: %g\n\n", (float)waitingNumerator/(float)numTasks);

    fclose(outFile);

    // Free memory
    free(taskList);
    free(readyQueue);
    free(allTasks);

    return 0;
}

