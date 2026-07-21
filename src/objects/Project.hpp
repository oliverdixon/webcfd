//
// Created by owd on 25/06/2026.
//

#ifndef ECHOMAP_PROJECT_HPP
#define ECHOMAP_PROJECT_HPP

#include <implot3d.h>

#include <filesystem>
#include <flat_map>
#include <map>
#include <memory>
#include <ranges>
#include <string_view>

#include "BidirectionalUnorderedMapping.hpp"
#include "Object.hpp"

namespace echomap
{

class Signal;
class Sensor;
class SignalFactory;

class Project : public Object<Project>
{
    std::flat_map<id_type, std::shared_ptr<Signal>> signals;
    std::flat_map<id_type, std::unique_ptr<Sensor>> sensors;

public:
    /**
     * Provides a mapping between stated paths of externally sourced signals, and paths in the WebAssembly VFS.
     *
     * The key indicates the path of the referenced file. The value composes an optional mapping of the corresponding
     * path in the VFS and a channel map of factories responsible for constructing the Signal of the file channel.
     *
     * @todo make private
     */
    std::map<
            std::filesystem::path,
            std::pair<std::optional<std::filesystem::path>, std::vector<std::unique_ptr<SignalFactory>>>>
            unloaded_signals;

    /**
     * Creates a new named Project.
     *
     * @param project_name Optional Project display name.
     */
    explicit Project(std::string_view project_name = {});

    ~Project();

    /**
     * Transfers ownership of a Signal into the Project.
     *
     * @param signal The shared-ownership container of the Signal object to transfer.
     * @return An observing pointer to the inserted Signal, or <code>nullptr</code> if insertion was de-duplicated.
     */
    const Signal* add_signal(std::shared_ptr<Signal>&& signal);

    /**
     * Transfers ownership of a Sensor into the Project.
     *
     * @param sensor The owning container of the Sensor object to transfer.
     * @return An observing pointer to the inserted Sensor, or <code>nullptr</code> if insertion was de-duplicated.
     */
    const Sensor* add_sensor(std::unique_ptr<Sensor>&& sensor);

    /**
     * Creates a new association between a Signal and Sensor.
     *
     * This creates an immediately confirmed mapping: the Signal and Sensor provided must already belong to the Project.
     *
     * @param signal The associated Signal
     * @param sensor The associated Sensor
     *
     * @throws std::runtime_error The given Signal was not known to the Project.
     * @throws std::runtime_error The given Sensor was not known to the Project.
     * @throws std::runtime_error A component (the Signal or the Sensor) is already mapped.
     */
    void add_association(
            const Signal& signal,
            const Sensor& sensor
    );

    /**
     * Creates a new association between a Signal and Sensor.
     *
     * This requests a mapping: the Signal and Sensor of the given IDs may or may not already belong to the Project. If
     * they do not belong to the Project at the time of insertion, the association will only become observable on the
     * public Project API once both components are loaded; i.e., the prerequisites of
     * @ref add_association(const Signal&, const Sensor&) are met.
     *
     * @param signal_id The ID of the associated Signal
     * @param sensor_id The ID of the associated Sensor
     *
     * @throws std::runtime_error A component (the Signal or the Sensor) is already mapped.
     */
    void add_association(
            id_type signal_id,
            id_type sensor_id
    );

    /**
     * Provides a transformed view for stored Signal objects in the Project.
     *
     * @return A view containing observing references to all stored Signal objects.
     */
    [[nodiscard]] auto observe_signals() const noexcept
    {
        return signals | std::views::values |
               std::views::transform([](const std::shared_ptr<Signal>& container) -> const Signal& {
                   /*
                    * N.B. for this functor and all similar ones: this is an assertion, not an exception. If the Project
                    * possesses a container for something, but that container is empty, it's definitely violated an
                    * invariant. Failing on a name-based lookup would be exceptional, but this assertion failing
                    * indicates a bug.
                    *
                    * We intentionally produce references (as opposed to raw pointers) to indicate the non-optionality
                    * of these containers.
                    */
                   assert(container);
                   return *container;
               });
    }

    /**
     * Provides a view of Signal objects detained in shared-ownership containers.
     *
     * @return A view containing mutable sharable references to all stored Signal objects.
     */
    [[nodiscard]] auto share_signals() const noexcept
    {
        return signals | std::views::values;
    }

    /**
     * Checks if any Sensor objects are owned by the Project.
     *
     * @return Are there any Sensor objects in residence?
     */
    [[nodiscard]] size_t get_sensors_count() const noexcept;

    /**
     * Provides a transformed view for stored Sensor objects in the Project.
     *
     * @return A view containing observing references to all stored Sensor objects.
     */
    [[nodiscard]] auto observe_sensors() const noexcept
    {
        return sensors | std::views::values |
               std::views::transform([](const std::unique_ptr<Sensor>& container) -> const Sensor& {
                   assert(container);
                   return *container;
               });
    }

    /**
     * Provides a transformed view for defined channel mappings.
     *
     * @return A view containing observing references to all pairs of stored Signal-Sensor assocations.
     */
    [[nodiscard]] auto observe_associations() const
    {
        return requested_channel_mapping | std::views::transform([this](const auto& association) {
                   return resolve_pair(association.first, association.second);
               }) |
               std::views::filter([](const auto& optional) {
                   return optional.has_value();
               }) |
               std::views::transform([](const auto& optional) {
                   return *optional;
               });
    }

    [[nodiscard]] static ImPlot3DPoint get_sensor_point(
            int idx,
            const void* project_instance
    ) noexcept;

    /**
     * Retrieve a mutable reference to the Sensor with the given ID.
     *
     * @param sensor_id The numerical ID of the Sensor to retrieve.
     * @return A mutable reference to the Sensor with the specified ID.
     *
     * @throws std::runtime_error A Sensor with the specified ID is not owned by the Project.
     */
    [[nodiscard]] Sensor& get_mutable_sensor(id_type sensor_id);

private:
    /**
     * Convenience function to simultaneously resolve observing references to a Signal and a Sensor from the Project
     * storage based on their numerical IDs.
     *
     * Note that the returned Signal and Sensor needn't be associated in a channel mapping or any other way, besides
     * being owned by the current Project.
     *
     * @param signal_id The numerical ID of the desired Signal.
     * @param sensor_id The numerical ID of the desired Sensor.
     * @return A pair of observing references to the associated Signal and Sensor objects, owned by the Project.
     * @post The Signal owned by the Project references a non-empty container.
     * @post The Sensor owned by the Project references a non-empty container.
     * @throws std::runtime_error A Signal with the given ID is not owned by the Project.
     * @throws std::runtime_error A Sensor with the given ID is not owned by the Project.
     */
    [[nodiscard]] std::optional<std::pair<
            const Signal&,
            const Sensor&>>
    resolve_pair(
            id_type signal_id,
            id_type sensor_id
    ) const;

    /**
     * Mapping between Signal objects and Sensor objects (identified by their numerical IDs).
     *
     * The structure stores all "requested" mappings, a non-strict subset of which are "confirmed". A mapping request is
     * caused by an external caller, such as a factory, invoking @ref add_association(id_type, id_type),
     * which does not require that the corresponding Signal or Sensor is owned by the Project at the time of the
     * request.
     *
     * Mappings become confirmed once both the relevant Signal and Sensor have been loaded and are owned by the Project.
     *
     * Only confirmed mappings may be observed on the public API.
     */
    BidirectionalUnorderedMapping<id_type, id_type> requested_channel_mapping;
};

template <> constexpr std::string_view Object<Project>::class_name = "Project";

} // namespace echomap

#endif // ECHOMAP_PROJECT_HPP
