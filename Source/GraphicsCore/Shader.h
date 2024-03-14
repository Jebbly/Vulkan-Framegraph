#pragma once

#include <string>
#include <utility>
#include <vector>

#include <slang.h>
#include <slang-com-ptr.h>

class Shader {

};

class ShaderCompiler {
public:
    ShaderCompiler();
    ~ShaderCompiler() = default;
    
    Shader LoadShader(const std::string& shader_file, const std::string& entry_point_name);

private:
   Slang::ComPtr<slang::IGlobalSession> global_session_;

   // All shaders will share the same session
   Slang::ComPtr<slang::ISession> local_session_; 

   std::vector<const char*> search_paths_; 
   std::vector<std::pair<const char*, const char*>> shader_macros_;

   void CreateSession();
};