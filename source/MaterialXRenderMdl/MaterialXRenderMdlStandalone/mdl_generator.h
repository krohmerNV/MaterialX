/******************************************************************************
 * Copyright 2025 NVIDIA Corporation. All rights reserved.
 *****************************************************************************/

 // examples/mdl_sdk/dxr/mdl_d3d12/materialx/mdl_generator.h

#ifndef MATERIALX_MDL_GENERATOR_H
#define MATERIALX_MDL_GENERATOR_H

#include <MaterialXGenMdl/MdlShaderGenerator.h>
#include <string>

namespace mi { namespace neuraylib {
    class IMdl_configuration;
}}


// --------------------------------------------------------------------------------------------

/// Basic MDL code-gen from MaterialX.
/// A more sophisticated supports will be needed for full function support.
class Mdl_generator
{
public:
    struct Result
    {
        std::string materialx_file_name;
        std::string generated_mdl_code;
        std::string materialx_material_name;
        std::string generated_mdl_name;
    };

    /// Constructor.
    explicit Mdl_generator();

    /// Destructor.
    ~Mdl_generator() = default;

    /// Specify an additional absolute search path location (e.g. '/projects/MaterialX').
    /// This path will be queried when locating standard data libraries,
    /// XInclude references, and referenced images.
    void add_materialx_search_path(const std::string& mtlx_path);

    /// Specify an additional relative path to a custom data library folder
    /// (e.g. 'libraries/custom'). MaterialX files at the root of this folder will be included
    /// in all content documents.
    void add_materialx_library(const std::string& mtlx_library);

    /// Specify the MDL language version the code generator should produce.
    void set_mdl_version(MaterialX::GenMdlOptions::MdlVersion target_version);

    /// set the main mtlx file of the material to generate code from.
    bool set_source(const std::string& mtlx_material, const std::string& material_name);

    /// generate MDL code
    bool generate(mi::neuraylib::IMdl_configuration* mdl_configuration, Result& inout_result) const;

private:
    std::vector<std::string> m_mtlx_search_paths;
    std::vector<std::string> m_mtlx_relative_library_paths;
    std::string m_mtlx_source;
    std::string m_mtlx_material_name;
    MaterialX::GenMdlOptions::MdlVersion m_mdl_version;
};

#endif
