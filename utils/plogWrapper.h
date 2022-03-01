#pragma once

/** @file plogWrapper.h */
/*
 * Copyright (c) 2018 California Institute of Technology.
 * All rights reserved.
 * This program is distributed WITHOUT ANY WARRANTY whatsoever.
 * Do not redistribute this program without written permission.
 */

// wrapper for plog while transitioning from DM/WP locks
// includes our eew standard formatters and appenders and
// defines are convenience init function that will also
// synchronize with the original print locks.

#define RCSID_plogWrapper_h "$Id: plogWrapper.h $"

// standard eew includes 
#include "plog/Log.h"           // main plog include file
#include "ExternalMutexConsoleAppender.h"
#include "SimpleFormatter.h"
#include "SeverityFormatter.h"

// optionally include declarations for original print locks
#ifdef SYNC_DM_LOCK
#include "DMLib.h"
#endif
#ifdef SYNC_WP_LOCK
#include "wp.h"
#endif


// put our plog stuff in the eew namespace

namespace eew {

    // define formatters and appenders in global space in case someone wants to initialized things themselves

#ifndef mtx_console_apender

#ifndef EEW_PRINT_LOCK
#define EEW_PRINT_LOCK common_eew_print_lock
    static pthread_mutex_t common_eew_print_lock = PTHREAD_MUTEX_INITIALIZER;
#endif

    static eew::ExternalMutexConsoleAppender<eew::SimpleFormatter> Simple_appender(&EEW_PRINT_LOCK);
    static eew::ExternalMutexConsoleAppender<eew::SeverityFormatter> Severity_appender(&EEW_PRINT_LOCK);
#endif

#ifndef EEW_SEVERITY
#define EEW_SEVERITY plog::verbose
#endif
#ifndef EEW_FORMATTER
#define EEW_FORMATTER eew::Severity_formatter
#endif
#ifndef EEW_APPENDER
#define EEW_APPENDER eew::Severity_appender
#endif

    // define the convenience plog init with default params.  
    // eventualy application could call plog::init directly

    inline void PlogWrapperInit(const plog::Severity severity = EEW_SEVERITY, plog::IAppender* appender = &EEW_APPENDER)
    {
        plog::init(severity, appender);

#ifdef SYNC_DM_LOCK
        DMLib::setPrintLock(&EEW_PRINT_LOCK);
        LOGD << "DMLib printLock synchronized with plog appender" << std::endl;
#else
        LOGW << "DMLib printLock is NOT synchronized with plog appender" << std::endl;
#endif

#ifdef SYNC_WP_LOCK
        WPLib::setPrintLock(&EEW_PRINT_LOCK);
        LOGD << "WPLib printLock synchronized with plog appender" << std::endl;
#else
        LOGW << "WPLib printLock is NOT synchronized with plog appender" << std::endl;
#endif
    }
} // end namespace eew

// end file plogWrapper.h
