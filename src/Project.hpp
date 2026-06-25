//
// Created by owd on 25/06/2026.
//

#ifndef WEBCFD_PROJECT_HPP
#define WEBCFD_PROJECT_HPP

#include <memory>
#include <ranges>
#include <string_view>

#include "BidirectionalUnorderedMapping.hpp"
#include "audio/Sensor.hpp"
#include "audio/WAVData.hpp"

namespace WebCFD
{

class Project : public Object<Project>
{
    std::unordered_map<Signal::id_type, std::unique_ptr<Signal>> signals;
    std::unordered_map<Sensor::id_type, std::unique_ptr<Sensor>> sensors;

public:
    /**
     * Creates a new named Project.
     *
     * @param project_name Optional Project display name.
     */
    explicit Project(std::string_view project_name = {});

    /**
     * Transfers ownership of a Signal into the Project.
     *
     * @param signal The owning container of the Signal object to transfer.
     * @return An observing pointer to the inserted Signal, or <code>nullptr</code> if insertion was de-duplicated.
     */
    const Signal* add_signal(std::unique_ptr<Signal>&& signal);

    /**
     * Transfers ownership of a Sensor into the Project.
     *
     * @param sensor The owning container of the Sensor object to transfer.
     * @return An observing pointer to the inserted Sensor, or <code>nullptr</code> if insertion was de-duplicated.
     */
    const Sensor* add_sensor(std::unique_ptr<Sensor>&& sensor);

    /**
     * Creates a new association between a signal and sensor.
     *
     * @param signal The associated signal
     * @param sensor The associated sensor
     *
     * @throws std::runtime_error The given Signal was not known to the project.
     * @throws std::runtime_error The given Sensor was not known to the project.
     * @throws std::runtime_error A component (the Signal or the Sensor) is already mapped.
     */
    void add_association(
            const Signal& signal,
            const Sensor& sensor
    );

    /**
     * Checks if any Signal objects are owned by the Project.
     *
     * @return Are there any Signal objects in residence?
     */
    [[nodiscard]] bool are_signals_stored() const noexcept;

    /**
     * Provides a transformed view for stored Signal objects in the Project.
     *
     * @return A view containing observing references to all stored Signal objects.
     */
    [[nodiscard]] auto observe_signals() const noexcept
    {
        return signals | std::views::values |
               std::views::transform([](const std::unique_ptr<Signal>& container) -> const Signal& {
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
                   return *container.get();
               });
    }

    /**
     * Checks if any Sensor objects are owned by the Project.
     *
     * @return Are there any Sensor objects in residence?
     */
    [[nodiscard]] bool are_sensors_stored() const noexcept;

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
                   return *container.get();
               });
    }

    /**
     * Checks if any channel mappings exist in the Project?
     *
     * @return Are there any Signal-Sensor mappings defined?
     */
    [[nodiscard]] bool are_associations_defined() const noexcept;

    /**
     * Provides a transformed view for defined channel mappings.
     *
     * @return A view containing observing references to all pairs of stored Signal-Sensor assocations.
     */
    [[nodiscard]] auto observe_associations() const noexcept
    {
        return channel_mapping | std::views::transform([this](const auto& association) {
                   return resolve_pair(association.first, association.second);
               });
    }

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
    std::pair<
            const Signal&,
            const Sensor&>
    resolve_pair(
            Signal::id_type signal_id,
            Sensor::id_type sensor_id
    ) const;

    BidirectionalUnorderedMapping<Signal::id_type, Sensor::id_type> channel_mapping;
};

} // namespace WebCFD

#endif // WEBCFD_PROJECT_HPP
