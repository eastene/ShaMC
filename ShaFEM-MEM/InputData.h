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


#include <set>
#include <sstream>
#include <memory>

using namespace std;

//#define TRANSACTION_BUFFER_SIZE 1
#define TRANSACTION_BUFFER_SIZE 3000000  

#ifndef IO_BUFFER_SIZE
#define IO_BUFFER_SIZE (1024*1024)  //1MB
#endif

class InputData
{
	
 private:
	char	*buffer;	//buffer used to store data
	int		pos;		//posion of buffer to get next transaction
	int		size;		//size of the buffer
	set<Transaction> trans;
	set<Transaction>::iterator ptran;	
	unsigned long filepos; //size of whole file or data part it responsible for
	unsigned long filesize; //size of whole file or data part it responsible for
    std::stringstream* file;
	bool	Flag;
	
public:
	//static int a;
	int *index;	//list of mapping index of original item 
				//index = 0 : infrequent items
				//index = x : item with index (x-1) is frequent
				//index = -x: item with index (x-1) is frequent and the transaction must contain this item
	char *mask;
	Transaction	tran;

	InputData(std::stringstream* in): index(0),mask(0),filesize(0),filepos(0),size(0){Flag=true;Open(in);};
	//InputData(): index(0),mask(0),filesize(0),filepos(0),size(0){};
	 ~InputData() {Close();};
	bool Open(std::stringstream* in);
	void Close();
	bool IsOpen() {return (file?true:false);};
	void SetDataMask(int totalitems,Headlist* Heads,int PartID=0, int PartNum=1);
	void SetDataPartition(int PartID,int PartNum);
	int CountItemFrequence(Countlist* ItemCounts);
	unsigned long GetFileSize();

	Transaction* GetTransaction();
	Transaction* GetTransaction1();

};

