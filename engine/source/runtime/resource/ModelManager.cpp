#include "ModelManager.h"

#define TINYOBJLOADER_IMPLEMENTATION

namespace VlkEngine {
	
	void ModelManager::SetModel(std::string modelpath, std::string texturepath)
	{
		modelPath = modelpath;
		texturePath = texturepath;
		LoadModel();
	}

	stbi_uc* ModelManager::LoadModelTexture(int* texWidth, int* texHeight, int* texChannels)
	{
		return FileService::LoadAssetImage(texturePath, texWidth, texHeight, texChannels, STBI_rgb_alpha);
	}

	void ModelManager::LoadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{}; // Vertex : vertexindex in indices


#ifndef CALCULATE_VERTEX_NORMAL

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				if (index.normal_index >= 0) {
					vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
					};
				}
				if (index.texcoord_index >= 0) {
					vertex.texCoord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};
				}
				vertex.color = { 1.0f, 1.0f, 1.0f };
				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}

#else			
		std::unordered_map<uint32_t, std::vector<uint32_t>> vertexFaces{}; //vertexindex in objfile : faceindex
		std::unordered_map<uint32_t, glm::vec3> faceNormal{}; // faceindex : facenormal
		std::unordered_map<uint32_t, float> faceArea{}; // faceindex : facearea
		
		size_t faceoffset = 0;
		for (size_t s = 0; s < shapes.size(); s++) {
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++, faceoffset++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				if (fv < 3) continue;
				glm::vec3 veca = {
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset].vertex_index) + 0],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset].vertex_index) + 1],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset].vertex_index) + 2]
				};
				glm::vec3 vecb = {
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 1].vertex_index) + 0],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 1].vertex_index) + 1],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 1].vertex_index) + 2]
				};
				glm::vec3 vecc = {
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 2].vertex_index) + 0],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 2].vertex_index) + 1],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + 2].vertex_index) + 2]
				};
				faceNormal[faceoffset] = CalcTriangleNormal(veca, vecb, vecc);

				faceArea[faceoffset] = 0;
				for (size_t v = 1; v < fv - 1; v++) {
					glm::vec3 vece = {
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v].vertex_index) + 0],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v].vertex_index) + 1],
					attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v].vertex_index) + 2]
					};
					glm::vec3 vecf = {
						attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v + 1].vertex_index) + 0],
						attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v + 1].vertex_index) + 1],
						attrib.vertices[3 * size_t(shapes[s].mesh.indices[index_offset + v + 1].vertex_index) + 2]
					};
					faceArea[faceoffset] += CalcTriangleArea(veca, vece, vecf);
				}

				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					vertexFaces[idx.vertex_index].push_back(faceoffset);
				}
				index_offset += fv;
			}
		}

		faceoffset = 0;
		for (size_t s = 0; s < shapes.size(); s++) {
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++,faceoffset++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

				for (size_t v = 0; v < fv; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					Vertex vertex{};
					vertex.pos = {
						attrib.vertices[3 * idx.vertex_index + 0],
						attrib.vertices[3 * idx.vertex_index + 1],
						attrib.vertices[3 * idx.vertex_index + 2]
					};
					if (idx.texcoord_index >= 0) {
						vertex.texCoord = {
							attrib.texcoords[2 * idx.texcoord_index + 0],
							1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
						};
					}
					vertex.color = { 1.0f, 1.0f, 1.0f };

					vertex.normal = { 0,0,0 };
					float faceareasum = 0;
					for (uint32_t vf = 0; vf < vertexFaces[idx.vertex_index].size(); vf++) {
						vertex.normal += faceNormal[vertexFaces[idx.vertex_index][vf]] *
							(faceArea[vertexFaces[idx.vertex_index][vf]]);
						faceareasum += faceArea[vertexFaces[idx.vertex_index][vf]];
					}
					if (faceareasum != 0) {
						vertex.normal = glm::normalize(vertex.normal / faceareasum);
					}


					if (uniqueVertices.count(vertex) == 0) {
						uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
						vertices.push_back(vertex);
					}
					indices.push_back(uniqueVertices[vertex]);
				}
				index_offset += fv;
			}
		}
#endif
	}

	float ModelManager::CalcTriangleArea(glm::vec3 veca, glm::vec3 vecb, glm::vec3 vecc)
	{
		float a = glm::length(veca - vecb);
		float b = glm::length(vecb - vecc);
		float c = glm::length(vecc - veca);
		float p = (a + b + c)/2;
		return glm::sqrt(p * (p - a) * (p - b) * (p - c));
	}

	glm::vec3 ModelManager::CalcTriangleNormal(glm::vec3 veca, glm::vec3 vecb, glm::vec3 vecc)
	{
		return glm::cross(vecc - vecb, veca - vecb);
	}
}