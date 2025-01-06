/******************************************************************************
 * Copyright 2024 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

// examples/mdl_sdk/shared/utils/mdl.cpp
//
// Code shared by all examples

#include "mdl.h"

#include "io.h"

namespace mi { namespace examples { namespace mdl {

// Indicates whether that directory has a mdl/nvidia/sdk_examples subdirectory.
bool is_examples_root(const std::string& path)
{
    std::string subdirectory = path + io::sep() + "mdl/nvidia/sdk_examples";
    return io::directory_exists(subdirectory);
}

// Intentionally not implemented inline which would require callers to define MDL_SAMPLES_ROOT.
std::string get_examples_root()
{
    std::string path = mi::examples::os::get_environment("MDL_SAMPLES_ROOT");
    if (!path.empty())
        return io::normalize(path);


    path = io::get_executable_folder();
    while (!path.empty()) {
        if (is_examples_root(path))
            return io::normalize(path);
        path = io::dirname(path);
    }

#ifdef MDL_SAMPLES_ROOT
    path = MDL_SAMPLES_ROOT;
    if (is_examples_root(path))
        return io::normalize(path);
#endif

    return ".";
}

// Indicates whether that directory contains nvidia/core_definitions.mdl and nvidia/axf_to_mdl.mdl.
bool is_src_shaders_mdl(const std::string& path)
{
    std::string file1 = path + io::sep() + "nvidia" + io::sep() + "core_definitions.mdl";
    if (!io::file_exists(file1))
        return false;

    std::string file2 = path + io::sep() + "nvidia" + io::sep() + "axf_to_mdl.mdl";
    if (!io::file_exists(file2))
        return false;

    return true;
}

// Intentionally not implemented inline which would require callers to define MDL_SRC_SHADERS_MDL.
std::string get_src_shaders_mdl()
{
    std::string path = mi::examples::os::get_environment("MDL_SRC_SHADERS_MDL");
    if (!path.empty())
        return io::normalize(path);

#ifdef MDL_SRC_SHADERS_MDL
    path = MDL_SRC_SHADERS_MDL;
    if (is_src_shaders_mdl(path))
        return io::normalize(path);
#endif

    return ".";
}

}}}
