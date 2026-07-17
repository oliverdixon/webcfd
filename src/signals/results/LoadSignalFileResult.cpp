/**
 * @file
 *
 * LoadSignalFileResult implementation
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#include "LoadSignalFileResult.hpp"

namespace echomap
{

LoadSignalFileResult::LoadSignalFileResult(
        std::vector<std::unique_ptr<Signal>> loaded_signal
) :
    loaded_signals(std::move(loaded_signal))
{
}

} // namespace echomap
