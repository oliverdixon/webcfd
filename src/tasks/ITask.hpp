//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_ITASK_HPP
#define ECHOMAP_ITASK_HPP

#include <stop_token>

namespace echomap
{

class IResult;

class ITask
{
public:
    virtual ~ITask() = default;

    virtual std::unique_ptr<IResult> execute(std::stop_token stop_token) = 0;
};

} // namespace echomap

#endif // ECHOMAP_ITASK_HPP
