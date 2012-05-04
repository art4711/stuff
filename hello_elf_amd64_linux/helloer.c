#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <elf.h>

#define LOADADDR 0x00000000	/* Don't care. */

int
main(int argc, char **argv)
{
	unsigned char code[] = {
		0xb2, 0x0e,                			/* mov    $0xe,%dl - length of string. */
		0x48, 0x8d, 0x35, 0x0a, 0x00,  0x00, 0x00, 	/* lea    0xa(%rip),%rsi - address of string, as long instruction as an absolute load, but allows us to not use absolute addresses which gives more freedom to fiddle. */
		0xb0, 0x01,					/* mov    $0x1,%al - read is syscall 1. */
		0x89, 0xc7,                			/* mov    %eax,%edi - writing to fd 1. */
		0x0f, 0x05,                			/* syscall */
		0xb0, 0x3c,					/* mov    $0x3c,%al - exit is syscall 60. */
		0xeb, 0xfa,					/* jmp    back to syscall - we don't care about the exit code. */
		'H', 'e', 'l', 'l', 'o', ',', ' ', 'w', 'o', 'r', 'l', 'd', '!', '\n'
	};
	Elf64_Ehdr ehdr = {
		.e_ident = { 0x7f, 'E', 'L', 'F', ELFCLASS64, ELFDATA2LSB, EV_CURRENT, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		.e_type = ET_DYN,		/* Can be ET_DYN or ET_EXEC since the code is perfectly relocatable. ET_DYN makes things easier. */
		/* 0x0003 */
		.e_machine = EM_X86_64,
		/* 0x0038 */
		.e_version = 0x00000000,
		/* 0x00000000 - unused */
		.e_entry = LOADADDR + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr),
		/* 0x0000000000000078 */
		.e_phoff = sizeof(Elf64_Ehdr),
		/* 0x0000000000000040 */
		.e_shoff = 0,
		/* 0x0000000000000000 - unused */
		.e_flags = 0,
		/* 0x00000000 - unused */
		.e_ehsize = sizeof(Elf64_Ehdr),
		/* 0x0040 - unused */
		.e_phentsize = sizeof(Elf64_Phdr),
		/* 0x0038 - unused */
		.e_phnum = 1,
		/* 0x0001 - must be 1 */
		.e_shentsize = 0,
		/* 0x0000 - unused */
		.e_shnum = 0,
		/* 0x0000 - unused */
		.e_shstrndx = 0
		/* 0x0000 - unused */
	};
      	Elf64_Phdr phdr = {
		.p_type = PT_LOAD,
		/* 0x00000001 - must be 1 */
		.p_flags = 0x8888880|PF_R|PF_X,
		/* 0x00000003 - can be anything as long as the lowest bit is set. */
		.p_offset = 0,
		/* 0x0000000000000000 - must be 0 */
		.p_vaddr = LOADADDR,
		/* 0x0000000000000000 - must be loadaddr (0) */
		.p_paddr = 0,
		/* 0x0000000000000000 - unused */
		.p_filesz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + sizeof(code),
		.p_memsz = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) + sizeof(code),
		.p_align = 0
	};

	int fd;

	if ((fd = open("a.out", O_CREAT|O_RDWR|O_TRUNC, 0755)) == -1)
		err(1, "open");

	if (write(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
		err(1, "write");
	if (write(fd, &phdr, sizeof(phdr)) != sizeof(phdr))
		err(1, "write");
	if (write(fd, code, sizeof(code)) != sizeof(code))
		err(1, "write");
	
	return 0;
}
