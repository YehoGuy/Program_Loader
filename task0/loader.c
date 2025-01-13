/*
 * Task 0 - Program Header Iterator for ELF Files
 * Author: [Your Name]
 * Description: This program accepts a 32-bit ELF file as a command-line argument,
 * maps it into memory, and iterates over its program headers using a custom iterator function.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <elf.h>

/*
 * Function: foreach_phdr
 * ----------------------
 * Iterates over the program headers of an ELF file and applies a given function to each header.
 *
 * Parameters:
 *   map_start - Pointer to the start of the memory-mapped ELF file.
 *   func      - Function to be applied to each program header.
 *   arg       - Additional argument to pass to the function (not used in this task).
 *
 * Returns:
 *   0 on success, -1 on failure.
 */
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start; // ELF header at the start of the mapped file

    // Verify ELF magic number
    if (elf_header->e_ident[EI_MAG0] != ELFMAG0 ||
        elf_header->e_ident[EI_MAG1] != ELFMAG1 ||
        elf_header->e_ident[EI_MAG2] != ELFMAG2 ||
        elf_header->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Error: Not a valid ELF file.\n");
        return -1;
    }

    // Get program header table information
    Elf32_Phdr *ph_table = (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);
    int ph_count = elf_header->e_phnum;

    // Iterate over each program header and apply the provided function
    for (int i = 0; i < ph_count; i++) {
        func(&ph_table[i], i); // Apply the function to the current program header
    }

    return 0;
}

/*
 * Function: print_phdr_info
 * -------------------------
 * Prints information about a program header.
 *
 * Parameters:
 *   phdr - Pointer to the current program header.
 *   index - Index of the program header.
 */
void print_phdr_info(Elf32_Phdr *phdr, int index) {
    printf("Program header number %d at address %p\n", index, (void *)phdr);
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

    // Iterate over the program headers
    if (foreach_phdr(map_start, print_phdr_info, 0) < 0) {
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
