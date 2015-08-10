/**********Sand Castle Engine**********/
/**************************************/
/******** AUTHOR : Gwenn AUBERT *******/
/*********** FILE : Mesh.hpp **********/
/**************************************/
#ifndef SCE_MESH_HPP
#define SCE_MESH_HPP

#include "SCEDefines.hpp"
#include "Component.hpp"

namespace SCE {

    class Mesh  : public Component {

    public :

        ~Mesh();

        std::vector<ushort> *GetIndices();
        std::vector<vec3>   *GetVertices();
        //optional
        std::vector<vec3>   *GetNormals();
        std::vector<vec2>   *GetUvs();
        std::vector<vec3>   *GetTangents();
        std::vector<vec3>   *GetBitangents();

        /*****Static*****/

        static SCEHandle<Mesh> AddCustomMesh (  SCEHandle<Container>& container, const std::string &filename);
        static SCEHandle<Mesh> AddSphereMesh (  SCEHandle<Container>& container, const float &radius, const float &tesselation);
        static SCEHandle<Mesh> AddCubeMesh   (  SCEHandle<Container>& container, const float &cubeSize);
        static SCEHandle<Mesh> AddQuadMesh   (  SCEHandle<Container>& container, const float &width, const float &height);
        static SCEHandle<Mesh> AddConeMesh   (  SCEHandle<Container>& container, const float &length, const float &angle, const float &tesselation);
        static SCEHandle<Mesh> AddCustomMesh (  SCEHandle<Container>& container,
                                                std::vector<ushort> *indices,
                                                std::vector<vec3>   *vertices,
                                                std::vector<vec3>   *normals,
                                                std::vector<vec2>   *uvs,
                                                std::vector<vec3>   *tangents,
                                                std::vector<vec3>   *bitangents
                                            );

    protected :

        std::vector<ushort>     *mIndices;
        std::vector<vec3>       *mVertices;
        //optional
        std::vector<vec3>       *mNormals;
        std::vector<vec2>       *mUvs;
        std::vector<vec3>       *mTangents;
        std::vector<vec3>       *mBitangents;

    protected :

        //Mesh(   SCEHandle<Container>& container);
        Mesh(   SCEHandle<Container>& container, const std::string& filename);
        /*Mesh(   SCEHandle<Container>& container, const ushort &radius, const ushort &tesselation);
        Mesh(   SCEHandle<Container>& container, const ushort &size);*/
        Mesh(   SCEHandle<Container>& container,
                std::vector<ushort> *indices,
                std::vector<vec3>   *vertices,
                std::vector<vec3>   *normals,
                std::vector<vec2>   *uvs,
                std::vector<vec3>   *tangents,
                std::vector<vec3>   *bitangents
                );

    private :

        void                initMeshData(   std::vector<ushort> *indices,
                                            std::vector<vec3> *vertices,
                                            std::vector<vec3> *normals,
                                            std::vector<vec2> *uvs,
                                            std::vector<vec3> *tangents,
                                            std::vector<vec3> *bitangents
                                            );
        void                resetMeshData();
    };

}


#endif
