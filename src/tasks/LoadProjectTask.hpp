//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_LOADPROJECTTASK_HPP
#define ECHOMAP_LOADPROJECTTASK_HPP

#include <filesystem>

#include "../objects/persistence/JSONDeserialiser.hpp"
#include "ITask.hpp"

namespace echomap
{

class LoadProjectTask : public ITask
{
public:
    explicit LoadProjectTask(const std::filesystem::path& path);

    std::unique_ptr<IResult> execute(std::stop_token stop_token) override;

private:
    JSONDeserialiser deserialiser;
    std::string project_file_path;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTTASK_HPP
