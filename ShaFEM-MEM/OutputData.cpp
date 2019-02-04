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


#include "DataObject.h"
#include "OutputData.h"
#include <cstring>
#include <stdio.h>
#include <omp.h>
#include <iostream>
#include <cmath>

//----------------------------------------------------------------------------------------------
OutputData::OutputData(std::stringstream *out, int minsup, int itemcount) {
    lookup = 0;
    setcount = pos = 0;
    setpos = 0;
    setbuf = 0;
    buffer = 0;
    file = 0;
    setmaxsize(itemcount);
    file = out;

    if (file) {
        buffer = new char[IO_BUFFER_SIZE];
        lookup = new IntToString(minsup);
    }
};

//----------------------------------------------------------------------------------------------
OutputData::~OutputData() {
    close();

    if (setbuf) {
        delete[] setpos;
        delete[] setbuf;
        setcount = 0;
        setpos = 0;
        setbuf = 0;
    }
}

//----------------------------------------------------------------------------------------------
bool OutputData::open(std::stringstream *out, int minsup) {
    if (out->fail()) return false;

    pos = 0;

    file = out;
//	setvbuf( file, NULL, _IONBF, 0 );

    if (file) {
        buffer = new char[IO_BUFFER_SIZE];
        lookup = new IntToString(minsup);
    }

    return (file ? true : false);
}

//----------------------------------------------------------------------------------------------
//file should not close until all thread compele the computation
void OutputData::close() {
    if (file) {
        if (pos) //output file and buffer is not empty
        {
#pragma omp critical
            file->write(buffer, pos);
        }

        if (buffer) delete[] buffer;
        if (lookup) delete lookup;

        //because it is shared --> should not close until all other thread complete
        //the close is control explicited in code
        //file->flush();
        //fclose(file);

        //file = 0;
        buffer = 0;
        lookup = 0;
        pos = 0;
    }
}

void OutputData::flush() {
    if (file) {
        if (pos) //output file and buffer is not empty
        {
#pragma omp critical
            file->write(buffer, pos);
            pos = 0;
        }
    }
}

//----------------------------------------------------------------------------------------------
//set maximum size of itemset 
//----------------------------------------------------------------------------------------------
void OutputData::setmaxsize(int size) {
    if (!setbuf) setbuf = new char[(size + 1) * 20]; //20 : max lenght of an interger number
    if (!setpos) setpos = new int[(size + 1) * 20];
    memset(setpos, 0, sizeof(int) * (size + 1));
}

//----------------------------------------------------------------------------------------------
//this function is to be used with	IntToString verion 2
//----------------------------------------------------------------------------------------------
void OutputData::write(int item, int count, int size, double *mu_best) {
    setcount++;

    if (!file) return;

    //write to file when buffer is full, 20 = string lenght of largest iterger number
    if (pos > (IO_BUFFER_SIZE - 20 * (size + 1))) {
        // don't need critical section because fwrite is thread-safe on Linux & Windows
        // not true for string streams, critical section added
#pragma omp critical
        {
            file->write(buffer, pos);
            pos = 0;
        }
    }

    char *buf;
    int len;
    int temp; // stores only the length of the count, used for pruning

    //print the item
    if (buf = lookup->convertItem(item, len))
        memcpy(setbuf + setpos[size - 1], buf, sizeof(char) * len);
    else len = sprintf(setbuf + setpos[size - 1], "%d ", item);

    setpos[size] = setpos[size - 1] + len;

    //print the count (i.e. support)
    if (buf = lookup->convertCount(count, len))
        memcpy(setbuf + setpos[size], buf, sizeof(char) * len);
    else len = sprintf(setbuf + setpos[size], "(%d)\n", count);

    len = setpos[size] + len;

    double mu = count * pow((1 / 0.25), size);
    bool write = false;
#pragma omp critical
    {
        if (mu > *mu_best) {
            *mu_best = mu;
            write = true;
        }
    }

    //write the large buffer
    if (write){
        memcpy(buffer + pos, setbuf, sizeof(char) * len);
        pos += len;
    }
}

//----------------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------------
void OutputData::write(int *items, int lenght, int level, int count, int size, double *mu_best) {
    bool toRet = true;
    double mu = count * pow((1 / 0.25), size);
#pragma omp critical
    {
        if (mu > *mu_best) {
            *mu_best = mu;
            toRet = false;
        }
    }

    if (toRet) return; // skip adding itemset if not better than previously mined sets

    do {
        write(items[level++], count, size, mu_best);

        if (level < lenght) write(items, lenght, level, count, size + 1, mu_best); // TODO: make sure this works
        else break;

    } while (1);
}
