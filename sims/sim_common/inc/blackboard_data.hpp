#ifndef NOS3_BLACKBOARDDATA_HPP
#define NOS3_BLACKBOARDDATA_HPP

/*
** Includes
*/
#include <boost/interprocess/sync/interprocess_mutex.hpp>

/*
** Defines
*/

/*
** Namespace
*/
namespace Nos3
{
    namespace bip = boost::interprocess;

    struct BlackboardData {
        bip::interprocess_mutex mutex;
        double svb[3];
        double bvb[3];
        double Hvb[3];
        double GyroRate[3];
        int    CSSValid[6];
        double CSSIllum[6];
        int    FSSValid;
        double FSSSunAng[2];
        int    STValid;
        double STqn[4];
        double GPSPosN[3];
        double GPSVelN[3];
        double AccelAcc[3];
        double WhlH[3];
    };

}

#endif
