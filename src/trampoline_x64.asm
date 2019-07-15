.code

public generic_ret_trampoline
generic_ret_trampoline:
push   rsp
push   rax
push   rbx
push   rcx
push   rdx
push   rbp
push   rdi
push   rsi
push   r8
push   r9
push   r10
push   r11
push   r12
push   r13
push   r14
push   r15
mov    rcx,rsp
sub    rsp, 28h
call   qword ptr [offset PreHookAddressProc]
mov    rdx, qword ptr [rsp+168] ; return address is at offset 0xa8
mov    rcx, qword ptr [offset HTPHandle]
call   qword ptr [offset SaveReturnAddressProc]
lea    rax, [offset posthooktrampoline]
mov    qword ptr [rsp+168],rax
add    rsp,28h
pop    r15
pop    r14
pop    r13
pop    r12
pop    r11
pop    r10
pop    r9
pop    r8
pop    rsi
pop    rdi
pop    rbp
pop    rdx
pop    rcx
pop    rbx
pop    rax
pop    rsp
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
jmp    qword ptr [offset OriginaProcAddress]
posthooktrampoline:
push   rsp
push   rsp
push   rax
push   rbx
push   rcx
push   rdx
push   rbp
push   rdi
push   rsi
push   r8
push   r9
push   r10
push   r11
push   r12
push   r13
push   r14
push   r15
mov    rcx,rsp
sub    rsp, 28h
call   qword ptr [offset PostHookProcAddress]
mov    rcx, qword ptr [offset HTPHandle]
call   qword ptr [offset RestoreReturnAddressProc]
xchg   qword ptr [rsp+168],rax
add    rsp, 28h
pop    r15
pop    r14
pop    r13
pop    r12
pop    r11
pop    r10
pop    r9
pop    r8
pop    rsi
pop    rdi
pop    rbp
pop    rdx
pop    rcx
pop    rbx
pop    rax
pop    rsp
ret
PreHookAddressProc:
dq 4646464646464646h
OriginaProcAddress:
dq 4545454545454545h
PostHookProcAddress:
dq 4444444444444444h
SaveReturnAddressProc:
dq 4141414141414141h
RestoreReturnAddressProc:
dq 4242424242424242h
HTPHandle:
dq 4343434343434343h

public generic_ret_trampoline_size 
generic_ret_trampoline_size dq $-generic_ret_trampoline

public generic_trampoline
generic_trampoline:
push    rsp
push    rax
push    rbx
push    rcx
push    rdx
push    rbp
push    rdi
push    rsi
push    r8
push    r9
push    r10
push    r11
push    r12
push    r13
push    r14
push    r15
mov     rcx, rsp
sub     rsp, 28h
call    qword ptr [offset HookProc]
add     rsp, 28h
pop     r15
pop     r14
pop     r13
pop     r12
pop     r11
pop     r10
pop     r9
pop     r8
pop     rsi
pop     rdi
pop     rbp
pop     rdx
pop     rcx
pop     rbx
pop     rax
pop     rsp
;place for instructions to restore
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
jmp     qword ptr [offset OriginalProcAddress]
HookProc            dq 4142434445464748h
OriginalProcAddress dq 4142434445464748h

public generic_trampoline_size
generic_trampoline_size dq $-generic_trampoline
END