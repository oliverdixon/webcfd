/**
 * @file
 * @brief Project-loading result specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_LOADPROJECTRESULT_HPP
#define ECHOMAP_LOADPROJECTRESULT_HPP

#include <memory>

#include "../../objects/Project.hpp"

namespace echomap
{

class LoadProjectResult
{
public:
    explicit LoadProjectResult(std::unique_ptr<Project> loaded_project);

    [[nodiscard]] std::unique_ptr<Project> take_project() && noexcept;
    [[nodiscard]] const Project * observe_project() const noexcept;

private:
    std::unique_ptr<Project> loaded_project;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTRESULT_HPP
