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

    struct MeshData;

    class Mesh  : public Component {

    public :

        const MeshData&        GetMeshData() const;
        uint                   GetMeshId();

        /*****Static*****/

        static SCEHandle<Mesh> AddCustomMesh (  SCEHandle<Container>& container,
                                                const std::string &filename);
        static SCEHandle<Mesh> AddSphereMesh (  SCEHandle<Container>& container,
                                                float tesselation);
        static SCEHandle<Mesh> AddCubeMesh   (  SCEHandle<Container>& container);

        static SCEHandle<Mesh> AddQuadMesh   (  SCEHandle<Container>& container);

        static SCEHandle<Mesh> AddConeMesh   (  SCEHandle<Container>& container,
                                                float angle, float tesselation);
        static SCEHandle<Mesh> AddCustomMesh (  SCEHandle<Container>& container,
                                                const std::vector<ushort> &indices,
                                                const std::vector<vec3>   &vertices,
                                                const std::vector<vec3>   &normals,
                                                const std::vector<vec2>   &uvs,
                                                const std::vector<vec3>   &tangents,
                                                const std::vector<vec3>   &bitangents
                                             );

    protected :

        Mesh(   SCEHandle<Container>& container, const std::string& filename, bool windCW = false);
        Mesh(   SCEHandle<Container>& container,
                const std::vector<ushort> &indices,
                const std::vector<vec3>   &vertices,
                const std::vector<vec3>   &normals,
                const std::vector<vec2>   &uvs,
                const std::vector<vec3>   &tangents,
                const std::vector<vec3>   &bitangents
                );
        Mesh(   SCEHandle<Container>& container, uint meshId);

    private :

        uint        mMeshId;
    };

}


#endif
