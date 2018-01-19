;-------------------------------------------------------------------------------
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
;-------------------------------------------------------------------------------

;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern setup_gdt
extern setup_idt
extern clear_screen
extern init_serial
extern kernel_kickstart
extern setup_kheap

extern _edata
extern mem_heap_end

extern cpu_info                 ; CPU INFO structure

global loader                   ; Kernel entry point

;-----------------------------
; KERNEL STACK SETTIGNS
;-----------------------------
KERNEL_STACK_SIZE equ 16384

;-----------------------------
; MULTIBOOT SETTINGS
;-----------------------------
MODULEALIGN    equ 1<<0
MEMINFO        equ 1<<1

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

    ; Init CPU info structure
    mov  edi, cpu_info
    mov  esi, cpu_info ; Compute the
    add  esi, 4        ; end address of the structure
    xor  eax, eax
    clear_cpu_info:
        mov  [edi], eax
        add  edi, 4
        cmp  edi, esi
        jl   clear_cpu_info

    call get_cpuid_flag ; Get CPUID instruction availability

    ; Clear screen
    call clear_screen

    ; Init serial for debug
    call init_serial

    ; Setup kernel GDT
    call setup_gdt

    ; Setup kernel IDT
    call setup_idt

    ; Setup kernel heap management
    call setup_kheap

    ; Kernel C entry point
    call kernel_kickstart

  ; Hang forever, this should never happen
  .loop:
    cli
    hlt
    jmp .loop                   ; loop forever

  get_cpuid_flag:
    pushfd                                  ;Save EFLAGS
    pushfd                                  ;Store EFLAGS
    xor    dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                   ;Load stored EFLAGS
    pushfd                                  ;Store EFLAGS again
    pop    eax                              ;eax = modified EFLAGS
    xor    eax,[esp]                        ;eax = whichever bits were changed
    popfd                                   ;Restore original EFLAGS
    and    eax,0x00200000                   ;eax = 0 CPUID is not available
    mov    edi, cpu_info
    or     [edi], eax                       ; Store information in the structure
    ret

;-----------------------
; INIT THE STACK SPACE
;-----------------------
section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE
