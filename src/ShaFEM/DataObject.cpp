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


//#pragma once
#include "../../headers/ShaFEM/DataObject.h"
#include <stdlib.h>  
#include <cstring>
#include <stdio.h>

//--------------------------------------------------------------------------
//Sorting template functions
//--------------------------------------------------------------------------
template <typename T>
void ascendingsort(T *A,int size)
{
	if (size > 1) quicksort_ascending(A,0,size-1);
}
//------------------------------------------------
template <typename T>
void descendingsort(T *A,int size)
{
	if (size > 1) quicksort_descending(A,0,size-1);
}
//------------------------------------------------
template <typename T>
void insertionsort(T *A,int size,bool ascending)
{
	int i,j;
	T temp;
	if (ascending == true)  
	{
		for (i = 0; i < size ; i++)
			for (j = i+1; j < size ; j++)
				if (A[i]>A[j])	{temp = A[i]; A[i] = A[j]; A[j] = temp;}
	}
	else
	{
		for (i = 0; i < size ; i++)
			for (j = i+1; j < size ; j++)
				if (A[i]<A[j])	{temp = A[i]; A[i] = A[j]; A[j] = temp;}
	}
}
//------------------------------------------------
//sort all items in the ascending order
template <typename T>
void quicksort_ascending(T *A,int lo, int hi)
{
	int i=lo;
	int j=hi;
	int N=hi;
	T Y,X=A[(lo+hi)/2];

	// partition
	do
	{ 
		while (A[i] < X) i++; 
		while (A[j] > X) j--;
		if (i<=j)
		{
			Y = A[i]; A[i]=A[j]; A[j]=Y;
			i++; j--;
		}
	} while (i<=j);

	// recursion
	if (lo<j) quicksort_ascending(A, lo, j);
	if (i<hi) quicksort_ascending(A, i, hi);
}
//------------------------------------------------
//sort all items in the ascending order
template <typename T>
void quicksort_descending(T *A,int lo, int hi)
{
	int i=lo;
	int j=hi;
	int N=hi;
	T Y,X=A[(lo+hi)/2];
	// partition
	do
	{ 
		while (A[i] > X) i++; 
		while (A[j] < X) j--;
		if (i<=j)
		{
			Y = A[i]; A[i]=A[j]; A[j]=Y;
			i++; j--;
		}

	} while (i<=j);

	// recursion
	if (lo<j) quicksort_descending(A, lo, j);
	if (i<hi) quicksort_descending(A, i, hi);
}
/*
//------------------------------------------------
//sort itemset in the head list by the descending order of count
void quicksort_head1(Head* A,int* index, int lo, int hi,bool &isSort)
{
	int tmp;
	int i=lo;
	int j=hi;
	int mid = (lo+hi)/2;
	int N=hi;
	int X= A[mid].count;
	Head Y;

	// partition
	do
	{ 
		while (A[i].count > X) i++; 
		while (A[j].count < X) j--;
		if (i<=j)
		{
			Y = A[i]; A[i]=A[j]; A[j]=Y;
			tmp = index[i]; index[i] = index[j] ;index[j] = tmp;
			i++; j--;

			if (isSort==false) isSort = true;
		}
	} while (i<=j);
	// recursion
	if (lo<j) quicksort_head1(A,index,lo, j,isSort);
	if (i<hi) quicksort_head1(A,index,i, hi,isSort);
}
*/
//------------------------------------------------
//sort itemset in the head list by the descending order of count
void quicksort_item(Item* A, int lo, int hi)
{
	int i=lo;
	int j=hi;
	int mid = (lo+hi)/2;
	int N=hi;
	int X= A[mid].count;
	Item Y;

	// partition
	do
	{ 
		while (A[i].count > X) i++; 
		while (A[j].count < X) j--;
		if (i<=j)
		{
			Y = A[i]; 
			A[i]=A[j]; 
			A[j]=Y;
			i++; j--;
		}
	} while (i<=j);

	// recursion
	if (lo<j) quicksort_item(A,lo, j);
	if (i<hi) quicksort_item(A,i, hi);
}

//--------------------------------------------------------------------------
//List template 
//--------------------------------------------------------------------------
template <class T>
int List<T>::search(T key)
{

	for (int i = 0 ; i < size ; i++)
		if (items[i] == key) return i;

	return -1;
}

// resize the buffer to have room for more items
template <class T>
void List<T>::setmaxsize(int Maxsize)
{
	T* temp = new T[Maxsize];
		
	if (size)
	{	
		memcpy(temp,items,sizeof(T)*size);
		delete[] items;
	}

	items = temp;

	maxsize = Maxsize;
}

//double the buffer size of list but do not change list size
//used by object only to auto resize the buffer size
template <class T>
void List<T>::resize()
{
	maxsize = (size==0)?1:(2*size);
	T* temp = new T[maxsize];

	if (size)
	{
		memcpy(temp,items,sizeof(T)*size);
		delete[] items;
	}
	items = temp;
}

//add an itemset to list
template <class T>
void List<T>::add(T item)
{
	if(size >= maxsize)	resize();
	items[size++] = item;
}
//copy this list from the src list
template <class T>
void List<T>::copy(List *src)
{
	if (size != src->size) resize(src->size);

	memcpy(items,src->items,sizeof(T)*src->size);
}

//--------------------------------------------------------------------------
//Itemset Class (used for output)
//--------------------------------------------------------------------------
void Itemset::setmaxsize (int size)
{
	buffer = new char[(size + 1)*20]; //20 : max lenght of an interger number
	pos = new int[(size + 1)*20]; 
	memset(pos,0,sizeof(int)*(size+1));
}

void Itemset::print(int value, int count)
{
	lenght = pos[size-1];

	lenght += sprintf(buffer + pos[size-1],"%d ",value);

	pos[size] = lenght;

	lenght += sprintf(buffer + lenght,"(%d)\n",count);

}
//--------------------------------------------------------------------------
// IntToString (used for output)
//--------------------------------------------------------------------------
//version 2
IntToString::IntToString(int minsupport) 
{ 
	minsup = minsupport;
	for (int i = 0; i < LOOKUP_SIZE; i++)
	{
		itemLenghts[i] = sprintf(itemStrings[i],"%d ", i);
		countLenghts[i] = sprintf(countStrings[i],"(%d)\n", i + minsup);
	}
}

char* IntToString::convertItem(int item, int& length)
{
	if (item < LOOKUP_SIZE)
	{
		length = itemLenghts[item];
		return itemStrings[item];
	}

	return NULL;
}
char* IntToString::convertCount(int count, int& length)
{
	int countidx = count-minsup;
	if (countidx < LOOKUP_SIZE)
	{
		length = countLenghts[countidx];
		return countStrings[countidx];
	}

	return NULL;
}
//--------------------------------------------------------------------------
//Transaction
//--------------------------------------------------------------------------
//Transactions are used to store the indices of all frequent items of each 
//original transactions. Each index is used for fast access the appropriate 
//itemset in the head list
Transaction::Transaction() { items = 0 ; size = maxsize = 0; count = 1;}

void Transaction::sort() { ascendingsort(items,size); }
//	bool operator< (const Transaction  &e) const {return id < e.id;}

bool Transaction::operator< (const Transaction  &e) const {

	const int *a, *b;
	int d, n = ((size < e.size)?size:e.size);

	a = items;
	b = e.items;
	d = 0;

	for ( ; (n > 0) && (d == 0); a++, b++,n--) d = *a -*b;              

	if ((d == 0) &&  (size < e.size))  d =-1; 

	return (d<0);

}

void Transaction::setmaxsize(int Maxsize)
{
	int* temp = new int[Maxsize];
		
	if (size)
	{	
		memcpy(temp,items,sizeof(int)*size);
		delete[] items;
	}

	items = temp;

	maxsize = Maxsize;
}

//--------------------------------------------------------------------------
//Headlist store temporarily header table of FP-tree
//--------------------------------------------------------------------------
void Headlist::quicksort(Head* A, int lo, int hi)
{
	int i=lo;
	int j=hi;
	int mid = (lo+hi)/2;
	int N=hi;
	int X= A[mid].count;
	Head Y;

	// partition
	do
	{ 
		while (A[i].count > X) i++; 
		while (A[j].count < X) j--;
		if (i<=j)
		{
			Y = A[i]; 
			A[i]=A[j]; 
			A[j]=Y;
			i++; j--;
		}
	} while (i<=j);
	// recursion
	if (lo<j) quicksort(A,lo, j);
	if (i<hi) quicksort(A,i, hi);
}

void Headlist::add(Head item) 
{	if(size >= maxsize)	
	{	
		maxsize = (size==0)?1:(2*size);
		Head* temp = new Head[maxsize];

		if (size)
		{
			memcpy(temp,items,sizeof(Head)*size);
			delete[] items;
		}
		items = temp;
	}
	items[size++] = item; 
}

//--------------------------------------------------------------------------
//Countlist
//--------------------------------------------------------------------------

Countlist::Countlist()
{
	
	size = 0;
	maxsize = COUNT_SIZE;
	counts = new int[COUNT_SIZE];
	for(int i=0; i<COUNT_SIZE; i++) counts[i] = 0;
}

//resize the list so that it can store the count of itemset
void Countlist::resize(int newsize)
{
	int* temp = new int[newsize];

	//old chunk
	memcpy(temp,counts,sizeof(int)*maxsize); //for(int i=0; i<maxsize; i++) temp[i] = counts[i];

	//new chunk
	for(int i=maxsize; i<newsize; i++) temp[i] = 0;

	maxsize = newsize;

	delete []counts;
	counts = temp;
}

//increase the count of itemset into 1
void Countlist::count(int item)
{
	if(item >= size)
	{
		//if this itemset /is beyounce the max lenght of current count list
		if(item >= maxsize)	resize(2*item);
		
		size = item + 1;
	}
	counts[item]++;
}

//get number of itemset that has count > threshold
int Countlist::getItemCount(int threshold)
{
	int sum = 0;

	for (int i =0 ; i < size ;i++)	if (counts[i]>=threshold) sum++; 
	
	return sum;
}
