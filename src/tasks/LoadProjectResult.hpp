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

class LoadProjectResult : public IResult
{
public:
    explicit LoadProjectResult(std::unique_ptr<Project> loaded_project);

    void despatch(IResultHandler& handler) override;

    [[nodiscard]] std::unique_ptr<Project> take_project() noexcept;

private:
    std::unique_ptr<Project> loaded_project;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTRESULT_HPP
