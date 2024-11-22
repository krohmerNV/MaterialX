//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXTest/External/Catch/catch.hpp>
#include <MaterialXTest/MaterialXRender/RenderUtil.h>
#include <MaterialXTest/MaterialXGenMdl/GenMdl.h>

#include <MaterialXRenderMdl/MdlRenderer.h>

#include <MaterialXRender/StbImageLoader.h>
#if defined(MATERIALX_BUILD_OIIO)
    #include <MaterialXRender/OiioImageLoader.h>
#endif

#include <MaterialXGenMdl/MdlShaderGenerator.h>

#include <MaterialXFormat/Util.h>

namespace mx = MaterialX;

class MdlShaderRenderTester : public RenderUtil::ShaderRenderTester
{
  public:
    explicit MdlShaderRenderTester(mx::ShaderGeneratorPtr shaderGenerator) :
        RenderUtil::ShaderRenderTester(shaderGenerator)
    {
    }

  protected:
    void createRenderer(std::ostream& log) override;

    bool runRenderer(const std::string& shaderName,
                     mx::TypedElementPtr element,
                     mx::GenContext& context,
                     mx::DocumentPtr doc,
                     std::ostream& log,
                     const GenShaderUtil::TestSuiteOptions& testOptions,
                     RenderUtil::RenderProfileTimes& profileTimes,
                     const mx::FileSearchPath& imageSearchPath,
                     const std::string& outputPath = ".",
                     mx::ImageVec* imageVec = nullptr) override;

    MdlStringResolverPtr _mdlCustomResolver;
};

// Renderer setup
void MdlShaderRenderTester::createRenderer(std::ostream& log)
{
    std::string renderCommand(MATERIALX_MDL_RENDER_EXECUTABLE);
    mx::FilePath renderExec(MATERIALX_MDL_RENDER_EXECUTABLE);

    if (!renderExec.exists())
    {
        log << "MDL renderer executable not set: 'MATERIALX_MDL_RENDER_EXECUTABLE'" << std::endl;
        REQUIRE((false && "MDL renderer executable not set: 'MATERIALX_MDL_RENDER_EXECUTABLE'"));
        return;
    }

    _mdlCustomResolver = MdlStringResolver::create();
}

// Renderer execution
bool MdlShaderRenderTester::runRenderer(const std::string& shaderName,
                                        mx::TypedElementPtr element,
                                        mx::GenContext& context,
                                        mx::DocumentPtr doc,
                                        std::ostream& log,
                                        const GenShaderUtil::TestSuiteOptions& testOptions,
                                        RenderUtil::RenderProfileTimes& profileTimes,
                                        const mx::FileSearchPath&,
                                        const std::string& outputPath,
                                        mx::ImageVec* imageVec)
{
    std::cout << "Validating MDL rendering for: " << doc->getSourceUri() << std::endl;
    mx::ScopedTimer totalMDLTime(&profileTimes.languageTimes.totalTime);

    // Perform validation if requested
    if (testOptions.validateElementToRender)
    {
        std::string message;
        if (!element->validate(&message))
        {
            log << "Element is invalid: " << message << std::endl;
            return false;
        }
    }

    std::vector<mx::GenOptions> optionsList;
    getGenerationOptions(testOptions, context.getOptions(), optionsList);

    if (element && doc)
    {
        log << "------------ Run MDL validation with element: " << element->getNamePath() << "-------------------" << std::endl;

        for (const auto& options : optionsList)
        {
            profileTimes.elementsTested++;

            // prepare the output folder
            mx::FilePath outputFilePath = outputPath;

            // Use separate directory for reduced output
            if (options.shaderInterfaceType == mx::SHADER_INTERFACE_REDUCED)
            {
                outputFilePath = outputFilePath / mx::FilePath("reduced");
                // TODO pass this option to the renderer as well when supported
            }

            // Note: mkdir will fail if the directory already exists which is ok.
            {
                mx::ScopedTimer ioDir(&profileTimes.languageTimes.ioTime);
                outputFilePath.createDirectory();
            }
            std::string shaderPath = mx::FilePath(outputFilePath) / mx::FilePath(shaderName);

            try
            {
                // Use the same resolver as in the generator tests.
                // Here, we don't actually resolve but instead only use the MDL search paths.
                _mdlCustomResolver->initialize(doc, &log, {});

                // executable
                std::string renderCommand(MATERIALX_MDL_RENDER_EXECUTABLE);
                CHECK(!renderCommand.empty());

                 // use the same paths as the resolver
                for (const auto& sp : _mdlCustomResolver->getMdlSearchPaths())
                {
                    renderCommand += " --mdl_path \"" + sp.asString() + "\"";
                }

                mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
                mx::FilePath rootPath = searchPath.isEmpty() ? mx::FilePath() : searchPath[0];
                // set environment
                std::string iblFile = (rootPath / "resources/lights/san_giuseppe_bridge.hdr").asString();
                renderCommand += " --hdr \"" + iblFile + "\" --hdr_rotate 90";
                // set scene
                renderCommand += " --uv_scale 0.5 1.0 --uv_offset 0.0 0.0 --uv_repeat";
                renderCommand += " --uv_flip"; // this will flip the v coordinate of the vertices, which flips all the
                                               // UV operations. In contrast, the fileTextureVerticalFlip option will
                                               // only flip the image access nodes.
                renderCommand += " --camera 0 0 3 0 0 0 --fov 45";

                // set the material
                // the renderer supports MaterialX natively
                renderCommand += " --mat " + doc->getSourceUri() + "?name=" + element->getNamePath();

                // This must be a render args option. Rest are consistent between dxr and cuda example renderers.
                std::string renderArgs(MATERIALX_MDL_RENDER_ARGUMENTS);
                if (renderArgs.empty())
                {
                    // Assume MDL example DXR is being used and set reasonable arguments automatically
                    renderCommand += " --nogui --res 512 512 --iterations 1024 --max_path_length 3 --noaux --no_firefly_clamp";
                    renderCommand += " --background 0.073239 0.073239 0.083535";
                }
                else
                {
                    renderCommand += " " + renderArgs;
                }

                // Write out an .mdl file
                if (testOptions.dumpGeneratedCode)
                {
                    renderCommand += " -g " + shaderPath + ".mdl";
                }

                // set the output image file
#if defined(MATERIALX_BUILD_OIIO)
                renderCommand += " -o " + shaderPath + "_mdl.exr";
#else
                renderCommand += " -o " + shaderPath + "_mdl.png";
#endif
                // also create a full log
                renderCommand += " --log_file " + shaderPath + +".mdl_render_log.txt";

                // run the renderer executable
                int returnValue = std::system(renderCommand.c_str());

                mx::FilePath errorLogFile = shaderPath + ".mdl_render_log.txt";
                std::ifstream logStream(errorLogFile);
                mx::StringVec result;
                std::string line;
                bool writeLogCode = true;
                bool writeLogLine = false;
                while (std::getline(logStream, line))
                {
                    // add the return code to the log
                    if (writeLogCode)
                    {
                        log << renderCommand << std::endl;
                        log << "\tReturn code: " << std::to_string(returnValue) << std::endl;
                        writeLogCode = false;
                    }
                    // add errors and warnings to the log
                    if (line.find("[ERROR]") != std::string::npos || line.find("[WARNING]") != std::string::npos)
                    {
                        writeLogLine = true;
                    }
                    if (line.find("[INFO]") != std::string::npos || line.find("[VERBOSE]") != std::string::npos)
                    {
                        writeLogLine = false;
                    }
                    if (writeLogLine)
                    {
                        log << "\tLog: " << line << std::endl;
                    }
                }
                CHECK(returnValue == 0);
            }
            catch (mx::Exception& e)
            {
                log << ">> " << e.what() << "\n";
            }
        }
    }

    return true;
}

TEST_CASE("Render: MDL TestSuite", "[rendermdl]")
{
    mx::FileSearchPath searchPath = mx::getDefaultDataSearchPath();
    mx::FilePath optionsFilePath = searchPath.find("resources/Materials/TestSuite/_options.mtlx");

    MdlShaderRenderTester renderTester(mx::MdlShaderGenerator::create());
    renderTester.validate(optionsFilePath);
}
