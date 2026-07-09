//
// Created by owd on 7/9/26.
//

#ifndef WEBCFD_LOADPROJECTRESULT_HPP
#define WEBCFD_LOADPROJECTRESULT_HPP

#include <memory>

#include "IResult.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

class LoadProjectResult : public IResult
{
public:
    explicit LoadProjectResult(std::unique_ptr<Project> loaded_project);

    void apply(EchoMap& app) override;

private:
    std::unique_ptr<Project> loaded_project;
};

} // namespace echomap

#endif // WEBCFD_LOADPROJECTRESULT_HPP
