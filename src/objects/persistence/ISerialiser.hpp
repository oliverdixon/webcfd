//
// Created by owd on 07/07/2026.
//

#ifndef WEBCFD_ISERIALISER_HPP
#define WEBCFD_ISERIALISER_HPP

namespace WebCFD
{

class Project;
class Sensor;
class Signal;

class ISerialiser
{
public:
    virtual ~ISerialiser() = default;

    virtual std::string_view serialise_project(const Project& project) = 0;
    virtual std::string_view serialise_sensor(const Sensor& sensor) = 0;
    virtual std::string_view serialise_signal(const Signal& signal) = 0;
};

} // namespace WebCFD

#endif // WEBCFD_ISERIALISER_HPP
