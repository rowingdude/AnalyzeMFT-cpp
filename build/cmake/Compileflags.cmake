# Compiler detection and flag setting

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC and Clang flags
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    
    # Warning flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wformat-security
        -Wnull-dereference
        -Wstack-protector
        -Wtrampolines
        -Walloca
        -Wvla
        -Warray-bounds=2
        -Wimplicit-fallthrough=3
        -Wtraditional-conversion
        -Wshift-overflow=2
        -Wcast-qual
        -Wstringop-overflow=4
        -Wconversion
        -Wint-conversion
        -Wlogical-op
        -Wduplicated-cond
        -Wduplicated-branches
        -Wformat-overflow=2
        -Wformat-truncation=2
        -Wstringop-truncation
        -Wcast-align=strict
    )
    
    # Error on warnings for CI/CD
    if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS})
        add_compile_options(-Werror)
    endif()
    
    # SIMD flags
    if(ENABLE_SIMD)
        add_compile_options(-mavx2 -msse4.2 -mpclmul)
        add_definitions(-DSIMD_OPTIMIZED -DCRC32_HARDWARE)
    endif()
    
    # OpenMP
    if(ENABLE_OPENMP)
        add_compile_options(-fopenmp)
        add_link_options(-fopenmp)
    endif()
    
    # Sanitizers for debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
        option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
        option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
        
        if(ENABLE_ASAN)
            add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
            add_link_options(-fsanitize=address)
        endif()
        
        if(ENABLE_TSAN)
            add_compile_options(-fsanitize=thread)
            add_link_options(-fsanitize=thread)
        endif()
        
        if(ENABLE_UBSAN)
            add_compile_options(-fsanitize=undefined)
            add_link_options(-fsanitize=undefined)
        endif()
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC flags
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /MDd /DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /MD /DNDEBUG")
    
    # Warning flags
    add_compile_options(
        /W4
        /permissive-
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
        /we4289
        /w14296
        /w14311
        /w14545
        /w14546
        /w14547
        /w14549
        /w14555
        /w14619
        /w14640
        /w14826
        /w14905
        /w14906
        /w14928
    )
    
    # SIMD flags
    if(ENABLE_SIMD)
        add_compile_options(/arch:AVX2)
        add_definitions(-DSIMD_OPTIMIZED -DCRC32_HARDWARE)
    endif()
    
    # OpenMP
    if(ENABLE_OPENMP)
        add_compile_options(/openmp)
    endif()
    
endif()

# Position Independent Code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Link Time Optimization for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO is not supported: ${ipo_error}")
    endif()
endif()