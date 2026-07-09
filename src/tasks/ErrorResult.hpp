//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_ERRORRESULT_HPP
#define ECHOMAP_ERRORRESULT_HPP

#include <string>
#include <string_view>

#include "IResult.hpp"

namespace echomap
{

class ErrorResult : public IResult
{
public:
    explicit ErrorResult(std::string_view message);

    void apply(Project& project) override;

private:
    std::string message;
};

} // namespace echomap

#endif // ECHOMAP_ERRORRESULT_HPP
