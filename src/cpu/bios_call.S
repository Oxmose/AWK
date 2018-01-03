;-------------------------------------------------------------------------------
;
; File: bios_call.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 02/01/2018
;
; Version: 1.0
;
; Switch to real mode to do a bios call
;-------------------------------------------------------------------------------
[bits 32]

global bios_int  ; C function entry point
global _bios_int ; ASM entry point

extern enable_interrupt
extern disable_interrupt

struc regs16_t
    .di resw 1
    .si resw 1
    .bp resw 1
    .sp resw 1
    .bx resw 1
    .dx resw 1
    .cx resw 1
    .ax resw 1
    .gs resw 1
    .fs resw 1
    .es resw 1
    .ds resw 1
    .ef resw 1
endstruc

;-----------------------------------------------------------
; CODE REALOCATION
;-----------------------------------------------------------
%define CODE_LOCATION      0x7C00
%define OFFSET_ADDR(addr)  (((addr) - low_mem) + CODE_LOCATION)

;-----------------------------------------------------------
; SEGMENT DESCRIPTOR
;-----------------------------------------------------------
%define CODE32 0x08
%define DATA32 0x10
%define CODE16 0x18
%define DATA16 0x20

;-----------------------------------------------------------
; 16 BITS STACK
;-----------------------------------------------------------
%define STACK_16 (CODE_LOCATION - regs16_t_size)

section .text
    bios_int:
    	use32

        call disable_interrupt      ; Disable interrupts
        pusha                       ; Save context

        ; RELOCATE THE CODE TO LOW MEMORY SPACE

        mov  esi, low_mem                  ; Source address
        mov  edi, CODE_LOCATION            ; Destination address
        mov  ecx, (bios_int_end - low_mem) ; Size

        cld                                ; Clear direction flag
        rep  movsb                         ; Copy the code to low memory space

        jmp CODE_LOCATION                  ; Jump to newly relocated code

    low_mem:
        use32

    save_cpu_struct:
        mov  [OFFSET_ADDR(stack32_ptr)], esp ; Save 32bit stack pointer
        sidt [OFFSET_ADDR(idt32_ptr)]        ; Save 32bit IDT pointer
        sgdt [OFFSET_ADDR(gdt32_ptr)]        ; Save 32bit GDT pointer

    load_cpu_struct:
        lgdt [OFFSET_ADDR(gdt16_ptr)]        ; Load 16bit GDT pointer

        ; GET THE INT NUMBER

        lea  esi, [esp+0x24]
        lodsd

        ; SET THE INT NUMBERTO ISSUE

        mov  [OFFSET_ADDR(int_issue)], al

        ; COPY REGS DATA TO THE 16 BITS STACK

        mov  esi, [esi]                        ; Source address
        mov  edi, STACK_16                     ; Destination address
        mov  ecx, regs16_t_size                ; Size

        mov  esp, edi                          ; Save desitnation to stack

        rep  movsb                             ; Copy

        jmp  word CODE16:OFFSET_ADDR(rm_bootstrap)  ; Jumpt to RM RM bootstrap

    rm_bootstrap:
    	use16

        ; LOAD SEGMENT REGISTERS

        mov  ax, DATA16
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax

        ; Disable protected bit in CR0
        mov  eax, cr0
        and  al,  ~0x01
        mov  cr0, eax

        jmp  word 0x0000:OFFSET_ADDR(rm_body) ; Jump to real mode

    rm_body:
        use16
        xor  ax, ax
        mov  ds, ax
        mov  ss, ax

        ; LOAD BIOS IVT

        lidt [OFFSET_ADDR(idt16_ptr)]

        ; RESTORE REGS
        popa
        pop  gs
        pop  fs
        pop  es
        pop  ds

        db 0xCD  ; INT OPCODE

    int_issue:
        db 0x00

        xor  sp, sp
        mov  ss, sp
        mov  sp, CODE_LOCATION

        ; SAVE DATA TO STACK

        pushf
        push ds
        push es
        push fs
        push gs

    pm_bootstrap:

        pusha
        mov  eax, cr0
        inc  eax
        mov  cr0, eax



        jmp  dword CODE32:OFFSET_ADDR(pm_mode)  ; Jump to PM mode

    pm_mode:
        use32

        ; LOAD SEGMENT REGISTERS

        mov  ax, DATA32
        mov  ds, ax
        mov  es, ax
        mov  fs, ax
        mov  gs, ax
        mov  ss, ax

    restore_cpu_struct:
        lgdt [OFFSET_ADDR(gdt32_ptr)]        ; Restore 32bit GDT pointer
        lidt [OFFSET_ADDR(idt32_ptr)]        ; restore 32bit IDT pointer
        mov  esp, [OFFSET_ADDR(stack32_ptr)] ; Restore 32bit stack pointer

        ; SAVE DATA TO 32 BITS STACK (into regs parameter)

        mov  esi, STACK_16                     ; Source address
        lea  edi, [esp+0x28]                   ; Destination address
        mov  edi, [edi]                        ; Destination address
        mov  ecx, regs16_t_size                ; Size

        cld                                    ; Clear direction flag
        rep  movsb                             ; Copy 16Bits stack to 32Bits

        popa

        call enable_interrupt
        ret

;---------------------------------------------------------
; MEMORY STRUCTURES
;---------------------------------------------------------

    stack32_ptr:                               ; 32 Bits stack address
        dd 0x00000000

    idt32_ptr:                                 ; IDT pointer for 32bit access
        dw 0x0000                              ; IDT limit
        dd 0x00000000                          ; IDT base address

    gdt32_ptr:                                 ; GDT pointer for 32bit access
        dw 0x0000                              ; GDT limit
        dd 0x00000000                          ; GDT base address

    idt16_ptr:                                 ; BIOS IVT pointer
        dw 0x03FF                              ; IVT limit
        dd 0x00000000                          ; IVT base address

    gdt16:                                     ; GDT descriptor table
        .null:
            dd 0x00000000
            dd 0x00000000

        .code_32:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x9A
            db 0xCF
            db 0x00

        .data_32:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x92
            db 0xCF
            db 0x00

        .code_16:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x9A
            db 0x0F
            db 0x00

        .data_16:
            dw 0xFFFF
            dw 0x0000
            db 0x00
            db 0x92
            db 0x0F
            db 0x00

    gdt16_ptr:                                 ; GDT pointer for 16bit access
        dw gdt16_ptr - gdt16 - 1               ; GDT limit
        dd gdt16                               ; GDT base address

    bios_int_end:
