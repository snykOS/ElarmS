/** @file ExternalMutexConsoleAppender.h */
/*
 * Copyright (c) 2017 California Institute of Technology.
 * All rights reserved.
 * This program is distributed WITHOUT ANY WARRANTY whatsoever.
 * Do not redistribute this program without written permission.
 */

#pragma once

#define RCSID_ExternalMutexConsoleAppender_h "$Id: ExternalMutexConsoleAppender.h $"

#include <plog/Appenders/ConsoleAppender.h>
#include <pthread.h>
#include <stdexcept>

namespace eew {

    template <class Formatter>
    class ExternalMutexConsoleAppender : public plog::ConsoleAppender<Formatter> {

      protected:
        ::pthread_mutex_t *console_mutex;

      public:
        ExternalMutexConsoleAppender(::pthread_mutex_t *mutex) { console_mutex = mutex; }

        virtual void write(const plog::Record &record) {
            if (console_mutex != NULL) {
                ::pthread_mutex_lock(console_mutex);
                plog::ConsoleAppender<Formatter>::write(record);
                ::pthread_mutex_unlock(console_mutex);
            } else {
                throw std::runtime_error("ExternalMutexConsoleAppender: \
                                Null pthread_mutex_t pointer from constructor.");
            }
        }

    }; // end template class PthreadMutexedConsoleAppender

} // end namespace eew
