#include <pthread.h>
#include <iostream>
#include<time.h>
#include <semaphore.h>
#define NUM_THREADS 4
using namespace std;
int n = 1024;
float** A = new float* [n];
int p = 0;
float** pA = new float* [n];
pthread_mutex_t mutex;

typedef struct {
	int k;          // ��ȥ���ִ�
	int t_id;       // �߳�id
} threadParam_t;
//�ṹ�嶨��

//barrier����
pthread_barrier_t barrier_Divsion;
pthread_barrier_t barrier_Elimination;

void* threadFunc(void* param) {
	threadParam_t* p = (threadParam_t*)param;
	int t_id = p->t_id;

	for (int k = 0; k < n; ++k) {
		if (t_id == 0) {//ѡ��һ��t_idΪ0���߳����������������������̵߳ȴ�
			for (int j = k + 1; j < n; j++) {
				pA[k][j] = pA[k][j] / pA[k][k];
			}
			pA[k][k] = 1.0;
		}
		//��һ��ͬ����
		pthread_barrier_wait(&barrier_Divsion);

		// ѭ����������
		for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
			// ��ȥ
			for (int j = k + 1; j < n; ++j) {
				pA[i][j] = pA[i][j] - pA[i][k] * pA[k][j];
			}
			pA[i][k] = 0.0;
		}

		//�ڶ���ͬ����
		pthread_barrier_wait(&barrier_Elimination);
	}
	pthread_exit(NULL);
}

int main() {
	// ����A��n
	// ...
	srand(time(NULL));
	for (int i = 0; i < n; i++) {
		//A[i] = new float[n];
		pA[i] = new float[n];
	}

	for (int i = 0; i < n; i++) {
		pA[i][i] = 1.0;//�Խ���Ϊ1.0
		for (int j = 0; j < n; j++) {
			if (j >= i)pA[i][j] = rand() % 10;
			else pA[i][j] = 0;
		}
	}
	//�����Ǿ���

	for (int k = 0; k < n; k++) {
		for (int i = k + 1; i < n; i++) {
			for (int j = 0; j < n; j++) {
				pA[i][j] += pA[k][j];
			}
		}
	}

	//��ʼ��barrier
	pthread_barrier_init(&barrier_Divsion, NULL, NUM_THREADS);
	pthread_barrier_init(&barrier_Elimination, NULL, NUM_THREADS);

	// �����߳�
	pthread_t handles[NUM_THREADS];
	threadParam_t param[NUM_THREADS];
	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		param[t_id].t_id = t_id;
		pthread_create(&handles[t_id], NULL, threadFunc, &param[t_id]);
	}

	for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
		pthread_join(handles[t_id], NULL);
	}

	//�������е�barrier
	pthread_barrier_destroy(&barrier_Divsion);
	pthread_barrier_destroy(&barrier_Elimination);
	return 0;
}
