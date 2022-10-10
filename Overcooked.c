#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#define MAX_ingrNAME 20
#define MAX_ingrNum 20
#define MAX_qSize 10001
#define MAX_outputSize 2000
#define min(x,y) (x<y)?x:y
#define max(x,y) (x>y)?x:y
struct Recipes{
    char name[MAX_ingrNAME];
    int sNum,cNum,oNum;
    char s[MAX_ingrNum][MAX_ingrNAME];
    char c[MAX_ingrNum][MAX_ingrNAME];
    char o[MAX_ingrNum][MAX_ingrNAME];
};
struct Orders{
    int id;
    char name[MAX_ingrNAME];
    int arrival,deadline,money,punish;
    int recipe_index;
    int sNum,cNum,oNum;
    bool finish;
};
struct Players{
    bool avail;
    int availTime;
};
struct Output{
    int playerID;
    int time;
    int orderID;
    char jobType;
    char jobName[MAX_ingrNAME];
};
struct Recipes *recipes;
struct Orders *orders;
struct Players p1, p2;
struct Output output[MAX_outputSize];
int scheduleQ[MAX_qSize];
int front = -1;
int rear = -1;
int recipeNum,orderNum;
int maxDeadline=0;
int currentTime;
int currentOrder=0;
int sAvail = 0, cAvail = 0;
int whoOnS;                         //record order that is on the stove
int whoOnC;                         //record order that is cut
int earn = 0;
int outputIndex = 0;
int assign_job(int);                //assign appropriate job to the available player
void schedule_player();
void cleanQ();                      //clean job in scheduleQ that is done or will exceed it's deadline
void swap(int,int);                 //swap two order in scheduleQ
void modifyQ();                     //move job that has more profit to the front of the queue(have higher priority)
void enqueue(int);                  
void merge(int, int, int);
void sorting_order(int,int);        //sort orders based on their arrival time
void scheduling();
void read_recipes_file();
void read_orders_file();
FILE *players_file;
FILE *recipes_file;
FILE *orders_file;
int main(){
    read_recipes_file();
    read_orders_file();
    scheduling();
    return 0;
}
void read_recipes_file(){
    int tmpName = 0;
    char c;
    bool flag;                      //record if it's empty
    recipes_file = fopen("recipes.txt", "r");
    fscanf(recipes_file, "%d\n", &recipeNum);
    recipes = (struct Recipes *)calloc(recipeNum, sizeof(struct Recipes));
    for (int i = 0; i < recipeNum;i++){
        recipes[i].sNum = 0;
        recipes[i].cNum = 0;
        recipes[i].oNum = 0;
        for (int j = 0; j < MAX_ingrNum;j++){
            for (int k = 0; k < MAX_ingrNAME;k++){
                recipes[i].s[j][k] = '\0';
                recipes[i].c[j][k] = '\0';
                recipes[i].o[j][k] = '\0';
            }
        }
        fscanf(recipes_file, "%s ", recipes[i].name);
        flag = false;
        while(fscanf(recipes_file,"%c",&c)){
            if(c==' '){
                if(!flag)           //it's not empty
                    recipes[i].sNum++;
                tmpName = 0;
                break;
            }
            else if(c=='x'&&tmpName==0){
                flag = true;
            }
            else if(c==','){
                recipes[i].sNum++;
                tmpName = 0;
            }
            else{
                recipes[i].s[recipes[i].sNum][tmpName++] = c;
            }
        }
        flag = false;
        while (fscanf(recipes_file, "%c", &c)){
            if (c == ' '){
                if (!flag)
                    recipes[i].cNum++;
                tmpName = 0;
                break;
            }
            else if (c == 'x'&&tmpName==0){
                flag = true;
            }
            else if (c == ','){
                tmpName = 0;
                recipes[i].cNum++;
            }
            else{
                recipes[i].c[recipes[i].cNum][tmpName++] = c;
            }
        }
        flag = false;
        while(fscanf(recipes_file,"%c",&c)){
            if(c=='\n'||feof(recipes_file)){
                if(!flag)
                    recipes[i].oNum++;
                tmpName = 0;
                break;
            }
            else if(c=='x'){
                flag = true;
            }
            else if(c==','){
                tmpName = 0;
                recipes[i].oNum++;
            }
            else{
                recipes[i].o[recipes[i].oNum][tmpName++] = c;
            }
        }
    }
    fclose(recipes_file);
}
void read_orders_file(){
    orders_file = fopen("orders.txt", "r");
    fscanf(orders_file, "%d\n", &orderNum);
    orders = (struct Orders *)calloc(orderNum, sizeof(struct Orders));
    for (int i = 0; i < orderNum;i++){
        orders[i].finish = false;
        fscanf(orders_file, "%d %s %d%d%d%d\n", &orders[i].id, orders[i].name, &orders[i].arrival, &orders[i].deadline, &orders[i].money, &orders[i].punish);
        int j;
        for (j = 0; j < recipeNum;j++){
            if(!strcmp(orders[i].name,recipes[j].name)){
                orders[i].recipe_index = j;
                break;
            }
        }
        if(j==recipeNum)
            orders[i].recipe_index = -1;
        else{
            orders[i].sNum = recipes[orders[i].recipe_index].sNum;
            orders[i].cNum = recipes[orders[i].recipe_index].cNum;
            orders[i].oNum = recipes[orders[i].recipe_index].oNum;
        }
        if (orders[i].deadline > maxDeadline)
            maxDeadline = orders[i].deadline;
    }
    fclose(orders_file);
}
void scheduling(){
    players_file = fopen("players.txt", "w");
    p1.avail = true;
    p1.availTime = 0;
    p2.avail = true;
    p2.availTime = 0;
    sorting_order(0,orderNum-1);
    for (currentTime = 0; currentTime < maxDeadline;currentTime++){
        while(orders[currentOrder].arrival==currentTime){//enqueue the incoming order
            if(orders[currentOrder].recipe_index!=-1){
                enqueue(currentOrder);
                modifyQ();                               //modify the order that just be added in the queue
            }
            currentOrder++;
        }
        cleanQ();                                       //clean the node that is done or exceeds time
        if(currentTime==p1.availTime)
            p1.avail = true;
        if(currentTime==p2.availTime)
            p2.avail = true;
        schedule_player();
    }
    fprintf(players_file,"%d\n", outputIndex);
    for (int i = 0; i < outputIndex;i++){
        fprintf(players_file, "%d %d %d %c %s\n", output[i].playerID, output[i].time, output[i].orderID, output[i].jobType, output[i].jobName);
    }
        fclose(players_file);
}
void sorting_order(int l,int r){
    if (l < r) {
        int m = l + (r - l) / 2;
        sorting_order(l, m);
        sorting_order(m + 1, r);
        merge(l, m, r);
    }
}
void merge(int l,int m,int r){
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
    struct Orders L[n1], R[n2];
    for (i = 0; i < n1; i++)
        L[i] = orders[l + i];
    for (j = 0; j < n2; j++)
        R[j] = orders[m + 1 + j];
    i = 0; 
    j = 0; 
    k = l; 
    while (i < n1 && j < n2) {
        if (L[i].arrival < R[j].arrival) {
            orders[k] = L[i];
            i++;
        }
        else if(L[i].arrival > R[j].arrival){
            orders[k] = R[j];
            j++;
        }
        else{
            if (L[i].deadline <= R[j].deadline) {
                orders[k] = L[i];
                i++;
            }
            else if(L[i].deadline > R[j].deadline){
                orders[k] = R[j];
                j++;
            }
        }
        k++;
    }
    while (i < n1) {
        orders[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        orders[k] = R[j];
        j++;
        k++;
    }
}
void enqueue(int index){
    if(rear==MAX_qSize-1){
        printf("The scheduling queue is full\n");
        return;
    }
    scheduleQ[++rear] = index;
    return;
}
int dequeue(){
    if(front==rear){
        printf("The scheduling queue is empty\n");
        return -1;
    }
    return scheduleQ[++front];
}
void modifyQ(){
    int j;
    //bool noChange = false;
    for (int i = rear; i > front + 1; ){
        j = i - 1;
        /*if((orders[scheduleQ[i]].money+orders[scheduleQ[j]].punish>=orders[scheduleQ[j]].money+orders[scheduleQ[i]].punish)&&(orders[scheduleQ[i]].deadline-orders[scheduleQ[i]].arrival<orders[scheduleQ[j]].deadline-orders[scheduleQ[j]].arrival))
            swap(i, j);
        else if((orders[scheduleQ[i]].money+orders[scheduleQ[j]].punish<orders[scheduleQ[j]].money+orders[scheduleQ[i]].punish)&&(orders[scheduleQ[i]].deadline-orders[scheduleQ[i]].arrival>=orders[scheduleQ[j]].deadline-orders[scheduleQ[j]].arrival))
            noChange = true;
        else{

        }*/
        if(orders[scheduleQ[i]].money+orders[scheduleQ[j]].punish>orders[scheduleQ[j]].money+orders[scheduleQ[i]].punish){
            swap(i, j);
            i--;
        }
        else
            break;
        /*else
            noChange = true;
        if(noChange)
            break;*/
    }
}
void swap(int i,int j){
    int tmp = scheduleQ[i];
    scheduleQ[i] = scheduleQ[j];
    scheduleQ[j] = tmp;
    return;
}
void cleanQ(){
    for (int i = front + 1; i <= rear;i++){             //the order that is done
        if(orders[scheduleQ[i]].sNum==0&&orders[scheduleQ[i]].cNum==0&&orders[scheduleQ[i]].oNum==0&&orders[scheduleQ[i]].finish){//time<deadline
            for (int j = i; j > front + 1;j--)
                swap(j, j - 1);
            earn += orders[dequeue()].money;
        }
        //if(currentTime+(max(((min(orders[scheduleQ[i]].sNum*5,orders[scheduleQ[i]].cNum*3))+1+orders[scheduleQ[i]].oNum),max(orders[scheduleQ[i]].sNum*5,orders[scheduleQ[i]].cNum*3)))>orders[scheduleQ[i]].deadline){
            //printf("%d %d %d %d\n", currentTime,orders[scheduleQ[i]].id, currentTime+(max(((min(orders[scheduleQ[i]].sNum*5,orders[scheduleQ[i]].cNum*3))+1+orders[scheduleQ[i]].oNum),max(orders[scheduleQ[i]].sNum*5,orders[scheduleQ[i]].cNum*3))),orders[scheduleQ[i]].deadline);
        if(currentTime+(max(orders[scheduleQ[i]].sNum*5,orders[scheduleQ[i]].cNum*3))+1+orders[scheduleQ[i]].oNum>orders[scheduleQ[i]].deadline){
            for (int j = i; j > front + 1;j--)          //the order that will exceeds the deadline
                swap(j, j - 1);
            earn += orders[dequeue()].punish;
        }
    }
}
void schedule_player(){
    int working;
    if(rear==front)                                     //the queue is empty
        return;
    if(p1.avail){
        working = assign_job(1);
        if(working){
            p1.avail = false;
            p1.availTime = currentTime + working;
        }
    }
    if(p2.avail){
        working = assign_job(2);
        if(working){
            p2.avail = false;
            p2.availTime = currentTime + working;
        }
    }
}
int assign_job(int player){                             //return the process time
    for (int i = front + 1; i <= rear;i++){             //search the order that can be presented
        if(orders[scheduleQ[i]].sNum==0&&orders[scheduleQ[i]].cNum==0&&!orders[scheduleQ[i]].finish&&((currentTime>=cAvail)||((currentTime<cAvail)&&(whoOnC!=scheduleQ[i])))&&((currentTime>=sAvail)||((currentTime<sAvail)&&(whoOnS!=scheduleQ[i])))){
            int tmp = orders[scheduleQ[i]].oNum;
            orders[scheduleQ[i]].oNum = 0;
            orders[scheduleQ[i]].finish = true;
            output[outputIndex].playerID = player;
            output[outputIndex].time = currentTime;
            output[outputIndex].orderID = orders[scheduleQ[i]].id;
            output[outputIndex].jobType = 'f';
            memset(output[outputIndex].jobName, '\0', MAX_ingrNAME);
            outputIndex++;
            return tmp + 1;
        }
    }
    for (int i = front + 1; i <= rear;i++){
        if(currentTime>=cAvail&&orders[scheduleQ[i]].cNum!=0){
            cAvail = currentTime + 3;
            whoOnC = scheduleQ[i];
            output[outputIndex].playerID = player;
            output[outputIndex].time = currentTime;
            output[outputIndex].orderID = orders[scheduleQ[i]].id;
            output[outputIndex].jobType = 'c';
            memset(output[outputIndex].jobName, '\0', MAX_ingrNAME);
            strcpy(output[outputIndex].jobName, recipes[orders[scheduleQ[i]].recipe_index].c[orders[scheduleQ[i]].cNum-1]);
            outputIndex++;
            orders[scheduleQ[i]].cNum--;
            return 3;
        }
        if(currentTime>=sAvail&&orders[scheduleQ[i]].sNum!=0){
            sAvail = currentTime + 5;
            whoOnS = scheduleQ[i];
            output[outputIndex].playerID = player;
            output[outputIndex].time = currentTime;
            output[outputIndex].orderID = orders[scheduleQ[i]].id;
            output[outputIndex].jobType = 's';
            memset(output[outputIndex].jobName, '\0', MAX_ingrNAME);
            strcpy(output[outputIndex].jobName, recipes[orders[scheduleQ[i]].recipe_index].s[orders[scheduleQ[i]].sNum-1]);
            outputIndex++;
            orders[scheduleQ[i]].sNum--;
            return 5;
        }
    }
    return 0;
}