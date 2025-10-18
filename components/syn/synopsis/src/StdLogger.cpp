/**
 * @author Gary Doran (Gary.B.Doran.Jr@jpl.nasa.gov)
 * @date 2022.11.07
 *
 * @see StdLogger.hpp
 */
#include <iostream>
#include <cstdarg>

#include "StdLogger.hpp"


namespace Synopsis {


    // TODO: add a second constructor which takes a boolean for outputting to standard error stream, and update existing constructor
    StdLogger::StdLogger(
        bool output_all_to_stderr
    ) :
        output_all_to_stderr(output_all_to_stderr)
    {

    }

    void StdLogger::log(LogType type, const char* file, const int line, const char* fmt,  ...) {
        va_list args;
        va_start(args, fmt);

        FILE *out;
        std::string prefix;

        switch (type) {

            case LogType::INFO:
                if (this->output_all_to_stderr){
                    out = stderr;
                } else {
                    out = stdout;
                }
                prefix = "[INFO]";
                break;

            case LogType::WARN:
                if (this->output_all_to_stderr){
                    out = stderr;
                } else {
                    out = stdout;
                }
                prefix = "[WARN]";
                break;

            case LogType::ERROR:
            default:
                out = stderr;
                prefix = "[ERROR]";
                break;

        }

        fprintf(out, "%s", prefix.c_str());
        fprintf(out, " ");
        fprintf(out, "%s", file);
        fprintf(out, ", line ");
        fprintf(out, "%d", line);
        fprintf(out, ": ");
        vfprintf(out, fmt, args);
        fprintf(out, "\n");

        va_end(args);
    }


};
