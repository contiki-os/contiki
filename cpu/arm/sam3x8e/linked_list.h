/**
 * Copyright (c) 2013, Calipso project consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the Calipso nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific
 * prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/ 


#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_


#include "core_cmFunc.h"
#include <stddef.h>
#include "stdlib.h"
#include "smalloc.h"
#include "cc.h"
#include "skbuff.h"

#define LINKED_LIST_DEBUG	0

#define LINKED_LIST_CONCAT2(s1, s2) s1##s2
#define LINKED_LIST_CONCAT(s1, s2) LINKED_LIST_CONCAT2(s1, s2)

struct linked_list_t;

/* Simple single linked list implementation */
struct linked_list_t {
	void* val;
	struct linked_list_t *next;
};

/* Macros */
#define LINKED_LIST(name) struct linked_list_t* LINKED_LIST_CONCAT(name, _list)
#define LINKED_LIST_INIT(name) linked_list_init(LINKED_LIST_CONCAT(name, _list))





/* Initialize list, reserving memory for root position. FIXME - it works
 * but the call by value is redundant. Consider implementing it as void
 * but with the pointer to the linked list as a reference.
 */
inline struct linked_list_t* linked_list_init(struct linked_list_t* the_list) {
	
	if (the_list != NULL) {
		printf("WARNING: Linked list already initialized.\n");
		return the_list;
	}
	struct linked_list_t* _list = (struct linked_list_t*)smalloc(sizeof(struct linked_list_t));
	/* Initialize [maybe redundant] */
	if (_list) {
		_list->next = NULL;
		_list->val = NULL;
	}	
	/* Could be NULL if the memory allocation was not successful */
	return _list;
}

/* Return the [binary] status of the linked list, empty / non-empty. */
inline bool linked_list_is_empty(struct linked_list_t* the_list) {
	
	if (the_list == NULL) {
		return true;
	}		
	if (the_list->val == NULL) {
		return true;
	}		
	return false;
}


/* Return the current length of the linked list */
inline uint32_t linked_list_get_len(struct linked_list_t* list) {
	
	struct linked_list_t* the_list = list;
	
	if (the_list == NULL)  {
		printf("WARNING: Attempt to get the length of a NULL list.\n");
		return 0;
	}
	
	if (the_list->val == NULL) {
		return 0;
	}		
	
	uint16_t length = 0;
	
	while ((the_list != NULL) && (the_list->val != NULL)) {
		length++;
		the_list = the_list->next;
	}
	return length;
}

//************************************
// Method:    linked_list_add_tail
// FullName:  linked_list_add_tail
// Access:    public 
// Returns:   int the position where the item has been added, otherwise -1 for error
// Qualifier: Add item in the tail of the list, if the maximum number of items is not reached.
//			  Since the list root is not touched, there is no need to return it.	
// Parameter: struct linked_list_t * the_list The list into which the item is added
// Parameter: void * the_val The item [reference to its memory] to be added in the list
// Parameter: int max_position The maximum allowed size for this list. 
//************************************
inline int linked_list_add_tail(struct linked_list_t * the_list, void* the_val, int max_position, bool atomic) {
	
	if (the_val == NULL) {
		printf("WARNING: Attempting to add a NULL item in the list.\n");
		return -1;
	}
	
	if (the_list == NULL) {
		printf("ERROR: Attempting push on a NULL linked list.\n");
		return -1;
	}	
	
	if (the_list->val == NULL) {
		/* List is empty; add at the root position. */
		the_list->val = the_val;
		/* Check for inconsistence here and signal error. */
		if (the_list->next != NULL) {
			printf("ERROR: List was empty but next position was non NULL.\n");
			return -1;
		}		
		return 0; /* Item has been added at -0- position. */	
			
	} else {
		/* List is not empty; attempt to add at the tail. */
		int counter = 1;
		/* Assign list reference pointer. */
		struct linked_list_t* pos = the_list;
		while(pos->next != NULL) {
			pos = pos->next;
			counter++;
		}
		if (counter >= max_position) {
			/* The list is full so the element can not be added. Signal a warning. */
			printf("WARNING: Maximum number of list elements is reached! Item not added.\n");
			return -1;
		}
		/* Allocate resources for list position. FIXME - Consider fixed number of elements 
		 * and pre-allocated memory for speeding up the process, but sacrificing memory.
		 */
		if (atomic) {
			pos->next = (struct linked_list_t*)smalloc(sizeof(struct linked_list_t));
		
		} else {
			pos->next = (struct linked_list_t*)malloc(sizeof(struct linked_list_t));
		}
		
		if(!pos->next) {
			printf("ERROR: No memory for linked list element addition!\n");
			return -1;
		
		} else {
			pos->next->val = the_val;
			pos->next->next = NULL;
			/* Return the position index, where the element was added. */
			return counter;
		}			
	}
}



//************************************
// Method:    linked_list_add_to_front
// FullName:  linked_list_add_to_front
// Access:    public 
// Returns:   struct linked_list_t* the new linked_list root position after addition
// Qualifier: Add a new element in the beginning of the linked list
// Parameter: struct linked_list_t * _list 
// Parameter: void * new_element
//************************************
inline struct linked_list_t* linked_list_add_to_front( struct linked_list_t* _list, void* new_element )
{
	if (new_element == NULL) {
		printf("WARNING: Attempt to add a NULL element in the front of the list.\n");
		return _list; /* TODO - check whether it is better to return NULL here */
	}
	
	if (_list == NULL) {
		printf("ERROR: Attempt to add an element in a NULL List!\n");
		return NULL;
	}
	
	/* If the list is empty, add in the first position */
	if (_list->val == NULL) {
		/* Check for inconsistence here */
		if (_list->next != NULL) {
			printf("ERROR: Root position is empty, while next is not. Signaling and error.\n");
			return NULL;
		
		} else {
			/* Everything is fine */
			_list->val = new_element;
			return _list;
		}
		
	} else {
		/* List is not NULL, so add in the front, allocating more resources */
		struct linked_list_t* new_list_root = (struct linked_list_t*)smalloc(sizeof(struct linked_list_t));
		if(!new_list_root) {
			printf("ERROR: No memory for list element allocation.\n");
			return NULL;
		
		} else {
			new_list_root->val = new_element;
			new_list_root->next = _list;
			return new_list_root;
		}
	}	
}



/* Get the first item of the linked list, no removal  */
inline void* linked_list_get(struct linked_list_t* the_list) {
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to get the first element of a null list.\n");
		return NULL;
	}		
	
	/* Return value is in the first position; could be NULL if list is empty */
	return the_list->val;	 
}


/* Get a specific element from the linked list, no removal */
inline void* linked_list_get_element_at(struct linked_list_t* _list, int index ) {
	
	if (_list == NULL) {
		printf("WARNING: List is NULL: cannot get element at %u.\n", index);
		return NULL;
	}
	/* The check is performed in advance, so we may get some delay here. 
	 * Instead the check could be done dynamically.
	 */
	if (index >= linked_list_get_len(_list)) {
		printf("WARNING: Trying to get a specific element from a null position in the list [%u] List length: %lu.\n",
			index,
			linked_list_get_len(_list));
		return NULL;
	}

	struct linked_list_t* the_list = _list;
	int counter = 0;
	
	void* item_to_return = the_list->val;
	
	while (counter != index) {
		the_list = the_list->next;
		item_to_return = the_list->val;
		counter++;
	}
	return item_to_return;
}



//************************************
// Method:    linked_list_remove_first
// FullName:  linked_list_remove_first
// Access:    public 
// Returns:   struct linked_list_t* The new root position of the linked list, after removal
// Qualifier: Removes the first element from the list; does not free the memory of the element, 
//				only the list resources! Also, it does not free the root position, if list has 
//				a single element, or is empty. This method is NOT interrupt-safe. MUST, then, be
//				EXTERNALLY protected, if required so.
// Parameter: struct linked_list_t * the_list
//************************************
inline struct linked_list_t* linked_list_remove_first(struct linked_list_t* the_list) {
	
	struct linked_list_t* updt_list;
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to remove from a null list.\n");
		return NULL;
	}		
	
	if (linked_list_is_empty(the_list)) {
		printf("WARNING: Attempt to remove from an empty list.\n");
		return the_list;
	}
	
	/* List is for sure not empty */
	if (the_list->next == NULL) {
		/* List contains a single element. */
		if (the_list->val != NULL) {
			/* This is redundant, otherwise signal a warning. */
			#if LINKED_LIST_DEBUG
			printf("WARNING: The resources of the removed list-element need to be freed later.\n");
			#endif
			/* We free them now. */
			free((struct sk_buff*)(the_list->val));
			the_list->val = NULL; 
		
		} else {
			printf("WARNING: The list element is already NULL.\n");
		}			
		return the_list;
			
	} else {
		/* The list contains more elements */
		updt_list = the_list->next;
		free(the_list->val);
		the_list->val = NULL;
		free(the_list);
		the_list = NULL;
		return updt_list;
	}		
}	

inline struct linked_list_t* linked_list_pop_first(struct linked_list_t* the_list) {
	
	struct linked_list_t* updt_list;
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to remove from a null list.\n");
		return NULL;
	}
	
	if (linked_list_is_empty(the_list)) {
		printf("WARNING: Attempt to remove from an empty list.\n");
		return the_list;
	}
	
	/* List is for sure not empty */
	if (the_list->next == NULL) {
		/* List contains a single element. */
		if (the_list->val != NULL) {
			/* This is redundant, otherwise signal a warning. */
			#if LINKED_LIST_DEBUG
			printf("WARNING: The resources of the removed list-element need to be freed later.\n");
			#endif
			/* We free them now. */
			//free((struct sk_buff*)(the_list->val));
			//the_list->val = NULL;
		}
		return the_list;
		
	} else {
		/* The list contains more elements */
		updt_list = the_list->next;
		free(the_list->val);
		the_list->val = NULL;
		free(the_list);
		the_list = NULL;
	}
	return updt_list;
}	


/* Same as above, but with freeing the direct memory of the first element. */
inline struct linked_list_t* linked_list_remove_first_deep(struct linked_list_t* the_list) {
	
	struct linked_list_t* updt_list;
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to remove from a null list.\n");
		return NULL;
	}
	
	if (linked_list_is_empty(the_list)) {
		printf("WARNING: Attempt to remove from an empty list.\n");
		return the_list;
	}
	
	/* List is for sure not empty */
	if (the_list->next == NULL) {
		/* List contains a single element. */
		if (the_list->val != NULL) {
			/* Free the direct memory, the list element is pointing at. */
			free(the_list->val);
			the_list->val = NULL;
		
		} else {
			printf("WARNING: The list seems to be empty.\n");
		}
		return the_list;
		
	} else {
		/* The list contains more elements */
		updt_list = the_list->next;
		if (the_list->val == NULL) {
			printf("ERROR: The list has next element but NULL root.\n");
		
		} else {
			
			free(the_list->val);
		}				 
		free(the_list);
		return updt_list;
	}		
}		
	

/* Free list and all memory associated with it */
inline void linked_list_erase(struct linked_list_t* _list) {
	
	struct linked_list_t* the_list = _list;
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to erase a NULL list.\n");
		return;
	}
	
	if (linked_list_is_empty(the_list)) {
		printf("WARNING: Attempt to erase an empty list.\n");
		return;
	}
	/* List is non empty */
	while(!linked_list_is_empty(the_list)) {
		/* Remove elements one by one */
		the_list = linked_list_remove_first(the_list);
	}
}

/* Erase the list elements and the direct memory reservations, 
 * in which the elements are pointing at.
 */
inline void linked_list_erase_deep(struct linked_list_t** _list) {
	
	struct linked_list_t* the_list = *(_list);
	
	if (the_list == NULL) {
		printf("ERROR: Attempt to erase a NULL list.\n");
		return;
	}
	
	if (linked_list_is_empty(the_list)) {
		printf("WARNING: Attempt to erase an empty list.\n");
		return;
	}
	
	/* List is non empty */
	while(!linked_list_is_empty(*(_list))) {
		/* Remove elements one by one */
		*(_list) = linked_list_remove_first_deep(*(_list));
	}
}


inline struct linked_list_t* linked_list_remove_element_at(struct linked_list_t* _list, int index) {

	if (_list == NULL) {
		printf("WARNING: Attempt to remove from a NULL list.\n");
		return NULL;
	}
	
	if (index >= linked_list_get_len(_list)) {
		printf("ERROR: Attempt to remove an element outside the boundaries of the list.\n");
		return NULL;
	}
	
	if (index == 0) {
		/* Remove from the front */
		return linked_list_pop_first(_list);
	
	} else {
				
		/* Initialize counter */
		int counter = 0;
		
		struct linked_list_t* the_list = _list;
		/* Previous element */
		struct linked_list_t* the_list_pr = NULL;
		
		/* Walk through the list */
		while (counter < index) {
			counter++;
			the_list_pr = the_list;
			the_list = the_list->next;
		}
		
		if (the_list_pr == NULL) {
			printf("ERROR: previous is NULL while we do not remove the front element.\n");
			return NULL;
		
		} else {
			/* Free and reconnect */
			the_list_pr->next = the_list->next;
			free(the_list);
			return _list;
		}
	}	
}	


/* Swap specific element with the one in the beginning of the linked list */
inline struct linked_list_t* linked_list_move_to_front(struct linked_list_t* _list, int index) {
	
	if (_list == NULL) {
		printf("WARNING: Attempt to swap in a NULL list.\n");
		return NULL;
	}
	
	if (index == 0) {
		/* Nothing to move. Already in the front. */
		return _list;
	}
	
	if (index >= linked_list_get_len(_list)) {
		printf("ERROR: Attempt to remove from a position out of bounds.\n");
		return NULL;
	}	
		
	/* Obtain the requested element */
	void* element_at_index = linked_list_get_element_at(_list, index);
	
	/* Remove the requested position from the list. The new list head is returned */
	_list = linked_list_remove_element_at(_list, index);
	
	return linked_list_add_to_front(_list, element_at_index);	
}	

#endif /* LINKED_LIST_H_ */

