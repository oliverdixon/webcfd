//
// Created by owd on 7/9/26.
//

#include "LoadProjectResult.hpp"

#include "../EchoMap.hpp"

namespace echomap
{

LoadProjectResult::LoadProjectResult(
        std::unique_ptr<Project> loaded_project
) :
    loaded_project(std::move(loaded_project))
{
}

void LoadProjectResult::apply(
        EchoMap& app
)
{
    app.put_project(std::move(loaded_project));
}

} // namespace echomap
