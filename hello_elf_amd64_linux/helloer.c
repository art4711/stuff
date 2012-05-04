#include <unistd.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <err.h>
#include <elf.h>


#define MESSAGE "hello, world\n"
#define MESSAGE_SZ (sizeof(MESSAGE) - 1)

/*
 * Smallest possible (?) proper hello world for ELF amd64 linux.
 *
 * The code itself is:
 *	movb $13, %dl
 *	lea 2f(%rip), %rsi
 *	inc %eax
 *	mov %eax, %edi
 * 1:	syscall
 *	movb $60, %al
 *	jmp 1b
 * 2:  	.string	"hello, world\n"
 * 
 * The only wrong thing is the exit code.
 *
 * We depend on the kernel zeroing registers for us, but that should be a safe assumption
 * because the kernel doesn't want to leak registers to userland anyway (unless there's some
 * ABI I'm not aware of).
 *
 * The code is stuffed into unused fields in the ELF headers with relative jumps gluing it
 * together. The string is stuffed into the nicely contiguous space provided to us by e_shoff
 * (section headers are irrelevant for executables), e_flags and e_ehsize, that just perfectly
 * fit the One True Hello World Message.
 *
 * The program header overlaps with the elf header with some trickery involving a good guess
 * about what the kernel actually does with p_flags (see comment for e_unused4).
 *
 * I am not sure since when the Linux kernel supports ET_DYN binaries. Maybe it always had?
 * ET_DYN allows us to pull off a horrible horrible trick (see comment for e_unused1).
 * This wouldn't work without ET_DYN unless we can find some way to load the address of the
 * message into rsi with some shorter instruction.
 */

#define OVERLAP

int
main(int argc, char **argv)
{
#define RIP_OFFSET(from_field, to_field) ((char *)&blob.to_field - (char *)&blob.from_field)
	struct blob {
		unsigned char e_ident[8];
		unsigned char e_unused1[8];
		uint16_t e_type;
		uint16_t e_machine;
		unsigned char e_unused2[4];		/* e_version */
		uint64_t e_entry;
		uint64_t e_phoff;
		unsigned char e_unused3[8 + 4 + 2];	/* e_shoff + e_flags + e_ehsize */
		uint16_t e_phentsize;
		uint16_t e_phnum;
		unsigned char e_unused4[2 + 2 + 2];	/* e_shentsize + e_shnum + e_shstrndx */

#ifndef OVERLAP
		uint32_t p_type;
		uint32_t p_flags;
#else
#define p_type e_phnum
#endif
		uint64_t p_offset;
		uint64_t p_vaddr;
		unsigned char p_unused1[8];
		uint64_t p_filesz;
		uint64_t p_memsz;
		uint64_t p_align;
	} __attribute__((__packed__)) blob = {
		.e_ident = { 0x7f, 'E', 'L', 'F', ELFCLASS64, ELFDATA2LSB, EV_CURRENT, 0x00 },
		.e_unused1 = {
			/*
			 * First 8 bytes of e_ident must be what they are. The next 8 bytes
			 * are free for all so we make that our entry point.  We use
			 * "lea MESSAGEOFFSET(%rip), %rsi"  to load the address for write(2).
			 * lea is a 7 byte instruction. The 8th byte is a relative jump
			 * instruction with the jump offset conveniently provided to us by
			 * e_type, which is ET_DYN = 3. This lets us skip e_machine and
			 * execute the next instructions from e_version.
			 */
			0x48, 0x8d, 0x35, RIP_OFFSET(e_unused1[7], e_unused3), 0x00, 0x00, 0x00,
			0xeb
		},
		.e_type = ET_DYN,
		/* 0x03, 0x00 */
		.e_machine = EM_X86_64,
		/* 0x3e, 0x00 */
		.e_unused2 = {
			/*
			 * mov $0xe,%dl   - length of string for write(2)
			 * jmp <to rest of code>
			 */
			0xb2, MESSAGE_SZ, 				
			0xeb, RIP_OFFSET(e_unused2[4], e_unused4[2])
		},
		.e_entry = 8,
		.e_phoff = ((char *)&blob.p_type - (char *)&blob),
		.e_unused3 = MESSAGE,
		.e_phentsize = sizeof(Elf64_Phdr),
		.e_phnum = 1,
		/* 0x01 0x00 */
		.e_unused4 = {
			0x00, 0x00, 		/* Must be zero if OVERLAP */
			0xff, 0xc0,		/* inc %eax - syscall number for read is 1. */
			/* 
			 * The original code was:
			 *  mov $1, %al
			 *  mov %eax, %edi
			 * This set up the syscall number to 1 (write) and fd to 1 (stdout)
			 * and could be done in 4 bytes. But we can't use mov here, registers
			 * after exec come pre-zeroed, so we inc %eax instead. This is because
			 * the mov instruction is 0xb0, but we need the lowest bit of it set so
			 * we can make this and the phdr overlap.
			 *
			 * The lowest bit must be set because it overlaps with the p_flags in
			 * phdr and that specifies that the loaded region is executable. We don't
			 * need to specify readability (even though we do read from the region
			 * since the message is in it) because amd64 doesn't have an executable
			 * but not readable PTE. Unfortunately "inc" also makes the region
			 * writeable, so we violate W^X. Looking for a better instruction.
			 */
			0xeb, RIP_OFFSET(e_unused4[6], p_unused1)
		},
#ifndef OVERLAP
		/* This can overlap with .e_phnum. */
		.p_type = PT_LOAD,
		/* 0x01 0x00 0x00 0x00 - must be this */
		.p_flags = PF_R|PF_X,
		/* 0x03 0x00 0x00 0x00 - can be anything as long as the lowest bit is set. */
#endif
		.p_offset = 0,
		/* 0x0000000000000000 - must be 0 */
		.p_vaddr = 0,
		/* 0x0000000000000000 - must be 0 */
		.p_unused1 = {
			/*
			 * mov %eax, %edi
			 * syscall
			 * mov $0x3c, %al
			 * jmp <back to the syscall instruction>
			 */
			0x89, 0xc7,
			0x0f, 0x05,
			0xb0, 0x3c,
			0xeb, 0xfa,
		},
		.p_filesz = sizeof(blob),
		.p_memsz = sizeof(blob),
		.p_align = 0,
	};

	int fd;

	if ((fd = open("a.out", O_CREAT|O_RDWR|O_TRUNC, 0755)) == -1)
		err(1, "open");

	if (write(fd, &blob, sizeof(blob)) != sizeof(blob))
		err(1, "write");
	
	return 0;
}
