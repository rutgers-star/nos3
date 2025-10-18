/* Copyright (C) 2015 - 2025 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

#ifndef NOS3_SIMSHMEMDATAPOINT_HPP
#define NOS3_SIMSHMEMDATAPOINT_HPP

#include <sim_i_data_point.hpp>

namespace Nos3
{

    /** \brief Class to contain an entry of 42 simulation data.
     *
     */
    class SimShmemDataPoint : public SimIDataPoint
    {
    public:
        SimShmemDataPoint(double svb[3], double bvb[3], double Hvb[3], double GyroRate[3], int CSSValid[6], double CSSIllum[6], int FSSValid, 
                          double FSSSunAng[2], int STValid, double STqn[4], double GPSPosN[3], double GPSVelN[3], double AccelAcc[3], double WhlH[3]);
        double* get_svb() {return _svb;}
        double* get_bvb() {return _bvb;}
        double* get_Hvb() {return _Hvb;}
        double* get_GyroRate() {return _GyroRate;}
        int*    get_CSSValid() {return _CSSValid;}
        double* get_CSSIllum() {return _CSSIllum;}
        int      get_FSSValid() {return _FSSValid;}
        double* get_FSSSunAng() {return _FSSSunAng;}
        int      get_STValid() {return _STValid;}
        double* get_STqn() {return _STqn;}
        double* get_GPSPosN() {return _GPSPosN;}
        double* get_GPSVelN() {return _GPSVelN;}
        double* get_AccelAcc() {return _AccelAcc;}
        double* get_WhlH() {return _WhlH;}
        std::string to_string(void) const {std::string ret("SimShmemDataPoint"); return ret;}
    protected:
    private:
        double _svb[3];
        double _bvb[3];
        double _Hvb[3];
        double _GyroRate[3];
        int    _CSSValid[6];
        double _CSSIllum[6];
        int    _FSSValid;
        double _FSSSunAng[2];
        int    _STValid;
        double _STqn[4];
        double _GPSPosN[3];
        double _GPSVelN[3];
        double _AccelAcc[3];
        double _WhlH[3];
    };
}

#endif
