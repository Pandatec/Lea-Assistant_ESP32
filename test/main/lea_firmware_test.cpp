#include "unity.h"

#include <stdio.h>
#include <string.h>

extern "C" {
void app_main(void);
}

static void print_banner(const char *text)
{
    printf("\n#### %s #####\n\n", text);
}

void app_main(void)
{
    print_banner("Starting tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
    print_banner("Done");
}
