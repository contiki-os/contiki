/*
 * Copyright (c) 2006, Swedish Institute of Computer Science
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * @(#)$Id: crtk.c,v 1.1 2007/05/23 23:23:23 oliverschmidt Exp $
 */

/*
 * RUNES CRTK Implementation
 * Version 0.3
 *
 * Stefanos Zachariadis, UCL, s.zachariadis@cs.ucl.ac.uk
 */

#include <stdio.h>
#include <string.h>

#include "crtk.h"

static Attribute a = { "Author", "Stefanos Zachariadis", NULL };
static Entity attributes = { 0, COMPONENT, "crtk", &a, NULL };
static int entities;

static void construct(void) {
  printf("Instantiating RUNES middleware\n");
}

static void destroy(void) {
  printf("RUNES Middleware going away...\n");
}

static void printregistry(void) {
  printf("The Registry Currently Contains:\n");
  Entity *current;
  current=&attributes;
  while(current!=NULL) {
    Attribute *attrs;
    attrs=current->attributes;
    printf("Entity (%d,%d) \n",current->id, current->type);
    while(attrs!=NULL) {
      printf("   (%s,%s)\n",attrs->key,(char *)(attrs->value));
      attrs=attrs->next;
    }
    current=current->next;
  }
}


static void delent(Entity *e) {
  Entity *previous = &attributes; /* Not allowed to delete first! */
  Entity *current = previous->next;
  while(current != NULL) {
    if(e->id == current->id) {
      previous->next = current->next;
      current->next = NULL;
      return;
    }
    previous = current;
    current = current->next;
  }
}


static void putattr(Entity *e, Attribute *a) {
  Entity *current, *previous = NULL;
  current=&attributes;
  while(NULL!=current) {
    //printf("looping\n");
    if(e->id==current->id) { //the entity already exists in our databas
      //we need to see if an attribute with the same key exists
      Attribute *tmp;
      for(tmp=current->attributes;tmp->next!=NULL;tmp=tmp->next) {
	if(strcmp(tmp->key,a->key)==0) { //it does exist
	  tmp->value=a->value;
	  return;
	}
      }
      //we need to add this attribute
      tmp->next=a;
      a->next=NULL;
      return;
    }
    previous=current;
    current=current->next;    
  }
  e->next=NULL;
  previous->next=e;
  e->attributes=a;
  a->next=NULL;
}

static void getattr(Entity e, char *key, Attribute **atr) {
  //right, so we're going to go through our registry.
  Entity *current;
  current=&attributes;
  while(current!=NULL) {
    if(e.id==current->id) { //we found the element
      Attribute *tmp;      
      for(tmp=current->attributes;tmp!=NULL;tmp=tmp->next) {
	if(strcmp(tmp->key,key)==0) { //the key does exist
	  *atr=tmp;
	  return;
	}
      }
    }
    current=current->next;
  }
  atr=NULL;
}

static void connect(Interface i, Receptacle *r) {
  *r=i;
}



static void instantiate(Entity *e, Attribute *init) {  
  e->id=entities++;
  putattr(e,init);
}


static void getentities(Entity **e) {
  *e=&attributes;
}

IMPLEMENT_COMPONENT(crtk, { printregistry, delent, putattr, getattr,
			      connect, instantiate, getentities });
