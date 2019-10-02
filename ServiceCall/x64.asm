public __x64ServiceCall

.code
align 16

__x64ServiceCall proc frame;		NTSTATUS __x64ServiceCall(LPVOID lpServiceRoutine, PSERVICE_ROUTINE_PARAMETER lpParameters) {
.endprolog
; lpServiceRoutine in rcx
; lpParameters in rdx

; rbx save sizeof stack allocated
	push rbx;
	mov r10, rcx;
	mov eax, dword ptr[rdx];			DWORD dwStack = lpParameters->dwParameters;
	cmp rax, 4h;
	jnl _skip1;							if(dwStack < 4) dwStack = 4;
	mov rax, 4h;
_skip1:
	test rax, 1h;						if(dwStack % 2) dwStack++;
	jz _skip2;
	inc rax;
_skip2:
	mov rbx, rax;						DWORD dwStackAllocated = dwStack * 8;
	shl rbx, 3h;
	mov r8d, eax;
	sub r8d, dword ptr[rdx];
	cmp r8d, 0;							if(lpParameters->dwParameters < dwStack) rsp -= 8*(dwStack - lpParameters->dwParameters);
	je _skip3;
	shl r8, 3h;
	sub rsp, r8;
_skip3:
	mov eax, dword ptr[rdx];			DWORD dwIndex = lpParameters->dwParameters;
	lea r8d, dword ptr[eax - 1h];
	shl r8, 4h;
	add rdx, 8h;						PSERVICE_PARAMETER_BLOB param = lpParameters->Parameters + dwIndex;
	add rdx, r8;
_loop:
	cmp eax, 0h;						while(dwIndex) {
	je _break;
	push qword ptr[rdx + 8h];				__asm push param->ulParameter;
	sub rdx, 10h;
	dec eax;								dwIndex--;
	jmp _loop;							}
_break:
	mov rcx, qword ptr[rsp];
	mov rdx, qword ptr[rsp + 8h];
	mov r8, qword ptr[rsp + 10h];
	mov r9, qword ptr[rsp + 18h];
	call r10;							return reinterpret_cast<NTSTATUS (*)()>(lpServiceRoutine)();
	add rsp, rbx;
	pop rbx;
	ret;							}
__x64ServiceCall endp


end