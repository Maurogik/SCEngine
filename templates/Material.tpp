/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/**********FILE:Material.tpp***********/
/**************************************/


template<typename T>
void SCE::Material::SetUniformValue(const std::string& uniformName, const T& value)
{
    SCE::Debug::Assert(mUniforms.count(uniformName) > 0, "ERROR : This uniform does not exist");
    mUniforms[uniformName].data = new T(value);
}

template<typename T>
const T&  SCE::Material::GetUniformValue(const std::string& uniformName) const
{
    SCE::Debug::Assert(mUniforms.count(uniformName) > 0, "ERROR : This uniform does not exist");
    return *(T*)(mUniforms.at(uniformName).data);
}
