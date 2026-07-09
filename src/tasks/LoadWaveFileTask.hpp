//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_LOADWAVEFILETASK_HPP
#define ECHOMAP_LOADWAVEFILETASK_HPP
#include <filesystem>

#include "ITask.hpp"

namespace echomap
{

class LoadWaveFileTask : ITask
{
public:
    explicit LoadWaveFileTask(const std::filesystem::path& path);

    std::unique_ptr<IResult> execute(std::stop_token stop_token) override;

private:
    std::filesystem::path wave_file_path;
};

} // namespace echomap

#endif // ECHOMAP_LOADWAVEFILETASK_HPP
