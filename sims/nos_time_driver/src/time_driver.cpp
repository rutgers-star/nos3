/* Copyright (C) 2009 - 2021 National Aeronautics and Space Administration. All Foreign Rights are Reserved to the U.S. Government.

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

/* Standard Includes */
#include <thread>
#include <string>
#include <curses.h>
#include <climits>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <ItcLogger/Logger.hpp>

#include <Client/Bus.hpp>

#include <sim_hardware_model_factory.hpp>
#include <sim_i_hardware_model.hpp>
#include <sim_config.hpp>
#include <sim_coordinate_transformations.hpp>
#include <time_driver.hpp>

namespace Nos3
{
    REGISTER_HARDWARE_MODEL(TimeDriver,"TimeDriver");

    ItcLogger::Logger *sim_logger;

    /*************************************************************************
     * Constructors / destructors
     *************************************************************************/

    TimeDriver::TimeDriver(const boost::property_tree::ptree& config) : SimIHardwareModel(config),
        _active(config.get("simulator.active", true)),
        _time_counter(0), _display_counter(0), _pause_ticks(UINT_MAX)
    {
        std::string default_time_uri = config.get("common.nos-connection-string", "tcp://127.0.0.1:12001");
        std::string default_time_bus_name = "command";
        struct TimeBusInfo tbi;

        if (_active) 
        {
            sim_logger->debug("TimeDriver::TimeDriver: Creating time sender\n");

            // Use config data if it exists
            if (config.get_child_optional("simulator.hardware-model.connections")) 
            {
                BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, config.get_child("simulator.hardware-model.connections")) 
                {
                    std::ostringstream oss;
                    write_xml(oss, v.second);
                    sim_logger->trace("TimeDriver::TimeDriver - simulator.hardware-model.connections.connection subtree:\n%s", oss.str().c_str());

                    // v.first is the name of the child.
                    // v.second is the child tree.
                    if (v.second.get("type", "").compare("time") == 0) {
                        tbi.time_bus_name = v.second.get("bus-name", default_time_bus_name);
                        tbi.time_uri = v.second.get("nos-connection-string-override", default_time_uri);
                        bool found = false;
                        // Prefer slow search of vector in constructor so that the run method has fast indexing/iteration
                        for (unsigned int i = 0; i < _time_bus_info.size(); i++) {
                            if ((tbi.time_bus_name.compare(_time_bus_info[i].time_bus_name) == 0) &&
                                (tbi.time_uri.compare(_time_bus_info[i].time_uri) == 0)              ) {
                                    found = true;
                                    break;
                            }
                        }
                        if (!found) {
                            tbi.time_bus.reset(new NosEngine::Client::Bus(_hub, tbi.time_uri, tbi.time_bus_name));
                            tbi.time_bus->enable_set_time();
                            _time_bus_info.push_back(std::move(tbi));
                        }
                    }
                }
            }

            sim_logger->debug("TimeDriver::TimeDriver: Time sender created!\n");
        }
    }

    /*************************************************************************
     * Mutating public worker methods
     *************************************************************************/

    void TimeDriver::run(void)
    {
        gettimeofday(&_now, NULL);
        _then = _now;

        if (_active) 
        {
            initscr();
            erase();
            keypad(stdscr, TRUE);
            nodelay(stdscr, TRUE);
            noecho();
            int key = getch();
            double pause_duration, pause_at;

            while (1) 
            {
                do {
                    //std::this_thread::sleep_for(std::chrono::microseconds(_real_microseconds_per_tick/10));
                    gettimeofday(&_now, NULL);
                    _last_time_diff = time_diff();
                } while (_last_time_diff < _real_microseconds_per_tick);

    			int64_t ticks_per_second = 1000000/_real_microseconds_per_tick;
                _then = _now;
                
                if (ticks_per_second < 10 ||  // prevent division by zero on the next line
                    ((_display_counter % (ticks_per_second/10)) == 0)) { // only report about every 1/10th of a second
                    update_display();
                }

                switch(key) {
                    case 'p':
                    case 'P':
                        if (_pause_ticks == UINT_MAX) _pause_ticks = _time_counter;
                        else _pause_ticks = UINT_MAX;
                        break;
                    case '+':
                        if (100 * _real_microseconds_per_tick > _sim_microseconds_per_tick) _real_microseconds_per_tick /= 2; // no faster than 200x real time
                        break;
                    case '-':
                        if (_real_microseconds_per_tick < _sim_microseconds_per_tick * 100) _real_microseconds_per_tick *= 2; // no slower than 0.005x real time
                        break;
                    case 'r':
                    case 'R':
                        scanf("%lf", &pause_duration);
                        _pause_ticks = _time_counter + pause_duration * 1000000 / _sim_microseconds_per_tick;
                        break;
                    case 'u':
                    case 'U':
                        scanf("%lf", &pause_at);
                        _pause_ticks = (pause_at - _absolute_start_time) * 1000000 / _sim_microseconds_per_tick;
                        break;
                }

                if (_time_counter < _pause_ticks) {
                    update_time_busses();
                    _time_counter++;
                }
                _display_counter++;

                key = getch();
            }
            endwin();
        } 
        else 
        {
            sim_logger->info("TimeDriver::run:  Time driver is not active");
        }
    }

    void TimeDriver::update_time_busses(void)
    {
        for (unsigned int i = 0; i < _time_bus_info.size(); i++) {

            if(!_time_bus_info[i].time_bus->is_connected())
            {
                sim_logger->info("time bus disconnected... reconnecting");
                _time_bus_info[i].time_bus.reset(new NosEngine::Client::Bus(_hub, _time_bus_info[i].time_uri, _time_bus_info[i].time_bus_name));
                _time_bus_info[i].time_bus->enable_set_time();
            }
            _time_bus_info[i].time_bus->set_time(_time_counter);
        }
    }

    void TimeDriver::update_display(void)
    {
        int32_t year, month, day, hour, minute;
        double second;

        wmove(stdscr, 0, 0);
        double abs_time = _absolute_start_time + (double(_time_counter * _sim_microseconds_per_tick)) / 1000000.0;
        SimCoordinateTransformations::AbsTime2YMDHMS(abs_time, year, month, day, hour, minute, second);
        double speed_up = ((double)_sim_microseconds_per_tick) / ((double)_real_microseconds_per_tick);
        double actual_speed_up = (double)_sim_microseconds_per_tick / _last_time_diff;

        printw("TimeDriver::send_tick_to_nos_engine:\n");
        printw("  tick = %d, absolute time = %f = %4.4d/%2.2d/%2.2dT%2.2d:%2.2d:%05.2f\n", _time_counter, abs_time, year, month, day, hour, minute, second);
        printw("  real microseconds per tick = %ld, ", _real_microseconds_per_tick);
        printw("attempted speed-up = %5.2f\n", speed_up);
        printw("  actual speed-up = %5.2f, state = %s", actual_speed_up, (_pause_ticks <= _time_counter) ? "paused" : ((_pause_ticks < UINT_MAX) && (_pause_ticks > _time_counter)) ? "pausing" : "playing");
        if ((_pause_ticks < UINT_MAX) && (_pause_ticks > _time_counter)) printw(" at %f", _absolute_start_time + (double(_pause_ticks * _sim_microseconds_per_tick)) / 1000000.0);
        printw("\n\nPress: 'p' to pause/play,\n       '+' to decrease delay by 2x,\n       '-' to increase delay by 2x\n");
        printw(    "       'r <number>' to run <number> more seconds,\n       'u <number>' to run until <number> absolute time\n");
        refresh();
    }

    double TimeDriver::time_diff(void)
    {
        double then, now, diff;

        then = (double)_then.tv_sec*1000000 + (double)_then.tv_usec;
        now = (double)_now.tv_sec*1000000 + (double)_now.tv_usec;

        diff = (now - then);

        return diff;
    }

    void TimeDriver::command_callback(NosEngine::Common::Message msg)
    {
        double pause_duration, pause_at;

        /* Get the data out of the message */
        NosEngine::Common::DataBufferOverlay dbf(const_cast<NosEngine::Utility::Buffer&>(msg.buffer));

        std::string command = dbf.data;
        std::string response = "TimeDriver::command_callback:  INVALID COMMAND! (Try HELP)";
        boost::to_upper(command);
        if (command.compare(0, 4, "HELP") == 0) 
        {
            response = "TimeDriver::command_callback: Valid commands are PAUSE, UNPAUSE, DECREASE, INCREASE, RUN <number>, and UNTIL <number>";
        }
        else if (command.compare(0, 5, "PAUSE") == 0) 
        {
            _pause_ticks = _time_counter;
            response = "PAUSE";
        }
        else if (command.compare(0, 7, "UNPAUSE") == 0) 
        {
            _pause_ticks = UINT_MAX;
            response = "UNPAUSE";
        }
        else if (command.compare(0, 8, "DECREASE") == 0) 
        {
            _real_microseconds_per_tick /= 2;
            response = "DECREASE";
        }
        else if (command.compare(0, 8, "INCREASE") == 0) 
        {
            _real_microseconds_per_tick *= 2;
            response = "INCREASE";
        }
        else if (command.compare(0, 4, "RUN ") == 0) 
        {
            pause_duration = std::stod(command.substr(4)); // number of seconds
            _pause_ticks = _time_counter + pause_duration * 1000000 / _sim_microseconds_per_tick;
            response = "RUN";
        }
        else if (command.compare(0, 6, "UNTIL ") == 0) 
        {
            pause_at = std::stod(command.substr(6)); // absolute time in seconds
            _pause_ticks = (pause_at - _absolute_start_time) * 1000000 / _sim_microseconds_per_tick;
            response = "RUN";
        }

        _command_node->send_reply_message_async(msg, response.size(), response.c_str());
    }

}
