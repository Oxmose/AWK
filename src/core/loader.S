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

global loader                   ; Kernel entry point

;-----------------------------
; KERNEL STACK SETTIGNS
;-----------------------------
KERNEL_STACK_SIZE equ 32768

;-----------------------------
; MULTIBOOT SETTINGS
;-----------------------------
MAGIC_NUMBER equ 0x1BADB002           ; define the magic number constant
FLAGS        equ 0x0                  ; multiboot flags
CHECKSUM     equ -MAGIC_NUMBER-FLAGS  ; calculate the checksum

section .text:
  align 4                     ; the code must be 4 byte aligned
  dd MAGIC_NUMBER             ; write the magic number to the machine code,
  dd FLAGS                    ; the flags,
  dd CHECKSUM                 ; and the checksum

  loader:                        
    ; Make sure interrupts are disabled and clear flags
    push  dword 0
    popfd

    ; Init stack pointer
    mov  esp, kernel_stack + KERNEL_STACK_SIZE
    
    ; Init base pointer
    xor ebp, ebp

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
    hlt
    jmp .loop                   ; loop forever

;-----------------------
; INIT THE STACK SPACE
;-----------------------       
section .bss
  align 4
  kernel_stack:
    resb KERNEL_STACK_SIZE