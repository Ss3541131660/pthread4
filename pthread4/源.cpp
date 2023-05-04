#include <pthread.h>
#include <iostream>
#include<time.h>
#include <semaphore.h>
#define NUM_THREADS 4
using namespace std;
int n = 1024;
float pA[1024][1024];
pthread_mutex_t mutex;

typedef struct {
	int k;          // 消去的轮次
	int t_id;       // 线程id
} threadParam_t;
//结构体定义

//barrier定义
pthread_barrier_t barrier_Divsion;
pthread_barrier_t barrier_Elimination;

void* threadFunc(void* param) {
	threadParam_t* p = (threadParam_t*)param;
	int t_id = p->t_id;

	for (int k = 0; k < n; ++k) {
		if (t_id == 0) {//选出一个t_id为0的线程做除法操作，其他工作线程等待
			for (int j = k + 1; j < n; j++) {
				pA[k][j] = pA[k][j] / pA[k][k];
			}
			pA[k][k] = 1.0;
		}
		//第一个同步点
		pthread_barrier_wait(&barrier_Divsion);

		// 循环划分任务
		for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
			// 消去
			for (int j = k + 1; j < n; ++j) {
				pA[i][j] = pA[i][j] - pA[i][k] * pA[k][j];
			}
			pA[i][k] = 0.0;
		}

		//第二个同步点
		pthread_barrier_wait(&barrier_Elimination);
	}
	pthread_exit(NULL);
}

int main() {
	// 读入A和n
	// ...
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		pA[i][i] = 1.0;//对角线为1.0
		for (int j = 0; j < n; j++) {
			if (j > i)pA[i][j] = rand() % 10;
			else if(j<i)pA[i][j] = 0;
		}
	}
	//上三角矩阵

	for (int k = 0; k < n; k++) {
		for (int i = k + 1; i < n; i++) {
			for (int j = 0; j < n; j++) {
				pA[i][j] += pA[k][j];
			}
		}
	}

	//初始化barrier
	pthread_barrier_init(&barrier_Divsion, NULL, NUM_THREADS);
	pthread_barrier_init(&barrier_Elimination, NULL, NUM_THREADS);

	// 创建线程
	pthread_t handles[NUM_THREADS];
	threadParam_t param[NUM_THREADS];
	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
	}

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		pthread_join(handles[t_id], NULL);
	}

	//销毁所有的barrier
	pthread_barrier_destroy(&barrier_Divsion);
	pthread_barrier_destroy(&barrier_Elimination);
	return 0;
}
