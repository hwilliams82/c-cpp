# c-cpp-Memory-Pool-Utility
	I wrote this simple little tool a little more than 3 years ago while under the delusion that I'd be building a game engine (hysterical laughter). Nonetheless, it's a solid multi-threaded utility which operates automatically on native 32 or 64-bit windows platforms. Unfortunately, I never did get around to adapting it to Unix environments, perhaps someone out there would like to?

	There are two main files:
	BrutePool.h (The functional memory pool)
	BruteMEM.cpp (The portion which stress tests the application)
	 
	You must first create a MManager instance. 
	Then initialize the memory by calling MManager::CreatePool()
	From there, you can allocate/deallocate to your hearts content using a methodology similar to c's malloc and free (Simply call mpalloc() & mpfree() respectively...)
