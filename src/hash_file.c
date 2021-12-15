#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "bf.h"
#include "hash_file.h"
#define MAX_OPEN_FILES 20

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {      \
    BF_PrintError(code);    \
    return HT_ERROR;        \
  }                         \
}
/* ------------------------------------------------------------ */
int* id;               // id of each open file
int gd=1;
/* ------------------------------------------------------------ */

unsigned short bit_select(int num, short start, short end)
{
    assert(end >= start);

    const unsigned short mask = (1 << (end-start+1)) - 1;
    const unsigned short shift = start - 1;

    return (num & (mask << shift)) >> shift;
}

unsigned short Hash(unsigned short id){
    unsigned short hashed_id=0;
    unsigned short t1=bit_select(id,1,7)+1;
    unsigned short t2=bit_select(id,8,10)+1;
    unsigned short t3=bit_select(id,6,9)+1;
    unsigned short t4=bit_select(id,3,5)+1;
    unsigned short t5=bit_select(id,1,4)+1;
    t1=t1<<8;
    t2=t2<<4;
    t5=t5<<5;
    //hashed_id *= t5;
    hashed_id=((((((t1*(t2)*(t3+t1)*t4*t5)%(t5*t4-t3)+t2/t4+t3*t1)*t3)*(t5*t5*t2))*56789)%65535);
    return hashed_id;
}

int num_of_digits(int n)  
{  
    int counter=0; // variable declaration  
    while(n!=0)  
    {  
        n=n/10;  
        counter++;  
    }  
    return counter;  
}  

/* ------------------------------------------------------------ */
HT_ErrorCode HT_Init() {

    id = (int*) malloc(MAX_OPEN_FILES* sizeof(int));
    for(int i=0 ; i<MAX_OPEN_FILES ; ++i)
        id[i] = -1; // -1 indicates that this is free space and can hold a file id
    printf("HT_Init: Returning 'HT_OK'\n");
    return HT_OK;
}

HT_ErrorCode HT_CreateIndex(const char *filename, int depth) {
    CALL_BF(BF_CreateFile(filename));
    int fd;
    CALL_BF(BF_OpenFile(filename, &fd));
    int i;
    

    // creating index block
    BF_Block* block;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(fd,block));
    char* data = BF_Block_GetData(block);
    data[0]=1;
    data[1]=2;
    //memset(data, (char)depth, sizeof(char));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    
    // creating block 1 for 0b0
    BF_Block* block1;
    BF_Block_Init(&block1);
    CALL_BF(BF_AllocateBlock(fd,block1));
    char* data1 = BF_Block_GetData(block1);
    data1[511] = -1;
    data1[510] = 1;
    data1[509] = 0;
    for(i=0; i<8 ; i++){
        data1[i*62]=0;
    }
    BF_Block_SetDirty(block1);
    CALL_BF(BF_UnpinBlock(block1));

    // creating block 2 for 0b1
    BF_Block* block2;
    BF_Block_Init(&block2);
    CALL_BF(BF_AllocateBlock(fd,block2));
    char* data2 = BF_Block_GetData(block2);
    data2[511] = -1;
    data2[510] = 1;
    data2[509] = 0;
    for(i=0; i<8 ; i++){
        data2[i*62]=0;
    }
    BF_Block_SetDirty(block2);
    CALL_BF(BF_UnpinBlock(block2));

    /*CALL_BF(BF_CloseFile(fd));
    CALL_BF(BF_Close());*/

    printf("HT_CreateFile: Created file successfully'\n");
    return HT_OK;
}

HT_ErrorCode HT_OpenIndex(const char *fileName, int *indexDesc){
    int idF;
    CALL_BF(BF_OpenFile(fileName,&idF));
    for(int i=0 ; i<MAX_OPEN_FILES ; ++i)
    {
        if (id[i] == -1) // if free space to store id input
        {
            *indexDesc = i;
            id[i] = idF;
            break;
        }
    }
    printf("HT_OpenFile: Opened file successfully!\n");
    return HT_OK;
}

HT_ErrorCode HT_CloseFile(int indexDesc) {
    CALL_BF(BF_CloseFile(id[indexDesc]))
    id[indexDesc] = -1;
    printf("HT_CloseFile: Closed file successfully!\n");
    return HT_OK;
}

HT_ErrorCode HT_InsertEntry(int indexDesc, Record record) {
    //printf("GD IS %d\n", gd);
    BF_Block* index_block;
    BF_Block_Init(&index_block);
    int t;
    CALL_BF(BF_GetBlock(id[indexDesc], 0, index_block));
    char* index = BF_Block_GetData(index_block); // this is the index
    unsigned short temp_hash = Hash((unsigned)record.id); // number to hash
    unsigned short head = bit_select(temp_hash, 16-gd+1, 16);
    int i;
    BF_Block* block;
    BF_Block_Init(&block);
    if(index[head]<0){
        t=index[head]+2*128;
    }
    else{
        t=index[head];
    }
    CALL_BF(BF_GetBlock(id[indexDesc], t,block));
    char* data = BF_Block_GetData(block);
    //printf("RECORD ID IS %d,HASH IS %d AND DATA[509] IS %d\n",record.id , head ,data[509]);
    int gtemp=1;
    for( int te=0; te<gd; te++){
        gtemp=gtemp*2;
    }
    //printf("GD IS %d AND GTEMP IS %d\n", gd, gtemp);
    if(data[509]<8)
    {
        for(i=0; i<8 ; i++){
            if(data[i*62]==0){
                int size = num_of_digits(record.id);
                unsigned int offset = i*62;
                sprintf(data+offset,"%d",record.id);
                offset += num_of_digits(record.id);
                data[offset] = ' ';
                offset++;
                memcpy(data+offset, record.name, strlen(record.name));
                offset += strlen(record.name);
                data[offset++] = ' ';
                memcpy(data+offset, record.surname, strlen(record.surname));
                offset += strlen(record.surname);
                data[offset++] = ' ';
                memcpy(data+offset, record.city, strlen(record.city));
                offset += strlen(record.city);
                data[offset] = ' ';
                ++data[509];
                break;
            }
        }
    }else{
        if(data[510]<gd){
            //printf("ITS MY TIMEEEE\n");
            Record temp;
            BF_Block* block1;
            BF_Block_Init(&block1);
            CALL_BF(BF_AllocateBlock(id[indexDesc],block1));
            char* new_block = BF_Block_GetData(block1);
            new_block[511] = -1;
            new_block[510] = data[510]+1;
            new_block[509] = 0;
            for(int of=0; of<8 ; of++){
                new_block[of*62]=0;
            }
            int bet;
            head = bit_select(temp_hash, 16-gd+1, 16);
            int count;
            int prev=index[head];
            BF_GetBlockCounter(id[indexDesc],&count);
            index[head]=count-1;
            int ld=data[510];
            data[510]++;
            int j;
            
            int realmax2;
            int bet2;
            int maxstep=gd-ld;
            //printf("max sterp is %d\n",maxstep);
            int realmaxstep=1;
            for(j=1; j<maxstep; j++){
                realmaxstep=realmaxstep*2;
            }
            realmax2=realmaxstep*2;
            
            //printf("real max is %d\n", realmaxstep);
            bet=head;
            for(j=1; j<maxstep; j++){
                bet=bet/2;
            }
            bet2=bet/2;
            //printf("bet bef is %d\n", bet);
            for(j=1; j<maxstep; j++){
                bet=bet*2;
                bet2=bet2*2;
            }
            bet2=bet2*2;
            //printf("bet after some is %d\n", bet);
            //printf("realmax is %d\n", realmaxstep);
            for(j=0; j<realmaxstep; j++)
            {
                index[bet+j]=count-1;
            }
            //printf("--------\n");
            //printf("Global Deapth: %d\n",gd);
            //for(j=0; j<gtemp; j++){
            //    printf("index[%d] is %d\n",j,index[j]);
            //}
            //printf("--------\n");
            int beforafte=bet2;
            realmax2--;
            //printf("lower lim %d\n",bet2);
            //printf("upper lim %d\n",realmax2+bet2);
            beforafte=( beforafte + realmax2+bet2)/2; 
            //printf("middle is %d and head is %d\n", beforafte,head);
            if(head>beforafte)
            {
                //printf("after\n");
                beforafte=1;
            }
            else
            {
                //printf("before\n");
                beforafte=0;
            }
            if(beforafte==1)
            {
                t=index[bet-realmaxstep];
                if(index[bet-realmaxstep]<0){
                    t=index[bet-realmaxstep]+2*128;
                }
                CALL_BF(BF_GetBlock(id[indexDesc], t,block));
                //printf("block to get is %d\n", t);
            }
            else
            { 
                t=index[bet+realmaxstep];
                if(index[bet+realmaxstep]<0){
                    t=index[bet+realmaxstep]+2*128;
                }
                CALL_BF(BF_GetBlock(id[indexDesc], t,block));
                //printf("block to get is %d\n", t);
            }
            //printf("bet after is %d\n", bet);
            char* data = BF_Block_GetData(block);
            int fl;
            for(i=0; i<8 ; i++){
                if((data+i*62)!=0){   
                    sscanf(data+i*62, "%d ", &temp.id);
                    temp_hash = Hash((unsigned)temp.id);
                    temp_hash = bit_select(temp_hash, 16-gd+1, 16);
                    fl=0;
                    //printf("--new run--\n");
                    for(j=0; j<realmaxstep; j++)
                    {   
                        //printf("temp_hash is %d and index[bet] is %d and id is %d\n", temp_hash, bet+j ,temp.id);
                        if(temp_hash==bet+j){
                            fl++;
                        }        
                    }    
                        if(fl>0){
                            sscanf(data+i*62, "%d %s %s %s", &temp.id, temp.name, temp.surname, temp.city);
                            //printf(" and i got in");
                            int size = num_of_digits(temp.id);
                            int offset = new_block[509]*62;
                            sprintf(new_block+offset,"%d",temp.id);
                            offset += num_of_digits(temp.id);
                            new_block[offset] = ' ';
                            offset++;
                            memcpy(new_block+offset, temp.name, strlen(temp.name));
                            offset += strlen(temp.name);
                            new_block[offset++] = ' ';
                            memcpy(new_block+offset, temp.surname, strlen(temp.surname));
                            offset += strlen(temp.surname);
                            new_block[offset++] = ' ';
                            memcpy(new_block+offset, temp.city, strlen(temp.city));
                            offset += strlen(temp.city);
                            new_block[offset] = ' ';
                            ++new_block[509];
                            data[i*62]=0;
                            data[509]--;
                            //printf("\n");
                        }
                        //printf("\n");
                }
            }
            //printf("head is %d\n", head);
            
            //printf("head is %d\n", index[head]);
            t=index[head];
            if(index[head]<0){
                t=index[head]+2*128;
            }
            
            //printf("head is %d\n", t);
            CALL_BF(BF_GetBlock(id[indexDesc], t,block));
            
            //printf("after this\n");
            data = BF_Block_GetData(block);
            temp_hash = Hash((unsigned)record.id); // number to hash
            head = bit_select(temp_hash, 16-gd+1, 16);
            for(i=0; i<8 ; i++){
                if(data[i*62]==0){
                    int size = num_of_digits(record.id);
                    unsigned int offset = i*62;
                    sprintf(data+offset,"%d",record.id);
                    offset += num_of_digits(record.id);
                    data[offset] = ' ';
                    offset++;
                    memcpy(data+offset, record.name, strlen(record.name));
                    offset += strlen(record.name);
                    data[offset++] = ' ';
                    memcpy(data+offset, record.surname, strlen(record.surname));
                    offset += strlen(record.surname);
                    data[offset++] = ' ';
                    memcpy(data+offset, record.city, strlen(record.city));
                    offset += strlen(record.city);
                    data[offset] = ' ';
                    ++data[509];
                    break;
                }
            }
        }
        else
        {
            for(int j=gtemp-1; j>=0; j--){
                index[j*2]=index[j];
                index[j*2+1]=index[j];
            }
            gd++;
            gtemp*=2;
            Record temp;
            
            BF_Block* block1;
            BF_Block_Init(&block1);
            CALL_BF(BF_AllocateBlock(id[indexDesc],block1));
            char* new_block = BF_Block_GetData(block1);
            new_block[511] = -1;
            new_block[510] = gd;
            new_block[509] = 0;
            for(int of=0; of<8 ; of++){
                new_block[of*62]=0;
            }       
            int bet;
            head = bit_select(temp_hash, 16-gd+1, 16);
            int count;
            BF_GetBlockCounter(id[indexDesc],&count);
            index[head]=count-1;
            
            if(head%2==0){
                //CALL_BF(BF_GetBlock(id[indexDesc], index[head+1],block));
                bet=head+1;  
            }
            else
            {
                bet=head-1;
            }
            if(index[bet]<0){
                t=index[bet]+2*128;
            }
            else{
                t=index[bet];
            }
            
            CALL_BF(BF_GetBlock(id[indexDesc], t,block));
            char* data = BF_Block_GetData(block);
            data[510]=gd;
            for(i=0; i<8 ; i++){
                sscanf(data+i*62, "%d ", &temp.id);
                temp_hash = Hash((unsigned)temp.id);
                temp_hash = bit_select(temp_hash, 16-gd+1, 16);
                if(temp_hash==index[bet]){
                    sscanf(data+i*62, "%d %s %s %s", &temp.id, temp.name, temp.surname, temp.city);
                    int size = num_of_digits(temp.id);
                    int offset = new_block[509]*62;
                    sprintf(new_block+offset,"%d",temp.id);
                    offset += num_of_digits(temp.id);
                    new_block[offset] = ' ';
                    offset++;
                    memcpy(new_block+offset, temp.name, strlen(temp.name));
                    offset += strlen(temp.name);
                    new_block[offset++] = ' ';
                    memcpy(new_block+offset, temp.surname, strlen(temp.surname));
                    offset += strlen(temp.surname);
                    new_block[offset++] = ' ';
                    memcpy(new_block+offset, temp.city, strlen(temp.city));
                    offset += strlen(temp.city);
                    new_block[offset] = ' ';
                    ++new_block[509];
                    data[i*62]=0;
                    data[509]--;
                }
            }
            //printf("index block is %d\n", index[head]);
            //printf("block is %d\n", t);
            //printf("here is the error\n");
            if(index[head]<0){
                t=index[head]+2*128;
            }
            else{
                t=index[head];
            }
            CALL_BF(BF_GetBlock(id[indexDesc], t,block));
            data = BF_Block_GetData(block);
            temp_hash = Hash((unsigned)record.id); // number to hash
            head = bit_select(temp_hash, 16-gd+1, 16);
            for(i=0; i<8 ; i++){
                if(data[i*62]==0){
                    int size = num_of_digits(record.id);
                    unsigned int offset = i*62;
                    sprintf(data+offset,"%d",record.id);
                    offset += num_of_digits(record.id);
                    data[offset] = ' ';
                    offset++;
                    memcpy(data+offset, record.name, strlen(record.name));
                    offset += strlen(record.name);
                    data[offset++] = ' ';
                    memcpy(data+offset, record.surname, strlen(record.surname));
                    offset += strlen(record.surname);
                    data[offset++] = ' ';
                    memcpy(data+offset, record.city, strlen(record.city));
                    offset += strlen(record.city);
                    data[offset] = ' ';
                    ++data[509];
                    break;
                }
            }
            //printf("--------\n");
            //printf("Global Deapth: %d\n",gd);
            //for(int j=0; j<gtemp; j++){
            //    printf("index[%d] is %d\n",j,index[j]);
            //}
            //printf("--------\n");
        }
    }
    CALL_BF(BF_UnpinBlock(index_block));
    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    return HT_OK;

}

HT_ErrorCode HT_PrintAllEntries(int indexDesc, int *idKey) {
    printf("----------------------------------------------------------------\n\n");
    printf("Searching for %d\n", *idKey);
    Record printData;
    int blocks;
    int temp_hash;
    BF_GetBlockCounter(id[indexDesc],&blocks);
    int sum=0;
    for(int j=1; j < blocks; j++){
        BF_Block* block1;
        BF_Block_Init(&block1);
        CALL_BF(BF_GetBlock(id[indexDesc], j, block1));
        char* data1 = BF_Block_GetData(block1);
        int offset = 0;
        int i=0;
        int f=0;
        sum+=data1[509];
        while(f<data1[509])
        {
            if(data1[i*62]!=0){
                sscanf(data1+i*62, "%d %s %s %s", &printData.id, printData.name, printData.surname, printData.city);
                if(printData.id==*idKey || idKey==NULL){
                    printf("\n--DATA %d--\n",j);
                    printf("Local Deapth: %d\n",data1[510]);
                    temp_hash = Hash((unsigned)printData.id);
                    temp_hash = bit_select(temp_hash, 16-gd+1, 16);
                    printf("hashed_ID: * %d *\n",temp_hash);
                    printf("id: %d\n",printData.id);
                    printf("name: %s\n",printData.name);
                    printf("surname: %s\n",printData.surname);
                    printf("city: %s\n",printData.city);
                    printf("\n\n");
                }
                f++;
            }
            i++;
        }
    CALL_BF(BF_UnpinBlock(block1));
    }
    printf("----------------------------------------------------------------\n\n");
    return HT_OK;
}

HT_ErrorCode HashStatistics(const char* filename){
    int indexDesc;
    HT_OpenIndex(filename, &indexDesc);
    int blocks;
    BF_GetBlockCounter(id[indexDesc],&blocks);
    int sum=0;
    int min=15; //καθε bucket max 9 αρα 15 λογικο οριο
    int max=0;
    for(int j=1; j < blocks; j++){
        BF_Block* block1;
        BF_Block_Init(&block1);
        CALL_BF(BF_GetBlock(id[indexDesc], j, block1));
        char* data1 = BF_Block_GetData(block1);
        sum+=data1[509];
        if(data1[509]>max){
            max=data1[509];
        }
        if(data1[509]<min){
            min=data1[509];
        }
    CALL_BF(BF_UnpinBlock(block1));
    }
    printf("\n\nTotal inputs: %d\n", sum);
    printf("Max inputs: %d\n", max);
    printf("Minimum inputs: %d\n", min);
    printf("Average inputs: %d\n\n", sum/blocks);
    //HT_CloseFile(indexDesc);
    return HT_OK;

}


