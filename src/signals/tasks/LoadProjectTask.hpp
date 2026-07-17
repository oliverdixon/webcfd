/**
 * @file
 * @brief Project-loading task specification
 * @author Oliver Dixon
 * @date 2026-07-10
 */

#ifndef ECHOMAP_LOADPROJECTTASK_HPP
#define ECHOMAP_LOADPROJECTTASK_HPP

#include <filesystem>

#include "../../objects/persistence/JSONDeserialiser.hpp"
#include "ITask.hpp"

namespace echomap
{

/**
 * Denotes a unit of work for loading and constructing an EchoMap Project from its serialised form on the filesystem.
 *
 * @warning The LoadProjectTask is not thread-safe due to persisting global state of the parser for re-use across
 *  Project loads. It should be called only a single thread.
 *
 * @see JSONDeserialiser::deserialise_project(std::string_view)
 */
class LoadProjectTask : public ITask
{
public:
    /**
     * Create a new LoadProjectTask for a serialised Project.
     *
     * @param path The path of the serialised Project file.
     */
    explicit LoadProjectTask(const std::filesystem::path& path);

private:
    /**
     * Loads, deserialises, constructs, and validates the serialised Project file at the provided path.
     *
     * @return A LoadProjectResult detaining the loaded Project.
     *
     * @throws std::runtime_error If the Project could not be fully loaded and validated.
     * @see JSONDeserialiser::deserialise_project
     */
    WorkerResult execute_work() override;

    /**
     * Shared instance of the deserialiser.
     *
     * It is useful to persist the deserialiser object across numerous Project loads, since we may re-use the parser.
     * See
     * https://github.com/simdjson/simdjson/blob/b485fb2af4862176f291a6fc9d2cb1cfd05631ac/doc/performance.md#reusing-the-parser-for-maximum-efficiency.
     * This comes at the small cost of making the LoadProjectTask not thread safe, as noted in the API docs.
     */
    static JSONDeserialiser deserialiser;

    /**
     * The deserialiser takes just strings for paths, so we "downgrade" the typed path to a string.
     */
    std::string project_file_path;
};

} // namespace echomap

#endif // ECHOMAP_LOADPROJECTTASK_HPP
