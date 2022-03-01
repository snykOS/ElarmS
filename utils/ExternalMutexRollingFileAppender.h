/** @file ExternalMutexRollingFileAppender.h */
/*
 * Copyright (c) 2017 California Institute of Technology.
 * All rights reserved.
 * This program is distributed WITHOUT ANY WARRANTY whatsoever.
 * Do not redistribute this program without written permission.
 */

#pragma once

#define RCSID_ExternalMutexRollingFileAppender_h "$Id: ExternalMutexRollingFileAppender.h $"

#include <algorithm>
#include <pthread.h>
#include <plog/Util.h>
#include <plog/Converters/UTF8Converter.h>

namespace eew {

    /** Modified RollingFileAppender that accepts an outside pthread
     *  mutex for use with DMlib and wplib printlocks */
    template <class Formatter, class Converter = plog::UTF8Converter>
    class ExternalMutexRollingFileAppender : public plog::IAppender {

      public:
        // Constructor that calls many base class functions
        ExternalMutexRollingFileAppender(::pthread_mutex_t *mutex_arg, // shared pthread mutex from
                                                                       // application
                                         const util::nchar *fileName, size_t maxFileSize = 0,
                                         int maxFiles = 0)
            : m_fileSize(), m_maxFileSize((std::max)(maxFileSize, static_cast<size_t>(1000))),
              m_lastFileNumber((std::max)(maxFiles - 1, 0)), m_firstWrite(true) {
            file_mutex = mutex_arg; // assign outside mutex
            util::splitFileName(fileName, m_fileNameNoExt, m_fileExt);
        }

        virtual void write(const Record &record) {
            ::pthread_mutex_lock(file_mutex); // lock shared file mutex
            if (m_firstWrite) {
                openLogFile();
                m_firstWrite = false;
            } else if (m_lastFileNumber > 0 && m_fileSize > m_maxFileSize &&
                       static_cast<size_t>(-1) != m_fileSize) {
                rollLogFiles();
            }

            int bytesWritten =
                m_file.write(Converter::convert(Formatter::format(record)));

            if (bytesWritten > 0) {
                m_fileSize += bytesWritten;
            }
            ::pthread_mutex_unlock(file_mutex); // unlock shared file mutex
        }                                       // end function write(Record&)

      private:
        void rollLogFiles() {
            m_file.close();

            util::nstring lastFileName = buildFileName(m_lastFileNumber);
            util::File::unlink(lastFileName.c_str());

            for (int fileNumber = m_lastFileNumber - 1; fileNumber >= 0; --fileNumber) {
                util::nstring currentFileName = buildFileName(fileNumber);
                util::nstring nextFileName = buildFileName(fileNumber + 1);

                util::File::rename(currentFileName.c_str(), nextFileName.c_str());
            }
            openLogFile();
        }

        void openLogFile() {
            util::nstring fileName = buildFileName();
            m_fileSize = m_file.open(fileName.c_str());

            if (0 == m_fileSize) {
                int bytesWritten = m_file.write(Converter::header(Formatter::header()));

                if (bytesWritten > 0) {
                    m_fileSize += bytesWritten;
                }
            }
        }

        util::nstring buildFileName(int fileNumber = 0) {
            util::nstringstream ss;
            ss << m_fileNameNoExt;
            if (fileNumber > 0) {
                ss << '.' << fileNumber;
            }

            if (!m_fileExt.empty()) {
                ss << '.' << m_fileExt;
            }
            return ss.str();
        }

      private:
        ::pthread_mutex_t *file_mutex;
        util::File m_file;
        size_t m_fileSize;
        const size_t m_maxFileSize;
        const int m_lastFileNumber;
        util::nstring m_fileExt;
        util::nstring m_fileNameNoExt;
        bool m_firstWrite;
    }; // end class ExternalMutexRollingFileAppender
} // end namespace eew
// end file: ExternalMutexRollingFileAppender.h
