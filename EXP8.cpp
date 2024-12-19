#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <math.h>
#define n 10
#define m 3
#define k 20
int Sequence[k];//����ҳ���������
int PageAccessTime[n];//����ҳ�����ʱ��
int PageArray[m];//ϵͳ�����ҳ��
int hitPage = 0;
void SearchPageAccess() {//�ҳ��������
	int MinTime = PageAccessTime[0];
	int MinIndex = 0;
	for (int i = 1; i < m; i++) {
		if (PageAccessTime[PageArray[i]] < MinTime) {
			MinTime = PageAccessTime[PageArray[i]];
			MinIndex = i;
		}
	}
	PageArray[MinIndex] = -1;
}
int ChangePageArray(int Page,int currentTime) {
	for (int i = 0; i < m; i++) {//�п���ҳ��
		if (PageArray[i] == -1) {
			PageArray[i] = Page;
			PageAccessTime[Page] = currentTime;
			return 0;
		}
		else if (PageArray[i] == Page) {//����
			hitPage++;
			return 1;
		}
	}
	SearchPageAccess();
	for (int i = 0; i < m; i++) {
		if (PageArray[i] == -1) {
			PageArray[i] = Page;
			PageAccessTime[Page] = currentTime;
			return 0;
		}
	}
}
void LRU() {
	//��ʼ����LRUҳ�����
	printf("SeqID\tWorking Set\n");
	for (int i = 0; i < k; i++) {
		int Page = Sequence[i];//Ҫ���ʵ�ҳ��
		int result=ChangePageArray(Page, i);
		printf("%d\t", i + 1);
		for (int j = 0; j < m; j++) {
			printf("%d ",  PageArray[j]);
		}
		if (result) {
			printf("   *hit*\n");
		}
		else
			printf("\n");
	}
	int Miss = k - hitPage;
	double Pagefault = (double)Miss / k;
	printf("Hit = %d,", hitPage);
	printf("Miss = %d\n", Miss);
	printf("Page fault Rate = %d/%d =%.4f \n", Miss, k, Pagefault);
}
int main() {
	//����ҳ���������
	srand((unsigned)time(NULL));
	
	//��ʼ��
	printf("Squence:");
	for (int i = 0; i < k; i++) {
		Sequence[i] = rand() % n;
		printf("%d ", Sequence[i]);
	}
	printf("\n");
	for (int i = 0; i < m; i++) {
		PageArray[i] = -1;
	}
	for (int i = 0; i < n; i++) {
		PageAccessTime[i] = -1;
	}
	LRU();
	return 0;
}