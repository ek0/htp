# Hook The Planet

Dynamic hooking framework for windows executables.

## Features

- Inline hooking using address or module name/function name
- Access the thread context through the HTPContext struct
- Thread safe
- Align stack before entering hook, allowing for raw hooking in the middle of function codes

## Caveats

- Doesn't take RIP relative instructions yet

## Building the SDK

This SDK is best used as a submodule along with Cmake. First fetch the project using:
```
git submodule add https://github.com/ek0/htp.git <path_to_submoule>
git submodule update --init --recursive
```

In your `CMakeLists.txt` add the following line:
```
add_subdirectory(<path_to_submodule>)
```

## Using the SDK

Here is a basic example hooking `LoadLibraryA` from kernel32:
```cpp
#include "htp.h"

HTPHandle* htp = nullptr;

// From htp.h
//
//struct HTPContext
//{
//#if _M_X64
//    uint64_t r15;
//    uint64_t r14;
//    uint64_t r13;
//    uint64_t r12;
//    uint64_t r11;
//    uint64_t r10;
//    uint64_t r9;
//    uint64_t r8;
//    uint64_t rsi;
//    uint64_t rdi;
//    uint64_t rbp;
//    uint64_t rdx;
//    uint64_t rcx;
//    uint64_t rbx;
//    uint64_t rax;
//    uint64_t rsp;
//#else
//    uint32_t edi;
//    uint32_t esi;
//    uint32_t ebp;
//    uint32_t esp;
//    uint32_t ebx;
//    uint32_t edx;
//    uint32_t ecx;
//    uint32_t eax;
//#endif
//};
void HookLoadLibraryA(HTPContext* ctx)
{
	char* module_name = reinterpret_cast<char*>(ctx->rcx);
	printf("Loading %s...\n", module_name);
}

int main(int argc, char** argv)
{
	htp = HTPInit();
	if(!SetupInlineHook(htp, "kernel32", "LoadLibraryA", HookLoadLibraryA))
	{
		// Handle Error;
	}
	HTPClose(htp);
	return 0;
}
```

An example of pre-hook (executed when entering the function) and post-hook (executed before exiting the function).
This is most likely how you will use the SDK from an injected DLL.
```cpp
#include "htp.h"

void PreHookRandomFunction(HTPContext* ctx)
{
	// Do something
}

void PostHookRandomFunction(HTPContext* ctx)
{
	// change return code
	ctx->rax = 0xDEADBEEF;
}

HTPHandle* htp = nullptr;

int main(int argc, char** argv)
{
	htp = HTPInit();
	// 0x1234567 is the fictional function RVA.
	if(!SetupInlineHook(htp, HTPGetImageBase(htp) + 0x1234567, PreHookRandomFunction, PostHookRandomFunction))
	{
		// Handle Error;
	}
	HTPClose(htp);
	return 0;
}
```

See `tests/test_hook.cpp` for more examples.

# Credits

Big thanks to @w4kfu
