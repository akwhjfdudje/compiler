    .globl main
main:
    push     %ebp
    movl     %esp, %ebp
    # statement
    movl     $42, %eax
    neg %eax
    pushl    %eax
    # statement
    movl      -4(%ebp), %eax
    push      %eax
    movl     $0, %eax
    pop       %ecx
    cmpl      %eax, %ecx
    movl      $0, %eax
    setg      %al
    cmpl     $0, %eax
    je     _LLLL0
    # statement
    movl      -4(%ebp), %eax
    movl     %ebp, %esp
    pop      %ebp
    movl     %eax, %eax
    ret
    jmp   _LLLL1
_LLLL0:
_LLLL1:
    # statement
    movl      -4(%ebp), %eax
    push      %eax
    movl     $0, %eax
    pop       %ecx
    cmpl      %eax, %ecx
    movl      $0, %eax
    setl      %al
    cmpl     $0, %eax
    je     _LLLL2
    # statement
    movl      -4(%ebp), %eax
    neg %eax
    movl     %ebp, %esp
    pop      %ebp
    movl     %eax, %eax
    ret
    jmp   _LLLL3
_LLLL2:
_LLLL3:
    # statement
    movl     $0, %eax
    movl     %ebp, %esp
    pop      %ebp
    movl     %eax, %eax
    ret
    ret
