//
// Created by owd on 7/9/26.
//

#include "LoadWaveFileTask.hpp"

namespace echomap
{

LoadWaveFileTask::LoadWaveFileTask(
        const std::filesystem::path& path
) :
    wave_file_path(path)
{
}

std::unique_ptr<IResult> LoadWaveFileTask::execute(
        std::stop_token stop_token
)
{
    std::puts(wave_file_path.c_str());
    return {};
}

} // namespace echomap
