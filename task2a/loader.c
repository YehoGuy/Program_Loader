#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

// !!! change in line 45: func(&ph_table[i], i) --> func(&ph_table[i], arg)
// and in makefile linking

/*
 * Function: foreach_phdr
 * ----------------------
 * Iterates over the program headers of an ELF file and applies a given function to each header.
 *
 * Parameters:
 *   map_start - Pointer to the start of the memory-mapped ELF file.
 *   func      - Function to be applied to each program header.
 *   arg       - Additional argument to pass to the function.
 *
 * Returns:
 *   0 on success, -1 on failure.
 */
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start; // ELF header at the start of the mapped file

    // Verify ELF magic number
    // check that the first 4 bytes of the file
    // match the ELF identification magic numbers: 0x7f, 'E', 'L', 'F'
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 ||
        elf_header->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header->e_ident[EI_MAG2] != ELFMAG2 ||
        elf_header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Error: Not a valid ELF file.\n");
        return -1;
    }

    // Get program header table information
    // elf_header->e_phoff gives the offset (in bytes) from the start of the file to the program header table.
    Elf32_Phdr *ph_table = (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);
    int ph_count = elf_header->e_phnum;

    // Iterate over each program header and apply the provided function
    for (int i = 0; i < ph_count; i++) {
        func(&ph_table[i], arg); // Apply the function to the current program header
    }

    return 0;
}

/*
 * Function: print_phdr_with_flags
 * -------------------------------
 * Prints detailed information about a program header along with mmap protection
 * and mapping flags.
 *
 * Parameters:
 *   phdr - Pointer to the current program header.
 *   index - Additional argument (not used here).
 */
void print_phdr_with_flags(Elf32_Phdr *phdr, int index) {
    const char *type;
    switch (phdr->p_type) {
        case PT_NULL: type = "NULL"; break;
        case PT_LOAD: type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP: type = "INTERP"; break;
        case PT_NOTE: type = "NOTE"; break;
        case PT_PHDR: type = "PHDR"; break;
        default: printf("Unknown program header type: 0x%x\n", phdr->p_type); type = "UNKNOWN"; break;
    }

    // construct permission flag string
    // using bitwise-and (&): If the bit corresponding to PF_R/W/X is 
    // set the result of the expression is non-zero.
    char flags[4] = "---";
    if (phdr->p_flags & PF_R) flags[0] = 'R'; // Readable
    if (phdr->p_flags & PF_W) flags[1] = 'W'; // Writable
    if (phdr->p_flags & PF_X) flags[2] = 'E'; // Executable

    // Determine mmap protection flags
    // using bitwise-or (|): If any of the bits corresponding to PROT_READ/WRITE/EXEC is
    // set, it is included in the result.
    int prot_flags = 0;
    if (phdr->p_flags & PF_R) prot_flags |= PROT_READ;
    if (phdr->p_flags & PF_W) prot_flags |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot_flags |= PROT_EXEC;

    // Determine mmap mapping flags (default: private mapping)
    // Each process works with its private copy of the file's contents in memory. 
    // If the process modifies a mapped page, a copy is created for that process only
    int map_flags = MAP_PRIVATE;

    printf("%-8s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %-3s 0x%x PROT=0x%x MAP=0x%x\n",
           type, phdr->p_offset, phdr->p_vaddr, phdr->p_paddr,
           phdr->p_filesz, phdr->p_memsz, flags, phdr->p_align,
           prot_flags, map_flags);
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    int fd = open(filename, O_RDONLY); // Open the ELF file
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    // Get the file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        close(fd);
        return 1;
    }

    // Map the file into memory
    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map_start == MAP_FAILED) {
        perror("Error mapping file");
        close(fd);
        return 1;
    }

    // Print the header
    printf("Type     Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align  PROT     MAP\n");

    // Iterate over the program headers
    if (foreach_phdr(map_start, print_phdr_with_flags, 0) < 0) {
        fprintf(stderr, "Error iterating over program headers.\n");
        munmap(map_start, file_size);
        close(fd);
        return 1;
    }

    // Clean up
    munmap(map_start, file_size);
    close(fd);

    return 0;
}
