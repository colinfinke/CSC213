#include "sorted-list.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Initialize a sorted list.
 *
 * \param lst This is a pointer to space that should be initialized as a sorted list. The caller of
 * this function retains ownership of the memory that lst points to (meaning the caller must free it
 * if the pointer was returned from malloc)
 */
void sorted_list_init(sorted_list_t* lst) {
  //sets a head node with value -1 that points to null
  lst->value = -1;
  lst->next = NULL;
}

/**
 * Destroy a sorted list. Free any memory allocated to store the list, but not the list itself.
 *
 * \param lst This is a pointer to the space that holds the list being destroyred. This function
 * should free any memory used to represent the list, but the caller retains ownership of the lst
 * pointer itself.
 */
void sorted_list_destroy(sorted_list_t* lst) {
  //node for being destroyed and node for keeping track of position
  struct sorted_list* prevNode = lst;
  struct sorted_list* currNode = lst->next;
  while(currNode!=NULL){
    //loops through nodes and frees them
    prevNode = currNode;
    currNode = currNode->next;
    free(prevNode);
  }

  //resets lst in case it is used again
  lst->value = -1;
  lst->next = NULL;
}

/**
 * Add an element to a sorted list, maintaining the lowest-to-highest sorted order.
 *
 * \param lst   The list that is being inserted to
 * \param value The value being inserted
 */
void sorted_list_insert(sorted_list_t* lst, int value) {
  //initializes new node and frees space for it, setting value to value and next to null
  struct sorted_list* newNode = (struct sorted_list*) malloc(sizeof(struct sorted_list));
  newNode->value = value;
  newNode->next = NULL;

  //if there are no nodes, makes the new node the first node
  if(lst->next == NULL){
    lst->next = newNode;
    return;
  }

  //loops through lst until the next value is higher than the current, finding newNode's position
  struct sorted_list* currNode = lst;
  while(currNode != NULL && currNode->next != NULL && currNode->next->value < value){
    currNode = currNode->next;
  }

  //adds new node in position
  newNode->next = currNode->next;
  currNode->next = newNode;
}

/**
 * Count how many times a value appears in a sorted list.
 *
 * \param lst The list being searched
 * \param value The value being counted
 * \returns the number of times value appears in lst
 */
size_t sorted_list_count(sorted_list_t* lst, int value) {
  int count = 0;
  struct sorted_list *currNode = lst->next;
  while(currNode!=NULL){
    //loops through lst and increments count if currNode's value is the same as value
    if(currNode->value==value) count++;
    currNode = currNode->next;
  }
  return count;
}

/**
 * Print the values in a sorted list in ascending order, separated by spaces and followed by a
 * newline.
 *
 * \param lst The list to print
 */
void sorted_list_print(sorted_list_t* lst) {
  struct sorted_list *currNode = lst->next;
  while(currNode!=NULL){
    //loops through lst and prints each node's value
    if(currNode->next==NULL) {
      printf("%d\n", currNode->value);
    }
    else {
      printf("%d ", currNode->value);
    }
    currNode=currNode->next;
  }
}
