

GLOBAL gen1
GLOBAL mstLookupTable1
GLOBAL gen2
GLOBAL mstLookupTable2
GLOBAL gen3
GLOBAL mstLookupTable3
GLOBAL gen4
GLOBAL mstLookupTable4
GLOBAL gen5
GLOBAL mstLookupTable5

SECTION .bss

mstLookupTable1: resb 16
mstLookupTable2: resb 12
mstLookupTable3: resb 40
mstLookupTable4: resb 18
mstLookupTable5: resb 32

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
	mov     [mstLookupTable1 + ecx], al
	inc     ecx
	cmp     ecx, 15
	jbe     short loop1
	LEAVE

gen2:
	ENTER
	LEAVE

gen3:
	ENTER
	mov     eax, 7
        mov     edi, mstLookupTable2
        push    esi
        lea     ecx, [mstLookupTable3+1]
        sub     edi, eax
loop3:
        mov     dl, [edi+eax]
        lea     esi, [eax-6]
        cmp     esi, 8
        mov     [ecx-1], dl
        jl      short loc_4199AF
        lea     esi, [eax-14]
loc_4199AF:
        mov     dl, [mstLookupTable2 + esi]
        lea     esi, [eax-8]
        test    esi, esi
        mov     [ecx], dl
        jge     short loc_4199C0
        mov     esi, eax
loc_4199C0:
        mov     dl, [mstLookupTable2 + esi]
        lea     esi, [eax-5]
        cmp     esi, 8
        mov     [ecx+1], dl
        jl      short loc_4199D4
        lea     esi, [eax-0Dh]
loc_4199D4:
        mov     dl, [mstLookupTable2 + esi]
        lea     esi, [eax-9]
        test    esi, esi
        mov     [ecx+2], dl
        jge     short loc_4199E7
        lea     esi, [eax-1]
loc_4199E7:
        mov     dl, [mstLookupTable2 + esi]
        inc     eax
        mov     [ecx+3], dl
        add     ecx, 5
        lea     edx, [eax-7]
        cmp     edx, 8
        jb      short loop3
        pop     esi
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
	mov     [mstLookupTable4 + esi], cl
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
	mov     [mstLookupTable5 + esi], al
	inc     esi
	cmp     esi, 32
	jb      short loop5
	LEAVE
