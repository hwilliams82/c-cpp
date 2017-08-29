/*
  BRUTEPOOL 
  BY: HAROLD E. WILLIAMS JR.
  2/4/2014
*/
#pragma once
#define __USING_WINDOWS
//#define __USING_UNIX

#include <vector>
#ifdef __USING_WINDOWS
#include <Windows.h>
#endif
#ifdef __USING_UNIX
#include <unistd.h>
#include <mutex>
#endif
/*NOTES******************************
**MEMORY ALLOCATION DATA STRUCTURE
**1 Byte (bool) is Occupied (true = occupied | false = free)
**_size_t Bytes(4 32Bit | 8 64Bit) Memory Block Size
**_size_t Bytes(4 32Bit | 8 64Bit) Pointer to Previous Memory Block
**_size_t Bytes(4 32Bit | 8 64Bit) Pointer to Next Memory Block
************************************/

class MPool{
public:
	void* _pFreeFirst;
	void* _pFreeLast;
	//void* _pUsedFirst;
	//void* _pUsedLast;
	void* _pMemRegion;
	size_t PoolSize;
	unsigned char _nullBuffer;
	#ifdef __USING_WINDOWS
	CRITICAL_SECTION cs;
	size_t FreeSpace;
	#endif
	#ifdef __USING_UNIX
	
	#endif
	MPool(size_t Size, unsigned char NullBuffer = 2){
		_nullBuffer = NullBuffer;
		FreeSpace = 0;
		#ifdef __USING_WINDOWS
		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);
		#endif
		#ifdef __USING_UNIX
		
		#endif
		_pFreeFirst = _pFreeLast = /*_pUsedFirst = _pUsedLast = */_pMemRegion = NULL;
		do{
			//LOOP CONTINUALLY UNTIL WE GET A GOOD BLOCK OF MEMORY\
			THIS LOOP SHOULD ONLY HAPPEN NO MORE THAN A COUPLE TIMES

			
			if(Size <= (1 + (sizeof(size_t) * 3) + NullBuffer)){//handle this ridiculous possibily lol
				Size = 2 + (sizeof(size_t) * 3)  + NullBuffer;
			}
			//TRY TO CREATE A NEW BLOCK OF MEMORY OF Size SIZE
			try{
				_pMemRegion = new char[Size];
			}catch(std::bad_alloc& err){

			//IF THE ALLOCATION DOESN'T WORK, GET THE AMOUNT OF SYSTEM MEMORY AVAILABLE\
			DIVIDE THIS AMOUNT BY TWO TO COME TO A SIZE THAT THE SYSTEM SHOULD BE ABLE TO HANDLE
				#ifdef __USING_WINDOWS
					MEMORYSTATUSEX status;
					status.dwLength = sizeof(status);
					GlobalMemoryStatusEx(&status);
					Size = (size_t)status.ullAvailPhys / 2;
					if(Size <= (1 + (sizeof(size_t) * 3)+NullBuffer)){
						return;
					}
				#endif
				#ifdef __USING_UNIX
					long pages = sysconf(_SC_PHYS_PAGES);
					long page_size = sysconf(_SC_PAGE_SIZE);
					Size =  (size_t)((pages * page_size) / 2);
					if(Size < 20){
						return;
					}
				#endif
			}
		}while(!_pMemRegion);
		if(_pMemRegion){
			_pFreeFirst = _pFreeLast = _pMemRegion;
			//THE NEW FREE MEMORY BLOCK MUST COMPENSATE FOR THE SPACE USED BY THE ALLOCATION AND BUFFER DATA
			size_t compSize = Size - ((sizeof(size_t) * 3) + 1 + NullBuffer);
			bool isOccupied = false;
			memcpy(_pMemRegion, &isOccupied, sizeof(bool));
			memcpy((char*)_pMemRegion + 1, &compSize, sizeof(size_t));
			memcpy((char*)_pMemRegion + 1 + sizeof(size_t), &_pFreeFirst, sizeof(size_t));
			memcpy((char*)_pMemRegion + 1 + (sizeof(size_t)*2), &_pFreeLast, sizeof(size_t));
			memset((char*)_pMemRegion + 1 + (sizeof(size_t)*3) + compSize, 0, NullBuffer); //Commenting out this line will save time. I left it here for debugging purposes.
			FreeSpace = compSize;
			PoolSize = Size;
		}
	#ifdef __USING_WINDOWS
			LeaveCriticalSection(&cs);
	#endif
	#ifdef __USING_UNIX

	#endif
	}
	~MPool(){
		if(_pMemRegion){
			#ifdef __USING_WINDOWS
			EnterCriticalSection(&cs);
			#endif
			#ifdef __USING_UNIX
			#endif
			delete _pMemRegion;
			#ifdef __USING_WINDOWS
			LeaveCriticalSection(&cs);
			#endif
			#ifdef __USING_UNIX
			#endif
			_pFreeFirst = _pFreeLast = /*_pUsedFirst = _pUsedLast = */_pMemRegion = NULL;
		}
	}
};

class MManager{
public:
	static size_t _totalFreeSpace;
	static std::vector<MPool*>*Pools;
	static CRITICAL_SECTION csPools;
	static bool CreatePool(size_t Size){
		//CREATE A NULL POINTER
		MPool * newPool = NULL;

		try{
			//ALLOCATE THE REQUESTED SPACE
			newPool = new MPool(Size, 2);
		}catch(std::bad_alloc& err){
			err.what(); //POST APPROPRIATE WARNING USING THIS LATER...
		}
		if(newPool){
			if(newPool->_pMemRegion){
				#ifdef __USING_WINDOWS
				EnterCriticalSection(&MManager::csPools);
				#endif
				#ifdef __USING_UNIX
				#endif
				MManager::Pools->push_back(newPool);
				MManager::_totalFreeSpace += newPool->FreeSpace;
			}
		}else{
			#ifdef __USING_WINDOWS
			LeaveCriticalSection(&MManager::csPools);
			#endif
			#ifdef __USING_UNIX
			#endif
			return false;
		}
		#ifdef __USING_WINDOWS
		LeaveCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
		return true;
	}
	static size_t NumPools(void){
		size_t tNumPools = 0;
		#ifdef __USING_WINDOWS
		EnterCriticalSection(&MManager::csPools);
		tNumPools = MManager::Pools->size();
		LeaveCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
		return tNumPools;
	}
	MManager(){
		_totalFreeSpace = 0;
		//TRY TO CREATE A NEW POOL VECTOR
		#ifdef __USING_WINDOWS
		InitializeCriticalSection(&MManager::csPools);
		EnterCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
		MManager::Pools = NULL;
		try{
			MManager::Pools = new std::vector<MPool*>();
		}catch(std::bad_alloc& err){

		}
		#ifdef __USING_WINDOWS
		LeaveCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
	}

	~MManager(){
		#ifdef __USING_WINDOWS
		EnterCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
		//IF THE Pool VECTOR IS NOT NULL
		if(Pools){
			//LOOP THROUGH THE VECTOR, DEALLOCATING THE MEMORY USED, AND SETTING EACH ELEMENT TO NULL (which probably isn't necessary at this point)
			std::vector<MPool*>::iterator i;
			for(i = MManager::Pools->begin(); i != MManager::Pools->end(); i++){
				if(*i){//IF THE VALUE THAT i POINTS TO IS NOT NULL
					free(*i);
					*i = NULL;
				}
			}
			Pools->clear();
			delete Pools;
			Pools = NULL;
		}
		_totalFreeSpace = 0;
		#ifdef __USING_WINDOWS
		LeaveCriticalSection(&MManager::csPools);
		DeleteCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
	}

};
std::vector<MPool*>* MManager::Pools;
size_t MManager::_totalFreeSpace;
CRITICAL_SECTION MManager::csPools;

void* mpalloc(size_t _reqBlockSize){
	void* _newBlock = NULL;
	if(_reqBlockSize < 1){
		return NULL;
	}
	//SET SOME DATA FOR THE MEMORY BLOCK OFFSETS
	//SINCE THIS TOOL IS MEANT FOR 32/64 BIT SYSTEMS
	//IT'LL BE ADVANTAGEOUS TO GET THE OFFSETS EARLY AND
	//ONE TIME ONLY BECAUSE IT'S LIKELY THESE VALUES WILL BE REUSED
	//SEVERAL TIMES IN THIS CALL
	unsigned char _ptrSizeOffset = 1;
	unsigned char _ptrPrevOffset = sizeof(size_t*) + 1;
	unsigned char _ptrNextOffset = (sizeof(size_t*)*2) + 1;
	unsigned char _headerSize = 1 + (sizeof(size_t*)*3);
	int i = 0;

	//HERE'S A GOOD SPACE TO ALLOCATE A NEW POOL IF WE'RE COMPLETELY OUT OF MEMORY\
	AND ALSO CHECK IF THE SYSTEM IS OUT OF MEMORY
	bool newPoolTrue = false;
	size_t AvgPoolSize = 0;
	#ifdef __USING_WINDOWS
	EnterCriticalSection(&MManager::csPools);
	#endif
	#ifdef __USING_UNIX
	#endif
	if(_reqBlockSize > MManager::_totalFreeSpace){
		if(MManager::Pools->size() > 0){
			for(int pl_it = 0; pl_it < MManager::Pools->size(); pl_it++){
				AvgPoolSize += (*MManager::Pools)[pl_it]->PoolSize;
			}
			AvgPoolSize /= MManager::Pools->size();
			if((newPoolTrue = MManager::CreatePool(AvgPoolSize*2)) == true){
				i = MManager::Pools->size() -1;//since we already know there's not enough free space with the previous memory\
				blocks, just go ahead and set the pool iterator to the last pool we just created
				//*NOTE! Also, this section isn't thread safe.
			}else{
				return NULL;
				//also maybe throw an exception here
			}
		}else{
			if((newPoolTrue = MManager::CreatePool(1024*1024 * 10)) == true){//allocate 1MB by default
				i = MManager::Pools->size() -1;
			}else{
				return NULL;
				//also maybe throw an exception here too
			}
		}
	}
	#ifdef __USING_WINDOWS
	LeaveCriticalSection(&MManager::csPools);
	#endif
	#ifdef __USING_UNIX
	#endif

	//LOOP THROUGH EACH POOL 
	do{

	//ENTERING THREAD SAFE AREA HERE
		#ifdef __USING_WINDOWS
		if(TryEnterCriticalSection(&(*MManager::Pools)[i]->cs)){
		#endif
		#ifdef __USING_UNIX
		#endif

		void* SeekPtr = (*MManager::Pools)[i]->_pFreeFirst;
		unsigned char NullBuffer = (*MManager::Pools)[i]->_nullBuffer;

		//HERE WE'RE GOING TO CHECK FIRST IF THE POOL EVEN HAS ENOUGH FREE SPACE
		if((*MManager::Pools)[i]->FreeSpace >= _reqBlockSize){
			//GO THROUGH THE LIST TO FIND THE FIRST ADEQUATELY SIZED EMPTY BLOCK
			do{
				//CHECK IF THE BLOCK IS LARGE ENOUGH TO SPLIT INTO TWO CHUNKS
				//size_t dbgBlockSize = (size_t)*(size_t*)((char*)SeekPtr + _ptrSizeOffset);
				//size_t dbgNeededSize = _reqBlockSize + _headerSize + NullBuffer;
				if((size_t)*(size_t*)((char*)SeekPtr + _ptrSizeOffset) >= _reqBlockSize + _headerSize + NullBuffer){//*(bool*)SeekPtr == false){
				
					//CREATE A TEMPORARY POINTER TO THE TO-BE-NEWLY-CREATED FREE BLOCK AND SOME ADDITIONAL NEEDED VARIABLES TO MAKE LIFE EASIER
					void* _NewFreePartitionPosition = ((char*)SeekPtr + _headerSize + _reqBlockSize + NullBuffer);
					//bool newIsOccupied = false;
					size_t origSize = *(size_t*)((char*)SeekPtr + _ptrSizeOffset);
					size_t newSize = origSize - (_reqBlockSize + NullBuffer + _headerSize);
					//GET THE POSITIONS OF THE PREVIOUS AND NEXT BLOCKS IF THEY EXIST, OTHERWISE, SET THEM (RESPECTIVELY) TO THE NEW PARTITION POSITION
					void* _tmpPrevBlock;(void*)*(size_t*)((char*)SeekPtr + _ptrPrevOffset) < SeekPtr ? _tmpPrevBlock = (void*)*(size_t*)((char*)SeekPtr + _ptrPrevOffset) : _tmpPrevBlock = _NewFreePartitionPosition;
					void* _tmpNextBlock;(void*)*(size_t*)((char*)SeekPtr + _ptrNextOffset) > SeekPtr ? _tmpNextBlock = (void*)*(size_t*)((char*)SeekPtr + _ptrNextOffset) : _tmpNextBlock = _NewFreePartitionPosition;
				
					//MAKE SURE TO CHANGE THE PREV/NEXT POINTERS OF THE Next and Free FREE BLOCKS RESPECTIVELY AS NEEDED
					if(_tmpPrevBlock != _NewFreePartitionPosition)
						if((void*)(size_t)*((char*)_tmpPrevBlock + _ptrNextOffset) != SeekPtr) memcpy((char*)_tmpPrevBlock + _ptrNextOffset, &_NewFreePartitionPosition, sizeof(size_t));
					if(_tmpNextBlock != _NewFreePartitionPosition)
						if((void*)(size_t)*((char*)_tmpNextBlock + _ptrPrevOffset) != SeekPtr) memcpy((char*)_tmpNextBlock + _ptrPrevOffset, &_NewFreePartitionPosition, sizeof(size_t));
				
					//CREATE A NEW OCCUPIED CHUNK OF MEMORY
					_NewFreePartitionPosition = new(_NewFreePartitionPosition) bool(false);
					memcpy((char*)_NewFreePartitionPosition + _ptrSizeOffset, &newSize, sizeof(size_t));
					memset((char*)_NewFreePartitionPosition + _headerSize + newSize, 0, NullBuffer);
					memcpy((char*)_NewFreePartitionPosition + _ptrPrevOffset, &_tmpPrevBlock, sizeof(size_t));
					memcpy((char*)_NewFreePartitionPosition + _ptrNextOffset, &_tmpNextBlock, sizeof(size_t));
					//SET THE VALUES OF THE NEWLY OCCUPIED CHUNK OF MEMORY
					//newIsOccupied = true;
					//memcpy(SeekPtr, &newIsOccupied, sizeof(bool));
					SeekPtr = new(SeekPtr) bool(true);
					memcpy((char*)SeekPtr + _ptrSizeOffset, &_reqBlockSize, sizeof(size_t));
					memset((char*)SeekPtr + _headerSize + _reqBlockSize, 0,NullBuffer);
	/******************************************************************************************************
					//DO I REALLY NEED TO SET BLOCK POINTERS BETWEEN OCCUPIED BLOCKS OF MEMORY?
					//OR JUST LEAVE THEM AS IS? I COULD GAIN A SLIGHT TIME SAVINGS BY LEAVING THEM ALONE.
					//AT THE TIME BEING, I REALLY ONLY THINK I NEED BLOCK POINTERS FOR FREE (EMPTY) BLOCKS
	******************************************************************************************************/

					//CHECK IF THE ADDRESS OF THE PREVIOUS FREE CHUNK IS EQUAL TO CURRENT FREE BLOCK OR ACTUALLY TO ANOTHER FREE BLOCK
					//CHECK/CHANGE THE FREE MEMORY POINTERS OF THE POOL SINCE THE FREE BLOCK IS MOVING
					if(SeekPtr == (*MManager::Pools)[i]->_pFreeFirst){
						//this means the current block of free memory is the lowest (or leftmost) block				
						(*MManager::Pools)[i]->_pFreeFirst = _NewFreePartitionPosition;
					}
					if(SeekPtr == (*MManager::Pools)[i]->_pFreeLast){
						(*MManager::Pools)[i]->_pFreeLast = _NewFreePartitionPosition;
					}
					//ADJUST THE POOLs FREE SPACE TO REFLECT THE SPACE LOST IN THE NEW ALLOCATION
					(*MManager::Pools)[i]->FreeSpace -= (origSize - newSize);
					#ifdef __USING_WINDOWS
					EnterCriticalSection(&MManager::csPools);
					MManager::_totalFreeSpace -= (origSize - newSize);
					LeaveCriticalSection(&MManager::csPools);
					_newBlock = (void*)((char*)SeekPtr + _headerSize);
					LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
					#endif
					#ifdef __USING_UNIX
					#endif
					//FINALLY RETURN FROM THIS FUNCITON
					return (_newBlock);


				}else if((*(size_t*)((char*)SeekPtr + _ptrSizeOffset) >= _reqBlockSize) && *(size_t*)((char*)SeekPtr + _ptrSizeOffset) < _reqBlockSize + _headerSize + NullBuffer){
				//IF THE BLOCK IS TOO SMALL TO PARTITION BUT LARGE ENOUGH TO USE, SIMPLY SET IT TO OCCUPIED AND BUFFER THE REST OF THE SPACE WITH ZERO?
					//return block of memory
					void* _tmpPrevBlock = (void*)*(size_t*)((char*)SeekPtr + _ptrPrevOffset);
					void* _tmpNextBlock = (void*)*(size_t*)((char*)SeekPtr + _ptrNextOffset);

					//********************THIS IS WHERE WE TELL THE NEXT HIGHER AND LOWER BLOCKS WHERE TO POINT********************/
					//********************************************PROVIDED THEY EXIST**********************************************/
					//IF THERE IS A FREE BLOCK LOWER THAN THE CURRENT BLOCK
					if(SeekPtr > _tmpPrevBlock){
						//CHECK IF THE NEXT POINTER OF THE LOWER FREE BLOCK IS GOING TO POINT TO A HIGHER BLOCK
						if(SeekPtr < _tmpNextBlock){
							memcpy(((char*)_tmpPrevBlock + _ptrNextOffset), &_tmpNextBlock, sizeof(size_t*));
						//OR IF IT'S GOING TO POINT TO ITSELF BECAUSE THERE IS NO HIGHER FREE BLOCK
						}else{
							memcpy(((char*)_tmpPrevBlock + _ptrNextOffset), &_tmpPrevBlock, sizeof(size_t*));
						}
					}
					//DO THE SAME THING FOR THE NEXT FREE BLOCK IF IT EXISTS
					if(SeekPtr < _tmpNextBlock){
						if(SeekPtr > _tmpPrevBlock){
							memcpy(((char*)_tmpNextBlock + _ptrPrevOffset), &_tmpPrevBlock, sizeof(size_t*));
						}else{
							memcpy(((char*)_tmpNextBlock + _ptrPrevOffset), &_tmpNextBlock, sizeof(size_t*));
						}
					}
					//*NOTE! You may wonder "what if we're taking the last free block of memory or the only block of memory, \
					the above code doesn't detail where	to point this blocks prev/next pointers". Well, rest assured that\
					since any lone free block at the beginning and end always points back to itself, Therefore there is no\
					need to handle this instance again.

					//*****************THIS IS WHERE WE TELL THE POOL OBJECT TO POINT ITS FIRST FREE / LAST FREE POINTERS************/
					//**************************ALSO PROVIDED THAT THERE ARE ACTUALLY FREE BLOCKS TO WHICH TO POINT*******************/
					if(SeekPtr == (*MManager::Pools)[i]->_pFreeFirst && SeekPtr == (*MManager::Pools)[i]->_pFreeLast){
						(*MManager::Pools)[i]->_pFreeFirst = (*MManager::Pools)[i]->_pFreeLast = NULL;
					}else if(SeekPtr == (*MManager::Pools)[i]->_pFreeFirst && SeekPtr != (*MManager::Pools)[i]->_pFreeLast){
						//this means the current block of free memory is the lowest (or leftmost) block
						(*MManager::Pools)[i]->_pFreeFirst = (void*)*(size_t*)((char*)SeekPtr + _ptrNextOffset);
					}else if(SeekPtr == (*MManager::Pools)[i]->_pFreeLast && SeekPtr != (*MManager::Pools)[i]->_pFreeFirst ){
						(*MManager::Pools)[i]->_pFreeLast = (void*)*(size_t*)((char*)SeekPtr + _ptrPrevOffset);
					}

					//********************************AFTER THAT STUFF'S SAID AND DONE, ALLOCATE THE BLOCK OF MEMORY AND RETURN*******/
					SeekPtr = new(SeekPtr) bool(true);//i'm gonna try this and see what happens...
					//*NOTE! I've decided here not to set the bytes greater than _reqBlockSize to zero because,\
					in this case, it won't hurt anything and it's faster to do nothing.
					(*MManager::Pools)[i]->FreeSpace -= *(size_t*)((char*)SeekPtr + _ptrSizeOffset);
					#ifdef __USING_WINDOWS
					EnterCriticalSection(&MManager::csPools);
					MManager::_totalFreeSpace -= *(size_t*)((char*)SeekPtr + _ptrSizeOffset);
					LeaveCriticalSection(&MManager::csPools);
					_newBlock = (void*)((char*)SeekPtr + _headerSize);
					LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
					#endif
					#ifdef __USING_UNIX
					#endif
					return(_newBlock);
				}else{
					//SET THE SeekPtr TO THE NEXT FREE CHUNK
					SeekPtr = (void*)*((char*)SeekPtr + _ptrNextOffset);
				}
				//IF WE STILL DON'T FIND ANY USABLE SPACE AT THE END OF EVEN THE LAST POOL, CREATE A \
				NEW POOL AND GO THROUGH THE LOOP AGAIN AT THE BEGINNING OF THE NEXT POOL
				if(SeekPtr == (*MManager::Pools)[i]->_pFreeLast){
					if(i == MManager::NumPools()){
					//FIRST RELEASE THE CRITICAL SECTION ASSOCIATED WITH THE CURRENT POOL
					#ifdef __USING_WINDOWS
						LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
						//CREATE A NEW POOL (The same methodology as above)
						EnterCriticalSection(&MManager::csPools);
					#endif
					#ifdef __USING_UNIX
					#endif
						for(int pl_it = 0; pl_it < MManager::Pools->size(); pl_it++){
							AvgPoolSize += (*MManager::Pools)[pl_it]->PoolSize;
						}
						AvgPoolSize /= MManager::Pools->size();
						if(MManager::CreatePool(AvgPoolSize*2) == true){
							i = MManager::Pools->size() -1;
						}else{
							#ifdef __USING_WINDOWS
							LeaveCriticalSection(&MManager::csPools);
							#endif
							#ifdef __USING_UNIX
							#endif
							return NULL;
							//also maybe throw an exception here
						}
						#ifdef __USING_WINDOWS
						LeaveCriticalSection(&MManager::csPools);
						//ENTER THE NEXT POOLS CRITICAL SECTION
						EnterCriticalSection(&(*MManager::Pools)[i]->cs);					
						#endif
						#ifdef __USING_UNIX
						#endif

						//SET THE SeekPtr TO THE NEXT POOLS _pFreeFirst AND CONTINUE...
						SeekPtr = (*MManager::Pools)[i]->_pFreeFirst;
					}
				}
			}while(SeekPtr && SeekPtr != (*MManager::Pools)[i]->_pFreeLast);
		}
		#ifdef __USING_WINDOWS
		LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
		#endif
		#ifdef __USING_UNIX
		#endif
		}
	//LEAVING THREAD SAFE AREA

		//THIS NEXT LITTLE AREA ENSURES THAT THIS LOOP KEEPS CYCLING THROUGH THE VARIOUS POOLS 
		//UNTIL IT FINDS A POOL WHOS CRITICAL SECTION/MUTEX ISN'T BEING OCCUPIED
		//THEREFORE, IF THERE ARE MORE THAN 1 POOL, ONE THREAD DOESN'T BLOCK UNTIL ANOTHER FREES UP THE CRITICAL SECTION/MUTEX
		//IT CAN SIMPLY MOVE ONTO THE NEXT POOL AND SEE IF IT CAN ALLOCATE MEMORY FROM THERE
		if(i == MManager::NumPools() - 1 && !_newBlock){
			i = -1;
		}
	}while(++i < MManager::NumPools()); //*NOTE! MManager::NumPools() is thread-safe (i think)...

	return _newBlock;
}
template<typename T> void mpfree(T* ObjectToFree){
	void* BlockPtr = (char*)ObjectToFree - 1 - (sizeof(void*)*3);//point to the block of memory by way of the object pointer
	size_t i;
	//1). FIND OUT WHICH POOL THIS PIECE OF MEMORY BELONGS TO
	#ifdef __USING_WINDOWS
	EnterCriticalSection(&MManager::csPools);
	#endif
	#ifdef __USING_UNIX
	#endif
	for(i = 0; i < MManager::Pools->size(); i++){
		if(BlockPtr >= (*MManager::Pools)[i]->_pMemRegion && BlockPtr < (void*)(size_t*)((char*)(*MManager::Pools)[i]->_pMemRegion + (*MManager::Pools)[i]->PoolSize)){
			break;
		}
	}	
	//in case BlockPtr could not be found,\
	this should indicate someone accidentally tries to free memory that's doesn't rightfully belong to any pool \
	simply return.
	if(i >= MManager::Pools->size()){
		#ifdef __USING_WINDOWS
		LeaveCriticalSection(&MManager::csPools);
		#endif
		#ifdef __USING_UNIX
		#endif
		return;
	}
	#ifdef __USING_WINDOWS
	LeaveCriticalSection(&MManager::csPools);
	#endif
	#ifdef __USING_UNIX
	#endif


	//2). Form some useful variables
	unsigned char _ptrSizeOffset = 1;
	unsigned char _ptrPrevOffset = sizeof(size_t*) + 1;
	unsigned char _ptrNextOffset = (sizeof(size_t*)*2) + 1;
	unsigned char _headerSize = 1 + (sizeof(size_t*)*3);
	#ifdef __USING_WINDOWS
	EnterCriticalSection(&(*MManager::Pools)[i]->cs);
	unsigned char NullBuff = (*MManager::Pools)[i]->_nullBuffer;
	LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
	#endif
	#ifdef __USING_UNIX
	//enter mutex
	unsigned char NullBuff = (*MManager::Pools)[i]->_nullBuffer;
	//leave mutex
	#endif
	size_t BlockSize =  *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + _headerSize + NullBuff;
	size_t BlockSpace =  *(size_t*)((char*)BlockPtr + _ptrSizeOffset);
	size_t NewSize;//this will be used later down the road


	//loop through each free block starting from the first \
	free block until the address of the looping block\
	is higher than the address of the memory being freed\
	once it is, we now have a pointer to the next highest free memory\
	and, thusly, the address of the next lowest free block\
	If the first free looping block is already higher than the \
	new free block, just check if the pointer of the previous pointer\
	is equal to the the address of the first looping block\
	this may prove to be rather efficient \
	I may want to also compare the position of the newly freed block and\
	those of the _pFirst and _pLast free blocks to see if I should \
	loop forward from the first or backward from the last possibly increasing efficiency
	
	//NOTE don't forget to defragment neighboring free blocks here as well.

	//IF EITHER FIRST_FREE OR LAST_FREE IS NULL (They should either both be null or populated at the sime time)
	#ifdef __USING_WINDOWS
	EnterCriticalSection(&(*MManager::Pools)[i]->cs);
	#endif
	#ifdef __USING_UNIX
	#endif

	if(!(*MManager::Pools)[i]->_pFreeFirst || !(*MManager::Pools)[i]->_pFreeLast){
		//handle the two instances separately just to be safe
		if(!(*MManager::Pools)[i]->_pFreeFirst){
			(*MManager::Pools)[i]->_pFreeFirst = BlockPtr;
			size_t* BlockPrev = new((size_t*)((char*)BlockPtr + _ptrPrevOffset)) size_t(*(size_t*)&BlockPtr);
		}
		if(!(*MManager::Pools)[i]->_pFreeLast){
			(*MManager::Pools)[i]->_pFreeLast = BlockPtr;
			size_t* BlockNext = new((size_t*)((char*)BlockPtr + _ptrNextOffset)) size_t(*(size_t*)&BlockPtr);
		}
		NewSize = *(size_t*)((char*)BlockPtr + _ptrSizeOffset);
	}else{
	//OTHERWISE....
		void* NextHigher,* NextLower;
		//void* dbgBlockEnd = (size_t*)((char*)BlockPtr + BlockSize);//debug and test this...
		if((*MManager::Pools)[i]->_pFreeFirst >= (size_t*)((char*)BlockPtr + BlockSize)){

		//IF FIRST_FREE IS GREATER THAN THIS BLOCK
			size_t* BlockNext = new((char*)BlockPtr+_ptrNextOffset) size_t(*(size_t*)&(*MManager::Pools)[i]->_pFreeFirst);
			size_t* BlockPrev = new((char*)BlockPtr + _ptrPrevOffset) size_t(*(size_t*)&BlockPtr);
			size_t* pFreeFirstPrev = new((char*)(*MManager::Pools)[i]->_pFreeFirst + _ptrPrevOffset) size_t(*(size_t*)&BlockPtr);
			NextLower = BlockPtr;
			NextHigher = (*MManager::Pools)[i]->_pFreeFirst;
			(*MManager::Pools)[i]->_pFreeFirst = BlockPtr;
		}else if((*MManager::Pools)[i]->_pFreeLast < BlockPtr){

		//IF LAST_FRE IS LESS THAN THIS BLOCK
			size_t* BlockPrev = new((char*)BlockPtr+_ptrPrevOffset) size_t(*(size_t*)&(*MManager::Pools)[i]->_pFreeLast);
			size_t* BlockNext = new((char*)BlockPtr + _ptrNextOffset) size_t(*(size_t*)&BlockPtr);
			size_t* pFreeLastNext = new((char*)(*MManager::Pools)[i]->_pFreeLast + _ptrNextOffset) size_t(*(size_t*)&BlockPtr);
			NextHigher = BlockPtr;
			NextLower = (*MManager::Pools)[i]->_pFreeLast;
			(*MManager::Pools)[i]->_pFreeLast = BlockPtr;
		}else if(BlockPtr > (*MManager::Pools)[i]->_pFreeFirst && BlockPtr < (*MManager::Pools)[i]->_pFreeLast){

		//IF THE BLOCK IS BETWEEN THE FIRST AND LAST EMPTIES
			if((size_t*)BlockPtr - (size_t*)(*MManager::Pools)[i]->_pFreeFirst <= (size_t*)(*MManager::Pools)[i]->_pFreeLast - (size_t*)BlockPtr){
			//IF BLOCK PTR IS CLOSER TO FREE FIRST OR IF IT'S EQUIDISTANT BETWEEN FREE FIRST AND FREE LAST
				NextLower = (*MManager::Pools)[i]->_pFreeFirst;
				while((void*)*(size_t*)((char*)NextLower + _ptrNextOffset) < BlockPtr){
					NextLower = (void*)*(size_t*)((char*)NextLower + _ptrNextOffset);
				}
				NextHigher = (void*)*(size_t*)((char*)NextLower + _ptrNextOffset);
				//*NOTE (debug)! ensure that NextLower is less than BlockPtr
			}else if((size_t*)(*MManager::Pools)[i]->_pFreeLast - (size_t*)BlockPtr < (size_t*)BlockPtr - (size_t*)(*MManager::Pools)[i]->_pFreeFirst){
			//IF BLOCK PTR IS CLOSER TO FREE LAST
				NextHigher = (*MManager::Pools)[i]->_pFreeLast;
				while((void*)*(size_t*)((char*)NextHigher + _ptrPrevOffset) > BlockPtr){
					NextHigher = (void*)*(size_t*)((char*)NextHigher + _ptrPrevOffset);
				}
				NextLower = (void*)*(size_t*)((char*)NextHigher + _ptrPrevOffset);
			}
		}
			
		//size_t* dbgNextLower = (size_t*)((char*)NextLower + _headerSize + NullBuff + *(size_t*)((char*)NextLower + _ptrSizeOffset));
		//size_t* dbgNextHigher = (size_t*)((char*)BlockPtr + _headerSize + NullBuff + *(size_t*)((char*)BlockPtr + _ptrSizeOffset));

		//DO THE CONDITIONAL DEFRAGMENTATION HERE AND MAKE SURE THE BLOCKS POINT TO THE CORRECT NEIGHBORS
		bool DefragLower = false;bool DefragHigher = false;
		if(BlockPtr == (void*)(size_t*)((char*)NextLower + _headerSize + NullBuff + *(size_t*)((char*)NextLower + _ptrSizeOffset))){
			DefragLower = true;
		}
		if(NextHigher == (void*)(size_t*)((char*)BlockPtr + _headerSize + NullBuff + *(size_t*)((char*)BlockPtr + _ptrSizeOffset))){
			DefragHigher = true;
		}

		//CHECKING FOR ONE OF THE FOUR POSSIBLE CONDITIONS OF DEFRAGMENTATION
		if(DefragLower && DefragHigher){
			void* NextNextHigher = (void*)*(size_t*)((char*)NextHigher + _ptrNextOffset);
			//IF THE NextHigher != _pFreeLast
			if(NextHigher < (*MManager::Pools)[i]->_pFreeLast){
				//set the Next Lower _next pointer to point to the next next higher block...
				size_t* nlNext = new(((char*)NextLower + _ptrNextOffset)) size_t(*(size_t*)&NextNextHigher);
				//set the next next higher blocks _prev pointer to point to the next lower...
				size_t* nnhPrev = new(((char*)NextNextHigher + _ptrPrevOffset)) size_t(*(size_t*)&NextLower);//not at all sure about this one...
			}else{
				(*MManager::Pools)[i]->_pFreeLast = NextLower;
				//this should indicate we're at the end of the memory, therefore, point next lower's next to itself
				size_t* nlNext = new(((char*)NextLower + _ptrNextOffset)) size_t(*(size_t*)&NextLower);
			}
			//NextLowers prev pointer should already be ok
			//SET THE CONGLOMERATED BLOCKS NEW SIZE
			NewSize = /**(size_t*)((char*)NextLower + _ptrSizeOffset) +*/ *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + /**(size_t*)((char*)NextHigher + _ptrSizeOffset) +*/ (NullBuff * 2) + (_headerSize * 2);
			size_t* newBlockSize = new(((char*)NextLower + _ptrSizeOffset)) size_t(*(size_t*)((char*)NextLower + _ptrSizeOffset) + *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + *(size_t*)((char*)NextHigher + _ptrSizeOffset) + (NullBuff * 2) + (_headerSize * 2));
			BlockPtr = NextLower;


		}else if(DefragLower && !DefragHigher){
			if(BlockPtr == (*MManager::Pools)[i]->_pFreeLast){//I MAY NOT NEED THIS, IT'S ALL KIND OF A BLUR AT THIS POINT (now I kind of do think I need this block...)
				(*MManager::Pools)[i]->_pFreeLast = NextLower;
				size_t* nlNext = new((char*)NextLower + _ptrNextOffset) size_t(*(size_t*)&NextLower);
			}
			NewSize = /**(size_t*)((char*)NextLower + _ptrSizeOffset) +*/ *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + _headerSize + NullBuff;
			size_t* newBlockSize = new(((char*)NextLower + _ptrSizeOffset)) size_t(*(size_t*)((char*)NextLower + _ptrSizeOffset) + *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + _headerSize + NullBuff);
			BlockPtr = NextLower;


		}else if(DefragHigher && !DefragLower){
			if(NextHigher == (*MManager::Pools)[i]->_pFreeLast){//BUT I HAVE TO DO THIS WHILE IT'S (RELATIVELY) FRESH IN MY MIND
				(*MManager::Pools)[i]->_pFreeLast = BlockPtr;
				size_t* bpNext = new((char*)BlockPtr + _ptrNextOffset) size_t(*(size_t*)&BlockPtr);
			}
			if(BlockPtr > (*MManager::Pools)[i]->_pFreeFirst){
				size_t* nlNext = new(((char*)NextLower + _ptrNextOffset)) size_t(*(size_t*)&BlockPtr); //point next lower to block
				size_t* blPrev = new(((char*)BlockPtr + _ptrPrevOffset)) size_t(*(size_t*)&NextLower);//point block to next lower via the NextHigher->_pPrevOffset value
			}else{
				size_t* blPrev = new(((char*)BlockPtr + _ptrPrevOffset)) size_t(*(size_t*)&BlockPtr);//point block to next lower via the NextHigher->_pPrevOffset value
			}
			void* NextNextHigher = (void*)*(size_t*)((char*)NextHigher + _ptrNextOffset);//GAIN A POINTER TO THE NEXT NEXT HIGHER (either points to actual next-next-higher or next-higher)
			
			if(BlockPtr < (*MManager::Pools)[i]->_pFreeLast){
				size_t* nnhPrevnew = new(((char*)NextNextHigher + _ptrPrevOffset)) size_t(*(size_t*)&BlockPtr);//point the next-next-higher->_prev value to BlockPtr
				size_t* blNext = new(((char*)BlockPtr + _ptrNextOffset)) size_t(*(size_t*)&NextNextHigher);//point the BlockPointer->_next to next-next-higher
			}else{
				size_t* blNext = new(((char*)BlockPtr + _ptrNextOffset)) size_t(*(size_t*)&BlockPtr);//point the BlockPointer->_next to next-next-higher
			}
			//SET THE SIZE OF THE NEW BLOCK AND GET IT READY TO CONGLOMERATE
			NewSize = *(size_t*)((char*)BlockPtr + _ptrSizeOffset) + /**(size_t*)((char*)NextHigher + _ptrSizeOffset)*/ + _headerSize + NullBuff;
			size_t* newBlockSize = new(((char*)BlockPtr + _ptrSizeOffset)) size_t(*(size_t*)((char*)BlockPtr + _ptrSizeOffset) + *(size_t*)((char*)NextHigher + _ptrSizeOffset) + _headerSize + NullBuff);


		}else if(!DefragLower && !DefragHigher){

			//HERE EITHER THESE WILL POINT TO A VALID BLOCK, OR ITSELF
			if(BlockPtr > (*MManager::Pools)[i]->_pFreeFirst){
				size_t* nlNext = new(((char*)NextLower + _ptrNextOffset)) size_t(*(size_t*)&BlockPtr);
				size_t* blkPrev = new(((char*)BlockPtr + _ptrPrevOffset)) size_t(*(size_t*)&NextLower);
			}else{
				size_t* blkPrev = new(((char*)BlockPtr + _ptrPrevOffset)) size_t(*(size_t*)&BlockPtr);
			}
			if(BlockPtr < (*MManager::Pools)[i]->_pFreeLast){
				size_t* nhPrev = new(((char*)NextHigher + _ptrPrevOffset)) size_t(*(size_t*)&BlockPtr);
				size_t* blkNext = new(((char*)BlockPtr + _ptrNextOffset)) size_t(*(size_t*)&NextHigher);
			}else{
				size_t* blkNext = new(((char*)BlockPtr + _ptrNextOffset)) size_t(*(size_t*)&BlockPtr);
			}
			NewSize = *(size_t*)((char*)BlockPtr + _ptrSizeOffset);
		}
	}
	bool* newlyFreed = new(BlockPtr) bool(false);
	//*Note! I may take the following out in order to save time...but maybe not
	#ifndef NDEBUG
	memset((char*)BlockPtr + _headerSize, 0xCD, *(size_t*)((char*)BlockPtr + _ptrSizeOffset));//this will set the current free space within the block to the uninitialized character
	#endif
	//*Note! I was going to do the defragmentation here, but that would require more writing to bytes\
	it seems more efficient to handle them above if necessary
	(*MManager::Pools)[i]->FreeSpace += NewSize;// *(size_t*)((char*)BlockPtr + _ptrSizeOffset);
	#ifdef __USING_WINDOWS
	LeaveCriticalSection(&(*MManager::Pools)[i]->cs);
	EnterCriticalSection(&MManager::csPools);
	MManager::_totalFreeSpace += NewSize; //*(size_t*)((char*)BlockPtr + _ptrSizeOffset);
	LeaveCriticalSection(&MManager::csPools);
	#endif
	#ifdef __USING_UNIX
	#endif
}