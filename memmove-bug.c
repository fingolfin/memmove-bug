#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

typedef void *(*memmove_t)(void *, const void *, size_t);

// naive backwards memmove
void *my_memmove_backwards(void *dest, const void *src, size_t n)
{
    while (n-- > 0) {
        ((char *)dest)[n] = ((const char *)src)[n];
    }
    return dest;
}

void run_test(memmove_t move)
{
    unsigned int seed;  // state of PRNG
    uint32_t i;
    uint32_t *mem;

    const size_t offset = 698706;
    const uint32_t words_to_alloc = (1UL<<29);
    const uint32_t words_to_move = words_to_alloc - offset;
    
    assert(words_to_move+offset <= words_to_alloc);

    printf("allocating %u = 0x%x bytes...\n", 4*words_to_alloc, 4*words_to_alloc);
    mem = malloc(words_to_alloc * 4);
    if (!mem) {
        fprintf(stderr, "ABORT: failed to allocate memory\n");
        exit(1);
    }

    printf("start = %p, end = %p\n", mem, mem + words_to_alloc);
    if (! ((uint32_t)mem < 0x80000000 && 0x80000000 < (uint32_t)(mem + words_to_move + offset)) ) {
        fprintf(stderr, "ABORT: allocated range does not cross 0x80000000 boundary \n");
        exit(1);
    }

    // init with pseudo random data that we can verify later on
    printf("init memory...\n");
    seed = 42;
    for (i = 0; i < words_to_move; i++) {
        mem[i] = rand_r(&seed);
    }

    printf("move memory...\n");
    move(mem + offset, mem, 4*words_to_move);

    printf("check memory...\n");
    seed = 42;
    for (i = 0; i < words_to_move; i++) {
        const uint32_t actual = mem[i+offset];
        const uint32_t expected = rand_r(&seed);
        if (actual != expected) {
            fprintf(stderr, "ABORT: mismatch at offset %u: expected %u but got %u\n", i, expected, actual);
            exit(1);
        }
    }
    
    printf("no problems detected\n");

    free(mem);
}

int main(int argc, char **argv)
{
    if (sizeof(void *) != 4) {
        fprintf(stderr, "ABORT: this test requires a 32-bit system (compile using '-m32' ?)\n");
        exit(1);
    }

    memmove_t move = &memmove;
    if (argc >= 2) {
        if (!strcmp(argv[1], "-m"))
            move = &my_memmove_backwards;
    }

    run_test(move);

    return 0;
}
