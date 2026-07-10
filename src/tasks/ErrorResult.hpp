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

class EchoMap;

class ErrorResult : public IResult<EchoMap>
{
public:
    explicit ErrorResult(std::string_view message);

    void apply(EchoMap& app) override;

private:
    std::string message;
};

} // namespace echomap

#endif // ECHOMAP_ERRORRESULT_HPP
