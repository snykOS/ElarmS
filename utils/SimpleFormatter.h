/** @file SimpleFormatter.h */
/*
 * Copyright (c) 2017 California Institute of Technology.
 * All rights reserved.
 * This program is distributed WITHOUT ANY WARRANTY whatsoever.
 * Do not redistribute this program without written permission.
 */

#pragma once

#define RCSID_SimpleFormatter_h "$Id: SimpleFormatter.h $"

#include <iomanip>
#include <plog/Util.h>

namespace eew {

    /** SimpleFormatter allows a plog::Appender to write a simple
     *  log message without record data like Date, Time, Severity, etc.
     *
     *      Example usage:
     *          LOGD << "This is an example message.";
     *
     *      Example output:
     *          This is an example message.
     */
    class SimpleFormatter {

        public:
            /** Returns a header for a new file.  In our case it is empty.
            */
            static plog::util::nstring header() { return plog::util::nstring(); }

            /** Returns a string with just the basic message without the date, time, severity, etc.
             *  Note that formatter will append a linefeed at the end of the message so the application does not have to.
             */
            static plog::util::nstring format(const plog::Record &record) {
                plog::util::nstringstream ss;
                ss << record.getMessage() << std::endl;
                return ss.str();
            } // format

    }; // end class SimpleFormatter

} // end namespace plog
// end file SimpleFormatter.h
