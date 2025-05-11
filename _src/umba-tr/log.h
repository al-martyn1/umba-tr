#pragma once

#include "umba/log.h"


// source parsing errors
// requires std::stringr curFile, unsigned lineNo in log scope
#define LOG_ERR                      UMBA_LOG_ERR_INPUT
#define LOG_WARN(warnType)           UMBA_LOG_WARN_INPUT(warnType)

// options and other errors
#define LOG_ERR_OPT                  UMBA_LOG_ERR
#define LOG_WARN_OPT(warnType)       UMBA_LOG_WARN(warnType)

#define LOG_MSG_OPT                  UMBA_LOG_MSG

