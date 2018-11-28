#!/bin/bash
# by KangLin(kl222@126.com)

set -e

cd $1

PROJECT_DIR=`pwd`

#Only test all platform and configure in tag
if [ "$appveyor_repo_tag" != "true" ]; then
    if [ "${Platform}" = "64" -o "${Configuration}" = "Release" -o "${Configuration}" = "release" ]; then
        echo "Don't test, When 64 bits and release, appveyor_repo_tag = false"
        cd ${PROJECT_DIR}
        exit 0
    fi
fi

case ${BUILD_TARGERT} in
    windows_msvc)
        case ${TOOLCHAIN_VERSION} in
            15)
                PRJ_GEN="Visual Studio 15 2017"
            ;;
            14)
                PRJ_GEN="Visual Studio 14 2015"
            ;;
            12)
                PRJ_GEN="Visual Studio 12 2013"
            ;;
            11)
                PRJ_GEN="Visual Studio 11 2012"
            ;;
            9)
                PRJ_GEN="Visual Studio 9 2008"
                if [ "${Platform}" = "64" ]; then
                    echo "Don't support Visual Studio 9 2008 for 64 bits in appveyor"
                    cd ${PROJECT_DIR}
                    exit 0
                fi
            ;;
        esac
        if [ "${Platform}" = "64" ]; then
            PRJ_GEN="${PRJ_GEN} Win64"
        fi
    ;;
    windows_mingw)
        PRJ_GEN="MSYS Makefiles"
    
        case ${TOOLCHAIN_VERSION} in
            630)
                if [ "${Platform}" = "64" ]; then
                    MINGW_PATH=/C/mingw-w64/x86_64-6.3.0-posix-seh-rt_v5-rev1/mingw64
                else
                    MINGW_PATH=/C/mingw-w64/i686-6.3.0-posix-dwarf-rt_v5-rev1/mingw32
                fi
            ;;
            530)
                if [ "${Platform}" = "32" ]; then
                    MINGW_PATH=/C/mingw-w64/i686-5.3.0-posix-dwarf-rt_v4-rev0/mingw32
                else
                    echo "Don't support ${TOOLCHAIN_VERSION} ${Platform} in appveyor."
                    cd ${PROJECT_DIR}
                    exit 0
                fi
            ;;
        esac
            
        if [ "${Platform}" = "64" ]; then
             export BUILD_CROSS_HOST=x86_64-w64-mingw32
        else
             export BUILD_CROSS_HOST=i686-w64-mingw32
        fi
        export BUILD_CROSS_SYSROOT=${MINGW_PATH}/${BUILD_CROSS_HOST}
        export PATH=${MINGW_PATH}/bin:$PATH
        CMAKE_PARA="${CMAKE_PARA} -DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/ci/CMake/Platforms/toolchain-mingw.cmake"
    ;;
    android*)
        PRJ_GEN="MSYS Makefiles"
        
        if [ "${APPVEYOR_BUILD_WORKER_IMAGE}" = "Visual Studio 2017" ]; then
            export ANDROID_NDK=/C/ProgramData/Microsoft/AndroidNDK64/android-ndk-r17
            HOST=windows-x86_64
        else
            export ANDROID_NDK=/C/ProgramData/Microsoft/AndroidNDK/android-ndk-r10e
            HOST=windows
        fi
        CMAKE_PARA="${CMAKE_PARA} -DCMAKE_TOOLCHAIN_FILE=$PROJECT_DIR/ci/CMake/Platforms/android.toolchain.cmake"
    
        case ${BUILD_TARGERT} in
            android_arm)
                if [ "${Platform}" = "64" ]; then
                    CMAKE_PARA="${CMAKE_PARA} -DANDROID_ABI=arm64-v8a"
                    export BUILD_CROSS_HOST=aarch64-linux-android
                    export BUILD_CROSS_SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_API}/arch-arm64
                else
                    export BUILD_CROSS_HOST=arm-linux-androideabi
                    CMAKE_PARA="${CMAKE_PARA} -DANDROID_ABI=armeabi-v7a"
                    export BUILD_CROSS_SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_API}/arch-arm
                fi
            ;;
            android_x86)
                if [ "${Platform}" = "64" ]; then
                    export BUILD_CROSS_HOST=x86_64
                    CMAKE_PARA="${CMAKE_PARA} -DANDROID_ABI=x86_64"
                    export BUILD_CROSS_SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_API}/arch-x86_64
                else
                    export BUILD_CROSS_HOST=x86
                    CMAKE_PARA="${CMAKE_PARA} -DANDROID_ABI=x86"
                    export BUILD_CROSS_SYSROOT=${ANDROID_NDK}/platforms/android-${ANDROID_API}/arch-x86
                fi
            ;;
        esac
        ANDROID_TOOLCHAIN_NAME=${BUILD_CROSS_HOST}-${TOOLCHAIN_VERSION}
        TOOLCHAIN_ROOT=${ANDROID_NDK}/toolchains/${ANDROID_TOOLCHAIN_NAME}/prebuilt/${HOST}
        export PATH=${TOOLCHAIN_ROOT}/bin:$PATH
        CMAKE_PARA="${CMAKE_PARA} -DANDROID_TOOLCHAIN_NAME=${ANDROID_TOOLCHAIN_NAME}"
    ;;
esac

if [ "${BUILD_TESTS}" = "ON" ]; then
    case ${BUILD_TARGERT} in
        windows*)
            # Install gtest ......
            cd ${PROJECT_DIR}
            mkdir build-gtest
            cd build-gtest
            GTEST_VERSION=1.8.1
            GTEST_INSTALL_PATH=${PROJECT_DIR}/install-gtest
            if [ ! -f ${PROJECT_DIR}/download/release-${GTEST_VERSION}.tar.gz ]; then 
                mkdir -p ${PROJECT_DIR}/download
                wget -O ${PROJECT_DIR}/download/release-${GTEST_VERSION}.tar.gz https://github.com/google/googletest/archive/release-${GTEST_VERSION}.tar.gz
            fi
            
            tar xf ${PROJECT_DIR}/download/release-${GTEST_VERSION}.tar.gz -C .
            cd googletest-release-${GTEST_VERSION}/
            cmake -DCVF_VERSION=${GTEST_VERSION} -DCMAKE_INSTALL_PREFIX:PATH=${GTEST_INSTALL_PATH} . \
                -G"${PRJ_GEN}" ${CMAKE_PARA}
            cmake --build . --target install 
            export PKG_CONFIG_PATH="$(find "${GTEST_INSTALL_PATH}" -type d -name pkgconfig)"
            sed 's,-I,-isystem ,g' -i "${PKG_CONFIG_PATH}"/gtest.pc  # suppress compile warnings
            CMAKE_PARA="${CMAKE_PARA} -DGTest_DIR=${GTEST_INSTALL_PATH}/lib/cmake/GTest -DBUILD_TESTS=ON"
        ;;
    esac
else
    CMAKE_PARA="${CMAKE_PARA} -DBUILD_TESTS=OFF"
fi

# Test automake
if [ "$TEST_AUTOMAKE" = "ON" ]; then
    case ${BUILD_TARGERT} in
        windows_mingw)
            echo "Test automake ......"
            
            pacman --noconfirm -S doxygen mingw-w64-i686-graphviz
            export PATH=$PATH:/C/msys64/mingw32/bin
            CONFIG_PARA="${CONFIG_PARA} --host=$BUILD_CROSS_HOST --target=$BUILD_CROSS_HOST"
            if [ "$SHARED" = "OFF" ]; then
                CONFIG_PARA="${CONFIG_PARA} --enable-static --disable-shared"
            else
                CONFIG_PARA="${CONFIG_PARA} --disable-static --enable-shared"
            fi
            
            cd ${PROJECT_DIR}
            bash autogen.sh
            mkdir -p build-automake
            cd build-automake
            if [ "${BUILD_TESTS}" = "ON" ]; then
                ../configure ${CONFIG_PARA} \
                    CFLAGS="--sysroot=${BUILD_CROSS_SYSROOT}" \
                    LDFLAGS="--sysroot=${BUILD_CROSS_SYSROOT}" \
                    GTEST_CFLAGS="-D__USE_MINGW_ANSI_STDIO -I${GTEST_INSTALL_PATH}/include" \
                    GTEST_LIBS=-L${GTEST_LIBS}/lib
            else
                ../configure ${CONFIG_PARA} \
                    CFLAGS="--sysroot=${BUILD_CROSS_SYSROOT}" \
                    LDFLAGS="--sysroot=${BUILD_CROSS_SYSROOT}"
            fi
            make -j`cat /proc/cpuinfo |grep 'cpu cores' |wc -l` all
            if [ "windows_mingw" = "${BUILD_TARGERT}" -a "${BUILD_TESTS}" = "ON" ]; then
                make -j`cat /proc/cpuinfo |grep 'cpu cores' |wc -l` check
                cat test/uriparser_test.log
                make -j`cat /proc/cpuinfo |grep 'cpu cores' |wc -l` distcheck
            fi
        ;;
        *)
            echo "Don't support TEST_AUTOMAKE on ${BUILD_TARGERT}"
            cd ${PROJECT_DIR}
            exit 0
        ;;
    esac
else
    # Test cmake
    cd ${PROJECT_DIR}
    mkdir -p build-cmake
    cd build-cmake
    echo "cmake .. -G\"${PRJ_GEN}\" -DCMAKE_INSTALL_PREFIX="${PROJECT_DIR}/install" -DCMAKE_BUILD_TYPE=${Configuration} ${CMAKE_PARA}"
    cmake .. \
        -G"${PRJ_GEN}" \
        -DCMAKE_INSTALL_PREFIX="${PROJECT_DIR}/install" \
        -DCMAKE_BUILD_TYPE=${Configuration} \
        -DCMAKE_VERBOSE_MAKEFILE=ON \
        ${CMAKE_PARA}
    cmake --build . --config ${Configuration} --target install --clean-first   
fi

cd ${PROJECT_DIR}
