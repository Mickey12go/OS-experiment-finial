#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#include <math.h>
#define MaxOrder 10
#define Process_num 3
#define MemUpper 8
#define MemLower 3
#define Free true
#define Used false
#define Fail -1
#define Success 1

struct Block {
	int id;
	int sizeK;
	int startAddr;
	bool status;
	int pid;
	struct Block* prev;
	struct Block* next;
};
struct PCB {
	int pid;//ID
	int neededMem;
	int status;
	int blockID;

};
Block* usedBlock[MaxOrder + 1] = { 0 };
Block* freeBlock[MaxOrder + 1] = { 0 };
PCB procs[Process_num];
int BID = 1;

int Allocate(PCB* procs) {
	int i;
	int k = procs->neededMem;
	Block* ptrFree;
	for (i = k; i < MaxOrder + 1; i++) {
		if (freeBlock[i] != NULL)//说明有比他内存大的内存块可以分,找到第一个就可以去分
			break;
	}
	if (i >= MaxOrder + 1) {
		procs->status = -1;
		return Fail;
	}
	//找到的是大块的，应该切分小块
	//由于buddy system的特点，应该向下切分两个大小相等的buddy区块
	while (i != k) {
		int buddyposition = rand() % 2;//[0,1]
		ptrFree = freeBlock[i];
		freeBlock[i] = freeBlock[i]->next;
		if (freeBlock[i] != NULL)
			freeBlock[i]->prev = NULL;
		ptrFree->next = NULL;

		Block* tempUsedBlock = new Block;
		tempUsedBlock->id = ptrFree->id;
		tempUsedBlock->sizeK = i - 1;
		tempUsedBlock->startAddr = (ptrFree->startAddr + (int)((1 - (double)buddyposition) * pow(2, (double)i - 1)));//伙伴块地址的计算
		tempUsedBlock->pid = -1;
		tempUsedBlock->status = true;
		tempUsedBlock->prev = NULL;
		tempUsedBlock->next = NULL;
		freeBlock[i - 1] = tempUsedBlock;

		Block* tempFreeBlock = new Block;
		tempFreeBlock->id = BID++;
		tempFreeBlock->sizeK = i - 1;
		tempFreeBlock->startAddr = (ptrFree->startAddr + (int)((1 - (double)buddyposition) * pow(2, (double)i - 1)));//伙伴块地址的计算
		tempFreeBlock->pid = -1;
		tempFreeBlock->status = true;
		tempFreeBlock->prev = NULL;
		tempFreeBlock->next = NULL;

		freeBlock[i - 1]->next = tempFreeBlock;
		tempFreeBlock->prev = freeBlock[i - 1];
		i--;
	}
	ptrFree = freeBlock[k];//将分配出的区块从大小为2^k的空闲区块链表中删除
	freeBlock[k] = freeBlock[k]->next;
	if (freeBlock[k] != NULL) {
		freeBlock[k]->prev = NULL;
	}
	ptrFree->status = false;
	ptrFree->pid = procs->pid;
	ptrFree->next = NULL;
	ptrFree->prev = NULL;
	Block* ptrUsed = usedBlock[k];//插入2^k的被分配区块链
	if (ptrUsed == NULL) {
		usedBlock[k] = ptrFree;
	}
	else {
		while (ptrUsed->next != NULL) {
			ptrUsed = ptrUsed->next;
		}
		ptrUsed->next = ptrFree;
		ptrFree->prev = ptrUsed;
	}
	printf("\n");
	return Success;
}
void Recycle(PCB* procs) {
	void Combine(int k);
	int k = procs->neededMem;
	Block* tempUsedBlock = usedBlock[k];
	usedBlock[k] = usedBlock[k]->next;
	if (usedBlock[k] != NULL) {
		usedBlock[k]->prev = NULL;
	}
	tempUsedBlock->status = true;
	tempUsedBlock->pid = -1;
	tempUsedBlock->next = NULL;
	tempUsedBlock->prev = NULL;

	Block* tempFreeBlock = freeBlock[k];
	if (freeBlock[k] == NULL) {
		freeBlock[k] = tempUsedBlock;
	}
	else {
		while (tempFreeBlock->next != NULL) {
			tempFreeBlock = tempFreeBlock->next;
		}
		tempFreeBlock->next = tempUsedBlock;
		tempUsedBlock->prev = tempFreeBlock;
		Combine(k);
	}
}
void Combine(int k) {
	for (int i = k; i < MaxOrder + 1; i++) {
		if (freeBlock[i] == NULL)
			break;
		Block* tempBlockPtr = freeBlock[i];
		while (tempBlockPtr->next != NULL) {
			tempBlockPtr = tempBlockPtr->next;
		}
		Block* freeBlockPtr = freeBlock[i];
		while (freeBlockPtr != tempBlockPtr) {
			int fin = 0;
			Block* linkedBlock = new Block;
			if (freeBlockPtr->startAddr % (int)pow(2, (double)i + 1) == 0 && freeBlockPtr->startAddr + (int)pow(2, i) == tempBlockPtr->startAddr) {
				linkedBlock->id = tempBlockPtr->id;
				linkedBlock->sizeK = freeBlockPtr->sizeK + 1;
				linkedBlock->startAddr = freeBlockPtr->startAddr;
				linkedBlock->pid = -1;
				linkedBlock->status = true;
				linkedBlock->prev = NULL;
				linkedBlock->next = NULL;
				fin = 1;
			}
			else if (freeBlockPtr->startAddr % (int)pow(2, (double)i + 1) == (int)pow(2, i) && freeBlockPtr->startAddr - (int)pow(2, i) == tempBlockPtr->startAddr) {
				linkedBlock->id = tempBlockPtr->id;
				linkedBlock->sizeK = freeBlockPtr->sizeK + 1;
				linkedBlock->startAddr = freeBlockPtr->startAddr;
				linkedBlock->pid = -1;
				linkedBlock->status = true;
				linkedBlock->prev = NULL;
				linkedBlock->next = NULL;
				fin = 1;
			}
			if (fin == 1) {
				printf("Combine block %d and %d of size 2^%d\n", tempBlockPtr->id, freeBlockPtr->id, i);
				if (freeBlockPtr->prev == NULL) {
					freeBlock[i] = freeBlock[i]->next;
					if (freeBlock[i] != NULL)
						freeBlock[i]->prev = NULL;
					freeBlockPtr->next = NULL;
				}
				else {
					freeBlockPtr->prev->next = freeBlockPtr->next;
					if (freeBlockPtr->next != NULL)
						freeBlockPtr->next->prev = freeBlockPtr->prev;
					freeBlockPtr->prev = NULL;
					freeBlockPtr->next = NULL;
				}
				if (tempBlockPtr->prev == NULL) {
					freeBlock[i] = NULL;
				}
				else if (tempBlockPtr->prev != NULL) {
					tempBlockPtr->prev->next = NULL;
					tempBlockPtr->prev = NULL;
				}
				Block* upperBlockPtr = freeBlock[i + 1];
				if (freeBlock[i + 1] == NULL) {
					freeBlock[i + 1] = linkedBlock;
				}
				else {
					while (upperBlockPtr->next != NULL) {
						upperBlockPtr = upperBlockPtr->next;
					}
					upperBlockPtr->next = linkedBlock;
					linkedBlock->prev = upperBlockPtr;
				}
				break;
			}
			freeBlockPtr = freeBlockPtr->next;
		}
	}
	printf("\n");
}
void Print() {
	int i;
	Block* usedBlockPtr;
	Block* freeBlockPtr;
	for (i = 0; i < MaxOrder + 1; i++) {
		freeBlockPtr = freeBlock[i];
		while (freeBlockPtr != NULL) {
			printf("freeBlockID:%2d, size:2^%d, startAddr:%3d\n", freeBlockPtr->id, freeBlockPtr->sizeK, freeBlockPtr->startAddr);
			freeBlockPtr = freeBlockPtr->next;
		}
	}
	printf("\n");
	for (i = 0; i < MaxOrder + 1; i++) {
		usedBlockPtr = usedBlock[i];
		while (usedBlockPtr != NULL) {
			printf("usedBlockID:%2d, size:2^%d, startAddr:%3d, pid:%d\n", usedBlockPtr->id, usedBlockPtr->sizeK, usedBlockPtr->startAddr, usedBlockPtr->pid);
			usedBlockPtr = usedBlockPtr->next;
		}
	}
	return;
}
int main() {
	int i;
	int result;
	srand((unsigned)time(NULL));
	for (i = 0; i < Process_num; i++) {
		procs[i].pid = i + 1;
		procs[i].status = 0;
		procs[i].blockID = -1;
	}
	procs[0].neededMem = 7;
	procs[1].neededMem = 4;
	procs[2].neededMem = 8;
	printf("ProcessID\tNeededMem\n");
	for (i = 0; i < Process_num; i++) {
		printf("%-15d\t2^%-15d\n", procs[i].pid, procs[i].neededMem);
	}
	freeBlock[MaxOrder] = new Block;
	freeBlock[MaxOrder]->id = BID++;
	freeBlock[MaxOrder]->pid = -1;
	freeBlock[MaxOrder]->sizeK = MaxOrder;
	freeBlock[MaxOrder]->startAddr = 0;
	freeBlock[MaxOrder]->status = true;
	freeBlock[MaxOrder]->prev = NULL;
	freeBlock[MaxOrder]->next = NULL;

	printf("**********Start Allocating Memory******************\n");
	Print();
	printf("***************************************************\n");
	for (i = 0; i < Process_num; i++) {
		printf("Allocate free memory block for Process %d of size 2^%d\n", procs[i].pid, procs[i].neededMem);
		result = Allocate(&procs[i]);
		if (result != Success)
			printf("Memory allocation failed for Process %d\n", procs[i].pid);
		Print();
		printf("***************************************************\n");
	}
	printf("**********Start Recovering Memory******************\n");
	for (i = 0; i < Process_num; i++) {
		if (procs[i].status != -1) {
			int k = procs[i].neededMem;
			printf("Recycle used memory block for Process %d of size 2^%d\n", procs[i].pid, procs[i].neededMem);
			Recycle(&procs[i]);
			Print();
			if (freeBlock[MaxOrder] != NULL) {
				printf("Memory has been reclaimed!\n");
			}
			printf("***************************************************\n");
		}
	}
	return 0;
}

