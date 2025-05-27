# Find OpenSSL
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_OPENSSL QUIET openssl)
endif()

find_path(OPENSSL_INCLUDE_DIR
    NAMES openssl/ssl.h
    PATHS ${PC_OPENSSL_INCLUDE_DIRS}
    PATH_SUFFIXES include
)

find_library(OPENSSL_SSL_LIBRARY
    NAMES ssl libssl
    PATHS ${PC_OPENSSL_LIBRARY_DIRS}
    PATH_SUFFIXES lib
)

find_library(OPENSSL_CRYPTO_LIBRARY
    NAMES crypto libcrypto
    PATHS ${PC_OPENSSL_LIBRARY_DIRS}
    PATH_SUFFIXES lib
)

if(OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
    set(OPENSSL_FOUND TRUE)
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
    
    if(NOT TARGET OpenSSL::SSL)
        add_library(OpenSSL::SSL UNKNOWN IMPORTED)
        set_target_properties(OpenSSL::SSL PROPERTIES
            IMPORTED_LOCATION "${OPENSSL_SSL_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
        )
    endif()
    
    if(NOT TARGET OpenSSL::Crypto)
        add_library(OpenSSL::Crypto UNKNOWN IMPORTED)
        set_target_properties(OpenSSL::Crypto PROPERTIES
            IMPORTED_LOCATION "${OPENSSL_CRYPTO_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPENSSL_INCLUDE_DIR}"
        )
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSSL
    REQUIRED_VARS OPENSSL_LIBRARIES OPENSSL_INCLUDE_DIRS
    VERSION_VAR OPENSSL_VERSION
)

mark_as_advanced(OPENSSL_INCLUDE_DIR OPENSSL_SSL_LIBRARY OPENSSL_CRYPTO_LIBRARY)