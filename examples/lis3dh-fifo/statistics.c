/*
 * Copyright (c) 2014, Eistec AB.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Some statistics functions for testing sensor noise.
 * \author
 *         Joakim Gebart <joakim.gebart@eistec.se>
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "statistics.h"

void
compute_stats_3d_int16(const int16_t *buffer, const size_t count,
                       stats_3d_t *stats)
{
  static const uint8_t dimension = 3;
  unsigned int i, j, v, u;

  for(i = 0; i < dimension; ++i) {
    stats->mean[i] = 0.0;
    stats->sum[i] = 0.0;
    stats->stddev[i] = 0.0;
    stats->max[i] = INT16_MIN;
    stats->min[i] = INT16_MAX;
    for(j = 0; j < dimension; ++j) {
      stats->cov[i][j] = 0.0;
    }
  }
  stats->count = count;
  for(v = 0; v < dimension; ++v) {
    /* Find min, max and compute sum */
    for(i = 0; i < count; ++i) {
      stats->max[v] = MAX(buffer[i * dimension + v], stats->max[v]);
      stats->min[v] = MIN(buffer[i * dimension + v], stats->min[v]);
      stats->sum[v] += (double)buffer[i * dimension + v];
    }
    /* Compute mean */
    stats->mean[v] = stats->sum[v] / ((double)count);
    /* Compute covariance */
    for(u = 0; u <= v; ++u) {
      for(i = 0; i < count; ++i) {
        stats->cov[v][u] +=
          ((double)buffer[i * dimension + v] -
           stats->mean[v]) * ((double)buffer[i * dimension + u] -
                              stats->mean[u]);
      }
      /* Divide by sample count, minus one. */
      stats->cov[v][u] /= ((double)count - 1.0);
      /* Symmetry */
      stats->cov[u][v] = stats->cov[v][u];
    }
    stats->stddev[v] = sqrt(stats->cov[v][v]);
  }
}
void
print_stats_3d(const stats_3d_t *stats)
{
  printf("Stats, %d samples:\r\n", stats->count);
  /* Poor man's floating point printf... */
  printf("Xlim: [%d,\t%d]\r\n", stats->min[0], stats->max[0]);
  printf("Ylim: [%d,\t%d]\r\n", stats->min[1], stats->max[1]);
  printf("Zlim: [%d,\t%d]\r\n", stats->min[2], stats->max[2]);
  printf(" max: [%d,\t%d,\t%d]\r\n",
         stats->max[0], stats->max[1], stats->max[2]);
  printf(" min: [%d,\t%d,\t%d]\r\n",
         stats->min[0], stats->min[1], stats->min[2]);
  printf("mean: [%d.%06d,\t%d.%06d,\t%d.%06d]\r\n",
         ((int)stats->mean[0]),
         ((int)(fmod(fabs(stats->mean[0] * 1000000.0), 1000000.0))),
         ((int)stats->mean[1]),
         ((int)(fmod(fabs(stats->mean[1] * 1000000.0), 1000000.0))),
         ((int)stats->mean[2]),
         ((int)(fmod(fabs(stats->mean[2] * 1000000.0), 1000000.0))));
  printf(" cov: [%d.%06d,\t%d.%06d,\t%d.%06d]\r\n"
         "      [%d.%06d,\t%d.%06d,\t%d.%06d]\r\n"
         "      [%d.%06d,\t%d.%06d,\t%d.%06d]\r\n",
         ((int)stats->cov[0][0]),
         ((int)(fmod(fabs(stats->cov[0][0] * 1000000.0), 1000000.0))),
         ((int)stats->cov[0][1]),
         ((int)(fmod(fabs(stats->cov[0][1] * 1000000.0), 1000000.0))),
         ((int)stats->cov[0][2]),
         ((int)(fmod(fabs(stats->cov[0][2] * 1000000.0), 1000000.0))),
         ((int)stats->cov[1][0]),
         ((int)(fmod(fabs(stats->cov[1][0] * 1000000.0), 1000000.0))),
         ((int)stats->cov[1][1]),
         ((int)(fmod(fabs(stats->cov[1][1] * 1000000.0), 1000000.0))),
         ((int)stats->cov[1][2]),
         ((int)(fmod(fabs(stats->cov[1][2] * 1000000.0), 1000000.0))),
         ((int)stats->cov[2][0]),
         ((int)(fmod(fabs(stats->cov[2][0] * 1000000.0), 1000000.0))),
         ((int)stats->cov[2][1]),
         ((int)(fmod(fabs(stats->cov[2][1] * 1000000.0), 1000000.0))),
         ((int)stats->cov[2][2]),
         ((int)(fmod(fabs(stats->cov[2][2] * 1000000.0), 1000000.0)))
         );
  printf("stdd: [%d.%06d,\t%d.%06d,\t%d.%06d]\r\n",
         ((int)stats->stddev[0]),
         ((int)(fmod(fabs(stats->stddev[0] * 1000000.0), 1000000.0))),
         ((int)stats->stddev[1]),
         ((int)(fmod(fabs(stats->stddev[1] * 1000000.0), 1000000.0))),
         ((int)stats->stddev[2]),
         ((int)(fmod(fabs(stats->stddev[2] * 1000000.0), 1000000.0))));
  printf(" sum: [%d,\t%d,\t%d]\r\n",
         (int)stats->sum[0], (int)stats->sum[1], (int)stats->sum[2]);
}
