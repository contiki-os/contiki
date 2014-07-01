/**
 * \file
 *         ECC acumulator performance evaluation using
 *         the flash memory
 * \author
 *         Andre Naz <andre.naz@ensimag.imag.fr>
 */

#include "ecc.h"
#include "owaecc.h"
#include "contiki.h"
#include "lib/random.h"
#include "sys/clock.h"
#include <stdio.h>              /* For printf() */
#include <string.h>


#define NB_RUNS 1
#define NB_VALID_KEYS 15
#define NB_INVALID_KEYS 3
#define NB_KEYS (NB_VALID_KEYS + NB_INVALID_KEYS)

static  point_t pb_key[NB_KEYS];
static  NN_DIGIT pr_key[NUMWORDS];
static point_t partial_acc[NB_KEYS];
static point_t full_acc;
/*---------------------------------------------------------------------------*/
PROCESS(startup_process, "Statup Process");
AUTOSTART_PROCESSES(&startup_process);
/*---------------------------------------------------------------------------*/
static void
display_digit(NN_DIGIT * digit)
{
  int8_t i;

  //printf("%u, %u \n", sizeof(unsigned int), sizeof(NN_DIGIT));
  for(i = NUMWORDS - 1; i > -1; i--) {
    printf("%x ", digit[i]);
  }
 // printf("%u, %u \n", sizeof(unsigned int), sizeof(NN_DIGIT));
}
/*---------------------------------------------------------------------------*/
static void
random_point(point_t * point)
{
  uint8_t i;

  for(i = 0; i < NUMWORDS - 1; i++) {
    point->x[i] = random_rand();
    point->y[i] = random_rand();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(startup_process, ev, data)
{
  uint8_t run = 0;
  uint16_t nb_verify = 0;
  uint16_t nb_partial_acc = 0;
  uint16_t nb_failure = 0;
  uint16_t nb_false_positive = 0;
  unsigned init_time;
  unsigned partial_acc_time;
  unsigned verify_time;
  unsigned cumulated_init_time = 0;
  unsigned cumulated_partial_acc_time = 0;
  unsigned cumulated_verify_time = 0;
  unsigned global_time;
  uint8_t i = 0, j = 0;
  unsigned global_time_d;
  unsigned average_init_time;
  unsigned average_partial_acc_time;
  unsigned average_verify_time;
  unsigned false_positive_rate;
  //unsigned_long average_init_energy;

  PROCESS_BEGIN();
 // printf("%u, %u \n", sizeof(unsigned int), sizeof(NN_DIGIT));
  global_time = clock_seconds();
  for(run = 0; run < NB_RUNS; run++) {
    printf("ROUND %d:\n", run);
    /* Initialize ecc. */
    ecc_init();
    printf("ecc init done.\n");
    /* Generate keys */
    printf("key generations...\n");
    init_time = clock_seconds();
    for(i = 0; i < NB_KEYS; i++) {
      ecc_gen_private_key(pr_key);
      ecc_gen_public_key(&pb_key[i], pr_key);
      display_digit(pb_key[i].x);
      printf("\n");
      display_digit(pb_key[i].y);
      printf("\n");
    }
    printf("keys generated\n");
    /* Compute the intial value of the accumulator */
    printf("computing accumulator initial value...\n");
    point_t initial_acc;

    //random_init(time(NULL));
    ecc_gen_private_key(pr_key);
    ecc_gen_public_key(&initial_acc, pr_key);
    printf("accumulator initial value.\n");
    /* Compute fully accumulated value */
    p_copy(&full_acc, &initial_acc);
    
    for(i = 0; i < NB_VALID_KEYS; i++) {
      owaecc_add_key(&full_acc, &pb_key[i]);
    }
    cumulated_init_time += (clock_seconds() - init_time);

    printf("full accumulator computed:\n");
    display_digit(full_acc.x);
    printf("\n");
    display_digit(full_acc.y);
    printf("\n");
    for(i = 0; i < NB_VALID_KEYS; i++) {
      p_copy(&partial_acc[i], &initial_acc);
      partial_acc_time = clock_seconds();
      for(j = 0; j < NB_VALID_KEYS; j++) {
        if(i != j) {
          owaecc_add_key(&partial_acc[i], &pb_key[j]);
        }
      }
      cumulated_partial_acc_time += clock_seconds() - partial_acc_time;
      nb_partial_acc++;
	}
      for(j = NB_VALID_KEYS; j < NB_KEYS; j++) {
        random_point(&partial_acc[j]);
      }
    printf("partial accumulators computed.\n");
    printf("verification tests: \n");
    for(i = 0; i < NB_KEYS; i++) {
      verify_time = clock_seconds();
      if(owaecc_check_key(&pb_key[i], &partial_acc[i], &full_acc) == 1) {
        cumulated_verify_time += clock_seconds() - verify_time;
        printf("user %d is valid\n", i);
      } else {
        printf("user %d is invalid\n", i);
        cumulated_verify_time += clock_seconds() - verify_time;
        nb_failure++;
      }
      nb_verify++;
    }
    printf("\n\n");
  }

  global_time_d = clock_seconds() - global_time;
  nb_false_positive = (NB_INVALID_KEYS * NB_RUNS) - nb_failure;
  average_init_time = cumulated_init_time / NB_RUNS;
  average_partial_acc_time = cumulated_partial_acc_time / nb_partial_acc;
  average_verify_time = cumulated_verify_time / nb_verify;
  false_positive_rate =
    (unsigned)(((double)nb_false_positive /
                (double)(NB_INVALID_KEYS * NB_RUNS)) * 100);

  printf("GLOBAL TEST REPORT:\n");
  printf("TIME:\n");
  printf("Number of runs %d in %u seconds\n", NB_RUNS, global_time_d);
  printf("Cumulated init time: %u seconds\n", cumulated_init_time);
  printf("Average init time: %u seconds\n", average_init_time);
  printf("Number of valid partial accumulator calculated: %d\n",
         nb_partial_acc);
  printf("Cumulated partial accumulator computation time: %u seconds\n",
         cumulated_partial_acc_time);
  printf("Average partial accumulator computation time: %u seconds\n",
         average_partial_acc_time);
  printf("Number of accumulator verification: %d\n", nb_verify);
  printf("Cumulated verification time: %u seconds\n", cumulated_verify_time);
  printf("Average verification time: %u\n", average_verify_time);
  printf("Number of verification failures: %d out of %d expected.\n",
         nb_failure, (NB_INVALID_KEYS * NB_RUNS));
  printf("Number of false positives: %d out of %d (rate: %u %%)\n",
         nb_false_positive, NB_INVALID_KEYS * NB_RUNS, false_positive_rate);

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
