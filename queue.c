# include <stdlib.h>
# include "queue.h"
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <inttypes.h>

void insertionSort(item a[], uint32_t length) // Item is the treeNode in this case.
{
    for (uint32_t i =1; i<length; i++)
    {
        item temp = a[i];//The temp is used for the insertion.
        int j=i-1;
        while(j>=0 && a[j]->count > temp->count)
        {
            a[j+1]=a[j];//It moves the j+1
            j=j-1;
        }
        a[j+1]=temp;//A[j+1] becomes temp from the top
    }
    return;
}



queue* newQueue(uint32_t size)
{
    queue *q=(queue*)calloc(1,sizeof(queue)); // Callocs the queue to the size.
    q->size=size;
    q->head=-1; // Head and tail -1 means that its empty. This is how I intialize it.
    q->tail=-1;
    q->Q=(item*)calloc(size,sizeof(item)); // Intiliazes the item in the queue.
    return q;
}
void delQueue(queue *q)
{
    free(q->Q); free(q); return;
}

bool emptyQ(queue *q)
{
    uint32_t negOne = -1;
    if(q->head==negOne && q->tail==negOne) // If they are both empty. Then it returns true.
    {
        return true; // For my queue, empty means negative one.
    }
    else
    {
        return false; // If its not empty
    }
}
bool fullQ(queue *q)
{
    if(q->tail== (q->size)-1) // If the tail is at the last position that means the queue is full.
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool enqueue(queue *q, item x)
{
    if ((((q->tail)+1)%q->size)==q->head) // If the queu is full.
    {
        printf("%s","Queue is full");
        return false;
    }
    else if(emptyQ(q)) // If the queue is empty, it makes the head and the tail 0.
    {
        q->head=q->tail=0;
    }
    else
    {
        q->tail=(q->tail+1)%q->size; // It increments the tail.
    }
    q->Q[q->tail]=x;
    insertionSort(q->Q,q->tail+1);
    return true;
}




bool dequeue(queue *q,item *i)
{
    *i=q->Q[q->head]; // The dequeue returns the head item.
    if(emptyQ(q))
    {
        return false;
    }
    else if(q->head==q->tail)
    {
        q->head=q->tail=-1;//mark empty queue by making both of them -1
    }
    else{


        q->head=(q->head+1)%q->size;
    }

    return true;
}
void Print(queue *q){
    for (uint32_t i= q->head;i<=q->tail;i++){
        printf("%c \n",q->Q[i]->symbol); // This is used for debugging the queue.
    }
    printf("The head is: %d\n",q->head); // Shows the head.
    printf("The tail is: %d\n",q->tail); // Shows the tail of the queue.
}
uint32_t queueSize(queue *q){ // This return the size of the queue.
    return q->head - q->tail; // Subtract the head from the tail.
}
