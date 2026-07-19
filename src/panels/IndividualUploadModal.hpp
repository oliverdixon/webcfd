/**
 * @file
 *
 * IndividualUploadModal specification
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

#ifndef ECHOMAP_INDIVIDUALUPLOADMODAL_HPP
#define ECHOMAP_INDIVIDUALUPLOADMODAL_HPP

#include <string>

#include "IPanel.hpp"

namespace echomap
{

class EchoMap;

/**
 * Provides a modal IPanel to map externally sourced Signal objects to separately uploaded files.
 *
 * This is most useful in browser (WebAssembly VFS) contexts where the application cannot interrogate the file system
 * directly.
 */
class IndividualUploadModal : public IPanel
{
public:
    explicit IndividualUploadModal(
            EchoMap* app,
            const Project* project
    );

    void draw() noexcept override;

    [[nodiscard]] const char* get_imgui_name() const noexcept override;

private:
    std::string panel_name;
    EchoMap * app;
    const Project* project;
};

} // namespace echomap

#endif // ECHOMAP_INDIVIDUALUPLOADMODAL_HPP
