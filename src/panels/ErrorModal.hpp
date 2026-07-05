/**
 * @file
 * @brief WebCFD modal error panel specification
 * @author Oliver Dixon
 * @date 2026-06-30
 */

#ifndef WEBCFD_ERRORMODAL_HPP
#define WEBCFD_ERRORMODAL_HPP

#include "IPanel.hpp"

#include <string>

namespace WebCFD
{

/**
 * Provides a persistent ImGui state to raise errors as a modal.
 *
 * Error modals provide a message, an optional detail string derived from an exception, and a single button to dismiss
 * the modal.
 */
class ErrorModal : public IPanel
{
public:
    /**
     * Raise the modal given a simple unformatted message.
     *
     * @param message The unformatted string literal to display.
     */
    void raise_error(std::string_view message) noexcept;

    /**
     * Raise the modal given a prefix and a detailed exception.
     *
     * @param new_prefix An unformatted string literal to display as the major text.
     * @param exception An exception containing a message to display as the minor text.
     *
     * Since this routine will often be called from exception handlers, it provides a no-exception guarantee (to prevent
     * program termination). If a low-level system call fails, such as memory allocation to copy the error text, the
     * exception is caught and logged with the system logger; the modal is not raised.
     */
    void raise_error(
            std::string_view new_prefix,
            const std::runtime_error& exception
    ) noexcept;

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

private:
    bool is_raised = false;
    std::string prefix = "An unknown error occurred.";
    std::optional<std::string> detail;

    const std::string panel_name = "Error!";
};

} // namespace WebCFD

#endif // WEBCFD_ERRORMODAL_HPP
