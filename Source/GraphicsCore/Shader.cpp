#include "Shader.h"

#include <iostream>

Shader::Shader(std::shared_ptr<Device> device, Slang::ComPtr<slang::IComponentType> program) :
    device_{ device },
    shader_module_{ VK_NULL_HANDLE }
{
    slang::ProgramLayout* layout = program->getLayout();
    slang::EntryPointReflection* entry_point = layout->getEntryPointByIndex(0);
    entry_point_ = entry_point->getName();

    Slang::ComPtr<slang::IBlob> spirv_code;
    Slang::ComPtr<slang::IBlob> diagnostics;
    SlangResult result = program->getEntryPointCode(
        0, 0, spirv_code.writeRef(), diagnostics.writeRef());

    if (diagnostics) {
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    }

    VkShaderModuleCreateInfo module_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv_code->getBufferSize(),
        .pCode = static_cast<const uint32_t*>(spirv_code->getBufferPointer()),
    };

    vkCreateShaderModule(device_->GetLogicalDevice(), &module_info, nullptr, &shader_module_);

    ExtractParameterLayouts(program);
}

Shader::~Shader() {
    if (shader_module_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_->GetLogicalDevice(), shader_module_, nullptr);
    }
}

void Shader::ExtractParameterLayouts(Slang::ComPtr<slang::IComponentType> program) {
    slang::ProgramLayout* layout = program->getLayout();
    /*
    uint32_t num_parameters = layout->getParameterCount();
    
    for (uint32_t parameter_index = 0; parameter_index < num_parameters; parameter_index++) {
        slang::VariableLayoutReflection* variable = layout->getParameterByIndex(parameter_index);
        std::cout << "set " << variable->getBindingSpace() << ", binding " << variable->getBindingIndex()
            << ": " << variable->getType()->getName() << " " << variable->getName() << std::endl;
    }
    */

    // TODO: this is just a dummy implementation for the HelloWorldCompute example
    DescriptorSetLayout::BindingInfo binding_info = {
        .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .stage_flags = VK_SHADER_STAGE_COMPUTE_BIT,
    };

    std::shared_ptr<DescriptorSetLayout> set_layout = std::make_shared<DescriptorSetLayout>(device_);
    set_layout->AddBinding(binding_info);
    set_layout->AddBinding(binding_info);
    set_layout->AddBinding(binding_info); // 3 buffers used here
    set_layout->Compile();
    parameter_layouts_.push_back(set_layout);
}

ShaderCompiler::ShaderCompiler(std::shared_ptr<Device> device) :
    device_{ device }
{
    search_paths_.push_back(SHADER_DIRECTORY);
    CreateSession();
}

std::shared_ptr<Shader> ShaderCompiler::LoadShader(const std::string& shader_file, const std::string& entry_point_name) {
    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = local_session_->loadModule(shader_file.c_str(), diagnostics.writeRef());

    if (diagnostics) {
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    }

    Slang::ComPtr<slang::IEntryPoint> entry_point;
    module->findEntryPointByName(entry_point_name.c_str(), entry_point.writeRef());

    slang::IComponentType* components[] = { module, entry_point };
    Slang::ComPtr<slang::IComponentType> program;
    local_session_->createCompositeComponentType(components, 2, program.writeRef(), diagnostics.writeRef());

    if (diagnostics) {
        fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
    }

    return std::make_shared<Shader>(device_, program);
}

void ShaderCompiler::CreateSession() {
    slang::createGlobalSession(global_session_.writeRef());

    slang::TargetDesc target_info = {
        .format = SLANG_SPIRV,
        .profile = global_session_->findProfile("glsl_450"),
        .flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY,
    };

    std::vector<slang::PreprocessorMacroDesc> macros;
    for (const auto& macro : shader_macros_) {
        macros.emplace_back(
            slang::PreprocessorMacroDesc {
                .name = macro.first,
                .value = macro.second,
            }
        );
    }

    slang::SessionDesc session_info = {
        .targets = &target_info,
        .targetCount = 1,
        .searchPaths = search_paths_.data(),
        .searchPathCount = static_cast<uint32_t>(search_paths_.size()),
        .preprocessorMacros = macros.data(),
        .preprocessorMacroCount = static_cast<uint32_t>(macros.size()),
    };

    global_session_->createSession(session_info, local_session_.writeRef());
}