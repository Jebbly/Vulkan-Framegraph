#include "Shader.h"

#include <iostream>

ShaderCompiler::ShaderCompiler() {
    search_paths_.push_back(SHADER_DIRECTORY);
    CreateSession();
}

std::shared_ptr<Shader> ShaderCompiler::LoadShader(const std::string& shader_file, const std::string& entry_point_name) {
    Slang::ComPtr<slang::IBlob> diagnostics;
    slang::IModule* module = local_session_->loadModule(shader_file.c_str(), diagnostics.writeRef());

    Slang::ComPtr<slang::IEntryPoint> entry_point;
    module->findEntryPointByName(entry_point_name.c_str(), entry_point.writeRef());

    slang::IComponentType* components[] = { module, entry_point };
    Slang::ComPtr<slang::IComponentType> program;
    local_session_->createCompositeComponentType(components, 2, program.writeRef());

    slang::ProgramLayout* layout = program->getLayout();

    Slang::ComPtr<slang::IBlob> spirv_code;
    SlangResult result = program->getEntryPointCode(
            0, 0, spirv_code.writeRef(), nullptr);

    // This should be recursive
    uint32_t parameters = layout->getParameterCount();
    for (uint32_t i = 0; i < parameters; i++) {
        slang::VariableLayoutReflection* variable = layout->getParameterByIndex(i);
        std::cout << "set " << variable->getBindingSpace() << ", binding " << variable->getBindingIndex()
                  << ": " << variable->getType()->getName() << " " << variable->getName() << std::endl;

        slang::ParameterCategory category = variable->getCategory();
        uint32_t category_count = variable->getCategoryCount();
        std::cout << "Category count: " << category_count << std::endl;

        slang::TypeLayoutReflection* type_layout = variable->getTypeLayout();
        slang::TypeReflection::Kind kind = type_layout->getKind();
        slang::TypeReflection* type_reflection = variable->getType()->getElementType();
        unsigned fieldCount = type_reflection->getFieldCount();
        for (unsigned ff = 0; ff < fieldCount; ff++)
        {
            slang::VariableReflection* field = type_reflection->getFieldByIndex(ff);
            std::cout << field->getType()->getName() << " " << field->getName() << std::endl;
        }

    }

    return std::make_shared<Shader>();
}

void ShaderCompiler::CreateSession() {
    slang::createGlobalSession(global_session_.writeRef());

    slang::TargetDesc target_info = {
        .format = SLANG_SPIRV,
        .profile = global_session_->findProfile("glsl_450"),
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