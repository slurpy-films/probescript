typedef unsigned int uint32_t;
typedef unsigned long size_t;

#ifdef _WIN32
__declspec(dllimport) void* __stdcall VirtualAlloc(void* lpAddress, unsigned long dwSize, unsigned long flAllocationType, unsigned long flProtect);
__declspec(dllimport) int __stdcall VirtualFree(void* lpAddress, unsigned long dwSize, unsigned long dwFreeType);

#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000
#define PAGE_READWRITE 0x04

typedef unsigned long size_t;

void* os_alloc(size_t size)
{
    return VirtualAlloc(0, (unsigned long)size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void os_free(void* ptr, size_t size)
{
    (void)size;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

#else

typedef unsigned long size_t;

extern long sysconf(int name);
extern int munmap(void *addr, size_t length);
extern void* mmap(void *addr, size_t length, int prot, int flags, int fd, long offset);

#define _SC_PAGESIZE 30
#define MAP_PRIVATE 2
#define MAP_ANONYMOUS 0x20
#define PROT_READ 1
#define PROT_WRITE 2

void* os_alloc(size_t size)
{
    long pageSize = sysconf(_SC_PAGESIZE);
    size_t alignedSize = ((size + pageSize - 1) / pageSize) * pageSize;
    void* ptr = mmap(0, alignedSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ((intptr_t)ptr < 0) return 0;
    return ptr;
}

void os_free(void* ptr, size_t size)
{
    long pageSize = sysconf(_SC_PAGESIZE);
    size_t alignedSize = ((size + pageSize - 1) / pageSize) * pageSize;
    munmap(ptr, alignedSize);
}

#endif

typedef void (*rc_free_func_t)(void* payload);

typedef struct {
    uint32_t refcount;
    rc_free_func_t destructor;
    size_t size;
} RCHeader;

typedef void* (*alloc_func_t)(size_t size);
typedef void  (*free_func_t)(void* ptr, size_t size);

typedef struct
{
    alloc_func_t alloc;
    free_func_t free;
} Allocator;

Allocator globalAllocator = {
    .alloc = os_alloc,
    .free = os_free
};

void* rc_alloc(Allocator* allocator, size_t size, rc_free_func_t destructor)
{
    size_t totalSize = sizeof(RCHeader) + size;
    void* raw = allocator->alloc(totalSize);
    if (!raw) return 0;

    RCHeader* header = (RCHeader*)raw;
    header->refcount = 1;
    header->destructor = destructor;
    header->size = totalSize;

    return (void*)(header + 1);
}

void rc_inc(void* payload)
{
    if (!payload) return;
    RCHeader* header = ((RCHeader*)payload) - 1;
    header->refcount += 1;
}

void rc_dec(Allocator* allocator, void* payload)
{
    if (!payload) return;
    RCHeader* header = ((RCHeader*)payload) - 1;
    if (--header->refcount == 0)
    {
        if (header->destructor)
            header->destructor(payload);

        allocator->free(header, header->size);
    }
}