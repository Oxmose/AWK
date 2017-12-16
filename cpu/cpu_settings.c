/*******************************************************************************
 *
 * File: cpu_settings.c
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

/* Header file */
#include "cpu_settings.h"

/* memset */
#include "../lib/string.h"

/* Generic int types */
#include "../lib/stdint.h"

/* kernel_success, kernel_print_unsigned_hex */
#include "../core/kernel_output.h"

/* Kernel GDT structure */
extern uint64_t cpu_gdt[GDT_ENTRY_COUNT];
extern uint16_t cpu_gdt_size;
extern uint32_t cpu_gdt_base;

/* Kernel IDT structure */
extern uint64_t cpu_idt[IDT_ENTRY_COUNT];
extern uint16_t cpu_idt_size;
extern uint32_t cpu_idt_base;

/**********************************
 * Kernel CPU settings functions
 *********************************/
void format_gdt_entry(uint64_t *entry, 
                      const uint32_t base, const uint32_t limit,  
                      const unsigned char type, const uint32_t flags)
{
    uint32_t lo_part = 0;
    uint32_t hi_part = 0;

    /*
     * Low part[31;0] = Base[15;0] Limit[15;0]
     */
    lo_part = ((base & 0xFFFF) << 16) | (limit & 0xFFFF);

    /* 
     * High part[7;0] = Base[23;16]
     */
    hi_part = (base >> 16) & 0xFF;
    /* 
     * High part[11;8] = Type[3;0]
     */
    hi_part |= (type & 0xF) << 8;
    /* 
     * High part[15;12] = Seg_Present[1;0]Privilege[2;0]Descriptor_Type[1;0]
     * High part[23;20] = Granularity[1;0]Op_Size[1;0]L[1;0]AVL[1;0]
     */
    hi_part |= flags & 0x00F0F000;
    
    /* 
     * High part[19;16] = Limit[19;16]
     */
    hi_part |= limit & 0xF0000;
    /* 
     * High part[31;24] = Base[31;24]
     */
    hi_part |= base & 0xFF000000;

    /* Set the value of the entry */
    *entry = lo_part | (((uint64_t) hi_part) << 32);
}

void setup_gdt(void)
{
    /* Blank the GDT, set the NULL descriptor */
    memset(cpu_gdt, 0, sizeof(uint64_t) * GDT_ENTRY_COUNT);

    /* Set the kernel code descriptor */
    uint32_t kernel_code_seg_flags = GDT_FLAG_GRANULARITY_4K | 
                                     GDT_FLAG_32_BIT_SEGMENT |
                                     GDT_FLAG_PL0 | 
                                     GDT_FLAG_SEGMENT_PRESENT |
                                     GDT_FLAG_CODE_TYPE;

    uint32_t kernel_code_seg_type =  GDT_TYPE_EXECUTABLE | 
                                     GDT_TYPE_READABLE | 
                                     GDT_TYPE_PROTECTED;

    format_gdt_entry(&cpu_gdt[KERNEL_CS / 8], 
                     KERNEL_CODE_SEGMENT_BASE, KERNEL_CODE_SEGMENT_LIMIT,
                     kernel_code_seg_type, kernel_code_seg_flags);

    

    /* Set the kernel data descriptor */
    uint32_t kernel_data_seg_flags = GDT_FLAG_GRANULARITY_4K | 
                                     GDT_FLAG_32_BIT_SEGMENT |
                                     GDT_FLAG_PL0 | 
                                     GDT_FLAG_SEGMENT_PRESENT |
                                     GDT_FLAG_DATA_TYPE;

    uint32_t kernel_data_seg_type =  GDT_TYPE_WRITABLE | 
                                     GDT_TYPE_GROW_DOWN;

    format_gdt_entry(&cpu_gdt[KERNEL_DS / 8], 
                     KERNEL_DATA_SEGMENT_BASE, KERNEL_DATA_SEGMENT_LIMIT,
                     kernel_data_seg_type, kernel_data_seg_flags);

    /* Set the GDT descriptor */
    cpu_gdt_size = ((sizeof(uint64_t) * GDT_ENTRY_COUNT) - 1);
    cpu_gdt_base = (uint32_t)&cpu_gdt;

    /* Load the GDT */
    __asm__ __volatile__("lgdt %0" :: "m" (cpu_gdt_size), "m" (cpu_gdt_base));

    /* Load segment selectors with a far jump for CS*/
    __asm__ __volatile__("movw %w0,%%ds" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%es" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%fs" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%gs" :: "r" (KERNEL_DS));
    __asm__ __volatile__("movw %w0,%%ss" :: "r" (KERNEL_DS));
    __asm__ __volatile__("ljmp %0, $flab \n\t flab: \n\t" :: "i" (KERNEL_CS));

    kernel_success("GDT Initialized at ", 19);
    kernel_print_unsigned_hex(cpu_gdt_base, 8);
    kernel_print("\n", 1);
}

uint32_t get_handler(unsigned int int_id)
{
    switch(int_id)
    {
        case 0:
            return (uint32_t)interrupt_handler_0;
        case 1:
            return (uint32_t)interrupt_handler_1;
        case 2:
            return (uint32_t)interrupt_handler_2;
        case 3:
            return (uint32_t)interrupt_handler_3;
        case 4:
            return (uint32_t)interrupt_handler_4;
        case 5:
            return (uint32_t)interrupt_handler_5;
        case 6:
            return (uint32_t)interrupt_handler_6;
        case 7:
            return (uint32_t)interrupt_handler_7;
        case 8:
            return (uint32_t)interrupt_handler_8;
        case 9:
            return (uint32_t)interrupt_handler_9;
        case 10:
            return (uint32_t)interrupt_handler_10;
        case 11:
            return (uint32_t)interrupt_handler_11;
        case 12:
            return (uint32_t)interrupt_handler_12;
        case 13:
            return (uint32_t)interrupt_handler_13;
        case 14:
            return (uint32_t)interrupt_handler_14;
        case 15:
            return (uint32_t)interrupt_handler_15;
        case 16:
            return (uint32_t)interrupt_handler_16;
        case 17:
            return (uint32_t)interrupt_handler_17;
        case 18:
            return (uint32_t)interrupt_handler_18;
        case 19:
            return (uint32_t)interrupt_handler_19;
        case 20:
            return (uint32_t)interrupt_handler_20;
        case 21:
            return (uint32_t)interrupt_handler_21;
        case 22:
            return (uint32_t)interrupt_handler_22;
        case 23:
            return (uint32_t)interrupt_handler_23;
        case 24:
            return (uint32_t)interrupt_handler_24;
        case 25:
            return (uint32_t)interrupt_handler_25;
        case 26:
            return (uint32_t)interrupt_handler_26;
        case 27:
            return (uint32_t)interrupt_handler_27;
        case 28:
            return (uint32_t)interrupt_handler_28;
        case 29:
            return (uint32_t)interrupt_handler_29;
        case 30:
            return (uint32_t)interrupt_handler_30;
        case 31:
            return (uint32_t)interrupt_handler_31;
        case 32:
            return (uint32_t)interrupt_handler_32;
        case 33:
            return (uint32_t)interrupt_handler_33;
        case 34:
            return (uint32_t)interrupt_handler_34;
        case 35:
            return (uint32_t)interrupt_handler_35;
        case 36:
            return (uint32_t)interrupt_handler_36;
        case 37:
            return (uint32_t)interrupt_handler_37;
        case 38:
            return (uint32_t)interrupt_handler_38;
        case 39:
            return (uint32_t)interrupt_handler_39;
        case 40:
            return (uint32_t)interrupt_handler_40;
        case 41:
            return (uint32_t)interrupt_handler_41;
        case 42:
            return (uint32_t)interrupt_handler_42;
        case 43:
            return (uint32_t)interrupt_handler_43;
        case 44:
            return (uint32_t)interrupt_handler_44;
        case 45:
            return (uint32_t)interrupt_handler_45;
        case 46:
            return (uint32_t)interrupt_handler_46;
        case 47:
            return (uint32_t)interrupt_handler_47;
        case 48:
            return (uint32_t)interrupt_handler_48;
        case 49:
            return (uint32_t)interrupt_handler_49;
        case 50:
            return (uint32_t)interrupt_handler_50;
        case 51:
            return (uint32_t)interrupt_handler_51;
        case 52:
            return (uint32_t)interrupt_handler_52;
        case 53:
            return (uint32_t)interrupt_handler_53;
        case 54:
            return (uint32_t)interrupt_handler_54;
        case 55:
            return (uint32_t)interrupt_handler_55;
        case 56:
            return (uint32_t)interrupt_handler_56;
        case 57:
            return (uint32_t)interrupt_handler_57;
        case 58:
            return (uint32_t)interrupt_handler_58;
        case 59:
            return (uint32_t)interrupt_handler_59;
        case 60:
            return (uint32_t)interrupt_handler_60;
        case 61:
            return (uint32_t)interrupt_handler_61;
        case 62:
            return (uint32_t)interrupt_handler_62;
        case 63:
            return (uint32_t)interrupt_handler_63;
        case 64:
            return (uint32_t)interrupt_handler_64;
        case 65:
            return (uint32_t)interrupt_handler_65;
        case 66:
            return (uint32_t)interrupt_handler_66;
        case 67:
            return (uint32_t)interrupt_handler_67;
        case 68:
            return (uint32_t)interrupt_handler_68;
        case 69:
            return (uint32_t)interrupt_handler_69;
        case 70:
            return (uint32_t)interrupt_handler_70;
        case 71:
            return (uint32_t)interrupt_handler_71;
        case 72:
            return (uint32_t)interrupt_handler_72;
        case 73:
            return (uint32_t)interrupt_handler_73;
        case 74:
            return (uint32_t)interrupt_handler_74;
        case 75:
            return (uint32_t)interrupt_handler_75;
        case 76:
            return (uint32_t)interrupt_handler_76;
        case 77:
            return (uint32_t)interrupt_handler_77;
        case 78:
            return (uint32_t)interrupt_handler_78;
        case 79:
            return (uint32_t)interrupt_handler_79;
        case 80:
            return (uint32_t)interrupt_handler_80;
        case 81:
            return (uint32_t)interrupt_handler_81;
        case 82:
            return (uint32_t)interrupt_handler_82;
        case 83:
            return (uint32_t)interrupt_handler_83;
        case 84:
            return (uint32_t)interrupt_handler_84;
        case 85:
            return (uint32_t)interrupt_handler_85;
        case 86:
            return (uint32_t)interrupt_handler_86;
        case 87:
            return (uint32_t)interrupt_handler_87;
        case 88:
            return (uint32_t)interrupt_handler_88;
        case 89:
            return (uint32_t)interrupt_handler_89;
        case 90:
            return (uint32_t)interrupt_handler_90;
        case 91:
            return (uint32_t)interrupt_handler_91;
        case 92:
            return (uint32_t)interrupt_handler_92;
        case 93:
            return (uint32_t)interrupt_handler_93;
        case 94:
            return (uint32_t)interrupt_handler_94;
        case 95:
            return (uint32_t)interrupt_handler_95;
        case 96:
            return (uint32_t)interrupt_handler_96;
        case 97:
            return (uint32_t)interrupt_handler_97;
        case 98:
            return (uint32_t)interrupt_handler_98;
        case 99:
            return (uint32_t)interrupt_handler_99;
        case 100:
            return (uint32_t)interrupt_handler_100;
        case 101:
            return (uint32_t)interrupt_handler_101;
        case 102:
            return (uint32_t)interrupt_handler_102;
        case 103:
            return (uint32_t)interrupt_handler_103;
        case 104:
            return (uint32_t)interrupt_handler_104;
        case 105:
            return (uint32_t)interrupt_handler_105;
        case 106:
            return (uint32_t)interrupt_handler_106;
        case 107:
            return (uint32_t)interrupt_handler_107;
        case 108:
            return (uint32_t)interrupt_handler_108;
        case 109:
            return (uint32_t)interrupt_handler_109;
        case 110:
            return (uint32_t)interrupt_handler_110;
        case 111:
            return (uint32_t)interrupt_handler_111;
        case 112:
            return (uint32_t)interrupt_handler_112;
        case 113:
            return (uint32_t)interrupt_handler_113;
        case 114:
            return (uint32_t)interrupt_handler_114;
        case 115:
            return (uint32_t)interrupt_handler_115;
        case 116:
            return (uint32_t)interrupt_handler_116;
        case 117:
            return (uint32_t)interrupt_handler_117;
        case 118:
            return (uint32_t)interrupt_handler_118;
        case 119:
            return (uint32_t)interrupt_handler_119;
        case 120:
            return (uint32_t)interrupt_handler_120;
        case 121:
            return (uint32_t)interrupt_handler_121;
        case 122:
            return (uint32_t)interrupt_handler_122;
        case 123:
            return (uint32_t)interrupt_handler_123;
        case 124:
            return (uint32_t)interrupt_handler_124;
        case 125:
            return (uint32_t)interrupt_handler_125;
        case 126:
            return (uint32_t)interrupt_handler_126;
        case 127:
            return (uint32_t)interrupt_handler_127;
        case 128:
            return (uint32_t)interrupt_handler_128;
        case 129:
            return (uint32_t)interrupt_handler_129;
        case 130:
            return (uint32_t)interrupt_handler_130;
        case 131:
            return (uint32_t)interrupt_handler_131;
        case 132:
            return (uint32_t)interrupt_handler_132;
        case 133:
            return (uint32_t)interrupt_handler_133;
        case 134:
            return (uint32_t)interrupt_handler_134;
        case 135:
            return (uint32_t)interrupt_handler_135;
        case 136:
            return (uint32_t)interrupt_handler_136;
        case 137:
            return (uint32_t)interrupt_handler_137;
        case 138:
            return (uint32_t)interrupt_handler_138;
        case 139:
            return (uint32_t)interrupt_handler_139;
        case 140:
            return (uint32_t)interrupt_handler_140;
        case 141:
            return (uint32_t)interrupt_handler_141;
        case 142:
            return (uint32_t)interrupt_handler_142;
        case 143:
            return (uint32_t)interrupt_handler_143;
        case 144:
            return (uint32_t)interrupt_handler_144;
        case 145:
            return (uint32_t)interrupt_handler_145;
        case 146:
            return (uint32_t)interrupt_handler_146;
        case 147:
            return (uint32_t)interrupt_handler_147;
        case 148:
            return (uint32_t)interrupt_handler_148;
        case 149:
            return (uint32_t)interrupt_handler_149;
        case 150:
            return (uint32_t)interrupt_handler_150;
        case 151:
            return (uint32_t)interrupt_handler_151;
        case 152:
            return (uint32_t)interrupt_handler_152;
        case 153:
            return (uint32_t)interrupt_handler_153;
        case 154:
            return (uint32_t)interrupt_handler_154;
        case 155:
            return (uint32_t)interrupt_handler_155;
        case 156:
            return (uint32_t)interrupt_handler_156;
        case 157:
            return (uint32_t)interrupt_handler_157;
        case 158:
            return (uint32_t)interrupt_handler_158;
        case 159:
            return (uint32_t)interrupt_handler_159;
        case 160:
            return (uint32_t)interrupt_handler_160;
        case 161:
            return (uint32_t)interrupt_handler_161;
        case 162:
            return (uint32_t)interrupt_handler_162;
        case 163:
            return (uint32_t)interrupt_handler_163;
        case 164:
            return (uint32_t)interrupt_handler_164;
        case 165:
            return (uint32_t)interrupt_handler_165;
        case 166:
            return (uint32_t)interrupt_handler_166;
        case 167:
            return (uint32_t)interrupt_handler_167;
        case 168:
            return (uint32_t)interrupt_handler_168;
        case 169:
            return (uint32_t)interrupt_handler_169;
        case 170:
            return (uint32_t)interrupt_handler_170;
        case 171:
            return (uint32_t)interrupt_handler_171;
        case 172:
            return (uint32_t)interrupt_handler_172;
        case 173:
            return (uint32_t)interrupt_handler_173;
        case 174:
            return (uint32_t)interrupt_handler_174;
        case 175:
            return (uint32_t)interrupt_handler_175;
        case 176:
            return (uint32_t)interrupt_handler_176;
        case 177:
            return (uint32_t)interrupt_handler_177;
        case 178:
            return (uint32_t)interrupt_handler_178;
        case 179:
            return (uint32_t)interrupt_handler_179;
        case 180:
            return (uint32_t)interrupt_handler_180;
        case 181:
            return (uint32_t)interrupt_handler_181;
        case 182:
            return (uint32_t)interrupt_handler_182;
        case 183:
            return (uint32_t)interrupt_handler_183;
        case 184:
            return (uint32_t)interrupt_handler_184;
        case 185:
            return (uint32_t)interrupt_handler_185;
        case 186:
            return (uint32_t)interrupt_handler_186;
        case 187:
            return (uint32_t)interrupt_handler_187;
        case 188:
            return (uint32_t)interrupt_handler_188;
        case 189:
            return (uint32_t)interrupt_handler_189;
        case 190:
            return (uint32_t)interrupt_handler_190;
        case 191:
            return (uint32_t)interrupt_handler_191;
        case 192:
            return (uint32_t)interrupt_handler_192;
        case 193:
            return (uint32_t)interrupt_handler_193;
        case 194:
            return (uint32_t)interrupt_handler_194;
        case 195:
            return (uint32_t)interrupt_handler_195;
        case 196:
            return (uint32_t)interrupt_handler_196;
        case 197:
            return (uint32_t)interrupt_handler_197;
        case 198:
            return (uint32_t)interrupt_handler_198;
        case 199:
            return (uint32_t)interrupt_handler_199;
        case 200:
            return (uint32_t)interrupt_handler_200;
        case 201:
            return (uint32_t)interrupt_handler_201;
        case 202:
            return (uint32_t)interrupt_handler_202;
        case 203:
            return (uint32_t)interrupt_handler_203;
        case 204:
            return (uint32_t)interrupt_handler_204;
        case 205:
            return (uint32_t)interrupt_handler_205;
        case 206:
            return (uint32_t)interrupt_handler_206;
        case 207:
            return (uint32_t)interrupt_handler_207;
        case 208:
            return (uint32_t)interrupt_handler_208;
        case 209:
            return (uint32_t)interrupt_handler_209;
        case 210:
            return (uint32_t)interrupt_handler_210;
        case 211:
            return (uint32_t)interrupt_handler_211;
        case 212:
            return (uint32_t)interrupt_handler_212;
        case 213:
            return (uint32_t)interrupt_handler_213;
        case 214:
            return (uint32_t)interrupt_handler_214;
        case 215:
            return (uint32_t)interrupt_handler_215;
        case 216:
            return (uint32_t)interrupt_handler_216;
        case 217:
            return (uint32_t)interrupt_handler_217;
        case 218:
            return (uint32_t)interrupt_handler_218;
        case 219:
            return (uint32_t)interrupt_handler_219;
        case 220:
            return (uint32_t)interrupt_handler_220;
        case 221:
            return (uint32_t)interrupt_handler_221;
        case 222:
            return (uint32_t)interrupt_handler_222;
        case 223:
            return (uint32_t)interrupt_handler_223;
        case 224:
            return (uint32_t)interrupt_handler_224;
        case 225:
            return (uint32_t)interrupt_handler_225;
        case 226:
            return (uint32_t)interrupt_handler_226;
        case 227:
            return (uint32_t)interrupt_handler_227;
        case 228:
            return (uint32_t)interrupt_handler_228;
        case 229:
            return (uint32_t)interrupt_handler_229;
        case 230:
            return (uint32_t)interrupt_handler_230;
        case 231:
            return (uint32_t)interrupt_handler_231;
        case 232:
            return (uint32_t)interrupt_handler_232;
        case 233:
            return (uint32_t)interrupt_handler_233;
        case 234:
            return (uint32_t)interrupt_handler_234;
        case 235:
            return (uint32_t)interrupt_handler_235;
        case 236:
            return (uint32_t)interrupt_handler_236;
        case 237:
            return (uint32_t)interrupt_handler_237;
        case 238:
            return (uint32_t)interrupt_handler_238;
        case 239:
            return (uint32_t)interrupt_handler_239;
        case 240:
            return (uint32_t)interrupt_handler_240;
        case 241:
            return (uint32_t)interrupt_handler_241;
        case 242:
            return (uint32_t)interrupt_handler_242;
        case 243:
            return (uint32_t)interrupt_handler_243;
        case 244:
            return (uint32_t)interrupt_handler_244;
        case 245:
            return (uint32_t)interrupt_handler_245;
        case 246:
            return (uint32_t)interrupt_handler_246;
        case 247:
            return (uint32_t)interrupt_handler_247;
        case 248:
            return (uint32_t)interrupt_handler_248;
        case 249:
            return (uint32_t)interrupt_handler_249;
        case 250:
            return (uint32_t)interrupt_handler_250;
        case 251:
            return (uint32_t)interrupt_handler_251;
        case 252:
            return (uint32_t)interrupt_handler_252;
        case 253:
            return (uint32_t)interrupt_handler_253;
        case 254:
            return (uint32_t)interrupt_handler_254;
        case 255:
            return (uint32_t)interrupt_handler_255;
        default:
            return (uint32_t)interrupt_handler_0;

    }
}

void format_idt_entry(uint64_t *entry, 
                      uint32_t handler,
                      const unsigned char type, const uint32_t flags)
{
    uint32_t lo_part = 0;
    uint32_t hi_part = 0;

    /*
     * Low part[31;0] = Selector[15;0] Handler[15;0]
     */
    lo_part = (KERNEL_CS << 16) | (handler & 0x0000FFFF);

    /* 
     * High part[7;0] = Handler[31;16] Flags[4;0] Type[4;0] ZERO[7;0]
     */
    hi_part = (handler & 0xFFFF0000) | 
              ((flags & 0xF0) << 8) | ((type & 0x0F) << 8);

    /* Set the value of the entry */
    *entry = lo_part | (((uint64_t) hi_part) << 32);
}

void setup_idt(void)
{
    /* Blank the IDT */
    memset(cpu_idt, 0, sizeof(uint64_t) * IDT_ENTRY_COUNT);

    /* Set interrupt handlers for each interrupt
     * This allows to redirect all interrupts to a global handler in C
     */
    uint32_t i;
    for(i = 0; i < IDT_ENTRY_COUNT; ++i)
    {
        format_idt_entry(&cpu_idt[i],
                         get_handler(i),
                         IDT_TYPE_INT_GATE, IDT_FLAG_PRESENT | IDT_FLAG_PL0);
    }

    /* Set the GDT descriptor */
    cpu_idt_size = ((sizeof(uint64_t) * IDT_ENTRY_COUNT) - 1);
    cpu_idt_base = (uint32_t)&cpu_idt;

    /* Load the GDT */
    __asm__ __volatile__("lidt %0" :: "m" (cpu_idt_size), "m" (cpu_idt_base));

    kernel_success("IDT Initialized at ", 19);
    kernel_print_unsigned_hex(cpu_idt_base, 8);
    kernel_print("\n", 1);    
}