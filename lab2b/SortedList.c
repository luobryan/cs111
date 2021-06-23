/*
NAME: Bryan Luo
EMAIL: luobryan@ucla.edu
ID: 605303956
*/ 
#include "SortedList.h"
#include <string.h>
#include <sched.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
    if (element == NULL){
        return;
    }
    if (list == NULL){
        return; 
    }
    SortedList_t* temp = list->next;
       if(temp == NULL){
	return; //error, becuase even in an empty list, list->next should point back to itself
    }
    else{
    while(temp!= list && (strcmp(temp->key, element->key) < 0)){
        temp = temp -> next; 
    }
    //at this point temp->key >= element->key OR we are back at list, and element
    // is great than all elements 
    if(opt_yield & INSERT_YIELD){
        sched_yield(); 
    }
    temp->prev->next = element; 
    element->prev = temp->prev; 
    temp->prev = element; 
    element -> next = temp;
   }
}

int SortedList_delete(SortedListElement_t *element){
    if (element == NULL){
        //nonsense parameter 
        return 1;
    }

    if (element->next == NULL || element ->prev == NULL){
        //in a doubly circular linked list, there should be no NULL "next" or "prev"
        //even if there's only 1 element, its next and prev should point to the "head" node
        return 1;
    }
    if (element->next->prev!=element || element->prev->next != element){
        //faulty pointers
        return 1;
    }
    if (opt_yield & DELETE_YIELD){
        sched_yield();
    }
    element->next->prev = element -> prev;
    element->prev->next = element -> next; 
    return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
 
    //If key or list is NULL, return NULL
    //technically, you might want to return the head node (i.e. list) if key is NULL
    //because head node key is NULL, but doesn't make sense
    if(list == NULL || key == NULL){ 
                return NULL;
    }
    SortedList_t* temp = list -> next; 
    if(temp==NULL){
	return NULL;  //error in this case because even if the list is empty, list->next should not be null, it should just point to itself
    }
    while(temp!=list){
        if (opt_yield & LOOKUP_YIELD){
            sched_yield(); 
        }
        if(temp->key!=NULL&&strcmp(temp->key,key)==0){
            return temp; 
        }
        temp = temp->next; 
    } 

    return NULL; 

}

int SortedList_length(SortedList_t *list){
    if (list == NULL){
        return -1; 
    }
    int length = 0; 
    SortedListElement_t* temp = list->next;
    if(temp == NULL){
	return -1; //error, because even in an empty list, list->next should point to itself, and not be null
    }
    while(temp != list){
        if (opt_yield & LOOKUP_YIELD){
            sched_yield(); 
        }
        length = length + 1; 
        temp = temp -> next; 
        if(temp == NULL){
            //for a circular linked list, it should never be null 
            return -1; 
        }
    }
    return length; 
}

