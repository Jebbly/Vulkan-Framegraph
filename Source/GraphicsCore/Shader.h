#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <slang.h>
#include <slang-com-ptr.h>
#include <vulkan/vulkan.h>

#include "Parameters.h"

class Shader {
public:
    Shader(std::shared_ptr<Device> device, Slang::ComPtr<slang::IComponentType> program);
    ~Shader();

    inline VkShaderModule GetModule() const {return shader_module_;}
    inline const std::vector<std::shared_ptr<DescriptorSetLayout>>& GetParameterLayouts() const {return parameter_layouts_;}
    inline const char* GetEntryPointName() const {return entry_point_;}

private:
    std::shared_ptr<Device> device_;

    VkShaderModule shader_module_;
    const char* entry_point_;
    std::vector<std::shared_ptr<DescriptorSetLayout>> parameter_layouts_;

    void ExtractParameterLayouts(Slang::ComPtr<slang::IComponentType> program);
};

class ShaderCompiler {
public:
    ShaderCompiler(std::shared_ptr<Device> device);
    ~ShaderCompiler() = default;
    
    std::shared_ptr<Shader> LoadShader(const std::string& shader_file, const std::string& entry_point_name);

private:
    std::shared_ptr<Device> device_;
    Slang::ComPtr<slang::IGlobalSession> global_session_;

    // All shaders will share the same session
    Slang::ComPtr<slang::ISession> local_session_; 

    std::vector<const char*> search_paths_; 
    std::vector<std::pair<const char*, const char*>> shader_macros_;

    void CreateSession();
};