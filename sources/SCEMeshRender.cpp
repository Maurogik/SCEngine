/******PROJECT:Sand Castle Engine******/
/**************************************/
/*********AUTHOR:Gwenn AUBERT**********/
/********FILE:SCEMeshRender.cpp********/
/**************************************/


#include "../headers/SCEMeshRender.hpp"
#include "../headers/SCEShaders.hpp"
#include "../headers/SCEMeshLoader.hpp"
#include "../headers/SCETools.hpp"
#include "../headers/SCEInternal.hpp"
#include "../headers/SCERenderStructs.hpp"


namespace SCE
{

namespace MeshRender
{
    enum ATTRIBUTE_TYPE
    {
        VERTEX_POSITION = 0,
        VERTEX_UV,
        VERTEX_NORMAL,
        VERTEX_TANGENT,
        VERTEX_BITANGENT,
        VERTEX_ATTRIB_COUNT
    };

    struct AttributeData
    {
        GLuint                      dataBufferId;
        void*                       buffer;
        size_t                      nbValues;
        GLenum                      type;
    };

    //store attributes locations per shader
    struct ShaderData
    {
        GLint attribLocations[VERTEX_ATTRIB_COUNT];
    };

    //Per mesh data
    struct MeshRenderData
    {
        MeshRenderData()
            : shaderData(),
              indiceBuffer(-1),
              indiceCount(0),
              vaoID(-1),
              attributes()
        {}
        std::map<GLuint,ShaderData>     shaderData;
        GLuint                          indiceBuffer;
        GLuint                          indiceCount;
        GLuint                          vaoID;
        std::vector<AttributeData>      attributes;
    };


    //unnamed namspace, translation unit local
    namespace
    {
        void cleanupGLRenderData(MeshRenderData& renderData);

        std::string attribNames[5] =
        {
            "vertexPosition_modelspace",
            "vertexUV",
            "vertexNormal_modelspace",
            "vertexTangent",
            "vertexBitangent"
        };

        struct MeshRendererData
        {
            MeshRendererData() : meshRenderData() {}
            ~MeshRendererData()
            {
                Internal::Log("Cleaning up mesh render system, will delete Vaos and Vbos");
                auto beginIt = begin(meshRenderData);
                auto endIt = end(meshRenderData);
                for(auto iterator = beginIt; iterator != endIt; iterator++) {
                    cleanupGLRenderData(iterator->second);
                }
            }

            std::map<uint, MeshRenderData>  meshRenderData;
        };

        /***    Mesh Render System Data variable    ***/
        MeshRendererData meshRendererData;


        void addAttribute(MeshRenderData& renderData,
                                         void* buffer,
                                         size_t size,
                                         GLenum type,
                                         size_t nbValues)
        {
            AttributeData attribData;

            glGenBuffers(1, &attribData.dataBufferId);
            glBindBuffer(GL_ARRAY_BUFFER, attribData.dataBufferId);
            glBufferData(GL_ARRAY_BUFFER
                         , size
                         , buffer, GL_STATIC_DRAW);

            attribData.type = type;
            attribData.nbValues = nbValues;
            attribData.buffer = buffer;
            renderData.attributes.push_back(attribData);
        }

        void initializeGLData(ui16 meshId)
        {
            Internal::Log("Initializing mesh renderer data");

            const MeshData& meshData = SCE::MeshLoader::GetMeshData(meshId);

            GLuint vaoId;
            /* Allocate and assign a Vertex Array Object*/
            glGenVertexArrays(1, &vaoId);

            /* Bind the Vertex Array Object as the current used object */
            glBindVertexArray(vaoId);

            //initialize and get reference at the same time
            MeshRenderData &renderData = meshRendererData.meshRenderData[meshId];

            renderData.indiceCount = meshData.indices.size();
            renderData.vaoID = vaoId;

            //vertex positions
            addAttribute(   renderData,
                            (void*)(&(meshData.vertices[0])),
                            meshData.vertices.size() * sizeof(glm::vec3),
                            GL_FLOAT,
                            3);

            //vertex uvs
            addAttribute( renderData,
                          (void*)(&(meshData.uvs[0]))
                        , meshData.uvs.size() * sizeof(glm::vec2)
                        , GL_FLOAT
                        , 2);

            //vertex normals
            addAttribute( renderData,
                          (void*)(&(meshData.normals[0]))
                        , meshData.normals.size() * sizeof(glm::vec3)
                        , GL_FLOAT
                        , 3);

            //vertex tangents
            if(meshData.tangents.size() > 0){
                addAttribute( renderData,
                              (void*)(&(meshData.tangents[0]))
                            , meshData.tangents.size() * sizeof(glm::vec3)
                            , GL_FLOAT
                            , 3);
            }

            //vertex bitangents
            if(meshData.bitangents.size() > 0){
                addAttribute( renderData,
                              (void*)(&(meshData.bitangents[0]))
                            , meshData.bitangents.size() * sizeof(glm::vec3)
                            , GL_FLOAT
                            , 3);
            }

            GLuint indiceBuffer;
            glGenBuffers(1, &indiceBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER
                         , meshData.indices.size() * sizeof(unsigned short)
                         , &(meshData.indices[0]), GL_STATIC_DRAW);

            renderData.indiceBuffer = indiceBuffer;

            glEnableVertexAttribArray(0); // Disable the Vertex Buffer Object
            glBindVertexArray(0); // Disable the Vertex Array Object
        }

        void initializeShaderData(MeshRenderData& renderData, GLuint programID)
        {
            ShaderData& data = renderData.shaderData[programID];

            //preload attribute locations
            for(int i = 0; i < VERTEX_ATTRIB_COUNT; ++i)
            {
                GLuint id = glGetAttribLocation(programID, attribNames[i].c_str());
                data.attribLocations[i] = id;
            }
        }

        void cleanupGLRenderData(MeshRenderData& renderData)
        {
            for(size_t i = 0; i < renderData.attributes.size(); ++i){
                glDeleteBuffers(1, &(renderData.attributes[i].dataBufferId));
            }
            glDeleteBuffers(1, &(renderData.indiceBuffer));
            glDeleteVertexArrays(1, &(renderData.vaoID));
        }
    }


    /***    Public interface    ***/

    void InitializeMeshRenderData(ui16 meshId)
    {
        //render data for this mesh are not initalized yet
        if(meshRendererData.meshRenderData.count(meshId) == 0)
        {
            initializeGLData(meshId);
        }
    }

    MeshRenderData& GetMeshRenderData(ui16 meshId, GLuint shaderProgram)
    {
        Debug::Assert(meshRendererData.meshRenderData.count(meshId) > 0, "Render data for mesh : "
                      + std::to_string(meshId) + " not initialized yet");

        MeshRenderData& renderData = meshRendererData.meshRenderData[meshId];

        if(renderData.shaderData.count(shaderProgram) == 0)
        {
            initializeShaderData(renderData, shaderProgram);
        }
        return renderData;
    }

    void DeleteMeshRenderData(ui16 meshId)
    {
        Debug::Assert(meshRendererData.meshRenderData.count(meshId) > 0, "Render data for mesh : "
                      + std::to_string(meshId) + " not initialized yet");

        MeshRenderData& renderData = meshRendererData.meshRenderData[meshId];

        cleanupGLRenderData(renderData);

        auto it = meshRendererData.meshRenderData.find(meshId);
        meshRendererData.meshRenderData.erase(it);
    }

    void RenderMesh(ui16 meshId, const glm::mat4& projectionMatrix,
                                   const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
    {
        GLint shaderProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

        SCE::ShaderUtils::BindDefaultUniforms(shaderProgram, modelMatrix, viewMatrix, projectionMatrix);

        MeshRenderData& meshRenderData = GetMeshRenderData(meshId, shaderProgram);
        const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];
        const std::vector<AttributeData>& attributes = meshRenderData.attributes;
        GLuint indiceCount = meshRenderData.indiceCount;
        GLuint indiceBuffer = meshRenderData.indiceBuffer;

        //bind mesh data
        glBindVertexArray(meshRenderData.vaoID);

        //set the attributes
        for(size_t i = 0; i < attributes.size(); ++i)
        {
            GLuint attribLocation = shaderData.attribLocations[i];

            if(attribLocation < (GLuint)-1)
            {
                glEnableVertexAttribArray(attribLocation);
                glBindBuffer(GL_ARRAY_BUFFER, attributes[i].dataBufferId);
                glVertexAttribPointer(
                            attribLocation,             // The attribute we want to configure
                            attributes[i].nbValues,     // size
                            attributes[i].type,         // typeC
                            GL_FALSE,                   // normalized?
                            0,                          // stride
                            (void*)0                    // array buffer offset
                            );
            }
        }

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,       // mode
                    indiceCount,       // count
                    GL_UNSIGNED_SHORT,  // type
                    (void*)0            // element array buffer offset
                    );

        //clean the attributes
        for(size_t i = 0; i < attributes.size(); ++i)
        {
            GLuint attribLocation = shaderData.attribLocations[i];
            if(attribLocation < (GLuint)-1)
            {
                glDisableVertexAttribArray(attribLocation);
            }
        }

        glBindVertexArray(0);
    }
}

}




