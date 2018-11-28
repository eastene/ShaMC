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


#ifndef IO_BUFFER_SIZE
#define IO_BUFFER_SIZE (1024*1024)  //1MB
#endif

#include <sstream>

class OutputData
{
public:
		unsigned int setcount;//total number itemsets
		int		pos;		//posion of buffer to get next transaction
		int*	setpos;		//position list of items in itemset
		char*	setbuf;		//buffer to store an itemset
		char*	buffer;		//buffer used to store data
		std::stringstream	*file;
		IntToString *lookup;

		OutputData(char *filename,int minsup,int itemcount);
		OutputData(std::stringstream *pfile,int minsup,int itemcount);
		 ~OutputData();
		bool open(char *filename,int minsup);
		void close();
		bool isopen() {return (file?true:false);};
		void write(int item, int count,int size);
		void write(int *items,int lenght,int level, int count,int size);
		void setmaxsize(int size);
		void RemoveFile(char *filename)	{remove(filename);};
		void MergeData(char *SrcFileName,char *DesFileName,bool isAppend);
};

