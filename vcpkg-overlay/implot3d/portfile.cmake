vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO oliverdixon/implot3d
    REF "47501aad2296a60698fa055b7ba74f27ecb6d400"
    SHA512 f1b52dc2e9d5c5ce981e0b6393a09735590bcec0c1892ef31a5d3e8f56bcd8b5df74cbf4fd897255173e57b5752f50a1fc845025f09717256ee4bb8d91d76b5c
    HEAD_REF add-custom-getter-support
)

file(COPY "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" DESTINATION "${SOURCE_PATH}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS_DEBUG
        -DIMPLOT3D_SKIP_HEADERS=ON
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
