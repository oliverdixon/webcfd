//
// Created by owd on 7/9/26.
//

#ifndef ECHOMAP_ERRORRESULT_HPP
#define ECHOMAP_ERRORRESULT_HPP

#include <string>
#include <string_view>

namespace echomap
{

class ErrorResult
{
public:
    explicit ErrorResult(std::string_view message);

    [[nodiscard]] std::string_view observe_message() const noexcept;

private:
    std::string message;
};

} // namespace echomap

#endif // ECHOMAP_ERRORRESULT_HPP
