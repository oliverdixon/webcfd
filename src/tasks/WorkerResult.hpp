/**
 * @file
 *
 * WorkerResult specification
 *
 * @author Oliver Dixon
 * @date 2026-07-15
 */

#ifndef ECHOMAP_WORKERRESULT_HPP
#define ECHOMAP_WORKERRESULT_HPP

#include <variant>

#include "DFTResult.hpp"
#include "DownsampleResult.hpp"
#include "ErrorResult.hpp"
#include "LoadProjectResult.hpp"

namespace echomap
{

struct WorkerResult : Object<WorkerResult>
{
    using ResultT = std::variant<ErrorResult, DFTResult, DownsampleResult, LoadProjectResult>;

    explicit WorkerResult(
            ResultT result
    ) :
        result(std::move(result))
    {
    }

    ResultT result;
};

template <> constexpr std::string_view Object<WorkerResult>::class_name = "WorkerResult";

} // namespace echomap

#endif // ECHOMAP_WORKERRESULT_HPP
