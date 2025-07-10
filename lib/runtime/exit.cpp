#if defined(_WIN32) || defined(_WIN64)

extern "C" void* __stdcall GetProcAddress(void*, const char*);
extern "C" void* __stdcall LoadLibraryA(const char*);

extern "C" int _fltused = 0;
#endif

extern "C"
{
    void exit(int code)
    {
#if defined(_WIN32) || defined(_WIN64)
        void* kernel32 = LoadLibraryA("kernel32.dll");
        using ExitProcessFn = void(__stdcall*)(unsigned int);
        ExitProcessFn ExitProcess = (ExitProcessFn)GetProcAddress(kernel32, "ExitProcess");
        ExitProcess(code);
#elif defined(__linux__)
        asm volatile (
            "movl %0, %%edi\n" // Move 'code' to edi
            "movq $60, %%rax\n"
            "syscall\n" // Make the syscall
            :
            : "r"(code)
            : "rdi", "rax"
        );
        for (;;); // Infinite loop for if the syscall fails (extremely dangerous if a program keeps going after call to exit)

#elif defined(__APPLE__)
        asm volatile (
            "movq %0, %%rdi\n"    // Move exit code to rdi
            "movq $0x2000001, %%rax\n" // Syscall number for exit on macOS (0x2000000 + 1)
            "syscall\n"
            :
            : "r"(code)
            : "rdi", "rax"
        );
        for (;;);
#endif
    }
}