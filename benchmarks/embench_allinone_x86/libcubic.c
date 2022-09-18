/* BEEBS cubic benchmark

   This version, copyright (C) 2013-2019 Embecosm Limited and University of
   Bristol

   Contributor: James Pallister <james.pallister@bristol.ac.uk>
   Contributor Jeremy Bennett <jeremy.bennett@embecosm.com>

   This file is part of Embench and was formerly part of the Bristol/Embecosm
   Embedded Benchmark Suite.

   SPDX-License-Identifier: GPL-3.0-or-later

   The original code is from http://www.snippets.org/. */

/* +++Date last modified: 05-Jul-1997 */

/*
 **  CUBIC.C - Solve a cubic polynomial
 **  public domain by Ross Cottrell
 */

#include <math.h>
#include "snipmath.h"

void
SolveCubic (float a, float b, float c, float d, int *solutions, float *x)
{
  float a1 = (float) (b / a);
  float a2 = (float) (c / a);
  float a3 = (float) (d / a);
  float Q = (a1 * a1 - 3.0L * a2) / 9.0L;
  float R = (2.0L * a1 * a1 * a1 - 9.0L * a1 * a2 + 27.0L * a3) / 54.0L;
  float R2_Q3 = (float) (R * R - Q * Q * Q);

  float theta;

  if (R2_Q3 <= 0)
    {
      *solutions = 3;
      theta = acosf (((float) R) / sqrtf ((float) (Q * Q * Q)));
      x[0] = -2.0 * sqrtf ((float) Q) * cosf (theta / 3.0) - a1 / 3.0;
      x[1] =
	-2.0 * sqrtf ((float) Q) * cosf ((theta + 2.0 * PI) / 3.0) - a1 / 3.0;
      x[2] =
	-2.0 * sqrtf ((float) Q) * cosf ((theta + 4.0 * PI) / 3.0) - a1 / 3.0;
    }
  else
    {
      *solutions = 1;
      x[0] = powf (sqrtf (R2_Q3) + fabsf ((float) R), 1 / 3.0);
      x[0] += ((float) Q) / x[0];
      x[0] *= (R < 0.0L) ? 1 : -1;
      x[0] -= (float) (a1 / 3.0L);
    }
}

/* vim: set ts=3 sw=3 et: */


/*
   Local Variables:
   mode: C
   c-file-style: "gnu"
   End:
*/


