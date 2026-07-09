//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_IRESULT_HPP
#define ECHOMAP_IRESULT_HPP

namespace echomap
{

class Project;

class IResult
{
public:
    virtual ~IResult() = default;

    virtual void apply(Project& project) = 0;
};

} // namespace echomap

#endif // ECHOMAP_IRESULT_HPP
