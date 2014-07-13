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
        Mesh(   std::vector<ushort> *indices,
                std::vector<vec3> *vertices,
                std::vector<vec3> *normals,
                std::vector<vec2> *uvs,
                std::vector<vec3> *tangents,
                std::vector<vec3> *bitangents
                );
        ~Mesh();


        std::vector<ushort> *GetIndices();
        std::vector<vec3>   *GetVertices();
        //optional
        std::vector<vec3>   *GetNormals();
        std::vector<vec2>   *GetUvs();
        std::vector<vec3>   *GetTangents();
        std::vector<vec3>   *GetBitangents();

        /*****Static*****/

        static Mesh*        LoadMesh(const std::string &filename);
        static Mesh*        CreateSphere(const ushort &radius, const ushort &tesselation);
        static Mesh*        CreateBox(const ushort &width, const ushort &height, const ushort &depth);

    protected :

        std::vector<ushort>     *mIndices;
        std::vector<vec3>       *mVertices;
        //optional
        std::vector<vec3>       *mNormals;
        std::vector<vec2>       *mUvs;
        std::vector<vec3>       *mTangents;
        std::vector<vec3>       *mBitangents;

    private :

        void resetMeshData();
    };

}


#endif
