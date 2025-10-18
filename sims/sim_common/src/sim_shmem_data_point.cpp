/* Copyright (C) 2015 - 2017 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

   This software is provided "as is" without any warranty of any, kind either express, implied, or statutory, including, but not
   limited to, any warranty that the software will conform to, specifications any implied warranties of merchantability, fitness
   for a particular purpose, and freedom from infringement, and any warranty that the documentation will conform to the program, or
   any warranty that the software will be error free.

   In no event shall NASA be liable for any damages, including, but not limited to direct, indirect, special or consequential damages,
   arising out of, resulting from, or in any way connected with the software or its documentation.  Whether or not based upon warranty,
   contract, tort or otherwise, and whether or not loss was sustained from, or arose out of the results of, or use of, the software,
   documentation or services provided hereunder

   ITC Team
   NASA IV&V
   ivv-itc@lists.nasa.gov
*/

#include <iomanip>
#include <limits>

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <ItcLogger/Logger.hpp>

#include <sim_shmem_data_point.hpp>

namespace Nos3
{

    extern ItcLogger::Logger *sim_logger;

    /*************************************************************************
     * Constructors
     *************************************************************************/
    SimShmemDataPoint::SimShmemDataPoint(double svb[3], double bvb[3], double Hvb[3], double GyroRate[3], int CSSValid[6], double CSSIllum[6], int FSSValid, 
        double FSSSunAng[2], int STValid, double STqn[4], double GPSPosN[3], double GPSVelN[3], double AccelAcc[3], double WhlH[3])
    {
        _svb[0] = svb[0];
        _svb[1] = svb[1];
        _svb[2] = svb[2];
        _bvb[0] = bvb[0];
        _bvb[1] = bvb[1];
        _bvb[2] = bvb[2];
        _Hvb[0] = Hvb[0];
        _Hvb[1] = Hvb[1];
        _Hvb[2] = Hvb[2];
        _GyroRate[0] = GyroRate[0];
        _GyroRate[1] = GyroRate[1];
        _GyroRate[2] = GyroRate[2];
        _CSSValid[0] = CSSValid[0];
        _CSSValid[1] = CSSValid[1];
        _CSSValid[2] = CSSValid[2];
        _CSSValid[3] = CSSValid[3];
        _CSSValid[4] = CSSValid[4];
        _CSSValid[5] = CSSValid[5];
        _CSSIllum[0] = CSSIllum[0];
        _CSSIllum[1] = CSSIllum[1];
        _CSSIllum[2] = CSSIllum[2];
        _CSSIllum[3] = CSSIllum[3];
        _CSSIllum[4] = CSSIllum[4];
        _CSSIllum[5] = CSSIllum[5];
        _FSSValid = FSSValid;
        _FSSSunAng[0] = FSSSunAng[0];
        _FSSSunAng[1] = FSSSunAng[1];
        _STValid = STValid;
        _STqn[0] = STqn[0];
        _STqn[1] = STqn[1];
        _STqn[2] = STqn[2];
        _STqn[3] = STqn[3];
        _GPSPosN[0] = GPSPosN[0];
        _GPSPosN[1] = GPSPosN[1];
        _GPSPosN[2] = GPSPosN[2];
        _GPSVelN[0] = GPSVelN[0];
        _GPSVelN[1] = GPSVelN[1];
        _GPSVelN[2] = GPSVelN[2];
        _AccelAcc[0] = AccelAcc[0];
        _AccelAcc[1] = AccelAcc[1];
        _AccelAcc[2] = AccelAcc[2];
        _WhlH[0] = WhlH[0];
        _WhlH[1] = WhlH[1];
        _WhlH[2] = WhlH[2];
    }
}