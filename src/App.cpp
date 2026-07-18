/**
 * @file
 * @brief EchoMap entry point
 * @author Oliver Dixon
 * @date 2026-05-05
 */

#include "EchoMap.hpp"
#include "Logger.hpp"
#include "StaticInstanceController.hpp"
#include "errors/ConfigurationError.hpp"

/**
 * EchoMap common entry point.
 *
 * @return OS status exit code. 0 for success, 1 for failure.
 */
int main()
{
    try {
#ifdef __EMSCRIPTEN__

        // Emscripten will manage our heap memory.

        // NOLINTBEGIN(*-owning-memory, *-cplusplus.NewDeleteLeaks)
        auto* const application = new echomap::EchoMap();
        [[maybe_unused]] auto* const controller = new echomap::StaticInstanceController(*application);
        application->run_event_loop();
        // NOLINTEND(*-owning-memory, *-cplusplus.NewDeleteLeaks)

#else

        // Native platforms can use the RAII facilities of EchoMap and StaticInstanceController.

        echomap::EchoMap application_inst;
        const echomap::StaticInstanceController instance_controller(application_inst);
        application.run_event_loop();

#endif
    } catch (const echomap::ConfigurationError& error) {
        echomap::Logger::log(echomap::Logger::Level::Error, error.what(), error.where());
        return 1;
    }

    return 0;
}
