// BruteMEM.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <vector>
#include <process.h>
#include "BrutePool.h" //maybe rename to MPool

using namespace std;

class Complex{ 
public: 
	Complex (unsigned int a, double b): r (a), c (b) {} 
	vector<int> MyVec;
	unsigned int r; // Real Part 
	double c; // Complex Part 
};
void MemTest();
void MPTest();
void MPTest2();
void MPTest3();
unsigned __stdcall MTTest();
int _tmain(int argc, _TCHAR* argv[]){
	size_t xs = sizeof(Complex);
	MManager* mPool = new MManager();
	bool created = MManager::CreatePool(1024*1024*300); //mPool->CreatePool(500) would also work
	//float* test[5];
	//test[0] = (float*)mpalloc(sizeof(float));
	//test[1] = (float*)mpalloc(sizeof(float));
	//test[2] = (float*)mpalloc(sizeof(float));
	//test[3] = (float*)mpalloc(sizeof(float));
	//test[4] = (float*)mpalloc(sizeof(float));

	//mpfree(test[0]);
	//mpfree(test[3]);
	//mpfree(test[2]);
	//mpfree(test[1]);
	//mpfree(test[4]);
	/*Complex * xp = (Complex*)mpalloc(sizeof(Complex));
	xp = new (xp) Complex(3209.238,8872651.9999278);
	xp->c = 42;
	xp->MyVec.push_back(int(1));
	xp->MyVec.push_back(int(2));
	xp->MyVec.push_back(int(3));
	void* vecpos = &xp->MyVec[0];
	test = new (test) float(3209.098f);
	float tmpf = 39802.8328f;
	test = &tmpf;*/
	MemTest();
	MPTest();
	MPTest2();
	MPTest3();
	delete mPool;
	cin.get();
return 0;	
}

void MemTest(){
	cout << "Beginning Tests\n";
	cout << "*******************************************************************************\n";
	cout << "Testing time taken by System allocation / deallocation (new/delete)...\n";
	Complex* array[1000]; 
	int numObjs =  1000;
	int totalIntervals = 1000;
	DWORD time = GetTickCount();
	for (int i = 0;i < totalIntervals; i++) { 
		for (int j = 0; j < numObjs; j++) { 
			array[j] = new Complex (i, j); 
		} 
		for (int j = 0; j < numObjs; j++) { 
			delete array[j]; 
		} 
	}	
	time = GetTickCount() - time;
	float ftime = (float)(time/1000.0f);
	cout << "The operation took "<< ftime << " second(s)\nto allocate and deallocate " << numObjs <<" objects " << totalIntervals << " times.\n";
	cout << "A total of " << (totalIntervals * numObjs) << " times.\n\n";
}
void MPTest(){
	cout << "*******************************************************************************\n";
	cout << "\nNow testing BrutePool...\nBeginning...\n";	
	cout << "This Test is a standard in-line allocation and deallocation.\n";
	Complex* array[1000];
	DWORD time = GetTickCount();
	for(int i = 0; i < 1000; i++){
		for(int j = 0; j < 1000; j++){
			array[j] = (Complex*)mpalloc(sizeof(Complex));
			array[j]= new(array[j]) Complex(i,j);
		}
		for(int j = 0; j < 1000; j++){
			mpfree(array[j]);
		}
	}	
	time = GetTickCount() - time;
	float ftime = (float)(time / 1000.0f);
	MManager::Pools;
	cout << "Time took with mpalloc: " << ftime << "\n";
	cout << "BrutePool also created/destroyed a total of 1000000 objects.\n\n";
}
unsigned __stdcall MTTest(void* tid){
	DWORD StartTime = GetTickCount();
	DWORD NumSeconds = 10;
	unsigned int ThreadID = *(unsigned int*)tid;
	srand(GetTickCount());
	do{
		//cout << "T_" << ThreadID << "\n";
		Complex* array[2000];
		for(int j = 0; j < 2000; j++){
			array[j] = (Complex*)mpalloc(sizeof(Complex));
			array[j]= new(array[j]) Complex(ThreadID,j);
		}
		vector<int>v1, v2;
		for(int i = 0; i <2000; i++){
			v1.push_back(i);
		}
		for(int i = 0; i <2000; i++){
			int entry = rand()% v1.size();
			v2.push_back(v1[entry]);
			v1.erase(v1.begin() + entry);
		}
		for(int i = 0; i < 2000; i++){
			mpfree(array[v2[i]]);
		}
	}while(GetTickCount() < StartTime + NumSeconds * 1000);
	//cout << "T_" << ThreadID << " exited.\n";
	_endthreadex(0);
	return 0;
}
void MPTest2(){
	cout << "*******************************************************************************\n";
	cout << "\nMPTest2 (Linear Alocation / Random Deallocation Test)\n Beginning...\n";
	srand(GetTickCount());
	DWORD Time = GetTickCount();
	Complex* array[2000];
	for(int j = 0; j < 2000; j++){
		array[j] = (Complex*)mpalloc(sizeof(Complex));
		array[j]= new(array[j]) Complex(1,j);
	}
	vector<int>v1, v2;
	for(int i = 0; i <2000; i++){
		v1.push_back(i);
	}
	for(int i = 0; i <2000; i++){
		int entry = rand()% v1.size();
		v2.push_back(v1[entry]);
		v1.erase(v1.begin() + entry);
	}
	for(int i = 0; i < 2000; i++){
		mpfree(array[v2[i]]);
	}
	Time = GetTickCount() - Time;
	float ftm = (float)(Time / 1000);
	cout << "MPTest2 Completed in " << ftm << "seconds.\n\n";
}
void MPTest3(){
	cout << "*******************************************************************************\n";
	cout << "MPTest3 (Multithread Endurance Test) Beginning...\n";
	DWORD beginTime = GetTickCount();
	//cout << "Running MultiThreaded Test...\n";
	unsigned char numThreads = 6;
	HANDLE thread[6];
	unsigned int IDs[6];
	for(unsigned int i = 0; i < numThreads; i++){
		IDs[i] = i;
		cout << "Creating Thread_" << i << "\n";
		thread[i] = (HANDLE)_beginthreadex(NULL,0, &MTTest, &i, 0, &IDs[i]);
	}
	WaitForMultipleObjects(numThreads, thread,true,INFINITE);
	beginTime = GetTickCount() - beginTime;
	float ftime = (float)(beginTime / 1000.0f);
	cout << "Memory Pool Endured for " << ftime << "seconds.\n";
	cout << "Address of Memory Region to check: " << (*MManager::Pools)[0]->_pMemRegion << "\n";
	cout << "First Block of free memory: " << (*MManager::Pools)[0]->_pFreeFirst << "\n";
	cout << "Last Block of free memory: " << (*MManager::Pools)[0]->_pFreeLast << "\n";
	cout << "Total Pool Size: " << (*MManager::Pools)[0]->PoolSize << "\n";
	cout << "Free Space: " << (*MManager::Pools)[0]->FreeSpace << "\n\n";
}