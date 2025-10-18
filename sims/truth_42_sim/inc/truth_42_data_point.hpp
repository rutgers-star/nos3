#ifndef NOS3_TRUTH42DATAPOINT_HPP
#define NOS3_TRUTH42DATAPOINT_HPP

#include <boost/shared_ptr.hpp>

#include <sim_42data_point.hpp>

namespace Nos3
{
    // vvv This is pretty standard for a data point (if one is needed to transfer data between a data provider and a hardware model for your sim)
    class Truth42DataPoint : public SimIDataPoint
    {
    public:
        // Constructor
        Truth42DataPoint(int16_t orbit, int16_t spacecraft, const boost::shared_ptr<Sim42DataPoint> dp);

        // Accessors
        std::string to_string(void) const;

        int16_t get_year(void) const {parse_data_point(); return _year;}
        int16_t get_doy(void) const {parse_data_point(); return _doy;}
        int16_t get_month(void) const {parse_data_point(); return _month;}
        int16_t get_day(void) const {parse_data_point(); return _day;}
        int16_t get_utc_hh(void) const {parse_data_point(); return _utc_hh;}
        int16_t get_utc_mm(void) const {parse_data_point(); return _utc_mm;}
        double  get_utc_ss(void) const {parse_data_point(); return _utc_ss;}
        std::vector<double> get_pos(void) const {parse_data_point(); return _pos;} // ECI position (meters=m)
        std::vector<double> get_vel(void) const {parse_data_point(); return _vel;} // ECI velocity (meters/second=m/s)
        std::vector<double> get_svb(void)  const {parse_data_point(); return _svb;}  // sun unit vector in body frame (unitless)
        std::vector<double> get_bvb(void)  const {parse_data_point(); return _bvb;}  // magnetic field vector in body frame (Teslas=T)
        std::vector<double> get_Hvb(void)  const {parse_data_point(); return _Hvb;}  // angular momentum in body frame (Newton-meter-seconds=Nms)
        std::vector<double> get_wn(void)   const {parse_data_point(); return _wn;}   // body 0 angular velocity of body in body frame (radians/second=1/s)
        std::vector<double> get_qn(void)   const {parse_data_point(); return _qn;}   // body 0 quaternion in inertial frame (unitless)
        std::vector<double> get_pos_ecef(void) const {parse_data_point(); return _pos_ecef;} // ECEF position (meters=m)
        std::vector<double> get_vel_ecef(void) const {parse_data_point(); return _vel_ecef;} // ECEF velocity (meters/seconds=m/s)
        double get_gyro_x(void) const {parse_data_point(); return _gyro_b_x;} // Gyroscope Linear Velocity in the Body Frame, X Component (radians/seconds=rad/s)
        double get_acc_x(void) const {parse_data_point(); return _acc_b_x;} // Linear Acceleration in the Body Frame, X Component (meters/seconds/seconds=m/s^2)
        double get_gyro_y(void) const {parse_data_point(); return _gyro_b_y;} // Gyroscope Linear Velocity in the Body Frame, Y Component (radians/seconds=rad/s)
        double get_acc_y(void) const {parse_data_point(); return _acc_b_y;} // Linear Acceleration in the Body Frame, Y Component (meters/seconds/seconds=m/s^2)
        double get_gyro_z(void) const {parse_data_point(); return _gyro_b_z;} // Gyroscope Linear Velocity in the Body Frame, Z Component (radians/seconds=rad/s)
        double get_acc_z(void) const {parse_data_point(); return _acc_b_z;} // Linear Acceleration in the Body Frame, Z Component (meters/seconds/seconds=m/s^2)
        double get_rwh_0(void) const{parse_data_point(); return _rw_momentum_0;} // Reaction Wheel 0 Angular Momentum, (Newtons-Meters-Seconds=Nms)
        double get_rwh_1(void) const{parse_data_point(); return _rw_momentum_1;} // Reaction Wheel 1 Angular Momentum, (Newtons-Meters-Seconds=Nms)
        double get_rwh_2(void) const{parse_data_point(); return _rw_momentum_2;} // Reaction Wheel 2 Angular Momentum, (Newtons-Meters-Seconds=Nms)

    private:
        // Disallow the big 4
        Truth42DataPoint(void) {};
        Truth42DataPoint(const Truth42DataPoint&) {};
        Truth42DataPoint& operator=(const Truth42DataPoint&) {return *this;};
        ~Truth42DataPoint(void) {};

        /// @name Private mutators
        //@{
        inline void parse_data_point(void) const {if (_not_parsed) do_parsing();}
        void do_parsing(void) const;
        //@}

        // Private data
        Sim42DataPoint _dp;
        int16_t _orb;
        int16_t _sc;
        // mutable below so parsing can be on demand:
        mutable bool _not_parsed;
        mutable int16_t _year;
        mutable int16_t _doy; // day of year
        mutable int16_t _month;
        mutable int16_t _day;
        mutable int16_t _utc_hh;
        mutable int16_t _utc_mm;
        mutable double  _utc_ss;
        mutable std::vector<double> _pos;
        mutable std::vector<double> _vel;
        mutable std::vector<double> _svb;
        mutable std::vector<double> _bvb;
        mutable std::vector<double> _Hvb;
        mutable std::vector<double> _wn;
        mutable std::vector<double> _qn;
        mutable std::vector<double> _pos_ecef;
        mutable std::vector<double> _vel_ecef;
        mutable double _acc_b_x;
        mutable double _acc_b_y;
        mutable double _acc_b_z;
        mutable double _gyro_b_x;
        mutable double _gyro_b_y;
        mutable double _gyro_b_z;
        mutable double _rw_momentum_0;
        mutable double _rw_momentum_1;
        mutable double _rw_momentum_2;
    };
}

#endif
