/*********************************************************
* File: thread.cpp
*
* Description: Playground to experiment with pthreads
*
* Author: Michael Noon and Addison Sandvik
*
**********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int z = 0;
pthread_mutex_t lock;

void *print_message_function( void *ptr );

int main() {
    pthread_t thread1, thread2;
     char const *message1 = "Thread 1";
     char const *message2 = "Thread 2";
     int  iret1, iret2;

    /* Create independent threads each of which will execute function */
     iret2 = pthread_create( &thread2, NULL, print_message_function, (void*) message2);
     iret1 = pthread_create( &thread1, NULL, print_message_function, (void*) message1);
     
     /* Wait till threads are complete before main continues. Unless we  */
     /* wait we run the risk of executing an exit which will terminate   */
     /* the process and all threads before the threads have completed.   */

     pthread_join( thread2, NULL); 
     pthread_join( thread1, NULL);

     printf("Thread 1 returns: %d\n",iret1);
     printf("Thread 2 returns: %d\n",iret2);
     printf("Z: %d\n", z);
     exit(0);
}

void *print_message_function( void *ptr )
{
     char *message;

     //pthread_mutex_lock(&lock);
     z = z + 1;
     //pthread_mutex_unlock(&lock);

     message = (char *) ptr;
     printf("%s \n", message);
     return NULL;
}
