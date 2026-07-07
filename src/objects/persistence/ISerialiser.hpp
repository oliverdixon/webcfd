//
// Created by owd on 07/07/2026.
//

#ifndef WEBCFD_ISERIALISER_HPP
#define WEBCFD_ISERIALISER_HPP

#include <memory>

namespace WebCFD
{

class Project;
class Sensor;
class Signal;

class ISerialiser
{
public:
    virtual ~ISerialiser() = default;

    virtual std::string_view serialise(const Project& project) = 0;
    virtual std::string_view serialise(const Sensor& sensor) = 0;
    virtual std::string_view serialise(const Signal& signal) = 0;
};

} // namespace WebCFD

#endif // WEBCFD_ISERIALISER_HPP
