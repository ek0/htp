.model FLAT, C
.code

public generic_ret_trampoline
generic_ret_trampoline:
pushad
mov ecx, esp
call $+5
pop ebx
mov eax, [ebx+5dh] ; PreHookProc
push ecx
call eax
; We know this our hook will restore ebx (non-volatile), 
; we can safely keep using it as an index
; Save return address and modify it
mov eax, [ebx+69h] ; SaveRetProcAddress
mov ecx, [esp+24h] ; ret address
push ecx
mov ecx, [ebx+71h] ; Handle
push ecx
call eax
; hijack the stack return
lea eax, [ebx+3ah]
mov dword ptr [esp+2ch], eax
add esp, 0ch
popad
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
jmp dword ptr [OriginalFunctionAddress]
push esp ; dummy push, to realign the stack
pushad
mov ecx, esp
call $+5
pop ebx
mov eax, [ebx+22h]
push ecx
call eax ; PostHookProc
; restore return address and modify it
mov eax, [ebx+2ah] ; RestoreHook
mov ecx, [ebx+2eh] ; Handle
push ecx
call eax
; Restoring the ret address
; esp+34 <- ret address
mov [esp+28h], eax
add esp, 8
popad
ret
PreHookProcAddress:
dd 41414141h
OriginalFunctionAddress:
dd 42424242h
PostHookProcAddress:
dd 43434343h
SaveRetProcAddress:
dd 44444444h
RestoreRetProcAddress:
dd 45454545h
Handle:
dd 46464646h

public generic_ret_trampoline_size
generic_ret_trampoline_size dd $-generic_ret_trampoline

public generic_trampoline
generic_trampoline:
pushad
mov ecx, esp
call $+5
pop ebx
mov eax, [ebx+25h]
push ecx
call eax
add esp, 4
popad
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
jmp dword ptr [OriginalFunctionAddress2]
HookProc:
dd 41414141h
OriginalFunctionAddress2:
dd 42424242h

public generic_trampoline_size
generic_trampoline_size dd $-generic_trampoline

END