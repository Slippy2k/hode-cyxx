
GLOBAL test_eq_15
GLOBAL test_abs
GLOBAL test_0x1485F0E1
GLOBAL test_0xBA2E8BA3
GLOBAL test_negsbb
GLOBAL test_andnegsbb

SECTION .text

arg0	equ 8
arg1	equ 12

test_eq_15:

    push ebp
    mov ebp, esp

	mov     edx, [ebp + arg0]
	mov     cl, 15
	cmp     cl, dl
	sbb     dl, dl
	and     dl, ~1
	mov     eax, edx

    pop ebp
    ret

test_abs:

    push ebp
    mov ebp, esp

	mov eax, [ebp + arg0]
	cdq
	xor eax, edx
	sub eax, edx

    pop ebp
    ret

test_0x1485F0E1:

    push ebp
    mov ebp, esp
    push edx
    push ecx

   	mov edx, [ebp + arg1] ; base_ptr
	mov ecx, [ebp + arg0] ; ptr

	mov     eax, 1485F0E1h
	sub     ecx, edx
	mul     ecx
	sub     ecx, edx
	shr     ecx, 1
	add     ecx, edx
	shr     ecx, 9
	mov     eax, ecx

    pop ecx
    pop edx
    pop ebp
    ret

test_0xBA2E8BA3:

    push ebp
    mov ebp, esp
    push edx
    push ecx

	mov edx, [ebp + arg1] ; base_ptr
	mov ecx, [ebp + arg0] ; ptr

	sub     ecx, edx
	mov     eax, 0BA2E8BA3h
	mul     ecx
	shr     edx, 5
	mov     eax, edx

    pop ecx
    pop edx
    pop ebp
    ret

test_negsbb:

    push ebp
    mov ebp, esp

	mov eax, [ebp + arg0]
	neg al
	sbb eax, eax

    pop ebp
    ret

test_andnegsbb:

    push ebp
    mov ebp, esp

	mov eax, [ebp + arg0]
	and eax, 2
	neg al
	sbb eax, eax
	and eax, -1024
	add eax, 512

   pop ebp
   ret

