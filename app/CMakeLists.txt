# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/cpp/IMU_CardBoard/)


# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
             src/main/cpp/NativeApp.cpp
             src/main/cpp/LogUtils.cpp
             src/main/cpp/MessageQueue.cpp
             src/main/cpp/RingBuffer.cpp
             src/main/cpp/GlProgram.cpp
             src/main/cpp/GlGeometry.cpp
             src/main/cpp/IMU_CardBoard/head_tracker.cpp
             src/main/cpp/IMU_CardBoard/sensors/gyroscope_bias_estimator.cpp
             src/main/cpp/IMU_CardBoard/sensors/lowpass_filter.cpp
             src/main/cpp/IMU_CardBoard/sensors/mean_filter.cpp
             src/main/cpp/IMU_CardBoard/sensors/median_filter.cpp
             src/main/cpp/IMU_CardBoard/sensors/neck_model.cpp
             src/main/cpp/IMU_CardBoard/sensors/pose_prediction.cpp
             src/main/cpp/IMU_CardBoard/sensors/sensor_fusion_ekf.cpp
             src/main/cpp/IMU_CardBoard/sensors/android/device_accelerometer_sensor.cpp
             src/main/cpp/IMU_CardBoard/sensors/android/device_gyroscope_sensor.cpp
             src/main/cpp/IMU_CardBoard/sensors/android/sensor_event_producer.cpp
             src/main/cpp/IMU_CardBoard/util/matrix_3x3.cpp
             src/main/cpp/IMU_CardBoard/util/matrix_4x4.cpp
             src/main/cpp/IMU_CardBoard/util/matrixutils.cpp
             src/main/cpp/IMU_CardBoard/util/rotation.cpp
             src/main/cpp/IMU_CardBoard/util/vectorutils.cpp
             src/main/cpp/IMU_Oculus/OVR_Alg.cpp
             src/main/cpp/IMU_Oculus/OVR_Allocator.cpp
             src/main/cpp/IMU_Oculus/OVR_Atomic.cpp
             src/main/cpp/IMU_Oculus/OVR_Fusion.cpp
             src/main/cpp/IMU_Oculus/OVR_Lockless.cpp
             src/main/cpp/IMU_Oculus/OVR_Log.cpp
             src/main/cpp/IMU_Oculus/OVR_Math.cpp
             src/main/cpp/IMU_Oculus/OVR_PhoneSensor.cpp
             src/main/cpp/IMU_Oculus/OVR_RefCount.cpp
             src/main/cpp/IMU_Oculus/OVR_SensorFilter.cpp
             src/main/cpp/IMU_Oculus/OVR_Std.cpp
             src/main/cpp/IMU_Oculus/OVR_System.cpp
             src/main/cpp/IMU_Oculus/OVR_ThreadCommandQueue.cpp
             src/main/cpp/IMU_Oculus/OVR_ThreadsPthread.cpp
             src/main/cpp/IMU_Oculus/OVR_Timer.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                        GLESv3
                        EGL
                        android
                        )