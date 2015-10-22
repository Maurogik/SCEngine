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
        GLint instanceMatrixAttrib;
    };

    //Per mesh data
    struct MeshRenderData
    {
        MeshRenderData()
            : shaderData(),
              indiceBuffer(-1),
              indiceCount(0),
              vaoID(-1),
              attributes(),
              instancesBuffer(-1),
              instancesCount(0)
        {}
        std::map<GLuint,ShaderData>     shaderData;
        GLuint                          indiceBuffer;
        GLuint                          indiceCount;
        GLuint                          vaoID;
        std::vector<AttributeData>      attributes;
        GLuint                          instancesBuffer;
        uint                            instancesCount;
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

        struct RendererData
        {
            RendererData() : meshRenderData() {}
            ~RendererData()
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
        RendererData rendererData;


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
            MeshRenderData &renderData = rendererData.meshRenderData[meshId];

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

            data.instanceMatrixAttrib = glGetAttribLocation(programID, "instanceMatrix");
        }

        void cleanupGLRenderData(MeshRenderData& renderData)
        {
            for(size_t i = 0; i < renderData.attributes.size(); ++i){
                glDeleteBuffers(1, &(renderData.attributes[i].dataBufferId));
            }
            glDeleteBuffers(1, &(renderData.indiceBuffer));

            if(renderData.indiceBuffer != GLuint(-1) )
            {
                glDeleteBuffers(1, &(renderData.instancesBuffer));
            }
            glDeleteVertexArrays(1, &(renderData.vaoID));
        }

        MeshRenderData& getMeshRenderData(ui16 meshId)
        {
            Debug::Assert(rendererData.meshRenderData.count(meshId) > 0, "Render data for mesh : "
                          + std::to_string(meshId) + " not initialized yet");

            MeshRenderData& renderData = rendererData.meshRenderData[meshId];
            return renderData;
        }

        void prepareMeshAttributes(MeshRenderData& meshRenderData, GLuint shaderProgram)
        {
            if(meshRenderData.shaderData.count(shaderProgram) == 0)
            {
                initializeShaderData(meshRenderData, shaderProgram);
            }

            const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];
            const std::vector<AttributeData>& attributes = meshRenderData.attributes;

            //bind mesh vao
            glBindVertexArray(meshRenderData.vaoID);

            //set the attributes
            for(size_t i = 0; i < attributes.size(); ++i)
            {
                GLuint attribLocation = shaderData.attribLocations[i];

                if(attribLocation != GLuint(-1))
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
        }

        void cleanMeshAttributes(MeshRenderData& meshRenderData, GLint shaderProgram)
        {
            const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];
            //clean the attributes
            for(size_t i = 0; i < meshRenderData.attributes.size(); ++i)
            {
                GLuint attribLocation = shaderData.attribLocations[i];
                if(attribLocation < (GLuint)-1)
                {
                    glDisableVertexAttribArray(attribLocation);
                }
            }
        }
    }


    /***    Public interface    ***/

    void InitializeMeshRenderData(ui16 meshId)
    {
        //render data for this mesh are not initalized yet
        if(rendererData.meshRenderData.count(meshId) == 0)
        {
            initializeGLData(meshId);
        }
    }

    void DeleteMeshRenderData(ui16 meshId)
    {
        Debug::Assert(rendererData.meshRenderData.count(meshId) > 0, "Render data for mesh : "
                      + std::to_string(meshId) + " not initialized yet");

        MeshRenderData& renderData = rendererData.meshRenderData[meshId];

        cleanupGLRenderData(renderData);

        auto it = rendererData.meshRenderData.find(meshId);
        rendererData.meshRenderData.erase(it);
    }

    void RenderMesh(ui16 meshId, const glm::mat4& projectionMatrix,
                    const glm::mat4& viewMatrix, const glm::mat4& modelMatrix)
    {
        GLint shaderProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

        SCE::ShaderUtils::BindDefaultUniforms(shaderProgram, modelMatrix, viewMatrix, projectionMatrix);

        MeshRenderData& meshRenderData = getMeshRenderData(meshId);
        prepareMeshAttributes(meshRenderData, shaderProgram);

        GLuint indiceCount = meshRenderData.indiceCount;
        GLuint indiceBuffer = meshRenderData.indiceBuffer;

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,       // mode
                    indiceCount,        // count
                    GL_UNSIGNED_SHORT,  // type
                    (void*)0            // element array buffer offset
                    );

        cleanMeshAttributes(meshRenderData, shaderProgram);

        glBindVertexArray(0);
    }

    void MakeMeshInstanced(ui16 meshId)
    {
        MeshRenderData &renderData = getMeshRenderData(meshId);
        glGenBuffers(1, &(renderData.instancesBuffer));
    }

    void SetMeshInstances(ui16 meshId, const std::vector<mat4> &instanceMatrices, GLenum drawType)
    {
        MeshRenderData &renderData = getMeshRenderData(meshId);
        SCE::Debug::Assert(renderData.instancesBuffer != GLuint(-1),
                           std::string("Mesh was not set as instances,") +
                           "use 'MakeMeshInstanced' to set mesh as instanced");

        glBindBuffer(GL_ARRAY_BUFFER, renderData.instancesBuffer);
        int size = sizeof(mat4) * instanceMatrices.size();
        glBufferData(GL_ARRAY_BUFFER, size, &(instanceMatrices[0]), drawType);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        renderData.instancesCount = instanceMatrices.size();
    }

    void DrawInstances(ui16 meshId, const glm::mat4& projectionMatrix,
                       const glm::mat4& viewMatrix)
    {
        GLint shaderProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &shaderProgram);

        SCE::ShaderUtils::BindDefaultUniforms(shaderProgram, glm::mat4(), viewMatrix, projectionMatrix);

        MeshRenderData& meshRenderData = getMeshRenderData(meshId);

        SCE::Debug::Assert(meshRenderData.instancesBuffer != GLuint(-1),
                           std::string("Mesh was not set as instances,") +
                           "use 'MakeMeshInstanced' to set mesh as instanced");

        prepareMeshAttributes(meshRenderData, shaderProgram);

        const ShaderData &shaderData = meshRenderData.shaderData[shaderProgram];

        //bind buffer attribute for instanced rendering
        glBindBuffer(GL_ARRAY_BUFFER, meshRenderData.instancesBuffer);
        GLint instanceAttribLoc = shaderData.instanceMatrixAttrib;

        for (int i = 0; i < 4; i++)
        {
            // Set up the vertex attribute
            glVertexAttribPointer(instanceAttribLoc + i,             // Location
                                  4, GL_FLOAT, GL_FALSE,       // vec4
                                  sizeof(mat4),                // Stride
                                  (void *)(sizeof(vec4) * i)); // Start offset
            // Enable it
            glEnableVertexAttribArray(instanceAttribLoc + i);
            // Make it instanced
            glVertexAttribDivisor(instanceAttribLoc + i, 1);
        }

        GLuint indiceCount = meshRenderData.indiceCount;
        GLuint indiceBuffer = meshRenderData.indiceBuffer;

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer);

        // Draw the triangles !
        glDrawElementsInstanced(
                    GL_TRIANGLES,       // mode
                    indiceCount,       // count
                    GL_UNSIGNED_SHORT,  // type
                    (void*)0,            // element array buffer offset
                    meshRenderData.instancesCount);

        cleanMeshAttributes(meshRenderData, shaderProgram);
        glDisableVertexAttribArray(instanceAttribLoc);

        glBindVertexArray(0);
    }

}

}




