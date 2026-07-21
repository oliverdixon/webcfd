/**
 * @file
 * @brief Project-loading result specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_LOADPROJECTRESULT_HPP
#define ECHOMAP_LOADPROJECTRESULT_HPP

#include <memory>

namespace echomap
{

class Project;

/**
 * Denotes a loaded Project completed by a LoadProjectTask job.
 */
class LoadProjectResult
{
public:
    explicit LoadProjectResult(std::unique_ptr<Project> loaded_project);
    ~LoadProjectResult() noexcept;

    LoadProjectResult(const LoadProjectResult&) = delete;
    LoadProjectResult& operator=(const LoadProjectResult&) = delete;

    LoadProjectResult(LoadProjectResult&&) noexcept;
    LoadProjectResult& operator=(LoadProjectResult&&) noexcept;

    [[nodiscard]] std::unique_ptr<Project> take_project() && noexcept;
    [[nodiscard]] const Project * observe_project() const noexcept;

private:
    std::unique_ptr<Project> loaded_project;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTRESULT_HPP
