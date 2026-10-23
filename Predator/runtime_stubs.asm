.CODE

PUBLIC __chkstk
__chkstk PROC
    push   rcx
    push   rax
    cmp    rax, 4096
    lea    rcx, QWORD PTR [rsp+24]
    jb     cs_done
cs_loop:
    sub    rcx, 4096
    test   DWORD PTR [rcx], eax
    sub    rax, 4096
    cmp    rax, 4096
    ja     cs_loop
cs_done:
    sub    rcx, rax
    test   DWORD PTR [rcx], eax
    pop    rax
    pop    rcx
    ret
__chkstk ENDP

PUBLIC memcpy
memcpy PROC
    push   rsi
    push   rdi
    mov    rax, rcx
    mov    rdi, rcx
    mov    rsi, rdx
    mov    rcx, r8
    rep    movsb
    pop    rdi
    pop    rsi
    ret
memcpy ENDP

PUBLIC memset
memset PROC
    push   rdi
    mov    r9,  rcx
    mov    rdi, rcx
    movzx  eax, dl
    mov    rcx, r8
    rep    stosb
    mov    rax, r9
    pop    rdi
    ret
memset ENDP

PUBLIC memmove
memmove PROC
    push   rsi
    push   rdi
    mov    rax, rcx
    test   r8,  r8
    jz     mv_done
    cmp    rcx, rdx
    jbe    mv_fwd
    mov    r9,  rdx
    add    r9,  r8
    cmp    rcx, r9
    jae    mv_fwd
    lea    rdi, [rcx+r8-1]
    lea    rsi, [rdx+r8-1]
    mov    rcx, r8
    std
    rep    movsb
    cld
    jmp    mv_done
mv_fwd:
    mov    rdi, rcx
    mov    rsi, rdx
    mov    rcx, r8
    rep    movsb
mv_done:
    pop    rdi
    pop    rsi
    ret
memmove ENDP

END