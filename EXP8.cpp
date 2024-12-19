#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <math.h>
#define n 10
#define m 3
#define k 20
int Sequence[k];//保存页面访问序列
int PageAccessTime[n];//保存页面访问时间
int PageArray[m];//系统分配的页面
int hitPage = 0;
void SearchPageAccess() {//找出被顶替的
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
	for (int i = 0; i < m; i++) {//有空余页面
		if (PageArray[i] == -1) {
			PageArray[i] = Page;
			PageAccessTime[Page] = currentTime;
			return 0;
		}
		else if (PageArray[i] == Page) {//命中
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
	//开始进行LRU页面调度
	printf("SeqID\tWorking Set\n");
	for (int i = 0; i < k; i++) {
		int Page = Sequence[i];//要访问的页面
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
	//生成页面访问序列
	srand((unsigned)time(NULL));
	
	//初始化
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