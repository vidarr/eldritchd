/**
 * minunit.h taken from http://www.jera.com/techinfo/jtns/jtn002.html
 * License: See web page above. License text reads:
 *
 * You may use the code in this tech note for any purpose,
 * with the understanding that it comes with NO WARRANTY.
 */
#ifndef __MINUNIT_H__
#include <stdio.h>
 #define mu_assert(message, test) do { if (!(test)) return message; } while (0)
 #define mu_run_test(test) do { char *message = test(); tests_run++; \
                                     if (message) return message; } while (0)
 extern int tests_run;

#define TEST_MAIN(testfunc)                \
     int tests_run = 0;                    \
     int main(int argc, char **argv) {     \
         char *result = testfunc();        \
         if (0 != result) {                \
             printf("%s\n", result);       \
         } else {                          \
             printf("ALL TESTS PASSED\n"); \
         }                                 \
         printf("Tests run: %i\n", tests_run); \
         return result != 0;               \
     }

#endif
