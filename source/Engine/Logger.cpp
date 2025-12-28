/**
 * \file
 * \author  JUNSEOK LEE
 * \author Jonathan Holmes
 * \author JUNSEOK LEE
 * \date 2025 Fall
 * \par CS200 Computer Graphics I
 * \copyright DigiPen Institute of Technology
 */

#include "Logger.hpp"
#include <iostream>

namespace CS230
{
    // DONE implement class here
    // note the proper way to redirect the rdbuf is `stream.basic_ios<char>::rdbuf(other_stream.rdbuf());`
    // note that we don't need a destructor ~Logger() if all we are doing is closing the streams. The std stream classes impl Rule of 5 and will auto flush & close themselves
    
    Logger::Logger(Severity severity, bool use_console, std::chrono::system_clock::time_point start_t)
        : minlev(severity), start_time(start_t), out_stream(nullptr), error_stream(nullptr)
    {
        severity_map[Severity::Verbose] = "Verbose";
        severity_map[Severity::Debug]   = "Debug";
        severity_map[Severity::Event]   = "Event";
        severity_map[Severity::Error]   = "Error";

        if (use_console)
        {
            out_stream = &std::cout;
            error_stream = &std::cerr;
        }
        else
        {
            log_file.open("log.txt");
            error_log_file.open("error_log.txt");

            out_stream = &log_file;
            error_stream = &error_log_file;
        }
    }

    void Logger::LogError(std::string text)
    {
        if (Severity::Error < minlev)
        {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - start_time;

        *error_stream << "[" << elapsed_seconds.count() << "s] ["
                    << severity_map.at(Severity::Error) << "]: "
                    << text << std::endl;
    }

    void Logger::LogEvent(std::string text)
    {
        if (Severity::Event < minlev)
        {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - start_time;

        *out_stream << "[" << elapsed_seconds.count() << "s] ["
                    << severity_map.at(Severity::Event) << "]: "
                    << text << std::endl;
    }

    void Logger::LogDebug(std::string text)
    {
        if (Severity::Debug < minlev)
        {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - start_time;

        *out_stream << "[" << elapsed_seconds.count() << "s] ["
                    << severity_map.at(Severity::Debug) << "]: "
                    << text << std::endl;
    }

    void Logger::LogVerbose(std::string text)
    {
        if (Severity::Verbose < minlev)
        {
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = now - start_time;

        *out_stream << "[" << elapsed_seconds.count() << "s] ["
                    << severity_map.at(Severity::Verbose) << "]: "
                    << text << std::endl;
    }

}