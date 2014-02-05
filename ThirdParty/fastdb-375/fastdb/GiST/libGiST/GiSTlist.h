// -*- Mode: C++ -*-

//         GiSTlist.h
//
// Copyright (c) 1996, Regents of the University of California
// $Header: /cvsroot/fastdb/fastdb/GiST/libGiST/GiSTlist.h,v 1.1 2007/07/18 20:45:49 knizhnik Exp $

#ifndef GISTLIST_H
#define GISTLIST_H

// A template list package, which is handy.


template <class T>
struct GiSTlistnode
{
  T entry;
  GiSTlistnode<T> *prev, *next;
};

template <class T>
class GiSTlist
{
  public:
    GiSTlist() { front = rear = NULL; }
//  GiSTlist(const GiSTlist<T>& l) { front = l.front; rear = l.rear; }
    int IsEmpty() { return front == NULL; }
    T RemoveFront() {
	assert(front != NULL);
	GiSTlistnode<T> *temp = front;
	T e = front->entry;
	front = front->next;
	if (front)
	    front->prev = NULL;
	else
	    rear = NULL;
	delete temp;
	return e;
    }
    T RemoveRear() {
	assert(rear != NULL);
	GiSTlistnode<T> *temp = rear;
	T e = rear->entry;
	rear = rear->prev;
	if (rear) rear->next = NULL; else front = NULL;
	delete temp;
	return e;
    }
    void Prepend(T entry) {
	GiSTlistnode<T> *temp = new GiSTlistnode<T>;

	temp->entry = entry;
	temp->next = front;
	temp->prev = NULL;

	if (rear == NULL)
	    rear = temp;
	else
	    front->prev = temp;
	front = temp;
    }
    void Append(T entry) {
	GiSTlistnode<T> *temp = new GiSTlistnode<T>;

	temp->entry = entry;
	temp->prev = rear;
	temp->next = NULL;

	if (front == NULL)
	    front = temp;
	else
	    rear->next = temp;
	rear = temp;
    }
  private:
    GiSTlistnode<T> *front, *rear;
};

#endif
