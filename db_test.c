#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct{
    int id;
    char name[15];
    char surname[20];
    char city[10];
}Record;

void bin(unsigned short n)
{
    unsigned short i;
    for (i = 1 << 15; i > 0; i = i / 2)
        (n & i) ? printf("1") : printf("0");
}

unsigned short Hash(int id) 
{
    unsigned short hashed_id = (unsigned short)id;
    hashed_id *= 1000;
    hashed_id -= 200000;
    hashed_id << 1;
    hashed_id *= 100;
    return hashed_id-23;
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

const char* names[] = {
  "Yannis",
  "Christofos",
  "Sofia",
  "Marianna",
  "Vagelis",
  "Maria",
  "Iosif",
  "Dionisis",
  "Konstantina",
  "Theofilos",
  "Giorgos",
  "Dimitris"
};

const char* surnames[] = {
  "Ioannidis",
  "Svingos",
  "Karvounari",
  "Rezkalla",
  "Nikolopoulos",
  "Berreta",
  "Koronis",
  "Gaitanis",
  "Oikonomou",
  "Mailis",
  "Michas",
  "Halatsis"
};

const char* cities[] = {
  "Athens",
  "San_Francisco",
  "Los_Angeles",
  "Amsterdam",
  "London",
  "New_York",
  "Tokyo",
  "Hong_Kong",
  "Munich",
  "Miami"
};

int main()
{   
    long now;
    now = time(NULL);
    srand((unsigned int)now);

    char* test;
    test = (char*)malloc(sizeof(char)*512);
    for(int i=0 ; i<512 ; ++i)
        test[i] = -1;

    Record record;

    int r;
    record.id = rand() % 10000;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);


    int size = num_of_digits(record.id);
    int offset = 0;
    
    sprintf(test+offset,"%d",record.id);
    offset += num_of_digits(record.id);
    test[offset] = ' ';
    ++offset;
    memcpy(test+offset, record.name, strlen(record.name));
    offset += strlen(record.name);
    test[offset++] = ' ';
    memcpy(test+offset, record.surname, strlen(record.surname));
    offset += strlen(record.surname);
    test[offset++] = ' ';
    memcpy(test+offset, record.city, strlen(record.city));
    offset += strlen(record.city);
    test[offset++] = ' ';
    test[offset++] = '\0';

    offset = 59;

    record.id = rand() % 10000;
    r = rand() % 12;
    memcpy(record.name, names[r], strlen(names[r]) + 1);
    r = rand() % 12;
    memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
    r = rand() % 10;
    memcpy(record.city, cities[r], strlen(cities[r]) + 1);

    sprintf(test+offset,"%d",record.id);
    offset += num_of_digits(record.id);
    test[offset] = ' ';
    ++offset;
    memcpy(test+offset, record.name, strlen(record.name));
    offset += strlen(record.name);
    test[offset++] = ' ';
    memcpy(test+offset, record.surname, strlen(record.surname));
    offset += strlen(record.surname);
    test[offset++] = ' ';
    memcpy(test+offset, record.city, strlen(record.city));
    offset += strlen(record.city);
    test[offset++] = ' ';
    test[offset++] = '\0';

    int flag=0;
    Record data[2];
    int field=0;
    offset=0;
    for(int i=0 ; i<2 ; ++i)
    {
        sscanf(test+offset, "%d %s %s %s", &data[i].id, data[i].name, data[i].surname, data[i].city);
        offset=59;
    }
    
    for(int i=0 ; i<2 ; ++i)
    {
        printf("Record%d:\n",i);
        printf("id: %d\n",data[i].id);
        printf("name: %s\n",data[i].name);
        printf("surname: %s\n",data[i].surname);
        printf("city: %s\n",data[i].city);
        printf("\n");
    }

}