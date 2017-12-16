# Noyau
KERNEL = kernel.bin

CORE_DIR = $(wildcard core/*.c core/*.S) 
CPU_DIR = $(wildcard cpu/*.c cpu/*.S) 
DRIVERS_DIR = $(wildcard drivers/*.c drivers/*.S)
LIB_DIR = $(wildcard lib/*.c lib/*.S)
SYNC_DIR = $(wildcard sync/*.c sync/*.S)

# on compile tous les fichiers assembleur et c du repertoire
SRCS = $(CORE_DIR) $(CPU_DIR) $(DRIVERS_DIR) $(LIB_DIR) $(SYNC_DIR)

# crt0.o doit etre linke en premier
OBJS = $(patsubst %.S,%.o,$(patsubst %.c,%.o,$(SRCS)))

CC = gcc
AS = nasm
LD = ld
OBJCOPY = objcopy
QEMU = /usr/bin/qemu-system-i386

CFLAGS = -m32 -g -gstabs -std=c99 -nostdinc -fno-builtin -nostdlib -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Werror -Wimplicit-fallthrough=0 -c
ASFLAGS = -f elf -w+gnu-elf-extensions 
LDFLAGS = -e loader -T linker.ld -melf_i386
QEMUOPTS = -cpu core2duo -rtc base=localtime -m 64M -gdb tcp::1234 -kernel

# cible principale, on nettoie systematiquement le repertoire avant
.PHONY: all
all: kernel.bin

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
	$(QEMU) $(QEMUOPTS) $(KERNEL)

debug:
	$(QEMU) $(QEMUOPTS) -S

bootable: all
	cp $(KERNEL) ../iso/boot/
	grub2-mkrescue -o ../iso/bootable.iso ../iso