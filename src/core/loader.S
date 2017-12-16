;----------------------------------------------
;
; File: loader.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 14/12/2017
;
; Version: 1.0
;
; Kernel entry point and cpu initialization
;
;---------------------------------------------

;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern setup_gdt
extern setup_idt
extern init_kernel_interrupt
extern setup_pic
extern clear_screen
extern kernel_kickstart
extern _edata
extern mem_heap_end

global loader                   ; Kernel entry point

;-----------------------------
; KERNEL STACK SETTIGNS
;-----------------------------
KERNEL_STACK_SIZE equ 32768

;-----------------------------
; MULTIBOOT SETTINGS
;-----------------------------
MODULEALIGN equ 1<<0
MEMINFO     equ 1<<1
FLAGS       equ MODULEALIGN | MEMINFO
MAGIC       equ 0x1BADB002
CHECKSUM    equ -(MAGIC + FLAGS)

section .multiboot
  align 4
  multiboot_header:
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

section .text:
  align 4
  loader:                        
    ; Make sure interrupts are disabled and clear flags
    push  dword 0
    popfd

    ; Init stack pointer
    mov  esp, kernel_stack + KERNEL_STACK_SIZE
    
    ; Init base pointer
    xor ebp, ebp

    ; Blank all uninitialized memory
    mov  edi, _edata
    xor  eax, eax
    blank_mem:  
        mov  [edi], eax
        add  edi, 4
        cmp  edi, mem_heap_end
        jl  blank_mem

    ; Cleat screen
    call clear_screen

    ; Setup kernel GDT
    call setup_gdt

    ; Setup kernel IDT
    call setup_idt

    ; Setup kernel interrupt handlers
    call init_kernel_interrupt

    ; Setup PIC
    call setup_pic

    ; Kickstrat kernel
    call kernel_kickstart
    
  ; Hang forever, this should never happen
  .loop:
    sti
    hlt
    jmp .loop                   ; loop forever

;-----------------------
; INIT THE STACK SPACE
;-----------------------       
section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE