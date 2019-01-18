#include <stdio.h>
#include <stepper-interrupt.h>
#include <stepper-move.h>
#include <limits.h>

#if 0
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

static unsigned int
isqrt(unsigned long x)
{
  unsigned int r;
  unsigned int b2 = 0x40000000;
  unsigned int b = 0x8000;
  while(x < b2) {
    b2 >>= 2;
    b >>= 1;
  }
  if (b == 0) return 0;
  r = b;
  b >>= 1;
  while(b > 0) {
    r += b;
    unsigned int t = r*r;
    if (t > x) {
      r -= b;
    }
    b >>=1;
  }
  return r;
}

#define ACC_FIRST_UP 0
#define ACC_K1_UP 1
#define ACC_LAST_UP 2
#define ACC_TOP 3
#define ACC_FIRST_DOWN 4
#define ACC_K1_DOWN 5
#define ACC_LAST_DOWN 6
#define ACC_END 7

typedef struct _AccDiff AccDiff;
struct _AccDiff
{
  long diff;
  unsigned long pos;
};


static inline long
base_acc(unsigned long t,unsigned long n, unsigned long l, unsigned long a_max)
{
  long a;
  if (t >= n) {
    if (t >= n+l) {
      a = -a_max;
    } else {
      a = 0;
    }
  } else {
    a = a_max;
  }
  return a;
}

static AccDiff acc[ACC_END+1];
StepperResult
stepper_move(unsigned int stepper_index, unsigned long *periodp,
	     unsigned long a_max,unsigned long v_max, long s_end)
{
  unsigned long start_period = *periodp;
  unsigned long s;
  unsigned long ds;
  unsigned long l;
  unsigned long da0;
  unsigned long k1 = 0;
  unsigned long n = (v_max+a_max-1)/a_max;
  unsigned long a_speed_adj = v_max - (n-1)*a_max;
  unsigned long s_res;
  long d;
  if (s_end >= 0) {
    s_res = s_end/2;
  } else {
    s_res = (-s_end)/2;
  }
  d = s_res - (long)a_max*(n*n-1) - (long)a_speed_adj;

  acc[ACC_END].diff = 0;
  acc[ACC_END].pos = UINT_MAX;
  if (d < 0) {
    l = 0;
    n = isqrt(s_res/a_max);
    if (n*(unsigned long long)n*a_max < s_res) n++;
    a_speed_adj = a_max;
    acc[ACC_LAST_UP].diff=0;
    acc[ACC_FIRST_DOWN].diff=0;
  } else {
    l = (d+v_max-1)/v_max;
    acc[ACC_LAST_UP].diff= a_speed_adj - a_max;
    acc[ACC_FIRST_DOWN].diff= a_max - a_speed_adj;
  }
  acc[ACC_LAST_UP].pos = n-1;
  acc[ACC_FIRST_DOWN].pos = n+l;
  
  s = a_max*(n*n-1) + a_speed_adj + l * (a_max*(n-1) + a_speed_adj);
  ds = s-s_res;

  da0 = ds/(2*n+l-1);
  acc[ACC_FIRST_UP].diff = -da0;
  acc[ACC_LAST_DOWN].diff = da0;
  acc[ACC_FIRST_UP].pos = 0;
  acc[ACC_LAST_DOWN].pos = 2*n+l-1;
  ds -= da0*(2*n+l-1);
  
  acc[ACC_K1_UP].diff = 0;
  acc[ACC_K1_DOWN].diff = 0;
  acc[ACC_K1_UP].pos = 0;
  acc[ACC_K1_DOWN].pos = 2*n+l-1;

  acc[ACC_TOP].diff = 0;
  acc[ACC_TOP].pos = n;
  
  if (ds > 0) {
    k1 = (2*n+l -ds)/2;
    if (k1 < n) {

      acc[ACC_K1_UP].diff = -1;
      acc[ACC_K1_DOWN].diff = 1;
      acc[ACC_K1_UP].pos = k1;
      acc[ACC_K1_DOWN].pos = 2*n+l-1 - k1;
      ds -= (2*(n-k1)+l-1);
    }
    if (ds > 0) {
      acc[ACC_LAST_UP].diff--;
      acc[ACC_TOP].diff = 1;
      acc[ACC_TOP].pos = n+ds-1;
    }
  }
#if 0
  {
    unsigned int k;
    PRINTF("n=%ld l=%ld a_max=%ld v_max=%ld s_res=%ld\n",
	   n,l ,a_max, v_max, s_res);
    for (k = 0; k < 7; k++) {
      PRINTF("  %ld@%ld", acc[k].diff, acc[k].pos);
    }
    PRINTF("\n");
  }
#endif
  {
    StepperResult res;
    unsigned int k;
    unsigned long t = 0;
    long da = 0;
    long a_prev = ULONG_MAX;
    for (k = 0; k < ACC_END; k++) {
      long a;
      da += acc[k].diff;
      if (acc[k].pos != acc[k+1].pos) { /* Next position is different */
	if (t != acc[k].pos) {
	  a = base_acc(t,n,l,a_max);
	  if (s_end < 0) a = -a;
	  if (a_prev != a) {
	    res = stepper_add_acc(stepper_index, t+start_period, a);
	    if (res != STEPPER_OK) return res;
	    PRINTF("%d: %ld@%ld\n", stepper_index, a, t+start_period);
	    a_prev = a;
	  }
	  t = acc[k].pos;
	}
	a = da + base_acc(t,n,l,a_max);
	if (s_end < 0) a = -a;
	if (a_prev != a) {
	  res = stepper_add_acc(stepper_index, t+start_period, a);
	  if (res != STEPPER_OK) return res;
	  PRINTF("%d: %ld@%ld\n", stepper_index, a, t+start_period);
	  a_prev = a;
	}
	t++;
	da = 0;
      }
    }
    res = stepper_add_acc(stepper_index, t+start_period, 0);
    PRINTF("%d: %d@%ld\n", stepper_index, 0, t+start_period);
    if (res != STEPPER_OK) return res;
    *periodp += t;
  }
  return STEPPER_OK;
}

