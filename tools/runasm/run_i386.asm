
GLOBAL test_eq_15
GLOBAL test_abs
GLOBAL test_0x1485F0E1
GLOBAL test_0xBA2E8BA3
GLOBAL test_negsbb
GLOBAL test_andnegsbb
GLOBAL test_mullongintlongint
GLOBAL test_cdq

SECTION .text

arg0	equ 8
arg1	equ 12
arg2    equ 16
arg3    equ 20

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

test_mullongintlongint:

   push ebp
   mov ebp, esp
   push edx
   push ebx
   push ecx

       mov edx, [ebp + arg0]
       mov eax, [ebp + arg1]
       mov ebx, [ebp + arg2]
       mov ecx, [ebp + arg3]

       shl eax, 16
       shrd eax, edx, 16
       shl ecx, 16
       shrd ecx, ebx, 16
       imul ecx
       shld edx, eax, 16

   pop ecx
   pop ebx
   pop edx
   pop ebp
   ret

# http://www.vnsecurity.net/research/2007/05/29/interesting-arithmetic-assembly-sequences.html
# Taking remainder of a division by a power of 2

test_cdq:

  push ebp
  mov ebp, esp
  push edx

    mov eax, [ebp + arg0]
    cdq
    and edx, 3
    add eax, edx
    sar eax, 2

  pop edx
  pop ebp
  ret
