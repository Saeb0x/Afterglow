#if !defined(AG_STACKALLOCATOR_H)
#define AG_STACKALLOCATOR_H

#include "Assert.h"
#include "Types.h"
#include "Utility.h"

struct StackAllocator
{
	uint8* Base;
	uint8* Cap;
	uint8* LowerHeap;
	uint8* UpperHeap;
};

enum class StackAllocatorHeap : uint8
{
	Lower = 0,
	Upper
};

struct StackAllocatorFrame
{
	uint8* Mark;
	StackAllocatorHeap Heap;
};

inline bool StackAllocatorInit(StackAllocator* allocator, void* memory, usize size)
{
	AG_ASSERT(allocator);
	AG_ASSERT(size > 0);

	if(!memory)
	{
		AG_ASSERT(false);
		return (false);
	}

	allocator->Base = (uint8*)memory;
	allocator->Cap = allocator->Base + size;
	allocator->LowerHeap = allocator->Base;
	allocator->UpperHeap = allocator->Cap;

	return (true);
}

inline void StackAllocatorShutdown(StackAllocator* allocator)
{
	AG_ASSERT(allocator);

	allocator->Base = nullptr;
	allocator->Cap = nullptr;
	allocator->LowerHeap = nullptr;
	allocator->UpperHeap = nullptr;
}

inline void* StackAllocatorAllocate(StackAllocator* allocator, StackAllocatorHeap heap, usize size,
									usize alignment)
{
	AG_ASSERT(allocator);
	AG_ASSERT(size > 0);

	// NOTE(saeb): Alignment must be a non-zero power of two.
	AG_ASSERT(alignment > 0 && (alignment & (alignment - 1)) == 0);

	usize lower = (usize)allocator->LowerHeap;
	usize upper = (usize)allocator->UpperHeap;

	void* memory;
	if(heap == StackAllocatorHeap::Upper)
	{
		if(size > upper - lower)
		{
			AG_ASSERT(false);
			return (nullptr); // Out of memory
		}

		// NOTE(saeb): Growing down, so align down; aligning up would move back into memory already
		// handed out.
		usize aligned = AG_ALIGNDOWN(upper - size, alignment);
		if(aligned < lower)
		{
			AG_ASSERT(false);
			return (nullptr); // Alignment padding collides with lower heap
		}

		allocator->UpperHeap = (uint8*)aligned;
		memory = (void*)aligned;
	}
	else
	{
		usize aligned = AG_ALIGNUP(lower, alignment);
		if(aligned > upper || size > upper - aligned)
		{
			AG_ASSERT(false);
			return (nullptr); // Out of memory or collision
		}

		allocator->LowerHeap = (uint8*)(aligned + size);
		memory = (void*)aligned;
	}

	return (memory);
}

inline usize StackAllocatorGetAvailableMemory(StackAllocator* allocator)
{
	AG_ASSERT(allocator);
	return (allocator->UpperHeap - allocator->LowerHeap);
}

inline usize StackAllocatorGetUsedMemory(StackAllocator* allocator)
{
	AG_ASSERT(allocator);
	return ((allocator->LowerHeap - allocator->Base) + (allocator->Cap - allocator->UpperHeap));
}

inline usize StackAllocatorGetHeapUsedMemory(StackAllocator* allocator, StackAllocatorHeap heap)
{
	AG_ASSERT(allocator);
	return ((heap == StackAllocatorHeap::Upper) ? (usize)(allocator->Cap - allocator->UpperHeap)
												: (usize)(allocator->LowerHeap - allocator->Base));
}

inline StackAllocatorFrame StackAllocatorGetFrame(StackAllocator* allocator,
												  StackAllocatorHeap heap)
{
	AG_ASSERT(allocator);

	StackAllocatorFrame frame;
	frame.Heap = heap;
	frame.Mark = (heap == StackAllocatorHeap::Upper) ? allocator->UpperHeap : allocator->LowerHeap;

	return (frame);
}

inline void StackAllocatorReleaseFrame(StackAllocator* allocator, StackAllocatorFrame frame)
{
	AG_ASSERT(allocator);

	// NOTE(saeb): Frames must be released newest first; releasing an older frame and then a newer
	// one would move the heap back over memory that was already given back.
	if(frame.Heap == StackAllocatorHeap::Upper)
	{
		AG_ASSERT(frame.Mark >= allocator->UpperHeap && frame.Mark <= allocator->Cap);
		allocator->UpperHeap = frame.Mark;
	}
	else
	{
		AG_ASSERT(frame.Mark <= allocator->LowerHeap && frame.Mark >= allocator->Base);
		allocator->LowerHeap = frame.Mark;
	}
}

#endif