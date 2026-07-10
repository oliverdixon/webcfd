/**
 * @file
 * @brief Project-loading result specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_LOADPROJECTRESULT_HPP
#define ECHOMAP_LOADPROJECTRESULT_HPP

#include <memory>

#include "IResult.hpp"
#include "../objects/Project.hpp"

namespace echomap
{

class EchoMap;

class LoadProjectResult : public IResult<EchoMap>
{
public:
    explicit LoadProjectResult(std::unique_ptr<Project> loaded_project);

    void apply(EchoMap& app) override;

private:
    std::unique_ptr<Project> loaded_project;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTRESULT_HPP
