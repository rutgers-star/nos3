/*******************************************************************************
** Purpose:
**   This file implements utility functions used by ADCS.
**
*******************************************************************************/

#include <math.h>
#include "generic_adcs_utilities.h"

/**********************************************************************/
/*  Vector Dot Product                                                */
double VoV(double A[3], double B[3])
{
    return (A[0] * B[0] + A[1] * B[1] + A[2] * B[2]);
}
/**********************************************************************/
/*  Vector Cross Product                                              */
void VxV(double A[3], double B[3], double C[3])
{
    C[0] = A[1] * B[2] - A[2] * B[1];
    C[1] = A[2] * B[0] - A[0] * B[2];
    C[2] = A[0] * B[1] - A[1] * B[0];
}
// Magnitude of v vector
double MAGV(double v[3])
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

/**********************************************************************/
/*  Quaternion product                                                */
void QxQ(double A[4], double B[4], double C[4])
{
    C[0] = A[3] * B[0] + A[2] * B[1] - A[1] * B[2] + A[0] * B[3];
    C[1] = -A[2] * B[0] + A[3] * B[1] + A[0] * B[2] + A[1] * B[3];
    C[2] = A[1] * B[0] - A[0] * B[1] + A[3] * B[2] + A[2] * B[3];
    C[3] = -A[0] * B[0] - A[1] * B[1] - A[2] * B[2] + A[3] * B[3];
}

/**********************************************************************/
/* Find components of V in A, given components of V in B, and qab     */
void QxV(double QAB[4], double Vb[3], double Va[3])
{
    double qq[4][4];
    long   i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = i; j < 4; j++)
            qq[i][j] = QAB[i] * QAB[j];
    }

    Va[0] = (qq[0][0] - qq[1][1] - qq[2][2] + qq[3][3]) * Vb[0] +
            2.0 * ((qq[0][1] + qq[2][3]) * Vb[1] + (qq[0][2] - qq[1][3]) * Vb[2]);
    Va[1] = (-qq[0][0] + qq[1][1] - qq[2][2] + qq[3][3]) * Vb[1] +
            2.0 * ((qq[1][2] + qq[0][3]) * Vb[2] + (qq[0][1] - qq[2][3]) * Vb[0]);
    Va[2] = (-qq[0][0] - qq[1][1] + qq[2][2] + qq[3][3]) * Vb[2] +
            2.0 * ((qq[0][2] + qq[1][3]) * Vb[0] + (qq[1][2] - qq[0][3]) * Vb[1]);
}
/**********************************************************************/
/* Find components of V in B, given components of V in A, and qab     */
void QTxV(double QAB[4], double Va[3], double Vb[3])
{
    double qq[4][4];
    long   i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = i; j < 4; j++)
            qq[i][j] = QAB[i] * QAB[j];
    }

    Vb[0] = (qq[0][0] - qq[1][1] - qq[2][2] + qq[3][3]) * Va[0] +
            2.0 * ((qq[0][1] - qq[2][3]) * Va[1] + (qq[0][2] + qq[1][3]) * Va[2]);
    Vb[1] = (-qq[0][0] + qq[1][1] - qq[2][2] + qq[3][3]) * Va[1] +
            2.0 * ((qq[1][2] - qq[0][3]) * Va[2] + (qq[0][1] + qq[2][3]) * Va[0]);
    Vb[2] = (-qq[0][0] - qq[1][1] + qq[2][2] + qq[3][3]) * Va[2] +
            2.0 * ((qq[0][2] - qq[1][3]) * Va[0] + (qq[1][2] + qq[0][3]) * Va[1]);
}
/**********************************************************************/
/*  Normalize a 3-vector if it is non-zero.                           */
void UNITV(double V[3])
{
    double A;

    A = sqrt(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
    if (A > 0.0)
    {
        V[0] /= A;
        V[1] /= A;
        V[2] /= A;
    }
}
double arccos(double x)
{
    if (x > 1.0)
        x = 1.0;
    if (x < -1.0)
        x = -1.0;

    return (acos(x));
}
/**********************************************************************/
double Limit(double x, double min, double max)
{
    return (x < min ? min : (x > max ? max : x));
}
/**********************************************************************/
/*  Scalar times 3x1 Vector                                           */
void SxV(double S, double V[3], double W[3])
{
    W[0] = S * V[0];
    W[1] = S * V[1];
    W[2] = S * V[2];
}
/**********************************************************************/
/*  Copy and normalize a 3-vector.  Return its magnitude              */
double CopyUnitV(double V[3], double W[3])
{
    double A;

    A = sqrt(V[0] * V[0] + V[1] * V[1] + V[2] * V[2]);
    if (A > 0.0)
    {
        W[0] = V[0] / A;
        W[1] = V[1] / A;
        W[2] = V[2] / A;
    }
    else
    {
        W[0] = 0.0;
        W[1] = 0.0;
        W[2] = 0.0;
    }
    return (A);
}
