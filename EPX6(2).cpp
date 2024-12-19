#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>
#define MaxMem 1024
#define Process_num 10

//内存结构体，每个内存块对应一个数据块
struct Block {
	int id;
	int size;
	int startAddr;//起始地址
	bool status;//状态 true为空闲 false为已占用
	int pid;//占用内存块的进程ID，未占用为-1
	struct Block* prev;//指向前一个内存块
	struct Block* next;//指向后一个内存块
};

//进程控制块
struct PCB {
	int pid;//进程ID
	int neededMem;//需要的内存大小
	int status;//1:成功 -1:失败
	int blockID;//成功分配的内存块ID，未成功分配为-1
	struct PCB* next;//指向下一个PCB
};

PCB procs[Process_num];
Block* freeBlock = NULL;//动态内存块，为Block类型，足够内存被占用，存储内存块的起始地址，new c++内存分配函数，第一个内存块为空闲内存块，存储内存块的起始地址，freeBlock指向它
Block* usedBlock = NULL;
int BID = 0;
int usedBlockID = 0;

void printMemory() {
	printf("\n******************************************Allocation********************************************************\n");
	Block* p = freeBlock;
	if (freeBlock == NULL) {
		printf("Not Memory at all!\n");
	}
	else {
		printf("************************************************************************************************************\n");
		printf("FreeBlkID\tStartAddr\tSize\n");
		printf("************************************************************************************************************\n");
		while (p != NULL) {
			printf("%-8d\t%-8d\t%-8d\n", p->id, p->startAddr, p->size);
			p = p->next;
		}
	}
	printf("\n******************************************Used**************************************************************\n");
	p = usedBlock;
	if (usedBlock == NULL) {
		printf("Not Process has been allocated!\n");
	}
	else {
		printf("************************************************************************************************************\n");
		printf("UsedBlkID\tStartAddr\tSize\tProcessID\n");
		printf("************************************************************************************************************\n");
		while (p != NULL) {
			printf("%-8d\t%-8d\t%-8d\t%-8d\n", p->id, p->startAddr, p->size, p->pid);
			p = p->next;
		}
	}
	printf("\n************************************************************************************************************\n");
}

void AllocateBlock(Block* ptrFree, PCB* proc) {
	//1初始化内存块信息
	ptrFree->status = false;
	ptrFree->pid = proc->pid;
	//2释放内存块
	if (ptrFree->prev == NULL) {
		freeBlock = ptrFree->next;
		if (freeBlock != NULL) {
			freeBlock->prev == NULL;
		}
	}
	else {//如果是中间节点
		ptrFree->prev->next = ptrFree->next;
		if (ptrFree->next != NULL) {
			ptrFree->next->prev = ptrFree->prev;
		}
	}
	//3添加到已用链表
	if (usedBlock == NULL) {
		usedBlock = ptrFree;
		ptrFree->prev = NULL;
		ptrFree->next = NULL;
	}
	else {//如果是空闲链表不为空，需要按地址顺序插入
		ptrFree->next = usedBlock;
		usedBlock->prev = ptrFree;
		ptrFree->prev = NULL;
		usedBlock = ptrFree;
	}
	//4修改进程状态
	proc->status = 1;
	proc->blockID = ptrFree->id;
}

void AddToUsedList(Block* block) {
	block->status = false;
	block->id = usedBlockID++;
	if (usedBlock == NULL) {
		usedBlock = block;
		block->prev = NULL;
		block->next = NULL;
	}
	else {//如果是空闲链表不为空，需要按地址顺序插入
		block->next = usedBlock;
		usedBlock->prev = block;
		block->prev = NULL;
		usedBlock = block;
	}
}

void SplitAndAllocate(Block* block, PCB* proc) {
	if (block->size < proc->neededMem) {
		proc->status = -1;
		return;
	}
	//获取内存块地址范围
	Block* newBlock = new Block;
	int maxStart = block->size - proc->neededMem;
	int randomStart = rand() % (maxStart + 1);
	//分配内存块
	newBlock->size = proc->neededMem;
	newBlock->startAddr = block->startAddr + randomStart;
	newBlock->status = false;
	newBlock->pid = proc->pid;
	//释放原内存块
	block->size -= proc->neededMem;
	block->startAddr += proc->neededMem;
	//添加到已用链表
	AddToUsedList(newBlock);
	proc->blockID = newBlock->id;
	proc->status = 1;
}

//内存分配函数
int NextFit(PCB* procs) {
	static Block* lastAlloc = freeBlock;//记录上次分配得位置
	Block* current;
	if (procs->pid == 1) {
		current = freeBlock;
		lastAlloc = freeBlock;
	}
	else {
		current = lastAlloc;
	}
	Block* startPoint = current;
	bool wrapped = false;
	while (current != NULL) {
		if (current->size >= procs->neededMem) {
			if (current->size == procs->neededMem) {
				AllocateBlock(current, procs);
			}
			else {
				SplitAndAllocate(current, procs);
			}
			lastAlloc = current;
			return 1;
		}
		current = current->next;
		if (current == NULL && !wrapped) {
			current = freeBlock;
			wrapped = true;
		}
		if (current == startPoint && wrapped) {
			break;
		}
	}
	procs->status = -1;
	return 0;
}

//合并相邻空闲块
void MergeFreeBlocks(Block* block) {
	//和后一块合并
	if (block->next != NULL && block->startAddr + block->size == block->next->startAddr) {
		block->size += block->next->size;
		Block* temp = block->next;
		block->next = temp->next;
		if (temp->next != NULL) {
			temp->next->prev = block;
		}
		delete temp;
	}
	//与前一个块合并
	if (block->prev != NULL && block->prev->startAddr + block->prev->size == block->startAddr) {
		block->prev->size += block->size;
		Block* temp = block;
		block->prev->next = temp->next;
		if (block->next != NULL) {
			block->next->prev = block->prev;
		}
		delete temp;
	}
}

//内存回收
void RecoveryMemory(PCB* proc) {
	if (proc->status != 1)
		return;
	Block* block = usedBlock;
	while (block != NULL) {
		if (block->id == proc->blockID) {//找到要回收的块
			//从已用链表移除
			if (block->prev == NULL) {
				usedBlock = block->next;
				if (usedBlock != NULL) {
					usedBlock->prev = NULL;
				}
			}
			else {//如果是中间节点
				block->prev->next = block->next;
				if (block->next != NULL) {
					block->next->prev = block->prev;
				}
			}
			//加入空闲链表
			block->status = true;
			block->pid = -1;
			if (freeBlock == NULL) {
				freeBlock = block;
				block->prev = NULL;
				block->next = NULL;
			}
			else {//空闲链表不为空，需要按地址顺序插入
				Block* current = freeBlock;
				Block* prev = NULL;
				while (current != NULL && current->startAddr < block->startAddr) {
					prev = current;
					current = current->next;
				}
				//插入到链表头部
				if (prev == NULL) {
					block->next = freeBlock;
					block->prev = NULL;
					freeBlock->prev = block;
					freeBlock = block;
				}
				else {//插入到中间位置
					block->next = current;
					block->prev = prev;
					prev->next = block;
					if (current != NULL) {
						current->prev = block;
					}
				}
			}
			MergeFreeBlocks(block);
			break;
		}
		block = block->next;
	}
}

int main() {
	SetConsoleOutputCP(65001);
	srand((unsigned)time(NULL));
	//初始化内存块
	freeBlock = new Block;
	freeBlock->id = BID++;
	freeBlock->startAddr = 0;
	freeBlock->size = MaxMem;
	freeBlock->status = true;
	freeBlock->prev = NULL;
	freeBlock->next = NULL;

	for (int i = 0; i < Process_num; i++) {
		procs[i].pid = i + 1;
		procs[i].status = 0;
		procs[i].blockID = -1;
		procs[i].neededMem = rand() % 101 + 100;
		procs[i].next = NULL;
	}
	for (int i = 0; i < Process_num; i++) {
		printf("ProcsID neededMem\n");
		printf("%d\t%d\n", procs[i].pid, procs[i].neededMem);
	}
	//NF内存分配
	printf("**********Start Allocating Memory*************\n");
	for (int i = 0; i < Process_num; i++) {
		printf("\nAllocating memory for Process %d, memory requirement = %d\n ", procs[i].pid, procs[i].neededMem);
		printMemory();
		if (NextFit(&procs[i])) {
			printf("Allocating successfully!\n");
		}
		else {
			printf("Allocating default! Memory not enough!\n");
		}
	}

	printf("**********Start Recoverying Memory*************\n");
	for (int i = 0; i < Process_num; i++) {
		if (procs[i].status == 1) {
			printf("\nRecoverying memory for Process %d\n", procs[i].pid);
			RecoveryMemory(&procs[i]);
			printMemory();
		}

	}
	return 0;
}