#include<stdio.h>
#include<semaphore.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

enum phi_state {thinking, hungry, eating};

struct InterfaceModule {
    sem_t mutex, next;
    int next_count;
};

void menter(struct InterfaceModule &IM) {
    sem_wait(&(IM.mutex));
}

void mleave(struct InterfaceModule &IM) {
    if (IM.next_count > 0)
        sem_post(&(IM.next));
    else
       sem_post(&(IM.mutex));
}

void mwait(sem_t &x_sem, int &x_count, struct InterfaceModule &IM) {
    x_count++;
    if (IM.next_count > 0)
        sem_post(&(IM.next));
    else
        sem_post(&(IM.mutex));
    sem_wait(&x_sem);
    x_count--;
}

void msignal(sem_t &x_sem, int &x_count, struct InterfaceModule &IM) {
    if (x_count > 0) {
        IM.next_count++;
        sem_post(&x_sem);
        sem_wait(&(IM.next));
        IM.next_count--;
    }
}

struct monitor{
    enum phi_state state[5];
    sem_t self[5];
    int self_count[5];
    struct InterfaceModule IM;

    void test(int k, int flag) {
        int last, next;
        last = k == 0 ? 4 : (k-1)%5;
        next = (k+1)%5;
        if ((state[last] != eating)&&(state[next] != eating)&&(state[k] == hungry)) {
            state[k] = eating;
            if (self_count[k] > 0) {
               int cur = (k+flag) == -1 ? 4 : (k+flag)%5;
               printf("Philosopher %d puts down chops.\n", cur);
            }
            msignal(self[k], self_count[k], IM);
        }
    }

    void pickup(int i) {
        menter(IM);
        printf("Philosopher %d is hungry.\n", i);
        state[i] = hungry;
        test(i, 0);
        if (state[i] != eating) {
            printf("Philosopher %d is waiting.\n", i);
            mwait(self[i], self_count[i], IM);
        }
        mleave(IM);
    }

    void putdown(int i) {
        menter(IM);
        int last, next;
        last = i == 0 ? 4 : (i-1)%5;
        next = (i+1)%5;
        state[i] = thinking;
        test(last, 1);
        test(next, -1);
        printf("Philosopher %d thinks.\n", i);
        mleave(IM);
    }
};

struct monitor dp;

void *philosophers(void *param) {
    int id = *((int*)param);
    while (true) {

        dp.pickup(id);

        int randnum = rand()%10;
        printf("Philosopher %d is eating.\n", id);
        sleep(randnum);

        dp.putdown(id);

        randnum = rand()%5;
        sleep(randnum);       
    }
    
}

int main() {
    for (int i = 0; i < 5; i++) {
        dp.state[i] = thinking;
        dp.self_count[i] = 0;
        sem_init(&(dp.self[i]), 0, 0);
    }
    dp.IM.next_count = 0;
    sem_init(&(dp.IM.mutex), 0, 1);
    sem_init(&(dp.IM.next), 0, 0);

    pthread_t tid[5];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    srand(time(NULL));

    for (int i = 0; i < 5; i++) {
        int* tem = (int*)malloc(sizeof(int));
        *tem = i;
        pthread_create(&tid[i], &attr, philosophers, (void*)tem);
    }

    sleep(20);
    return 0;
}
