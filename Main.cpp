// MemoryPool.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include "Memory/Allocation.h"

/**
*   \brief Execute a test on several pool's functionalities.
*/
bool memoryStandardUnitTest(MemoryPool* pool, bool dumpMemoryStates)
{
    // If bounds check is on, we need to alter some math here and there
    bool hasBoundsCheckOn = pool->hasBoundsCheckOn();
    BYTE poolSizeOffset = hasBoundsCheckOn ? 48 : 16;

    // Trash tests -------------------------------------------------------------------------
    // Check trash on creation
    BYTE* ptr = (BYTE*)pool->allocate(1);
    // I read outside the allocated chunk to check the memory creation trashing
    if( *(ptr+100) != pool->s_trashOnCreation ) return false;

    // Check trash on alloc
    if( *ptr != pool->s_trashOnAllocSignature ) return false;

    // Check trash on Free
    pool->free( ptr );
    if( *ptr != pool->s_trashOnFreeSignature ) return false;

    // Full allocation test ----------------------------------------------------------------
    ptr = (BYTE*)pool->allocate(1024);

    // Not enough memory
    if( ptr != NULL ) return false;

    // Now should be fine
    ptr = (BYTE*)pool->allocate(1024 - 16 * 6);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;
    pool->free(ptr);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;

    // Integrity tests ---------------------------------------------------------------------
    // Allocate some memory
    BYTE* block[4];
    block[0] = (BYTE*)pool->allocate(140);
    block[1] = (BYTE*)pool->allocate( 70);
    block[2] = (BYTE*)pool->allocate( 16);
    block[3] = (BYTE*)pool->allocate(  1);

    if(hasBoundsCheckOn)
    {
        // Copy several bytes around
        memset(block[0], 65, 140);
        memset(block[1], 67, 70);
        memset(block[2], 69, 8);
        memset(block[2] + 8, 70, 8);
        *block[3] = 'A';

        // This copy is fine, lets check this
        if( pool->integrityCheck() != true ) return false;

        // Copy some bytes around stomping over a block
        memset(block[0], 65, 141);

        // This copy is wrong, integrity check must detect this
        if( pool->integrityCheck() != false ) return false;

        //Lets fix the memory
        memset(block[0] + 140, '[', 1);

        // This copy is fine, lets check this
        if( pool->integrityCheck() != true ) return false;
    }

    if(dumpMemoryStates)
        MemoryPoolManager::it().dumpPool("severalAllocations_", pool);

    pool->free(block[0]);
    pool->free(block[1]);
    pool->free(block[2]);
    pool->free(block[3]);

    // This copy is fine, lets check this
    if( pool->getFreePoolSize() != pool->getTotalPoolSize() - poolSizeOffset ) return false;

    // Sparse allocation deallocation --------------------------------------------------------
    // Allocate some memory
    block[0] = (BYTE*)pool->allocate(512);
    block[1] = (BYTE*)pool->allocate(128);
    block[2] = (BYTE*)pool->allocate( 64);
    block[3] = (BYTE*)pool->allocate(  8);

    // This copy is fine, lets check this
    if( pool->integrityCheck() != true ) return false;

    pool->free(block[0]);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;
    pool->free(block[2]);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;
    pool->free(block[3]);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;
    pool->free(block[1]);
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;

    block[0] = (BYTE*)pool->allocate(128);
    block[1] = (BYTE*)pool->allocate(128);
    block[2] = (BYTE*)pool->allocate(128);
    pool->free(block[1]);
    block[1] = (BYTE*)pool->allocate(64);
    block[3] = (BYTE*)pool->allocate(64);
    pool->free(block[2]);
    block[2] = (BYTE*)pool->allocate(150);

    if(dumpMemoryStates)
        MemoryPoolManager::it().dumpPool("sparseAllocations_", pool);

    pool->free(block[0]);
    pool->free(block[1]);
    pool->free(block[2]);
    pool->free(block[3]);

    // This copy is fine, lets check this
    if( hasBoundsCheckOn && pool->integrityCheck() != true ) return false;

    if( pool->getFreePoolSize() != pool->getTotalPoolSize() - poolSizeOffset ) return false;

    if(dumpMemoryStates)
        MemoryPoolManager::it().dumpPool("allFree_", pool);

    return true;
}

// Entry point
int _tmain(int argc, _TCHAR* argv[])
{
    bool dumpMemoryStates = true;

    if(!memoryStandardUnitTest( POOL("TestPool"), dumpMemoryStates ) )              printf("Memory test on TestPool failed\n");
    else                                                                            printf("Memory test on TestPool succeeded\n");
    if(!memoryStandardUnitTest( POOL("BoundedTestPool"), dumpMemoryStates ) )       printf("Memory test on BoundedTestPool failed\n");
    else                                                                            printf("Memory test on BoundedTestPool succeeded\n");
    
    _getch();

	return 0;
}

