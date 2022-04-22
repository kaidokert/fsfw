# Determines the git version with git describe and returns it by setting
# the GIT_INFO list in the parent scope. The list has the following entries
# 1. Full version string
# 2. Major version
# 3. Minor version
# 4. Revision
# 5. git SHA hash and commits since tag
function(determine_version_with_git)
    include(GetGitRevisionDescription)
    git_describe(VERSION ${ARGN})
    string(FIND ${VERSION} "." VALID_VERSION)
    if(VALID_VERSION EQUAL -1)
        message(WARNING "Version string ${VERSION} retrieved with git describe is invalid")
        return()
    endif()
    # Parse the version information into pieces.
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" _VERSION_MAJOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" _VERSION_MINOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" _VERSION_PATCH "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+-(.*)" "\\1" VERSION_SHA1 "${VERSION}")
    set(GIT_INFO ${VERSION})
    list(APPEND GIT_INFO ${_VERSION_MAJOR})
    list(APPEND GIT_INFO ${_VERSION_MINOR})
    list(APPEND GIT_INFO ${_VERSION_PATCH})
    list(APPEND GIT_INFO ${VERSION_SHA1})
    set(GIT_INFO ${GIT_INFO} PARENT_SCOPE)
    message(STATUS "${MSG_PREFIX} Set git version info into GIT_INFO from the git tag ${VERSION}")
endfunction()
