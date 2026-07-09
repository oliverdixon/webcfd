#!/usr/bin/env bash

#
# @file
#
# Fetches local copies of vcpkg and the Emscripten SDK. The script exports variables into the environment, so should be
# sourced.
#
# @author Oliver Dixon
# @date 2026-07-09
#

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "error: this script must be sourced, not executed." >&2
    echo "usage: source $0" >&2
    exit 1
fi

#
# Fetch and checkout vcpkg if its location is not indicated in the environment by VCPKG_ROOT.
#
# @param $1 dependencies directory root path.
# @return 0 on success, 1 on failure (elaborated on stderr).
#
# @consumes VCPKG_ROOT for the root of an existing vcpkg installation.
# @consumes ECHOMAP_VCPKG_REF for the desired git commit-ish reference, used only when fetching vcpkg from upstream.
#           Defaults to 'master'.
# @produces VCPKG_ROOT for the root of the validated vcpkg installation.
#
echomap_bootstrap_vcpkg() {
  if [[ -z "${VCPKG_ROOT:-}" ]]; then
    mkdir -p "${deps_dir}" || return 1

    local vcpkg_ref="${ECHOMAP_VCPKG_REF:-master}"
    local vcpkg_root="$1/vcpkg"

    if [[ ! -d "${vcpkg_root}/.git" ]]; then
      echo "Fetching upstream vcpkg into ${vcpkg_root}."
      git clone https://github.com/microsoft/vcpkg "${vcpkg_root}" || return 1
    fi

    echo "Updating vcpkg to revision ${vcpkg_ref}."
    git -C "${vcpkg_root}" fetch --tags origin || return 1
    git -C "${vcpkg_root}" checkout "${vcpkg_ref}" || return 1

    if [[ ! -x "${vcpkg_root}/vcpkg" ]]; then
      echo "Bootstrapping vcpkg."
      "${vcpkg_root}/bootstrap-vcpkg.sh" -disableMetrics || return 1
    fi

    export VCPKG_ROOT=${vcpkg_root}
  else
    echo "Using existing VCPKG_ROOT=${VCPKG_ROOT}."

    if [[ ! -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]]; then
      echo "error: the existing VCPKG root does not provide a CMake toolchain file. Is it corrupted?" >&2
      return 1
    fi
  fi

  return 0
}

#
# Fetch and checkout the Emscripten SDK if its location is not indicated in the environment by EMSDK.
#
# @param $1 dependencies directory root path.
# @return 0 on success, 1 on failure (elaborated on stderr).
#
# @consumes EMSDK for the root of an existing Emscripten SDK installation.
# @consumes ECHOMAP_EMSDK_VERSION for the desired Emscripten SDK version to install, used only when fetching new emsdk.
#           Defaults to 'latest'.
# @produces EMSDK for the root of the validated Emscripten SDK installation.
#
echomap_bootstrap_emsdk() {
  if [[ -z "${EMSDK:-}" ]]; then
    mkdir -p "${deps_dir}" || return 1

    local emsdk_version=${ECHOMAP_EMSDK_VERSION:-latest}
    local emsdk_root="$1/emsdk"

    if [[ ! -d "${emsdk_root}/.git" ]]; then
      echo "Fetching upstream emsdk into ${emsdk_root}."
      git clone https://github.com/emscripten-core/emsdk "${emsdk_root}" || return 1
    fi

    echo "Updating emsdk."
    git -C "${emsdk_root}" pull --ff-only || return 1

    echo "Installing emsdk ${emsdk_version}."
    "${emsdk_root}/emsdk" install "${emsdk_version}" || return 1
    "${emsdk_root}/emsdk" activate "${emsdk_version}" || return 1

    export EMSDK="${emsdk_root}"
  else
    echo "Using existing EMSDK=${EMSDK}."

    if [[ ! -f "${EMSDK}/emsdk_env.sh" ]]; then
      echo "error: the existing EMSDK root does not provide a valid environment file. Is it corrupted?" >&2
      return 1
    fi
  fi

  source "${EMSDK}/emsdk_env.sh" >/dev/null || return 1
  return 0
}

#
# Bootstrap the environment by fetching or validating vcpkg and the Emscripten SDK.
#
# @return 0 on success, 1 on failure (elaborated on stderr).
# @consumes ECHOMAP_DEPS_DIR for the location of the fetched SDKs, defaults to 'third-party' in the git root.
#
echomap_bootstrap_env() {
  command -v git >/dev/null 2>&1 || {
    echo "error: git is required." >&2
    return 1
  }

  local deps_dir
  deps_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)/third-party"

  echomap_bootstrap_vcpkg "${deps_dir}" || return 1
  echomap_bootstrap_emsdk "${deps_dir}" || return 1
}

echomap_bootstrap_env "$@"
success=$?

if [[ $success -eq 0 ]]; then
  echo
  echo "Environment ready."
  echo "  VCPKG_ROOT=${VCPKG_ROOT}"
  echo "  EMSDK=${EMSDK}"
  echo
fi

return "$success"
