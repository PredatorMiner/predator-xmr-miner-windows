OPTION CASEMAP:NONE

IFDEF RAX
RANDOMX_SCRATCHPAD_MASK   EQU 2097088t
RANDOMX_DATASET_BASE_MASK EQU 2147483584t
RANDOMX_CACHE_MASK        EQU 4194303t

ROUND_AVX2 MACRO s0,s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11,s12,s13,s14,s15
    vmovq    xmm6,  QWORD PTR [rdx+s0*8]
    vmovq    xmm7,  QWORD PTR [rdx+s1*8]
    vpinsrq  xmm6,  xmm6, QWORD PTR [rdx+s2*8], 1
    vpinsrq  xmm7,  xmm7, QWORD PTR [rdx+s3*8], 1
    vmovq    xmm8,  QWORD PTR [rdx+s4*8]
    vmovq    xmm11, QWORD PTR [rdx+s5*8]
    vpinsrq  xmm8,  xmm8,  QWORD PTR [rdx+s6*8], 1
    vpinsrq  xmm11, xmm11, QWORD PTR [rdx+s7*8], 1
    vinserti128 ymm6, ymm6, xmm8,  1
    vinserti128 ymm7, ymm7, xmm11, 1
    vpaddq   ymm0, ymm0, ymm1
    vpaddq   ymm0, ymm0, ymm6
    vpxor    ymm3, ymm3, ymm0
    vpshufd  ymm3, ymm3, 10110001b
    vpaddq   ymm2, ymm2, ymm3
    vpxor    ymm1, ymm1, ymm2
    vpshufb  ymm1, ymm1, ymm4
    vpaddq   ymm0, ymm0, ymm1
    vpaddq   ymm0, ymm0, ymm7
    vpxor    ymm3, ymm3, ymm0
    vpshufb  ymm3, ymm3, ymm5
    vpaddq   ymm2, ymm2, ymm3
    vpxor    ymm1, ymm1, ymm2
    vpsrlq   ymm8, ymm1, 63
    vpaddq   ymm1, ymm1, ymm1
    vpxor    ymm1, ymm1, ymm8
    vpermq   ymm1, ymm1, 00111001b
    vpermq   ymm2, ymm2, 01001110b
    vpermq   ymm3, ymm3, 10010011b
    vmovq    xmm6,  QWORD PTR [rdx+s8*8]
    vmovq    xmm7,  QWORD PTR [rdx+s9*8]
    vpinsrq  xmm6,  xmm6, QWORD PTR [rdx+s10*8], 1
    vpinsrq  xmm7,  xmm7, QWORD PTR [rdx+s11*8], 1
    vmovq    xmm8,  QWORD PTR [rdx+s12*8]
    vmovq    xmm11, QWORD PTR [rdx+s13*8]
    vpinsrq  xmm8,  xmm8,  QWORD PTR [rdx+s14*8], 1
    vpinsrq  xmm11, xmm11, QWORD PTR [rdx+s15*8], 1
    vinserti128 ymm6, ymm6, xmm8,  1
    vinserti128 ymm7, ymm7, xmm11, 1
    vpaddq   ymm0, ymm0, ymm1
    vpaddq   ymm0, ymm0, ymm6
    vpxor    ymm3, ymm3, ymm0
    vpshufd  ymm3, ymm3, 10110001b
    vpaddq   ymm2, ymm2, ymm3
    vpxor    ymm1, ymm1, ymm2
    vpshufb  ymm1, ymm1, ymm4
    vpaddq   ymm0, ymm0, ymm1
    vpaddq   ymm0, ymm0, ymm7
    vpxor    ymm3, ymm3, ymm0
    vpshufb  ymm3, ymm3, ymm5
    vpaddq   ymm2, ymm2, ymm3
    vpxor    ymm1, ymm1, ymm2
    vpsrlq   ymm8, ymm1, 63
    vpaddq   ymm1, ymm1, ymm1
    vpxor    ymm1, ymm1, ymm8
    vpermq   ymm1, ymm1, 10010011b
    vpermq   ymm2, ymm2, 01001110b
    vpermq   ymm3, ymm3, 00111001b
ENDM

_RANDOMX_CONST SEGMENT PAGE READ 'CONST'

ALIGN 64
AES_HASH_1R_STATE0 DD 092b52c0dh, 09fa856deh, 0cc82db47h, 0d7983aadh
AES_HASH_1R_STATE1 DD 0338d996eh, 015c7b798h, 0f59e125ah, 0ace78057h
AES_HASH_1R_STATE2 DD 06a770017h, 0ae62c7d0h, 05079506bh, 0e8a07ce4h
AES_HASH_1R_STATE3 DD 0630a240ch, 007ad828dh, 079a10005h, 07e994948h
AES_HASH_1R_XKEY0  DD 0f6fa8389h, 08b24949fh, 090dc56bfh, 006890201h
AES_HASH_1R_XKEY1  DD 061b263d1h, 051f4e03ch, 0ee1043c6h, 0ed18f99bh
AES_GEN_1R_KEY0    DD 06daca553h, 062716609h, 0dbb5552bh, 0b4f44917h
AES_GEN_1R_KEY1    DD 06d7caf07h, 0846a710dh, 01725d378h, 00da1dc4eh
AES_GEN_1R_KEY2    DD 03f1262f1h, 09f947ec6h, 0f4c0794fh, 03e20e345h
AES_GEN_1R_KEY3    DD 06aef8135h, 0b1ba317ch, 016314c88h, 049169154h
AES_GEN_4R_KEY0    DD 06421aaddh, 0d1833ddbh, 02f546d2bh, 099e5d23fh
AES_GEN_4R_KEY1    DD 0b20e3450h, 0b6913f55h, 006f79d53h, 0a5dfcde5h
AES_GEN_4R_KEY2    DD 05c3ed904h, 0515e7bafh, 00aa4679fh, 0171c02bfh
AES_GEN_4R_KEY3    DD 085623763h, 0e78f5d08h, 0cd673785h, 0d8ded291h
AES_GEN_4R_KEY4    DD 0b5826f73h, 0e3d6a7a6h, 03d518b6dh, 0229effb4h
AES_GEN_4R_KEY5    DD 0c7566bf3h, 09c10b3d9h, 0e9024d4eh, 0b272b7d2h
AES_GEN_4R_KEY6    DD 0f273c9e7h, 0f765a38bh, 02ba9660ah, 0f63befa7h
AES_GEN_4R_KEY7    DD 07a7cd609h, 0915839deh, 00c06d1fdh, 0c0b0762dh

ALIGN 64
b2_iv_lo DQ 06a09e667f3bcc908h, 0bb67ae8584caa73bh, 03c6ef372fe94f82bh, 0a54ff53a5f1d36f1h
b2_iv_hi DQ 0510e527fade682d1h, 09b05688c2b3e6c1fh, 01f83d9abfb41bd6bh, 05be0cd19137e2179h

ALIGN 64
b2_iv_hi_fin DQ 0510e527fade682d1h, 09b05688c2b3e6c1fh
             DQ 0e07c265404be4294h, 05be0cd19137e2179h

ALIGN 32
b2_t0_128    DQ 128, 0, 0, 0

ALIGN 32
b2_rot24_mask DB 3,4,5,6,7,0,1,2, 11,12,13,14,15,8,9,10
              DB 3,4,5,6,7,0,1,2, 11,12,13,14,15,8,9,10

ALIGN 32
b2_rot16_mask DB 2,3,4,5,6,7,0,1, 10,11,12,13,14,15,8,9
              DB 2,3,4,5,6,7,0,1, 10,11,12,13,14,15,8,9

_RANDOMX_CONST ENDS
_RANDOMX_TEXT SEGMENT PAGE READ EXECUTE

PUBLIC randomx_prefetch_scratchpad
PUBLIC randomx_program_prologue
PUBLIC randomx_program_loop_load
PUBLIC randomx_program_read_dataset
PUBLIC randomx_dataset_init
PUBLIC randomx_program_epilogue
PUBLIC randomx_sshash_load
PUBLIC randomx_sshash_prefetch
PUBLIC randomx_sshash_init
PUBLIC fillAes1Rx4_hw
PUBLIC fillAes4Rx4_hw
PUBLIC hashAndFillAes1Rx4_hw
PUBLIC blake2b_compress_mid
PUBLIC blake2b_compress_fin

ALIGN 64
randomx_program_prologue PROC
    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15
    sub rsp, 184
    mov qword ptr [rsp+176], rcx
    vmovapd xmmword ptr [rsp+144], xmm6
    vmovapd xmmword ptr [rsp+128], xmm7
    vmovapd xmmword ptr [rsp+112], xmm8
    vmovapd xmmword ptr [rsp+96],  xmm9
    vmovapd xmmword ptr [rsp+80],  xmm10
    vmovapd xmmword ptr [rsp+64],  xmm11
    vmovapd xmmword ptr [rsp+48],  xmm12
    vmovapd xmmword ptr [rsp+32],  xmm13
    vmovapd xmmword ptr [rsp+16],  xmm14
    vmovapd xmmword ptr [rsp],   xmm15
    mov rbp, qword ptr [rcx+2560]
    mov rdi, qword ptr [rcx+2568]
    mov rsi, qword ptr [rcx+2576]
    mov rbx, 2048
    mov rax, rbp
    ror rbp, 32
    xor r8,  r8
    xor r9,  r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15
    vmovapd xmm8,  xmmword ptr [rcx+2368]
    vmovapd xmm9,  xmmword ptr [rcx+2384]
    vmovapd xmm10, xmmword ptr [rcx+2400]
    vmovapd xmm11, xmmword ptr [rcx+2416]
    vmovapd xmm13, xmmword ptr [mantissaMask]
    vmovapd xmm14, xmmword ptr [rcx+2432]
    vmovapd xmm15, xmmword ptr [scaleMask]
    rorx rdx,rax,32
    and eax, RANDOMX_SCRATCHPAD_MASK
    and edx, RANDOMX_SCRATCHPAD_MASK
    jmp NEAR PTR rx_program_loop_begin
randomx_program_prologue ENDP

ALIGN 64
mantissaMask:
    db 255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 0
scaleMask:
    db 0, 0, 0, 0, 0, 0, 240, 128, 0, 0, 0, 0, 0, 0, 240, 128

ALIGN 64
rx_program_loop_begin LABEL BYTE

randomx_program_loop_load PROC
    vcvtdq2pd xmm0, qword ptr [rsi+rdx]
    xor r8,  qword ptr [rsi+rax]
    vcvtdq2pd xmm1, qword ptr [rsi+rdx+8]
    xor r9,  qword ptr [rsi+rax+8]
    vcvtdq2pd xmm2, qword ptr [rsi+rdx+16]
    xor r10, qword ptr [rsi+rax+16]
    vcvtdq2pd xmm3, qword ptr [rsi+rdx+24]
    xor r11, qword ptr [rsi+rax+24]
    vcvtdq2pd xmm4, qword ptr [rsi+rdx+32]
    xor r12, qword ptr [rsi+rax+32]
    vcvtdq2pd xmm5, qword ptr [rsi+rdx+40]
    xor r13, qword ptr [rsi+rax+40]
    vcvtdq2pd xmm6, qword ptr [rsi+rdx+48]
    xor r14, qword ptr [rsi+rax+48]
    vcvtdq2pd xmm7, qword ptr [rsi+rdx+56]
    xor r15, qword ptr [rsi+rax+56]
    vandpd xmm4, xmm4, xmm13
    vorpd  xmm4, xmm4, xmm14
    vandpd xmm5, xmm5, xmm13
    vorpd  xmm5, xmm5, xmm14
    vandpd xmm6, xmm6, xmm13
    vorpd  xmm6, xmm6, xmm14
    vandpd xmm7, xmm7, xmm13
    vorpd  xmm7, xmm7, xmm14
    lea rcx, [rsi+rax]
    mov qword ptr [rsp+168], rcx
    lea rcx, [rsi+rdx]
    mov qword ptr [rsp+160], rcx
randomx_program_loop_load ENDP

randomx_program_read_dataset PROC
    mov ecx, ebp
    and ecx, RANDOMX_DATASET_BASE_MASK
    ror rbp, 32
    xor rbp, rax
    mov edx, ebp
    and edx, RANDOMX_DATASET_BASE_MASK
    prefetchnta [rdi+rdx]
    xor r8,  qword ptr [rdi+rcx]
    xor r9,  qword ptr [rdi+rcx+8]
    xor r10, qword ptr [rdi+rcx+16]
    xor r11, qword ptr [rdi+rcx+24]
    xor r12, qword ptr [rdi+rcx+32]
    xor r13, qword ptr [rdi+rcx+40]
    xor r14, qword ptr [rdi+rcx+48]
    xor r15, qword ptr [rdi+rcx+56]
randomx_program_read_dataset ENDP

randomx_prefetch_scratchpad PROC
    rorx rdx,rax,32
    and eax, RANDOMX_SCRATCHPAD_MASK
    and edx, RANDOMX_SCRATCHPAD_MASK
    prefetcht0 [rsi+rax]
    prefetcht0 [rsi+rdx]
randomx_prefetch_scratchpad ENDP

randomx_program_loop_store PROC
    mov rcx, qword ptr [rsp+160]
    mov qword ptr [rcx],  r8
    mov qword ptr [rcx+8],  r9
    mov qword ptr [rcx+16], r10
    mov qword ptr [rcx+24], r11
    mov qword ptr [rcx+32], r12
    mov qword ptr [rcx+40], r13
    mov qword ptr [rcx+48], r14
    mov qword ptr [rcx+56], r15
    mov rcx, qword ptr [rsp+168]
    vxorpd xmm0, xmm0, xmm4
    vxorpd xmm1, xmm1, xmm5
    vxorpd xmm2, xmm2, xmm6
    vxorpd xmm3, xmm3, xmm7
    vmovapd xmmword ptr [rcx],  xmm0
    vmovapd xmmword ptr [rcx+16], xmm1
    vmovapd xmmword ptr [rcx+32], xmm2
    vmovapd xmmword ptr [rcx+48], xmm3
randomx_program_loop_store ENDP

ALIGN 64
randomx_dataset_init PROC
    push rbx
    push rbp
    push rdi
    push rsi
    push r12
    push r13
    push r14
    push r15
    mov rdi, qword ptr [rcx]
    mov rsi, rdx
    mov rbp, r8
    mov rcx, r9
init_block_loop:
    mov rbx, rbp
    db 232
    dd 12288 - distance
    distance equ $ - offset randomx_dataset_init
    movnti qword ptr [rsi],  r8
    movnti qword ptr [rsi+8],  r9
    movnti qword ptr [rsi+16], r10
    movnti qword ptr [rsi+24], r11
    movnti qword ptr [rsi+32], r12
    movnti qword ptr [rsi+40], r13
    movnti qword ptr [rsi+48], r14
    movnti qword ptr [rsi+56], r15
    inc rbp
    add rsi, 64
    cmp rbp, rcx
    jb init_block_loop
    sfence
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx
    ret
randomx_dataset_init ENDP

ALIGN 64
randomx_program_epilogue PROC
    mov rcx, qword ptr [rsp+176]
    mov qword ptr [rcx+2176],  r8
    mov qword ptr [rcx+2184],  r9
    mov qword ptr [rcx+2192], r10
    mov qword ptr [rcx+2200], r11
    mov qword ptr [rcx+2208], r12
    mov qword ptr [rcx+2216], r13
    mov qword ptr [rcx+2224], r14
    mov qword ptr [rcx+2232], r15
    vmovapd xmmword ptr [rcx+2240],  xmm0
    vmovapd xmmword ptr [rcx+2256],  xmm1
    vmovapd xmmword ptr [rcx+2272],  xmm2
    vmovapd xmmword ptr [rcx+2288], xmm3
    vmovapd xmmword ptr [rcx+2304], xmm4
    vmovapd xmmword ptr [rcx+2320], xmm5
    vmovapd xmmword ptr [rcx+2336], xmm6
    vmovapd xmmword ptr [rcx+2352], xmm7
    vmovapd xmm6,  xmmword ptr [rsp+144]
    vmovapd xmm7,  xmmword ptr [rsp+128]
    vmovapd xmm8,  xmmword ptr [rsp+112]
    vmovapd xmm9,  xmmword ptr [rsp+96]
    vmovapd xmm10, xmmword ptr [rsp+80]
    vmovapd xmm11, xmmword ptr [rsp+64]
    vmovapd xmm12, xmmword ptr [rsp+48]
    vmovapd xmm13, xmmword ptr [rsp+32]
    vmovapd xmm14, xmmword ptr [rsp+16]
    vmovapd xmm15, xmmword ptr [rsp]
    add rsp, 184
    pop r15
    pop r14
    pop r13
    pop r12
    pop rsi
    pop rdi
    pop rbp
    pop rbx
    ret
randomx_program_epilogue ENDP

ALIGN 64
randomx_sshash_load PROC
    xor r8,  qword ptr [rbx]
    xor r9,  qword ptr [rbx+8]
    xor r10, qword ptr [rbx+16]
    xor r11, qword ptr [rbx+24]
    xor r12, qword ptr [rbx+32]
    xor r13, qword ptr [rbx+40]
    xor r14, qword ptr [rbx+48]
    xor r15, qword ptr [rbx+56]
randomx_sshash_load ENDP

randomx_sshash_prefetch PROC
    and rbx, RANDOMX_CACHE_MASK
    shl rbx, 6
    add rbx, rdi
    prefetchnta [rbx]
randomx_sshash_prefetch ENDP

ALIGN 64
randomx_sshash_init PROC
    lea r8, [rbx+1]
    and rbx, RANDOMX_CACHE_MASK
    shl rbx, 6
    add rbx, rdi
    prefetchnta [rbx]
    mov  rax, 5851f42d4c957f2dh
    imul r8,  rax
    mov  r9,  0810a978a59f5a1fch
    mov  r10, 0a77099df38c2d846h
    mov  r11, 08126b91cbf22495ch
    mov  r12, 0494d2597179f8a62h
    mov  r13, 09237efb9ceaaec0ch
    mov  r14, 02f2a56746ce62d78h
    mov  r15, 084853bf7b62ce54eh
    xor  r9,  r8
    xor  r10, r8
    xor  r11, r8
    xor  r12, r8
    xor  r13, r8
    xor  r14, r8
    xor  r15, r8
    jmp  NEAR PTR rx_program_end
randomx_sshash_init ENDP

ALIGN 64
rx_program_end LABEL BYTE

ALIGN 64
fillAes1Rx4_hw PROC
    sub rsp, 40
    vmovaps XMMWORD PTR [rsp],    xmm6
    vmovaps XMMWORD PTR [rsp+16], xmm7
    mov rdx, QWORD PTR [rcx+2576]
    add rcx, 2496
    vmovaps xmm0, XMMWORD PTR [rcx]
    vmovaps xmm1, XMMWORD PTR [rcx+16]
    vmovaps xmm2, XMMWORD PTR [rcx+32]
    vmovaps xmm3, XMMWORD PTR [rcx+48]
    vmovaps xmm4, XMMWORD PTR [AES_GEN_1R_KEY0]
    vmovaps xmm5, XMMWORD PTR [AES_GEN_1R_KEY1]
    vmovaps xmm6, XMMWORD PTR [AES_GEN_1R_KEY2]
    vmovaps xmm7, XMMWORD PTR [AES_GEN_1R_KEY3]
    mov r8, rdx
    lea rdx, [rdx+2097152]
    ALIGN 64
f1_loop_start:
    prefetchw [r8+512]
    prefetchw [r8+640]
    vaesdec xmm0, xmm0, xmm4
    vaesenc xmm1, xmm1, xmm5
    vaesdec xmm2, xmm2, xmm6
    vaesenc xmm3, xmm3, xmm7
    vmovaps XMMWORD PTR [r8],    xmm0
    vmovaps XMMWORD PTR [r8+16], xmm1
    vmovaps XMMWORD PTR [r8+32], xmm2
    vmovaps XMMWORD PTR [r8+48], xmm3
    vaesdec xmm0, xmm0, xmm4
    vaesenc xmm1, xmm1, xmm5
    vaesdec xmm2, xmm2, xmm6
    vaesenc xmm3, xmm3, xmm7
    vmovaps XMMWORD PTR [r8+64],  xmm0
    vmovaps XMMWORD PTR [r8+80],  xmm1
    vmovaps XMMWORD PTR [r8+96],  xmm2
    vmovaps XMMWORD PTR [r8+112], xmm3
    add r8, 128
    cmp r8, rdx
    jb f1_loop_start
    vmovaps XMMWORD PTR [rcx],    xmm0
    vmovaps XMMWORD PTR [rcx+16], xmm1
    vmovaps XMMWORD PTR [rcx+32], xmm2
    vmovaps XMMWORD PTR [rcx+48], xmm3
    vmovaps xmm6, XMMWORD PTR [rsp]
    vmovaps xmm7, XMMWORD PTR [rsp+16]
    add rsp, 40
    ret
fillAes1Rx4_hw ENDP

ALIGN 64
fillAes4Rx4_hw PROC
    sub rsp, 104
    vmovaps XMMWORD PTR [rsp],    xmm6
    vmovaps XMMWORD PTR [rsp+16], xmm7
    vmovaps XMMWORD PTR [rsp+32], xmm8
    vmovaps XMMWORD PTR [rsp+48], xmm9
    vmovaps XMMWORD PTR [rsp+64], xmm10
    vmovaps XMMWORD PTR [rsp+80], xmm11
    vmovaps xmm4,  XMMWORD PTR [AES_GEN_4R_KEY0]
    vmovaps xmm5,  XMMWORD PTR [AES_GEN_4R_KEY1]
    vmovaps xmm6,  XMMWORD PTR [AES_GEN_4R_KEY2]
    vmovaps xmm7,  XMMWORD PTR [AES_GEN_4R_KEY3]
    vmovaps xmm8,  XMMWORD PTR [AES_GEN_4R_KEY4]
    vmovaps xmm9,  XMMWORD PTR [AES_GEN_4R_KEY5]
    vmovaps xmm10, XMMWORD PTR [AES_GEN_4R_KEY6]
    vmovaps xmm11, XMMWORD PTR [AES_GEN_4R_KEY7]
    mov r8, rcx
    lea rdx, [rcx+2176]
    add rcx, 2496
    vmovaps xmm0, XMMWORD PTR [rcx]
    vmovaps xmm1, XMMWORD PTR [rcx+16]
    vmovaps xmm2, XMMWORD PTR [rcx+32]
    vmovaps xmm3, XMMWORD PTR [rcx+48]
    ALIGN 64
f4_loop_start:
    vaesdec  xmm0, xmm0, xmm4
    vaesenc  xmm1, xmm1, xmm4
    vaesdec  xmm2, xmm2, xmm8
    vaesenc  xmm3, xmm3, xmm8
    vaesdec  xmm0, xmm0, xmm5
    vaesenc  xmm1, xmm1, xmm5
    vaesdec  xmm2, xmm2, xmm9
    vaesenc  xmm3, xmm3, xmm9
    vaesdec  xmm0, xmm0, xmm6
    vaesenc  xmm1, xmm1, xmm6
    vaesdec  xmm2, xmm2, xmm10
    vaesenc  xmm3, xmm3, xmm10
    vaesdec  xmm0, xmm0, xmm7
    vaesenc  xmm1, xmm1, xmm7
    vaesdec  xmm2, xmm2, xmm11
    vaesenc  xmm3, xmm3, xmm11
    vmovaps  XMMWORD PTR [r8],    xmm0
    vmovaps  XMMWORD PTR [r8+16], xmm1
    vmovaps  XMMWORD PTR [r8+32], xmm2
    vmovaps  XMMWORD PTR [r8+48], xmm3
    vaesdec  xmm0, xmm0, xmm4
    vaesenc  xmm1, xmm1, xmm4
    vaesdec  xmm2, xmm2, xmm8
    vaesenc  xmm3, xmm3, xmm8
    vaesdec  xmm0, xmm0, xmm5
    vaesenc  xmm1, xmm1, xmm5
    vaesdec  xmm2, xmm2, xmm9
    vaesenc  xmm3, xmm3, xmm9
    vaesdec  xmm0, xmm0, xmm6
    vaesenc  xmm1, xmm1, xmm6
    vaesdec  xmm2, xmm2, xmm10
    vaesenc  xmm3, xmm3, xmm10
    vaesdec  xmm0, xmm0, xmm7
    vaesenc  xmm1, xmm1, xmm7
    vaesdec  xmm2, xmm2, xmm11
    vaesenc  xmm3, xmm3, xmm11
    vmovaps  XMMWORD PTR [r8+64],  xmm0
    vmovaps  XMMWORD PTR [r8+80],  xmm1
    vmovaps  XMMWORD PTR [r8+96],  xmm2
    vmovaps  XMMWORD PTR [r8+112], xmm3
    add r8, 128
    cmp r8, rdx
    jb f4_loop_start
    vmovaps XMMWORD PTR [rcx],    xmm0
    vmovaps XMMWORD PTR [rcx+16], xmm1
    vmovaps XMMWORD PTR [rcx+32], xmm2
    vmovaps XMMWORD PTR [rcx+48], xmm3
    vmovaps xmm6,  XMMWORD PTR [rsp]
    vmovaps xmm7,  XMMWORD PTR [rsp+16]
    vmovaps xmm8,  XMMWORD PTR [rsp+32]
    vmovaps xmm9,  XMMWORD PTR [rsp+48]
    vmovaps xmm10, XMMWORD PTR [rsp+64]
    vmovaps xmm11, XMMWORD PTR [rsp+80]
    add rsp, 104
    ret
fillAes4Rx4_hw ENDP

ALIGN 64
hashAndFillAes1Rx4_hw PROC
    sub rsp, 104
    vmovaps XMMWORD PTR [rsp],    xmm6
    vmovaps XMMWORD PTR [rsp+16], xmm7
    vmovaps XMMWORD PTR [rsp+32], xmm8
    vmovaps XMMWORD PTR [rsp+48], xmm9
    vmovaps XMMWORD PTR [rsp+64], xmm10
    vmovaps XMMWORD PTR [rsp+80], xmm11
    vmovaps xmm0,  XMMWORD PTR [AES_HASH_1R_STATE0]
    vmovaps xmm1,  XMMWORD PTR [AES_HASH_1R_STATE1]
    vmovaps xmm2,  XMMWORD PTR [AES_HASH_1R_STATE2]
    vmovaps xmm3,  XMMWORD PTR [AES_HASH_1R_STATE3]
    vmovaps xmm8,  XMMWORD PTR [AES_GEN_1R_KEY0]
    vmovaps xmm9,  XMMWORD PTR [AES_GEN_1R_KEY1]
    vmovaps xmm10, XMMWORD PTR [AES_GEN_1R_KEY2]
    vmovaps xmm11, XMMWORD PTR [AES_GEN_1R_KEY3]
    mov r8,  rcx
    add r8,  2496
    lea rdx, [rcx+2368]
    mov r10, QWORD PTR [rcx+2576]
    vmovaps xmm4, XMMWORD PTR [r8]
    vmovaps xmm5, XMMWORD PTR [r8+16]
    vmovaps xmm6, XMMWORD PTR [r8+32]
    vmovaps xmm7, XMMWORD PTR [r8+48]
    lea r11, [r10+2097152]
    lea rax, [r10+2093056]
ALIGN 64
haf_main_loop:
    prefetchw [r10+2048]
    prefetchw [r10+2112]
    prefetchw [r10+2176]
    prefetchw [r10+2240]
    vaesenc xmm0, xmm0, XMMWORD PTR [r10]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+16]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+32]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+48]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10],  xmm4
    vmovaps XMMWORD PTR [r10+16], xmm5
    vmovaps XMMWORD PTR [r10+32], xmm6
    vmovaps XMMWORD PTR [r10+48], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+64]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+80]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+96]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+112]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+64],  xmm4
    vmovaps XMMWORD PTR [r10+80],  xmm5
    vmovaps XMMWORD PTR [r10+96],  xmm6
    vmovaps XMMWORD PTR [r10+112], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+128]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+144]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+160]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+176]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+128], xmm4
    vmovaps XMMWORD PTR [r10+144], xmm5
    vmovaps XMMWORD PTR [r10+160], xmm6
    vmovaps XMMWORD PTR [r10+176], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+192]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+208]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+224]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+240]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+192], xmm4
    vmovaps XMMWORD PTR [r10+208], xmm5
    vmovaps XMMWORD PTR [r10+224], xmm6
    vmovaps XMMWORD PTR [r10+240], xmm7
    add r10, 256
    cmp r10, rax
    jb haf_main_loop
    ALIGN 64
haf_tail_loop:
    vaesenc xmm0, xmm0, XMMWORD PTR [r10]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+16]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+32]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+48]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10],  xmm4
    vmovaps XMMWORD PTR [r10+16], xmm5
    vmovaps XMMWORD PTR [r10+32], xmm6
    vmovaps XMMWORD PTR [r10+48], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+64]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+80]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+96]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+112]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+64],  xmm4
    vmovaps XMMWORD PTR [r10+80],  xmm5
    vmovaps XMMWORD PTR [r10+96],  xmm6
    vmovaps XMMWORD PTR [r10+112], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+128]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+144]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+160]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+176]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+128], xmm4
    vmovaps XMMWORD PTR [r10+144], xmm5
    vmovaps XMMWORD PTR [r10+160], xmm6
    vmovaps XMMWORD PTR [r10+176], xmm7
    vaesenc xmm0, xmm0, XMMWORD PTR [r10+192]
    vaesdec xmm1, xmm1, XMMWORD PTR [r10+208]
    vaesenc xmm2, xmm2, XMMWORD PTR [r10+224]
    vaesdec xmm3, xmm3, XMMWORD PTR [r10+240]
    vaesdec xmm4, xmm4, xmm8
    vaesenc xmm5, xmm5, xmm9
    vaesdec xmm6, xmm6, xmm10
    vaesenc xmm7, xmm7, xmm11
    vmovaps XMMWORD PTR [r10+192], xmm4
    vmovaps XMMWORD PTR [r10+208], xmm5
    vmovaps XMMWORD PTR [r10+224], xmm6
    vmovaps XMMWORD PTR [r10+240], xmm7
    add r10, 256
    cmp r10, r11
    jb haf_tail_loop
    vmovaps XMMWORD PTR [r8],  xmm4
    vmovaps XMMWORD PTR [r8+16], xmm5
    vmovaps XMMWORD PTR [r8+32], xmm6
    vmovaps XMMWORD PTR [r8+48], xmm7
    vmovaps xmm8, XMMWORD PTR [AES_HASH_1R_XKEY0]
    vmovaps xmm9, XMMWORD PTR [AES_HASH_1R_XKEY1]
    vaesenc xmm0, xmm0, xmm8
    vaesdec xmm1, xmm1, xmm8
    vaesenc xmm2, xmm2, xmm8
    vaesdec xmm3, xmm3, xmm8
    vaesenc xmm0, xmm0, xmm9
    vaesdec xmm1, xmm1, xmm9
    vaesenc xmm2, xmm2, xmm9
    vaesdec xmm3, xmm3, xmm9
    vmovaps XMMWORD PTR [rdx],  xmm0
    vmovaps XMMWORD PTR [rdx+16], xmm1
    vmovaps XMMWORD PTR [rdx+32], xmm2
    vmovaps XMMWORD PTR [rdx+48], xmm3
    vmovaps xmm6,  XMMWORD PTR [rsp]
    vmovaps xmm7,  XMMWORD PTR [rsp+16]
    vmovaps xmm8,  XMMWORD PTR [rsp+32]
    vmovaps xmm9,  XMMWORD PTR [rsp+48]
    vmovaps xmm10, XMMWORD PTR [rsp+64]
    vmovaps xmm11, XMMWORD PTR [rsp+80]
    add rsp, 104
    ret
hashAndFillAes1Rx4_hw ENDP

ALIGN 64
blake2b_compress_mid PROC
    sub   rsp, 96
    vmovdqu XMMWORD PTR [rsp],    xmm6
    vmovdqu XMMWORD PTR [rsp+16], xmm7
    vmovdqu XMMWORD PTR [rsp+32], xmm8
    vmovdqu XMMWORD PTR [rsp+48], xmm9
    vmovdqu XMMWORD PTR [rsp+64], xmm10
    vmovdqu XMMWORD PTR [rsp+80], xmm11
    vmovdqu ymm4,  YMMWORD PTR [b2_rot24_mask]
    vmovdqu ymm5,  YMMWORD PTR [b2_rot16_mask]
    vmovdqu ymm0,  YMMWORD PTR [rcx]
    vmovdqu ymm1,  YMMWORD PTR [rcx+32]
    vmovdqa ymm9,  ymm0
    vmovdqa ymm10, ymm1
    vmovdqu ymm2,  YMMWORD PTR [b2_iv_lo]
    vmovdqu ymm3,  YMMWORD PTR [b2_iv_hi]
    vmovdqu ymm6,  YMMWORD PTR [b2_t0_128]
    vpxor   ymm3,  ymm3, ymm6
    ROUND_AVX2  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
    ROUND_AVX2 14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3
    ROUND_AVX2 11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4
    ROUND_AVX2  7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8
    ROUND_AVX2  9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13
    ROUND_AVX2  2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9
    ROUND_AVX2 12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11
    ROUND_AVX2 13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10
    ROUND_AVX2  6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5
    ROUND_AVX2 10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13, 0
    ROUND_AVX2  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
    ROUND_AVX2 14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3
    vpxor   ymm0, ymm0, ymm2
    vpxor   ymm1, ymm1, ymm3
    vpxor   ymm0, ymm0, ymm9
    vpxor   ymm1, ymm1, ymm10
    vmovdqu YMMWORD PTR [rcx],    ymm0
    vmovdqu YMMWORD PTR [rcx+32], ymm1
    vmovdqu xmm6,  XMMWORD PTR [rsp]
    vmovdqu xmm7,  XMMWORD PTR [rsp+16]
    vmovdqu xmm8,  XMMWORD PTR [rsp+32]
    vmovdqu xmm9,  XMMWORD PTR [rsp+48]
    vmovdqu xmm10, XMMWORD PTR [rsp+64]
    vmovdqu xmm11, XMMWORD PTR [rsp+80]
    add   rsp, 96
    ret
blake2b_compress_mid ENDP

ALIGN 64
blake2b_compress_fin PROC
    sub   rsp, 96
    vmovdqu XMMWORD PTR [rsp],    xmm6
    vmovdqu XMMWORD PTR [rsp+16], xmm7
    vmovdqu XMMWORD PTR [rsp+32], xmm8
    vmovdqu XMMWORD PTR [rsp+48], xmm9
    vmovdqu XMMWORD PTR [rsp+64], xmm10
    vmovdqu XMMWORD PTR [rsp+80], xmm11
    vmovdqu ymm4,  YMMWORD PTR [b2_rot24_mask]
    vmovdqu ymm5,  YMMWORD PTR [b2_rot16_mask]
    vmovdqu ymm0,  YMMWORD PTR [rcx]
    vmovdqu ymm1,  YMMWORD PTR [rcx+32]
    vmovdqa ymm9,  ymm0
    vmovdqa ymm10, ymm1
    vmovdqu ymm2,  YMMWORD PTR [b2_iv_lo]
    vmovdqu ymm3,  YMMWORD PTR [b2_iv_hi_fin]
    vmovq   xmm6,  r8
    vpxor   ymm3,  ymm3, ymm6
    ROUND_AVX2  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
    ROUND_AVX2 14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3
    ROUND_AVX2 11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4
    ROUND_AVX2  7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8
    ROUND_AVX2  9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13
    ROUND_AVX2  2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9
    ROUND_AVX2 12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11
    ROUND_AVX2 13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10
    ROUND_AVX2  6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5
    ROUND_AVX2 10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13, 0
    ROUND_AVX2  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15
    ROUND_AVX2 14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3
    vpxor   ymm0, ymm0, ymm2
    vpxor   ymm1, ymm1, ymm3
    vpxor   ymm0, ymm0, ymm9
    vpxor   ymm1, ymm1, ymm10
    vmovdqu YMMWORD PTR [rcx],    ymm0
    vmovdqu YMMWORD PTR [rcx+32], ymm1
    vmovdqu xmm6,  XMMWORD PTR [rsp]
    vmovdqu xmm7,  XMMWORD PTR [rsp+16]
    vmovdqu xmm8,  XMMWORD PTR [rsp+32]
    vmovdqu xmm9,  XMMWORD PTR [rsp+48]
    vmovdqu xmm10, XMMWORD PTR [rsp+64]
    vmovdqu xmm11, XMMWORD PTR [rsp+80]
    add   rsp, 96
    ret
blake2b_compress_fin ENDP
_RANDOMX_TEXT ENDS

ENDIF
END