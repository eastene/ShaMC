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

#include <cstring>
#include <math.h>
#include <iostream>
#include "DataObject.h"
#include "InputData.h"

//----------------------------------------------------------------------------------
//Open the Input File 
//----------------------------------------------------------------------------------
bool InputData::Open(std::stringstream *in) {
    pos = 0;

    file = in; //fopen(FileName,"rt");
    filesize = GetFileSize();

    if (file) buffer = new char[IO_BUFFER_SIZE];

    return (file ? true : false);
}

//----------------------------------------------------------------------------------
//Close the Input File 
//----------------------------------------------------------------------------------
void InputData::Close() {
    if (file) {
        //if(buffer) delete[] buffer;
        //file->str("");
        //fclose(file);
        //delete file;
    }
    if (mask) {
        delete[] mask;
        mask = 0;
    }
    if (index) {
        delete[] index;
        index = 0;
    }
}

//----------------------------------------------------------------------------------
//Workload depends on the PartID and PartNum, this Function
//will set to read all transaction that contain the items whose
//id moded PartID is equal to zero 
//----------------------------------------------------------------------------------
void InputData::SetDataMask(int totalitems, Headlist *Heads, int PartID, int PartNum) {
    int i, n = Heads->size;

    tran.setmaxsize(n);

    mask = new char[n];
    memset(mask, 0, sizeof(char) * n);

    //create index list, index = 0
    index = new int[totalitems];

    //set index of frequent items
    memset(index, 0, sizeof(int) * totalitems);
    for (i = 0; i < n; i++) index[Heads->items[i].id] = (i % PartNum == PartID) ? -(i + 1) : (i + 1);

}
//----------------------------------------------------------------------------------
//Get Frequent Items in Next Transaction In the Data File, then Sort
//each item in transaction object is the index to headlist/itemlist
//this function is used for the first scan 
//only transactions with one of item whose index is negative is load
//so that only patterns contain the item in negative index are mined
//----------------------------------------------------------------------------------
//

Transaction *InputData::GetTransaction() {
    int i, j, item, num, idx;
    bool flag = false;
    bool isOK = false;
    char *buf = buffer + pos;
    item = tran.size = 0;

    do {
        if (pos == 0) {
            //read data into buffer if the buffer is empty
            //file->seekg(filepos, file->beg);
            unsigned long tmppos;
            file->read(buffer, IO_BUFFER_SIZE);
            size = file->gcount(); // - filepos;

            buf = buffer;

            if (file->tellg() != -1) {
                tmppos = file->tellg();
            } else {
                tmppos = filesize + filepos + 1;
            }

            //current file pos
            if ((tmppos - filepos) > filesize) size -= ((tmppos - filepos) - filesize);
        }

        for (i = pos; i < size; i++, buf++) {
            if ((*buf >= '0') && (*buf <= '9')) {
                item *= 10;
                item += int(*buf - '0');
                flag = true;
            } else if (flag == true) {
                idx = index[item];
                //if index of this item  == 0 --> infrequent item
                if (idx) {
                    mask[((idx > 0) ? idx : -idx) - 1] = 1;
                    if (idx < 0 && isOK == false) isOK = true;
                    if (tran.size < tran.maxsize)
                        tran.items[tran.size++] = ((idx > 0) ? idx : -idx) - 1;//tran.add(idx-1);
                }
                item = 0;
                flag = false;
            }

            // stop when reach a new line (i.e. the transaction ends)
            // and item transaction contain the items
            // ignore the transaction without item
            if (*buf == '\n' && tran.size) {
                pos = (i == (size - 1)) ? 0 : (i + 1);

                if (tran.size * (log((float) tran.size) / log(2.0) + 1) < tran.maxsize) {
                    //only sort on the short transaction
                    num = 0;
                    for (j = 0; j < tran.size; j++) {
                        if (mask[tran.items[j]] == 1) mask[tran.items[j]] = 0;
                        else {
                            tran.items[j] = tran.maxsize;
                            num++;
                        }
                    }
                    tran.sort();
                    tran.size -= num;
                } else {
                    //for long transaction, use the mask instead
                    num = tran.size;
                    tran.size = 0;
                    for (j = 0; j < tran.maxsize && num; j++) {
                        //if (mask[j]) { tran.add(j) ; num--;}
                        if (mask[j]) {
                            tran.items[tran.size++] = j;
                            num--;
                        }
                    }
                    memset(mask, 0, sizeof(char) * tran.maxsize);
                }
                if (isOK) return &tran;
            }
        }
        pos = 0;
    } while (size > 0);

    file->clear();
    file->seekg(filepos, file->beg);

    return 0;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
Transaction *InputData::GetTransaction1() {
    //if there is an transaction in the bag
    if (trans.empty() && (Flag == true)) {
        pair<set<Transaction>::iterator, bool> ret;
        set<Transaction>::iterator it;
        Transaction *p;
        Transaction t;
        int count = 0;
        t.count = 1;

        while (count < TRANSACTION_BUFFER_SIZE) {
            if (p = GetTransaction()) {
                t.size = p->size;
                t.items = new int[p->size];
                memcpy(t.items, p->items, sizeof(int) * p->size);
                ret = trans.insert(t);

                if (ret.second == false) {    //item exist
                    it = ret.first;
                    it->count = it->count + 1;
                    //			delete[] t.items;
                }
                count++;
            } else {
                Flag = false;
                break;
            }
        }

        if (count) {
            t.items = 0; //must have this line
            ptran = trans.begin();
        }
    }

    if (trans.size()) {
        tran.size = ptran->size;
        tran.count = ptran->count;
        memcpy(tran.items, ptran->items, sizeof(int) * tran.size);
        set<Transaction>::iterator it = ptran;
        ptran++;
        trans.erase(it);
        return &tran;
    } else {
        Flag = true;
        return 0;
    }
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
int InputData::CountItemFrequence(Countlist *ItemCounts) {
    int i;
    int item = 0;
    int trans = 0;
    unsigned long tmpsize;
    bool flag = false;
    char *buf;

    tmpsize = filesize;
    do {
        file->read(buffer, IO_BUFFER_SIZE);
        size = file->gcount();

        buf = buffer;

        if (tmpsize > size) tmpsize -= size;
        else {
            size = tmpsize;
            tmpsize = 0;
        }

        for (i = 0; i < size; i++, buf++) {
            if ((*buf >= '0') && (*buf <= '9')) {
                item *= 10;
                item += int(*buf - '0');
                flag = true;
            } else if (flag == true) {
                flag = false;
                ItemCounts->count(item);
                item = 0;
            }

            if (*buf == '\n') trans++;
        }
    } while (size > 0);

    file->clear();  // required to seek to beginning if read goes past end of file
    file->seekg(filepos, file->beg);

    return trans;
}

//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
unsigned long InputData::GetFileSize() {
    unsigned long size = 0;

    // get the file size
    size = file->str().size();

    return size;
}

//----------------------------------------------------------------------------------
//open the file based on the part id and the number of part
//----------------------------------------------------------------------------------
void InputData::SetDataPartition(int PartID, int PartNum) {
    char *buf;
    int n;

    //compute initial size and pos
    unsigned long fsize = GetFileSize();

    //if file is too small than one instance handle the mining task only
    if (fsize < IO_BUFFER_SIZE) {
        filesize = (PartID == 0) ? fsize : 0;
        filepos = 0;
        return;
    }

    filepos = (fsize / PartNum) * PartID;
    filesize = (PartID != PartNum - 1) ? (fsize / PartNum) : (fsize - (fsize / PartNum) * (PartNum - 1));

    //adjust pos to fit transactions
    if (PartID) {
        file->clear();
        file->seekg(filepos, ios_base::beg);
        buf = buffer;
        file->read(buffer, IO_BUFFER_SIZE);
        n = file->gcount();


        for (int i = 0; i < n; i++, buf++)
            if (*buf == '\n') {
                filepos += (i + 1);
                filesize -= (i + 1);
                break;
            }

    }

    //adjust size and pos to fit transactions
    if (PartID != PartNum - 1) {
        file->clear();
        file->seekg(filepos + filesize, ios_base::beg);
        buf = buffer;
        file->read(buffer, IO_BUFFER_SIZE);
        n = file->gcount();
        for (int i = 0; i < n; i++, buf++)
            if (*buf == '\n') {
                filesize += (i + 1);
                break;
            }

    }

    return;
}
