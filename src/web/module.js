// noinspection JSUndefinedPropertyAssignment,ES6ConvertVarToLetConst,JSUnresolvedReference

/**
 * @file
 *
 * Browser-side JavaScript support for the EchoMap Emscripten build.
 *
 * Provides the Emscripten `Module` object, canvas configuration, virtual filesystem upload helpers, and file picker
 * handlers callable from C++.
 *
 * @author Oliver Dixon
 * @date 2026-07-17
 */

"use strict";

const canvas = document.getElementById("canvas");

canvas.addEventListener("contextmenu", event => {
    event.preventDefault();
});

/**
 * Emscripten runtime configuration object.
 *
 * This object is read by the Emscripten-generated JavaScript glue code. It is deliberately declared with `var` because
 * Emscripten commonly expects `Module` to be a global `var` binding.
 *
 * @type {{
 *     canvas: HTMLElement | null,
 *     print: function(string): void,
 *     printErr: function(string): void,
 *     setStatus: function(string): void
 * }}
 */
var Module = {
    canvas,

    /**
     * Handles standard output from the Emscripten runtime.
     *
     * @param {string} text Text emitted by the runtime.
     * @returns {void}
     */
    print: text => console.log(text),

    /**
     * Handles standard error output from the Emscripten runtime.
     *
     * @param {string} text Text emitted by the runtime.
     * @returns {void}
     */
    printErr: text => console.error(text),

    /**
     * Handles status messages from the Emscripten runtime.
     *
     * Empty status strings are ignored.
     *
     * @param {string} text Status text emitted by the runtime.
     * @returns {void}
     */
    setStatus: text => {
        if (text) console.log(text);
    }
};

/**
 * EchoMap JavaScript helper namespace.
 *
 * Contains browser-side helpers used by C++/Emscripten code.
 */
const EchoMap = (() => {
    /**
     * Directory inside Emscripten's virtual filesystem where user-selected files are copied before being passed to C++.
     *
     * @type {string}
     */
    const uploadDirectory = "/uploads";

    /**
     * Pattern matching filename characters that are not allowed in uploaded virtual filesystem paths.
     *
     * @type {RegExp}
     */
    const safeFilenamePattern = /[^a-zA-Z0-9._()+ -]/g;

    /**
     * Describes a JS wrapper around a C function that imports a selected file.
     *
     * @typedef {Object} FileImportHandlerOptions
     * @property {string} accept Browser file input accept string.
     * @property {string} description Human-readable description used in error messages.
     * @property {string} cFunctionName Name of the exported C function to call.
     * @property {string[]} argTypes Emscripten ccall argument type list.
     * @property {function(string, ...*): *[]} makeArgs Converts the uploaded path and context arguments into ccall
     *  arguments.
     */

    /**
     * Ensures that the upload directory exists in Emscripten's virtual filesystem.
     *
     * This function is tolerant of the directory already existing.
     *
     * @returns {void}
     */
    function ensureUploadDirectory() {
        try {
            Module.FS.mkdirTree(uploadDirectory);
        } catch (_) {
            try {
                Module.FS.mkdir(uploadDirectory);
            } catch (_) {
                // Already exists.
            }
        }
    }

    /**
     * Creates a safe, unique virtual filesystem path for a browser File.
     *
     * The original filename is sanitised to avoid awkward path characters, and Date.now() is prefixed to reduce
     * collisions between repeated imports.
     *
     * @param {File} file Browser file selected by the user.
     * @returns {string} Path in Emscripten's virtual filesystem.
     */
    function makeSafeUploadPath(file) {
        const safeName = file.name.replace(safeFilenamePattern, "_");
        return `${uploadDirectory}/${Date.now()}_${safeName}`;
    }

    /**
     * Copies a browser File into Emscripten's virtual filesystem.
     *
     * @param {File} file Browser file selected by the user.
     * @returns {Promise<string>} Virtual filesystem path of the copied file.
     */
    async function copyFileToEmscriptenFS(file) {
        const bytes = new Uint8Array(await file.arrayBuffer());
        const path = makeSafeUploadPath(file);

        ensureUploadDirectory();
        Module.FS.writeFile(path, bytes);

        return path;
    }

    /**
     * Opens a browser file chooser and resolves with the selected file.
     *
     * If the user cancels the picker, the returned promise resolves with null.
     *
     * @param {string} accept Browser file input accept string, e.g. ".wav,audio/wav".
     * @returns {Promise<File | null>} Selected file, or null if no file was selected.
     */
    function chooseFile(accept) {
        return new Promise(resolve => {
            const input = document.createElement("input");

            input.type = "file";
            input.accept = accept;
            input.style.display = "none";

            document.body.appendChild(input);

            input.addEventListener("change", () => {
                const file = input.files && input.files[0];

                input.remove();

                resolve(file || null);
            }, {once: true});

            if (input.showPicker) input.showPicker(); else input.click();
        });
    }

    /**
     * Calls an exported C function through Emscripten ccall and logs failures.
     *
     * The C function is expected to return 0 on success and non-zero on failure.
     *
     * @param {string} cFunctionName Name of the exported C function.
     * @param {string[]} argTypes Emscripten ccall argument type list.
     * @param {*[]} args Arguments to pass to the C function.
     * @returns {number} Integer result returned by the C function.
     */
    function callChecked(cFunctionName, argTypes, args) {
        const result = Module.ccall(cFunctionName, "number", argTypes, args);

        if (result !== 0) console.error(`${cFunctionName} failed with code ${result}`);

        return result;
    }

    /**
     * Creates a Module-compatible file import handler.
     *
     * The returned function opens a browser file picker, copies the selected file into Emscripten's virtual filesystem,
     * and then calls the configured exported C function.
     *
     * Any arguments passed to the returned function are forwarded to makeArgs after the uploaded virtual filesystem
     * path.
     *
     * @param {FileImportHandlerOptions} options Import handler configuration.
     * @returns {function(...*): void} Function suitable for attaching to Module.
     */
    function makeFileImportHandler({
                                       accept, description, cFunctionName, argTypes, makeArgs
                                   }) {
        return (...contextArgs) => {
            void (async () => {
                try {
                    const file = await chooseFile(accept);
                    if (!file) return;

                    const path = await copyFileToEmscriptenFS(file);

                    callChecked(cFunctionName, argTypes, makeArgs(path, ...contextArgs));
                } catch (error) {
                    console.error(`Could not import ${description}:`, error);
                }
            })();
        };
    }

    return {
        makeFileImportHandler
    };
})();

/**
 * Opens a WAV file picker for replacing or loading an existing Signal.
 *
 * Called from C++ via Emscripten. The selected file is copied into the virtual filesystem and then passed to
 * echomap_on_signal_load_complete.
 *
 * @param {number} projectID Project identifier supplied by C++.
 * @param {number} signalID Signal identifier supplied by C++.
 *
 * @returns {void}
 */
Module.echomapOpenWAVFileChooserForExistingSignal = EchoMap.makeFileImportHandler({
    accept: ".wav,audio/wav,audio/x-wav",
    description: "WAV file",
    cFunctionName: "echomap_on_signal_load_complete",
    argTypes: ["number", "number", "string"],
    makeArgs: (path, projectID, signalID) => [projectID, signalID, path]
});

/**
 * Opens a project JSON file picker.
 *
 * Called from C++ via Emscripten. The selected file is copied into the virtual filesystem and then passed to
 * echomap_on_project_file_picked.
 *
 * @returns {void}
 */
Module.echomapOpenProjectFileChooser = EchoMap.makeFileImportHandler({
    accept: ".json,application/json",
    description: "project file",
    cFunctionName: "echomap_on_project_file_picked",
    argTypes: ["string"],
    makeArgs: path => [path]
});
