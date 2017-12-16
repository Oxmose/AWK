/*******************************************************************************
 *
 * File: cpu_settings.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 14/12/2017
 *
 * Version: 1.0
 *
 * X86 abstraction: setting functions and structures.
 * Used to set the GDT, IDT, TSS and other structures.
 ******************************************************************************/

#ifndef __CPU_SETTINGS_H_
#define __CPU_SETTINGS_H_

#include "../lib/stdint.h" /* Generic int types */

/**********************************
 * Kernel CPU constants
 *********************************/

/* GDT Settings */
#define GDT_ENTRY_COUNT 3

#define KERNEL_CS    0x08
#define KERNEL_DS    0x10

#define KERNEL_CODE_SEGMENT_BASE  0x00000000
#define KERNEL_CODE_SEGMENT_LIMIT 0x000FFFFF
#define KERNEL_DATA_SEGMENT_BASE  0x00000000
#define KERNEL_DATA_SEGMENT_LIMIT 0x000FFFFF

/* GDT Flags */
#define GDT_FLAG_GRANULARITY_4K   0x800000
#define GDT_FLAG_GRANULARITY_BYTE 0x000000
#define GDT_FLAG_16_BIT_SEGMENT   0x000000
#define GDT_FLAG_32_BIT_SEGMENT   0x400000
#define GDT_FLAG_64_BIT_SEGMENT   0x200000
#define GDT_FLAG_AVL              0x100000
#define GDT_FLAG_SEGMENT_PRESENT  0x008000
#define GDT_FLAG_PL0              0x000000
#define GDT_FLAG_PL1              0x002000
#define GDT_FLAG_PL2              0x004000
#define GDT_FLAG_PL3              0x006000
#define GDT_FLAG_CODE_TYPE        0x001000
#define GDT_FLAG_DATA_TYPE        0x001000
#define GDT_FLAG_SYSTEM_TYPE      0x000000

#define GDT_TYPE_EXECUTABLE       0x8
#define GDT_TYPE_GROW_UP          0x4
#define GDT_TYPE_GROW_DOWN        0x0
#define GDT_TYPE_CONFORMING       0x4
#define GDT_TYPE_PROTECTED        0x0
#define GDT_TYPE_READABLE         0x2
#define GDT_TYPE_WRITABLE         0x2
#define GDT_TYPE_ACCESSED         0x1

/* IDT Settings */
#define IDT_ENTRY_COUNT 256

/* GDT Flags */
#define IDT_FLAG_STORAGE_SEG 0x10
#define IDT_FLAG_PL0         0x00
#define IDT_FLAG_PL1         0x20
#define IDT_FLAG_PL2         0x40
#define IDT_FLAG_PL3         0x60
#define IDT_FLAG_PRESENT     0x80

#define IDT_TYPE_TASK_GATE 0x05
#define IDT_TYPE_INT_GATE  0x0E
#define IDT_TYPE_TRAP_GATE 0x0F

/**************************************
 * Kernel assembly interrupt handlers
 *************************************/
extern void interrupt_handler_0(void);
extern void interrupt_handler_1(void);
extern void interrupt_handler_2(void);
extern void interrupt_handler_3(void);
extern void interrupt_handler_4(void);
extern void interrupt_handler_5(void);
extern void interrupt_handler_6(void);
extern void interrupt_handler_7(void);
extern void interrupt_handler_8(void);
extern void interrupt_handler_9(void);
extern void interrupt_handler_10(void);
extern void interrupt_handler_11(void);
extern void interrupt_handler_12(void);
extern void interrupt_handler_13(void);
extern void interrupt_handler_14(void);
extern void interrupt_handler_15(void);
extern void interrupt_handler_16(void);
extern void interrupt_handler_17(void);
extern void interrupt_handler_18(void);
extern void interrupt_handler_19(void);
extern void interrupt_handler_20(void);
extern void interrupt_handler_21(void);
extern void interrupt_handler_22(void);
extern void interrupt_handler_23(void);
extern void interrupt_handler_24(void);
extern void interrupt_handler_25(void);
extern void interrupt_handler_26(void);
extern void interrupt_handler_27(void);
extern void interrupt_handler_28(void);
extern void interrupt_handler_29(void);
extern void interrupt_handler_30(void);
extern void interrupt_handler_31(void);
extern void interrupt_handler_32(void);
extern void interrupt_handler_33(void);
extern void interrupt_handler_34(void);
extern void interrupt_handler_35(void);
extern void interrupt_handler_36(void);
extern void interrupt_handler_37(void);
extern void interrupt_handler_38(void);
extern void interrupt_handler_39(void);
extern void interrupt_handler_40(void);
extern void interrupt_handler_41(void);
extern void interrupt_handler_42(void);
extern void interrupt_handler_43(void);
extern void interrupt_handler_44(void);
extern void interrupt_handler_45(void);
extern void interrupt_handler_46(void);
extern void interrupt_handler_47(void);
extern void interrupt_handler_48(void);
extern void interrupt_handler_49(void);
extern void interrupt_handler_50(void);
extern void interrupt_handler_51(void);
extern void interrupt_handler_52(void);
extern void interrupt_handler_53(void);
extern void interrupt_handler_54(void);
extern void interrupt_handler_55(void);
extern void interrupt_handler_56(void);
extern void interrupt_handler_57(void);
extern void interrupt_handler_58(void);
extern void interrupt_handler_59(void);
extern void interrupt_handler_60(void);
extern void interrupt_handler_61(void);
extern void interrupt_handler_62(void);
extern void interrupt_handler_63(void);
extern void interrupt_handler_64(void);
extern void interrupt_handler_65(void);
extern void interrupt_handler_66(void);
extern void interrupt_handler_67(void);
extern void interrupt_handler_68(void);
extern void interrupt_handler_69(void);
extern void interrupt_handler_70(void);
extern void interrupt_handler_71(void);
extern void interrupt_handler_72(void);
extern void interrupt_handler_73(void);
extern void interrupt_handler_74(void);
extern void interrupt_handler_75(void);
extern void interrupt_handler_76(void);
extern void interrupt_handler_77(void);
extern void interrupt_handler_78(void);
extern void interrupt_handler_79(void);
extern void interrupt_handler_80(void);
extern void interrupt_handler_81(void);
extern void interrupt_handler_82(void);
extern void interrupt_handler_83(void);
extern void interrupt_handler_84(void);
extern void interrupt_handler_85(void);
extern void interrupt_handler_86(void);
extern void interrupt_handler_87(void);
extern void interrupt_handler_88(void);
extern void interrupt_handler_89(void);
extern void interrupt_handler_90(void);
extern void interrupt_handler_91(void);
extern void interrupt_handler_92(void);
extern void interrupt_handler_93(void);
extern void interrupt_handler_94(void);
extern void interrupt_handler_95(void);
extern void interrupt_handler_96(void);
extern void interrupt_handler_97(void);
extern void interrupt_handler_98(void);
extern void interrupt_handler_99(void);
extern void interrupt_handler_100(void);
extern void interrupt_handler_101(void);
extern void interrupt_handler_102(void);
extern void interrupt_handler_103(void);
extern void interrupt_handler_104(void);
extern void interrupt_handler_105(void);
extern void interrupt_handler_106(void);
extern void interrupt_handler_107(void);
extern void interrupt_handler_108(void);
extern void interrupt_handler_109(void);
extern void interrupt_handler_110(void);
extern void interrupt_handler_111(void);
extern void interrupt_handler_112(void);
extern void interrupt_handler_113(void);
extern void interrupt_handler_114(void);
extern void interrupt_handler_115(void);
extern void interrupt_handler_116(void);
extern void interrupt_handler_117(void);
extern void interrupt_handler_118(void);
extern void interrupt_handler_119(void);
extern void interrupt_handler_120(void);
extern void interrupt_handler_121(void);
extern void interrupt_handler_122(void);
extern void interrupt_handler_123(void);
extern void interrupt_handler_124(void);
extern void interrupt_handler_125(void);
extern void interrupt_handler_126(void);
extern void interrupt_handler_127(void);
extern void interrupt_handler_128(void);
extern void interrupt_handler_129(void);
extern void interrupt_handler_130(void);
extern void interrupt_handler_131(void);
extern void interrupt_handler_132(void);
extern void interrupt_handler_133(void);
extern void interrupt_handler_134(void);
extern void interrupt_handler_135(void);
extern void interrupt_handler_136(void);
extern void interrupt_handler_137(void);
extern void interrupt_handler_138(void);
extern void interrupt_handler_139(void);
extern void interrupt_handler_140(void);
extern void interrupt_handler_141(void);
extern void interrupt_handler_142(void);
extern void interrupt_handler_143(void);
extern void interrupt_handler_144(void);
extern void interrupt_handler_145(void);
extern void interrupt_handler_146(void);
extern void interrupt_handler_147(void);
extern void interrupt_handler_148(void);
extern void interrupt_handler_149(void);
extern void interrupt_handler_150(void);
extern void interrupt_handler_151(void);
extern void interrupt_handler_152(void);
extern void interrupt_handler_153(void);
extern void interrupt_handler_154(void);
extern void interrupt_handler_155(void);
extern void interrupt_handler_156(void);
extern void interrupt_handler_157(void);
extern void interrupt_handler_158(void);
extern void interrupt_handler_159(void);
extern void interrupt_handler_160(void);
extern void interrupt_handler_161(void);
extern void interrupt_handler_162(void);
extern void interrupt_handler_163(void);
extern void interrupt_handler_164(void);
extern void interrupt_handler_165(void);
extern void interrupt_handler_166(void);
extern void interrupt_handler_167(void);
extern void interrupt_handler_168(void);
extern void interrupt_handler_169(void);
extern void interrupt_handler_170(void);
extern void interrupt_handler_171(void);
extern void interrupt_handler_172(void);
extern void interrupt_handler_173(void);
extern void interrupt_handler_174(void);
extern void interrupt_handler_175(void);
extern void interrupt_handler_176(void);
extern void interrupt_handler_177(void);
extern void interrupt_handler_178(void);
extern void interrupt_handler_179(void);
extern void interrupt_handler_180(void);
extern void interrupt_handler_181(void);
extern void interrupt_handler_182(void);
extern void interrupt_handler_183(void);
extern void interrupt_handler_184(void);
extern void interrupt_handler_185(void);
extern void interrupt_handler_186(void);
extern void interrupt_handler_187(void);
extern void interrupt_handler_188(void);
extern void interrupt_handler_189(void);
extern void interrupt_handler_190(void);
extern void interrupt_handler_191(void);
extern void interrupt_handler_192(void);
extern void interrupt_handler_193(void);
extern void interrupt_handler_194(void);
extern void interrupt_handler_195(void);
extern void interrupt_handler_196(void);
extern void interrupt_handler_197(void);
extern void interrupt_handler_198(void);
extern void interrupt_handler_199(void);
extern void interrupt_handler_200(void);
extern void interrupt_handler_201(void);
extern void interrupt_handler_202(void);
extern void interrupt_handler_203(void);
extern void interrupt_handler_204(void);
extern void interrupt_handler_205(void);
extern void interrupt_handler_206(void);
extern void interrupt_handler_207(void);
extern void interrupt_handler_208(void);
extern void interrupt_handler_209(void);
extern void interrupt_handler_210(void);
extern void interrupt_handler_211(void);
extern void interrupt_handler_212(void);
extern void interrupt_handler_213(void);
extern void interrupt_handler_214(void);
extern void interrupt_handler_215(void);
extern void interrupt_handler_216(void);
extern void interrupt_handler_217(void);
extern void interrupt_handler_218(void);
extern void interrupt_handler_219(void);
extern void interrupt_handler_220(void);
extern void interrupt_handler_221(void);
extern void interrupt_handler_222(void);
extern void interrupt_handler_223(void);
extern void interrupt_handler_224(void);
extern void interrupt_handler_225(void);
extern void interrupt_handler_226(void);
extern void interrupt_handler_227(void);
extern void interrupt_handler_228(void);
extern void interrupt_handler_229(void);
extern void interrupt_handler_230(void);
extern void interrupt_handler_231(void);
extern void interrupt_handler_232(void);
extern void interrupt_handler_233(void);
extern void interrupt_handler_234(void);
extern void interrupt_handler_235(void);
extern void interrupt_handler_236(void);
extern void interrupt_handler_237(void);
extern void interrupt_handler_238(void);
extern void interrupt_handler_239(void);
extern void interrupt_handler_240(void);
extern void interrupt_handler_241(void);
extern void interrupt_handler_242(void);
extern void interrupt_handler_243(void);
extern void interrupt_handler_244(void);
extern void interrupt_handler_245(void);
extern void interrupt_handler_246(void);
extern void interrupt_handler_247(void);
extern void interrupt_handler_248(void);
extern void interrupt_handler_249(void);
extern void interrupt_handler_250(void);
extern void interrupt_handler_251(void);
extern void interrupt_handler_252(void);
extern void interrupt_handler_253(void);
extern void interrupt_handler_254(void);
extern void interrupt_handler_255(void);

/**********************************
 * Kernel CPU structures 
 *********************************/

/* Setup a flat GDT for the kernel. Fills the entries in the GDT table and load
 * the new GDT. Set the segment registers (CS, DS, ES, FS, GS, SS).
 */
void setup_gdt(void);

/* Setup a simple IDT for the kernel. Fills the entries in the IDT table by
 * adding basic support to the x86 excetion (int 0 - 32)
 */
void setup_idt(void);

#endif /* __CPU_SETTINGS_H_ */