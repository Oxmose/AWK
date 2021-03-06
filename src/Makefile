# Noyau
KERNEL = kernel.bin

BOOT_DIR = $(wildcard boot/*.c boot/*.S)
CORE_DIR = $(wildcard core/*.c core/*.S)
CPU_DIR = $(wildcard cpu/*.c cpu/*.S)
DRIVERS_DIR = $(wildcard drivers/*.c drivers/*.S)
LIB_DIR = $(wildcard lib/*.c lib/*.S)
MEM_DIR = $(wildcard memory/*.c memory/*.S)
BIOS_DIR = $(wildcard bios/*.c bios/*.S)
STRLIB_DIR = $(wildcard lib/string/*.c lib/string/*.S)
STDLIB_DIR = $(wildcard lib/stdlib/*.c lib/stdlib/*.S)
STDIO_DIR = $(wildcard lib/stdio/*.c lib/stdio/*.S)
SYNC_DIR = $(wildcard sync/*.c sync/*.S)
COMM_DIR = $(wildcard comm/*.c comm/*.S)
TESTS_DIR = $(wildcard tests/core/*.c tests/core/*.S tests/other/*.c tests/other/*.S)
USER_DIR = $(wildcard user/*.c user/*.S)
GUI_DIR = $(wildcard gui/*.c gui/*.S)

# on compile tous les fichiers assembleur et c du repertoire
SRCS = $(BOOT_DIR) $(CPU_DIR) $(CORE_DIR) $(DRIVERS_DIR) $(LIB_DIR)\
$(STDLIB_DIR) $(STRLIB_DIR) $(SYNC_DIR) $(COMM_DIR) $(TESTS_DIR) $(USER_DIR)\
$(BIOS_DIR) $(MEM_DIR) $(STDIO_DIR) $(GUI_DIR)

# crt0.o doit etre linke en premier
OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRCS)))

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = /usr/bin/qemu-system-i386

CFLAGS = -m32 -g -gstabs -O0 -std=c99 -nostdinc -fno-builtin\
         -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -Wall\
		 -Wextra -Werror -Wimplicit-fallthrough=0 -c
ASFLAGS = -f elf -w+gnu-elf-extensions
LDFLAGS = -e loader -T linker.ld -melf_i386
QEMUOPTS = -cpu core2duo -d guest_errors -rtc base=localtime -m 128M\
           -gdb tcp::1234 -vga std -smp 4\
           -hda ../hdd.img -kernel

# cible principale, on nettoie systematiquement le repertoire avant
.PHONY: all
all: clean kernel.bin

# generation du noyau
kernel.bin: linker.ld $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL)

%.o: %.c
	$(CC) $(CFLAGS)  $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

clean:
	$(RM) $(OBJS) $(KERNEL)

run:
	$(QEMU) $(QEMUOPTS) $(KERNEL) -serial stdio

debug:
	$(QEMU) $(QEMUOPTS) $(KERNEL) -S

bootable: all
	cp $(KERNEL) ../iso/boot/
	rm -f ../iso/bootable.iso
	grub-mkrescue -o ../iso/bootable.iso ../iso
