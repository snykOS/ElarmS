/** @file SeverityFormatter.h */
/*
 * Copyright (c) 2018 California Institute of Technology.
 * All rights reserved.
 * This program is distributed WITHOUT ANY WARRANTY whatsoever.
 * Do not redistribute this program without written permission.
 */

#pragma once

#define RCSID_SeverityFormatter_h "$Id: SeverityFormatter.h $"

#include <iomanip>
#include <plog/Util.h>

namespace eew {

    /** SeverityFormatter allows a plog::Appender to write a simple
     *  log message with Severity as a prefix.
     *
     *      Example usage:
     *          LOGD << "This is an example debug message.";
     *
     *      Example output:
     *          DEBUG | This is an example debug message.
     */
    class SeverityFormatter {

        public:
            /** Returns a header for a new file.  In our case it is empty.
            */
            static plog::util::nstring header()
            {
                return plog::util::nstring();
            }

            /** Returns a string containing the record formatted with the severity code prefixed to every line.
             *  If severity code is None, then a blank will be used.
             *  If severity code is verbose or fatal, then additional information such as
             *  function name and line number will also be included but only on the first line.
             *  Note that formatter will append a linefeed at the end of the message so the application does not have to.
             */
            static plog::util::nstring format(const plog::Record &record)
            {
                plog::util::nstringstream ss;
                std::string prefix = "";
                plog::Severity severity = record.getSeverity();
                if (severity != plog::none)
                {
                    ss << std::setw(5) << severityToString(record.getSeverity());
                    ss << PLOG_NSTR(" | ");
                    prefix = ss.str();

                    // include extra info but only on first line
                    if (severity == plog::verbose || severity <= plog::fatal)
                    {
                        ss << record.getFunc() << PLOG_NSTR("@") << record.getLine() << "|";
                    }
                }

                // add prefix after every endl including at end of message
                const char* cp = record.getMessage();
                while (const char ch = *cp++) 
                {
                    ss << ch;
                    if (ch == '\n')
                        ss << prefix;
                }
                ss << std::endl;
                return ss.str();
            } // format

    }; // end class SeverityFormatter

} // end namespace eew
// end file SeverityFormatter.h
