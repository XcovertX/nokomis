#include <cstddef>

typedef struct resource_a_t resource_a_t;
typedef struct resource_b_t resource_b_t;
typedef struct resource_c_t resource_c_t;
typedef struct result_t result_t;

int do_task(result_t *result)
{
    resource_a_t *this_a = NULL;
    resource_b_t *this_b = NULL;
    resource_c_t *this_c = NULL;
    int ecode = 0;
    int ret = -1;

    // Try to acquire resource A
    if (acquire_a(&this_a) != 0 || !this_a)
        goto cleanup;

    // Try to acquire resource B
    this_b = acquire_b(this_a);
    if (!this_b)
        goto cleanup;

    // Try to acquire resource C
    acquire_c(this_a, &this_c, &ecode);
    if (ecode != 0 || !this_c)
        goto cleanup;

    // All resources acquired, perform subtask
    if (do_subtask(this_a, this_b, this_c, result) != 0)
        goto cleanup;

    ret = 0; // Success

cleanup:
    if (this_c)
        release_c(this_c);
    if (this_b)
        release_b(this_b);
    if (this_a)
        release_a(this_a);
    return ret;
}