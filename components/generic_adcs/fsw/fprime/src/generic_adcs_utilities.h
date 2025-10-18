/*******************************************************************************
** Purpose:
**   This file has utility functions used by ADCS.
**
*******************************************************************************/
#ifndef _GENERIC_ADCS_UTILITIES_H_
#define _GENERIC_ADCS_UTILITIES_H_

double VoV(double A[3], double B[3]);
void   VxV(double A[3], double B[3], double C[3]);
double MAGV(double v[3]);
void   QxQ(double A[4], double B[4], double C[4]);
void   QxV(double QAB[4], double Vb[3], double Va[3]);
void   QTxV(double QAB[4], double Va[3], double Vb[3]);
void   UNITV(double V[3]);
double arccos(double x);
double Limit(double x, double min, double max);
void   SxV(double S, double V[3], double W[3]);
double CopyUnitV(double V[3], double W[3]);

#endif
