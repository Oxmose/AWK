;----------------------------------------------
;
; File: int_handlers.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 15/12/2017
;
; Version: 1.0
;
; Global handlers for the 256 interrupts of the CPU.
; Setup the stack and call the C kernel general interrupt dispatcher.
;
;---------------------------------------------

extern kernel_interrupt_handler

section .text:
    %macro noerr_code_interrupt_handler 1 ; Interrupt that do not come with an err
                                            ; code.

    global interrupt_handler_%1
    interrupt_handler_%1:
        push    dword 0                     ; push 0 as dummy error code
        push    dword %1                    ; push the interrupt number
        jmp     generic_interrupt_handler    ; jump to the common handler
    %endmacro

    %macro err_code_interrupt_handler 1    ; Interrupt that do not come with an err
                                            ; code.

    global interrupt_handler_%1
    interrupt_handler_%1:
        push    dword %1                    ; push the interrupt number
        jmp     generic_interrupt_handler   ; jump to the common handler
    %endmacro

    generic_interrupt_handler:               ; Generic interrupt handler
        ; Save registers before calling interrupt

        push    ds
        push    es
        push    fs
        push    gs
        push    ss

        push    eax
        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi
        push    ebp
        push    esp

        ; call the C generic interrupt handler
        call    kernel_interrupt_handler

        ; Restore registers

        pop     esp
        pop     ebp
        pop     edi
        pop     esi
        pop     edx
        pop     ecx  
        pop     ebx
        pop     eax

        pop     ss
        pop     gs
        pop     fs
        pop     es
        pop     ds

        ; Restore the esp (interrupt and error code)
        add     esp, 8

        ; Return from interrupt
        iret

    ; Now create handlers for each interrupt
    noerr_code_interrupt_handler 8
    noerr_code_interrupt_handler 10
    noerr_code_interrupt_handler 11
    noerr_code_interrupt_handler 12
    noerr_code_interrupt_handler 13
    noerr_code_interrupt_handler 14
    noerr_code_interrupt_handler 17
    noerr_code_interrupt_handler 30

    noerr_code_interrupt_handler 0
    noerr_code_interrupt_handler 1
    noerr_code_interrupt_handler 2
    noerr_code_interrupt_handler 3
    noerr_code_interrupt_handler 4
    noerr_code_interrupt_handler 5
    noerr_code_interrupt_handler 6
    noerr_code_interrupt_handler 7
    noerr_code_interrupt_handler 9
    noerr_code_interrupt_handler 15
    noerr_code_interrupt_handler 16
    noerr_code_interrupt_handler 18
    noerr_code_interrupt_handler 19
    noerr_code_interrupt_handler 20
    noerr_code_interrupt_handler 21
    noerr_code_interrupt_handler 22
    noerr_code_interrupt_handler 23
    noerr_code_interrupt_handler 24
    noerr_code_interrupt_handler 25
    noerr_code_interrupt_handler 26
    noerr_code_interrupt_handler 27
    noerr_code_interrupt_handler 28
    noerr_code_interrupt_handler 29
    noerr_code_interrupt_handler 31
    noerr_code_interrupt_handler 32
    noerr_code_interrupt_handler 33
    noerr_code_interrupt_handler 34
    noerr_code_interrupt_handler 35
    noerr_code_interrupt_handler 36
    noerr_code_interrupt_handler 37
    noerr_code_interrupt_handler 38
    noerr_code_interrupt_handler 39
    noerr_code_interrupt_handler 40
    noerr_code_interrupt_handler 41
    noerr_code_interrupt_handler 42
    noerr_code_interrupt_handler 43
    noerr_code_interrupt_handler 44
    noerr_code_interrupt_handler 45
    noerr_code_interrupt_handler 46
    noerr_code_interrupt_handler 47
    noerr_code_interrupt_handler 48
    noerr_code_interrupt_handler 49
    noerr_code_interrupt_handler 50
    noerr_code_interrupt_handler 51
    noerr_code_interrupt_handler 52
    noerr_code_interrupt_handler 53
    noerr_code_interrupt_handler 54
    noerr_code_interrupt_handler 55
    noerr_code_interrupt_handler 56
    noerr_code_interrupt_handler 57
    noerr_code_interrupt_handler 58
    noerr_code_interrupt_handler 59
    noerr_code_interrupt_handler 60
    noerr_code_interrupt_handler 61
    noerr_code_interrupt_handler 62
    noerr_code_interrupt_handler 63
    noerr_code_interrupt_handler 64
    noerr_code_interrupt_handler 65
    noerr_code_interrupt_handler 66
    noerr_code_interrupt_handler 67
    noerr_code_interrupt_handler 68
    noerr_code_interrupt_handler 69
    noerr_code_interrupt_handler 70
    noerr_code_interrupt_handler 71
    noerr_code_interrupt_handler 72
    noerr_code_interrupt_handler 73
    noerr_code_interrupt_handler 74
    noerr_code_interrupt_handler 75
    noerr_code_interrupt_handler 76
    noerr_code_interrupt_handler 77
    noerr_code_interrupt_handler 78
    noerr_code_interrupt_handler 79
    noerr_code_interrupt_handler 80
    noerr_code_interrupt_handler 81
    noerr_code_interrupt_handler 82
    noerr_code_interrupt_handler 83
    noerr_code_interrupt_handler 84
    noerr_code_interrupt_handler 85
    noerr_code_interrupt_handler 86
    noerr_code_interrupt_handler 87
    noerr_code_interrupt_handler 88
    noerr_code_interrupt_handler 89
    noerr_code_interrupt_handler 90
    noerr_code_interrupt_handler 91
    noerr_code_interrupt_handler 92
    noerr_code_interrupt_handler 93
    noerr_code_interrupt_handler 94
    noerr_code_interrupt_handler 95
    noerr_code_interrupt_handler 96
    noerr_code_interrupt_handler 97
    noerr_code_interrupt_handler 98
    noerr_code_interrupt_handler 99
    noerr_code_interrupt_handler 100
    noerr_code_interrupt_handler 101
    noerr_code_interrupt_handler 102
    noerr_code_interrupt_handler 103
    noerr_code_interrupt_handler 104
    noerr_code_interrupt_handler 105
    noerr_code_interrupt_handler 106
    noerr_code_interrupt_handler 107
    noerr_code_interrupt_handler 108
    noerr_code_interrupt_handler 109
    noerr_code_interrupt_handler 110
    noerr_code_interrupt_handler 111
    noerr_code_interrupt_handler 112
    noerr_code_interrupt_handler 113
    noerr_code_interrupt_handler 114
    noerr_code_interrupt_handler 115
    noerr_code_interrupt_handler 116
    noerr_code_interrupt_handler 117
    noerr_code_interrupt_handler 118
    noerr_code_interrupt_handler 119
    noerr_code_interrupt_handler 120
    noerr_code_interrupt_handler 121
    noerr_code_interrupt_handler 122
    noerr_code_interrupt_handler 123
    noerr_code_interrupt_handler 124
    noerr_code_interrupt_handler 125
    noerr_code_interrupt_handler 126
    noerr_code_interrupt_handler 127
    noerr_code_interrupt_handler 128
    noerr_code_interrupt_handler 129
    noerr_code_interrupt_handler 130
    noerr_code_interrupt_handler 131
    noerr_code_interrupt_handler 132
    noerr_code_interrupt_handler 133
    noerr_code_interrupt_handler 134
    noerr_code_interrupt_handler 135
    noerr_code_interrupt_handler 136
    noerr_code_interrupt_handler 137
    noerr_code_interrupt_handler 138
    noerr_code_interrupt_handler 139
    noerr_code_interrupt_handler 140
    noerr_code_interrupt_handler 141
    noerr_code_interrupt_handler 142
    noerr_code_interrupt_handler 143
    noerr_code_interrupt_handler 144
    noerr_code_interrupt_handler 145
    noerr_code_interrupt_handler 146
    noerr_code_interrupt_handler 147
    noerr_code_interrupt_handler 148
    noerr_code_interrupt_handler 149
    noerr_code_interrupt_handler 150
    noerr_code_interrupt_handler 151
    noerr_code_interrupt_handler 152
    noerr_code_interrupt_handler 153
    noerr_code_interrupt_handler 154
    noerr_code_interrupt_handler 155
    noerr_code_interrupt_handler 156
    noerr_code_interrupt_handler 157
    noerr_code_interrupt_handler 158
    noerr_code_interrupt_handler 159
    noerr_code_interrupt_handler 160
    noerr_code_interrupt_handler 161
    noerr_code_interrupt_handler 162
    noerr_code_interrupt_handler 163
    noerr_code_interrupt_handler 164
    noerr_code_interrupt_handler 165
    noerr_code_interrupt_handler 166
    noerr_code_interrupt_handler 167
    noerr_code_interrupt_handler 168
    noerr_code_interrupt_handler 169
    noerr_code_interrupt_handler 170
    noerr_code_interrupt_handler 171
    noerr_code_interrupt_handler 172
    noerr_code_interrupt_handler 173
    noerr_code_interrupt_handler 174
    noerr_code_interrupt_handler 175
    noerr_code_interrupt_handler 176
    noerr_code_interrupt_handler 177
    noerr_code_interrupt_handler 178
    noerr_code_interrupt_handler 179
    noerr_code_interrupt_handler 180
    noerr_code_interrupt_handler 181
    noerr_code_interrupt_handler 182
    noerr_code_interrupt_handler 183
    noerr_code_interrupt_handler 184
    noerr_code_interrupt_handler 185
    noerr_code_interrupt_handler 186
    noerr_code_interrupt_handler 187
    noerr_code_interrupt_handler 188
    noerr_code_interrupt_handler 189
    noerr_code_interrupt_handler 190
    noerr_code_interrupt_handler 191
    noerr_code_interrupt_handler 192
    noerr_code_interrupt_handler 193
    noerr_code_interrupt_handler 194
    noerr_code_interrupt_handler 195
    noerr_code_interrupt_handler 196
    noerr_code_interrupt_handler 197
    noerr_code_interrupt_handler 198
    noerr_code_interrupt_handler 199
    noerr_code_interrupt_handler 200
    noerr_code_interrupt_handler 201
    noerr_code_interrupt_handler 202
    noerr_code_interrupt_handler 203
    noerr_code_interrupt_handler 204
    noerr_code_interrupt_handler 205
    noerr_code_interrupt_handler 206
    noerr_code_interrupt_handler 207
    noerr_code_interrupt_handler 208
    noerr_code_interrupt_handler 209
    noerr_code_interrupt_handler 210
    noerr_code_interrupt_handler 211
    noerr_code_interrupt_handler 212
    noerr_code_interrupt_handler 213
    noerr_code_interrupt_handler 214
    noerr_code_interrupt_handler 215
    noerr_code_interrupt_handler 216
    noerr_code_interrupt_handler 217
    noerr_code_interrupt_handler 218
    noerr_code_interrupt_handler 219
    noerr_code_interrupt_handler 220
    noerr_code_interrupt_handler 221
    noerr_code_interrupt_handler 222
    noerr_code_interrupt_handler 223
    noerr_code_interrupt_handler 224
    noerr_code_interrupt_handler 225
    noerr_code_interrupt_handler 226
    noerr_code_interrupt_handler 227
    noerr_code_interrupt_handler 228
    noerr_code_interrupt_handler 229
    noerr_code_interrupt_handler 230
    noerr_code_interrupt_handler 231
    noerr_code_interrupt_handler 232
    noerr_code_interrupt_handler 233
    noerr_code_interrupt_handler 234
    noerr_code_interrupt_handler 235
    noerr_code_interrupt_handler 236
    noerr_code_interrupt_handler 237
    noerr_code_interrupt_handler 238
    noerr_code_interrupt_handler 239
    noerr_code_interrupt_handler 240
    noerr_code_interrupt_handler 241
    noerr_code_interrupt_handler 242
    noerr_code_interrupt_handler 243
    noerr_code_interrupt_handler 244
    noerr_code_interrupt_handler 245
    noerr_code_interrupt_handler 246
    noerr_code_interrupt_handler 247
    noerr_code_interrupt_handler 248
    noerr_code_interrupt_handler 249
    noerr_code_interrupt_handler 250
    noerr_code_interrupt_handler 251
    noerr_code_interrupt_handler 252
    noerr_code_interrupt_handler 253
    noerr_code_interrupt_handler 254
    noerr_code_interrupt_handler 255