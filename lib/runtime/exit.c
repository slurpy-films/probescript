#if defined(_WIN32) || defined(_WIN64)

void* __stdcall GetProcAddress(void*, const char*);
void* __stdcall LoadLibraryA(const char*);

int _fltused = 0;

void exit(int code)
{
    void* kernel32 = LoadLibraryA("kernel32.dll");
    if (!kernel32) {
        // Loop forever if kernel32 is not found
        for(;;);
    }
    void(__stdcall *ExitProcess)(unsigned int) = 
        (void(__stdcall*)(unsigned int))GetProcAddress(kernel32, "ExitProcess");
    if (!ExitProcess) {
        for(;;);
    }
    ExitProcess(code);
}

#elif defined(__linux__)

void exit(int code)
{
    asm volatile (
        "movl %0, %%edi\n"
        "movq $60, %%rax\n"
        "syscall\n"
        :
        : "r"(code)
        : "rdi", "rax"
    );
    for(;;);
}

#elif defined(__APPLE__)

void exit(int code)
{
    asm volatile (
        "movq %0, %%rdi\n"
        "movq $0x2000001, %%rax\n"
        "syscall\n"
        :
        : "r"(code)
        : "rdi", "rax"
    );
    for(;;);
}

#endif