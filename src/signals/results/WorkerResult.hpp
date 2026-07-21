/**
 * @file
 *
 * WorkerResult specification
 *
 * @author Oliver Dixon
 * @date 2026-07-20
 */

#ifndef ECHOMAP_WORKERRESULT_HPP
#define ECHOMAP_WORKERRESULT_HPP

#include <variant>

#include "DFTResult.hpp"
#include "DownsampleResult.hpp"
#include "ErrorResult.hpp"
#include "LoadProjectResult.hpp"
#include "LoadSignalFileResult.hpp"

namespace echomap
{

/**
 * Issued once an ITask has completed its Worker cycle.
 */
using WorkerResult = std::variant<ErrorResult, DFTResult, DownsampleResult, LoadProjectResult, LoadSignalFileResult>;

} // namespace echomap

#endif // ECHOMAP_WORKERRESULT_HPP
