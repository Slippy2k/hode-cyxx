

GLOBAL gen1
GLOBAL unkLookupTable1
GLOBAL gen4
GLOBAL unkLookupTable4
GLOBAL gen5
GLOBAL unkLookupTable5

SECTION .bss

unkLookupTable1: resb 16
unkLookupTable4: resb 18
unkLookupTable5: resb 32

SECTION .data

SECTION .text

%macro ENTER 0
	push ebp
	push edi
	pushad
	mov ebp, esp
%endmacro

%macro LEAVE 0
	popad
	pop edi
	pop ebp
	ret
%endmacro

gen1:
	ENTER
	xor     ecx, ecx
loop1:
	test    cl, 1
	mov     eax, 8
	jz      short loc_4198E1
	test    cl, 2
	jz      short loc_4198D4
	mov     eax, 1
	jmp     short loc_419919
loc_4198D4:
	mov     eax, ecx
	and     al, 8
	neg     al
	sbb     eax, eax
	and     eax, 7
	jmp     short loc_419919
loc_4198E1:
	test    cl, 4
	jz      short loc_419903
	test    cl, 2
	jz      short loc_4198F2
	mov     eax, 3
	jmp     short loc_419919
loc_4198F2:
	mov     eax, ecx
	and     eax, 0FFh
	and     eax, 8
	or      al, 20h
	shr     eax, 3
	jmp     short loc_419919
loc_419903:
	test    cl, 2
	jz      short loc_41990F
	mov     eax, 2
	jmp     short loc_419919
loc_41990F:
	test    cl, 8
	jz      short loc_419919
	mov     eax, 6
loc_419919:
	mov     [unkLookupTable1 + ecx], al
	inc     ecx
	cmp     ecx, 15
	jbe     short loop1
	LEAVE

gen4:
	ENTER
	xor     esi, esi
loop4:
	mov     eax, 0AAAAAAABh
	mov     edi, 6
	mul     esi
	mov     ecx, edx
	mov     eax, esi
	xor     edx, edx
	div     edi
	shr     ecx, 2
	inc     edx
	cmp     edx, 3
	jbe     short loc_419A1C
	inc     edx
loc_419A1C:
	shl     cl, 3
	or      cl, dl
	mov     [unkLookupTable4 + esi], cl
	inc     esi
	cmp     esi, 18
	jb      short loop4
	LEAVE

gen5:
	ENTER
	xor     esi, esi
loop5:
	mov     eax, esi
	mov     ecx, esi
	and     eax, 18h
	and     ecx, 7
	jnz     short loc_419A42
	mov     ecx, 1
	jmp     short loc_419A54
loc_419A42:
	cmp     ecx, 4
	jnz     short loc_419A4E
	mov     ecx, 5
	jmp     short loc_419A53
loc_419A4E:
	cmp     ecx, 3
	jbe     short loc_419A54
loc_419A53:
	dec     ecx
loc_419A54:
	shr     eax, 3
	mov     dl, 6
	imul    dl
	add     al, cl
	dec     al
	mov     [unkLookupTable5 + esi], al
	inc     esi
	cmp     esi, 32
	jb      short loop5
	LEAVE
