#include <vector>
#include <glm/glm.hpp>

#include "tangentspace.hpp"

void computeTangentBasis(
	// inputs
	std::vector<glm::vec3> & vertices,
	std::vector<glm::vec2> & uvs,
	std::vector<glm::vec3> & normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
){

	for (unsigned int i=0; i<vertices.size(); i+=3 ){

		// Shortcuts for vertices
		glm::vec3 & v0 = vertices[i+0];
		glm::vec3 & v1 = vertices[i+1];
		glm::vec3 & v2 = vertices[i+2];

		// Shortcuts for UVs
		glm::vec2 & uv0 = uvs[i+0];
		glm::vec2 & uv1 = uvs[i+1];
		glm::vec2 & uv2 = uvs[i+2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1-v0;
		glm::vec3 deltaPos2 = v2-v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1-uv0;
		glm::vec2 deltaUV2 = uv2-uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);

	}

	// See "Going Further"
	for (unsigned int i=0; i<vertices.size(); i+=1 )
	{
		glm::vec3 & n = normals[i];
		glm::vec3 & t = tangents[i];
		glm::vec3 & b = bitangents[i];
		
		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));
		
		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f){
			t = t * -1.0f;
		}

	}
}


void computeTangentBasisIndexed(std::vector<short unsigned int>& indices,
                                std::vector<glm::vec3>& vertices,
                                std::vector<glm::vec2>& uvs,
                                std::vector<glm::vec3>& normals,
                                std::vector<glm::vec3>& tangents,
                                std::vector<glm::vec3>& bitangents)
{
    //fill in the arrays (to be able to access them later)
    for (unsigned int i=0; i<vertices.size(); i++ ){
        tangents.push_back(glm::vec3(0.0));
        bitangents.push_back(glm::vec3(0.0));
    }

    for (unsigned int i=0; i<indices.size(); i+=3 ){

        // Shortcuts for vertices
        glm::vec3 & v0 = vertices[indices[i+0]];
        glm::vec3 & v1 = vertices[indices[i+1]];
        glm::vec3 & v2 = vertices[indices[i+2]];

        // Shortcuts for UVs
        glm::vec2 & uv0 = uvs[indices[i+0]];
        glm::vec2 & uv1 = uvs[indices[i+1]];
        glm::vec2 & uv2 = uvs[indices[i+2]];

        // Edges of the triangle : postion delta
        glm::vec3 deltaPos1 = v1-v0;
        glm::vec3 deltaPos2 = v2-v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1-uv0;
        glm::vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
        glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

        // Set the same tangent for all three vertices of the triangle.
        // They will be merged later, in vboindexer.cpp
        tangents[indices[i+0]] = tangent;
        tangents[indices[i+1]] = tangent;
        tangents[indices[i+2]] = tangent;

        // Same thing for binormals
        bitangents[indices[i+0]] = bitangent;
        bitangents[indices[i+1]] = bitangent;
        bitangents[indices[i+2]] = bitangent;

    }

    // See "Going Further"
    for (unsigned int i=0; i<vertices.size(); i+=1 )
    {
        glm::vec3 & n = normals[i];
        glm::vec3 & t = tangents[i];
        glm::vec3 & b = bitangents[i];

        // Gram-Schmidt orthogonalize
        t = glm::normalize(t - n * glm::dot(n, t));

        // Calculate handedness
        if (glm::dot(glm::cross(n, t), b) < 0.0f){
            t = t * -1.0f;
        }

    }
}
