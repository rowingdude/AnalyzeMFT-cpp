
cmake_minimum_required(VERSION 3.20)

function(check_sanitizer_compatibility)
    if(ENABLE_ASAN AND ENABLE_TSAN)
        message(FATAL_ERROR "AddressSanitizer and ThreadSanitizer cannot be enabled simultaneously")
    endif()
    if(ENABLE_ASAN AND ENABLE_MSAN)
        message(FATAL_ERROR "AddressSanitizer and MemorySanitizer cannot be enabled simultaneously")
    endif()
    if(ENABLE_TSAN AND ENABLE_MSAN)
        message(FATAL_ERROR "ThreadSanitizer and MemorySanitizer cannot be enabled simultaneously")
    endif()
endfunction()

set(CMAKE_CXX_STANDARD 20 CACHE STRING "C++ standard to be used")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_ADVANCED_VECTORIZATION "Enable advanced vectorization optimizations" OFF)
option(ENABLE_LOOP_UNROLLING "Enable aggressive loop unrolling" OFF)
option(ENABLE_PREFETCHING "Enable data prefetching optimizations" OFF)
option(ENABLE_CACHE_OPTIMIZATION "Enable cache-aware optimizations" ON)
option(ENABLE_ARCH_TUNING "Enable architecture-specific tuning" ON)
option(ENABLE_RUNTIME_DISPATCH "Enable runtime CPU feature dispatch" OFF)

set(ARCH_TUNING "native" CACHE STRING "Target CPU architecture for tuning (native, skylake, zen3, armv8-a, etc.)")
set_property(CACHE ARCH_TUNING PROPERTY STRINGS 
    native skylake haswell zen3 armv8-a armv8.2-a cortex-a72 cortex-a76)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|IntelLLVM")
        set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -DDEBUG -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

        add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wformat=2
        -Wformat-security
        -Wnull-dereference
        -Wstack-protector
        -Wtrampolines
        -Wvla
        -Warray-bounds=2
        -Wimplicit-fallthrough=3
        -Wconversion
        -Wsign-conversion
        -Wfloat-conversion
        -Wlogical-op
        -Wduplicated-cond
        -Wduplicated-branches
        -Wformat-overflow=2
        -Wformat-truncation=2
        -Wcast-align=strict
        -Wstrict-overflow=2
        -Wshift-overflow=2
        -Wundef
        -Wshadow
        -Wstrict-aliasing=3
    )

        if(ENABLE_ARCH_TUNING)
        if(ARCH_TUNING STREQUAL "native")
            add_compile_options(-march=native -mtune=native)
        else()
            add_compile_options(-march=${ARCH_TUNING} -mtune=${ARCH_TUNING})
        endif()
    endif()

        if(ENABLE_SIMD OR ENABLE_ADVANCED_VECTORIZATION)
        include(CheckCXXCompilerFlag)

                if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
            check_cxx_compiler_flag("-mavx512f -mavx512dq -mavx512bw -mavx512vl" COMPILER_SUPPORTS_AVX512)
            if(COMPILER_SUPPORTS_AVX512)
                add_compile_options(-mavx512f -mavx512dq -mavx512bw -mavx512vl)
                add_definitions(-DAVX512_SUPPORTED)
            endif()

            check_cxx_compiler_flag("-mavx2 -mfma -mbmi2" COMPILER_SUPPORTS_AVX2)
            if(COMPILER_SUPPORTS_AVX2)
                add_compile_options(-mavx2 -mfma -mbmi2)
                add_definitions(-DAVX2_SUPPORTED)
            endif()

            check_cxx_compiler_flag("-msse4.2 -mpclmul -maes" COMPILER_SUPPORTS_SSE42)
            if(COMPILER_SUPPORTS_SSE42)
                add_compile_options(-msse4.2 -mpclmul -maes)
                add_definitions(-DSSE42_SUPPORTED -DCRC32_HARDWARE)
            endif()
        endif()

                if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
            check_cxx_compiler_flag("-mfpu=neon" COMPILER_SUPPORTS_NEON)
            if(COMPILER_SUPPORTS_NEON)
                add_compile_options(-mfpu=neon)
                add_definitions(-DNEON_SUPPORTED)
            endif()

            if(ENABLE_ADVANCED_VECTORIZATION)
                check_cxx_compiler_flag("-march=armv8-a+sve" COMPILER_SUPPORTS_SVE)
                if(COMPILER_SUPPORTS_SVE)
                    add_compile_options(-march=armv8-a+sve)
                    add_definitions(-DSVE_SUPPORTED)
                endif()
            endif()
        endif()
    endif()

        if(ENABLE_LOOP_UNROLLING)
        add_compile_options(-funroll-loops -floop-interchange -floop-strip-mine)
    endif()

        if(ENABLE_PREFETCHING)
        add_compile_options(-fprefetch-loop-arrays -mno-omit-leaf-frame-pointer)
    endif()

        if(ENABLE_CACHE_OPTIMIZATION)
        add_compile_options(
            -falign-functions=32
            -falign-loops=32
            -falign-jumps=32
            -falign-labels=32
        )
    endif()

        if(ENABLE_RUNTIME_DISPATCH AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-mllvm -x86-cmov-converter=true)
        add_definitions(-DRUNTIME_DISPATCH_ENABLED)
    endif()

        if(DEFINED ENV{CI} OR DEFINED ENV{GITHUB_ACTIONS})
        add_compile_options(-Werror)
    endif()

        if(ENABLE_OPENMP)
        find_package(OpenMP REQUIRED)
        add_compile_options(${OpenMP_CXX_FLAGS})
        add_link_options(${OpenMP_CXX_FLAGS})
    endif()

        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
        option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
        option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
        option(ENABLE_MSAN "Enable MemorySanitizer" OFF)

        check_sanitizer_compatibility()

        if(ENABLE_ASAN)
            add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
            add_link_options(-fsanitize=address)
        endif()

        if(ENABLE_TSAN)
            add_compile_options(-fsanitize=thread)
            add_link_options(-fsanitize=thread)
        endif()

        if(ENABLE_UBSAN)
            add_compile_options(-fsanitize=undefined,integer,nullability)
            add_link_options(-fsanitize=undefined,integer,nullability)
        endif()

        if(ENABLE_MSAN AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
            add_link_options(-fsanitize=memory)
        endif()
    endif()

        option(ENABLE_PGO "Enable Profile-Guided Optimization" OFF)
    if(ENABLE_PGO AND CMAKE_BUILD_TYPE STREQUAL "Release")
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            add_compile_options(-fprofile-generate)
            add_link_options(-fprofile-generate)
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_compile_options(-fprofile-instr-generate)
            add_link_options(-fprofile-instr-generate)
        endif()
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /MDd /DDEBUG /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD /DNDEBUG /GL")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi /MD /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/O1 /MD /DNDEBUG")

        add_compile_options(
        /W4
        /permissive-
        /wd4251
        /wd4275
        /w14242
        /w14254
        /w14263
        /w14265
        /w14287
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

        if(ENABLE_SIMD OR ENABLE_ADVANCED_VECTORIZATION)
        if(ENABLE_ARCH_TUNING)
            if(ARCH_TUNING MATCHES "skylake|haswell")
                add_compile_options(/arch:AVX2)
            elseif(ARCH_TUNING MATCHES "zen3")
                add_compile_options(/arch:AVX512)
            else()
                add_compile_options(/arch:AVX2)
            endif()
        else()
            add_compile_options(/arch:AVX2)
        endif()
        add_definitions(-DSIMD_OPTIMIZED -DCRC32_HARDWARE)
    endif()

        if(ENABLE_CACHE_OPTIMIZATION)
        add_compile_options(/Oa /Ot)
    endif()

        if(ENABLE_OPENMP)
        add_compile_options(/openmp:experimental)
    endif()

        if(ENABLE_PGO AND CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/GL)
        add_link_options(/LTCG /GENPROFILE)
    endif()
endif()

if(NOT MSVC)
    set(SECURITY_FLAGS
        -fstack-protect-strong
        -D_FORTIFY_SOURCE=2
        -fstack-check
        -fno-strict-aliasing
    )
    add_compile_options(${SECURITY_FLAGS})
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "LTO is not supported: ${ipo_error}")
    endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)