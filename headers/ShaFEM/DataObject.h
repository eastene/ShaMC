/*
   Author:  Lan Vu  

Copyright (c) 2017, University of Colorado Denver All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
   - Neither the name of University of Colorado Denver nor the names of its
       contributors may be used to endorse or promote products derived from
       this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#include <iomanip>
#include <omp.h>

typedef unsigned int TID_DATA;

//--------------------------------------------------------------------------
//Data structures
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//FP-tree node
//--------------------------------------------------------------------------

struct Node {
	int	id;			//id == index to its head nodes
	int	count;		//count of node
	Node*	parent;	//pointer to its parent node
	Node*	next;	//point to next nodes that have same parent node , = null end node in the list
	Node*	child;	//pointer to its first child node
	Node*	sibling;//pointer to its first child node

	void init(int Id, int Count,Node* Parent,Node* Child, Node* Sibling,Node* Next)
	{
		id = Id;
		count = Count;
		child = Child;
		sibling = Sibling;
		parent = Parent;
		next = Next;
	}
	void init() { init(-1,0,0,0,0,0);};
};



//--------------------------------------------------------------------------
//Entry of header table
//--------------------------------------------------------------------------
/*
struct Head {

	int	id;			//original id of items (in database)
	int	count;		//count of items
	int	nodes;		//count of nodes
	Node* next;		//point to next nodes t
	Head(int Id, int Count, int Nodes, Node* Next)
	{
		init(Id,Count,Nodes,Next);
	}
	Head() { init(-1,0,0,0) ;}

	void init(int Id, int Count, int Nodes, Node* Next)
	{
		id = Id;
		count = Count;
		nodes = Nodes;
		next = Next;
	}
	void init() { init(-1,0,0,0) ;};
};

//
*/

struct Head {

	int	id;			//original id of items (in database)
	int	count;		//count of items
	int	nodes;		//count of nodes
	Node* next;		//point to next nodes t
	Node* last;		//point to next nodes t


	Head(int Id, int Count, int Nodes, Node* Next)
	{
		init(Id,Count,Nodes,Next);
	}
	Head() { init(-1,0,0,0) ;}

	~Head()
	{
	}

	void init(int Id, int Count, int Nodes, Node* Next)
	{
		last = Next;
		id = Id;
		count = Count;
		nodes = Nodes;
		next = Next;
	}
	void init() { init(-1,0,0,0) ;};
};

//--------------------------------------------------------------------------
//TID bit vector
//--------------------------------------------------------------------------
struct Item {

public:
	TID_DATA *tid;
	int	id;		//the lastest id in the itemset
	int	count;	//size of the tid list
	Item() 
	{
		tid = 0;
		id = 0;
		count = 0;
	}
};

//--------------------------------------------------------------------------
//Sorting template functions
//--------------------------------------------------------------------------
template <typename T>
void ascendingsort(T *A,int size);

//------------------------------------------------
template <typename T>
void descendingsort(T *A,int size);

//------------------------------------------------
template <typename T>
void insertionsort(T *A,int size,bool ascending);

//------------------------------------------------
//sort all items in the ascending order
template <typename T>
void quicksort_ascending(T *A,int lo, int hi);

//------------------------------------------------
//sort all items in the ascending order
template <typename T>
void quicksort_descending(T *A,int lo, int hi);

//------------------------------------------------
//sort item in the head list by the descending order of count
void quicksort_item(Item* A, int lo, int hi);
//--------------------------------------------------------------------------
//List template 
//--------------------------------------------------------------------------
template <class T>
class List
{

public:
	List():maxsize(0), size(0),items(0){}
	~List(){ if (items) { delete[] items; items = 0; }}
	void destroy() { if (items) { delete[] items; items = 0; }}
	void add(T item);
	int search(T key);
	void copy(List *src);
	void sort(bool ascending = true);
	void setmaxsize(int Maxsize);
  	T &operator[](int index) { return items[index]; } 

	T *items;
	int size;
	int maxsize;
protected:
	void resize();
};

//--------------------------------------------------------------------------
//Itemset Class (used for output)
//--------------------------------------------------------------------------
class Itemset 
{
public:
	int		size;
	int		lenght;
	int		totalcount;
	int*	pos;
	char*	buffer;
	
	Itemset() { size = lenght = totalcount = 0 ; pos = 0 ; buffer = 0;}
	~Itemset() { if (buffer) delete[] buffer ; if (pos) delete[] pos;}

	void setmaxsize (int size);
	void print(int value, int count);
};


//--------------------------------------------------------------------------
// IntToString (used for output)
//--------------------------------------------------------------------------
//version 2
class IntToString
{
private:
	const static int LOOKUP_SIZE = (1024 * 64);
	char itemStrings[LOOKUP_SIZE][10];
	char countStrings[LOOKUP_SIZE][10];
	int itemLenghts[LOOKUP_SIZE];
	int countLenghts[LOOKUP_SIZE];
	int minsup;
public:
	IntToString(int minsupport) ;
	char *convertItem(int item, int& length);
	char *convertCount(int count, int& length);
};


//--------------------------------------------------------------------------
//Indexlist
//--------------------------------------------------------------------------
class Indexlist : public List<int> {};

//--------------------------------------------------------------------------
//Transaction
//--------------------------------------------------------------------------
//Transactions are used to store the indices of all frequent items of each 
//original transactions. Each index is used for fast access the appropriate 
//item in the head list
class Transaction : public List<int>
{
public:
	//http://stackoverflow.com/questions/3775414/assignment-of-data-member-in-read-only-structure-class-in-stl-set
	mutable int count;
	Transaction();
	void sort() ;
//	bool operator< (const Transaction  &e) const {return id < e.id;}
	bool operator< (const Transaction  &e) const ;
	void setmaxsize(int Maxsize);
};

//--------------------------------------------------------------------------
//Headlist store temporarily header table of FP-tree
//--------------------------------------------------------------------------
class Headlist : public List<Head>
{
public:	
	void sort() { quicksort(items,0,size-1);};
	void quicksort(Head* A, int lo, int hi);
	void add(Head item) ;
};

//--------------------------------------------------------------------------
//Countlist
//--------------------------------------------------------------------------
//Countlist is an list that store of items in the database
//Countlist size = max index of items + 1
//count[i] means item with index i has count[i]
#define COUNT_SIZE 10
class Countlist
{
public:
	Countlist();
	//~Countlist(){ if (counts) {delete[] counts; counts = 0;}}
	//void destroy() { if (counts) {delete[] counts; counts = 0;}}
	void count(int item);
	int getItemCount(int threshold=1);
  	int &operator[](int index) { return counts[index]; } 

	
	int size;   //current size of list
	int *counts; //list of items, not frequent items

protected:
	int maxsize;//expandable size of list
	void resize(int newsize); //resize the buffer to have room for more items

};
