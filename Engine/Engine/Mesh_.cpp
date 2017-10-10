#include "Mesh_.h"
#include "Glew/include/glew.h"
#include "SDL/include/SDL_opengl.h"
#include "Primitive.h"
#include "Application.h"
#include "GeometryManager.h"
#include "ModuleRenderer3D.h"

// Constructors =================================
Mesh_::Mesh_(std::vector<Vertex> vertices, std::vector<uint> indices, std::vector<Texture> textures) :vertices(vertices), indices(indices), textures(textures)
{
	this->vertices = vertices;
	this->textures = textures;
	this->indices = indices;
	
	SetupMesh();
}

// Destructors ==================================
Mesh_::~Mesh_()
{
	vertices.clear();
	textures.clear();
	indices.clear();
}

void Mesh_::SetupMesh()
{
	glGenBuffers(1, &VertexBufferObject);
	glGenBuffers(1, &ElementBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);

	//Build mesh vertex normals
	std::vector<math::float3> vertex_normals;
	uint size = vertices.size();
	for (uint k = 0; k < size; k++)
	{
		vertex_normals.push_back(vertices.data()[k].position);
		vertex_normals.push_back((vertices.data()[k].position + vertices.data()[k].normals));
	}

	glGenBuffers(1, &normalsID);
	glBindBuffer(GL_ARRAY_BUFFER, normalsID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(math::float3) * 2, &vertex_normals.data()[0], GL_STATIC_DRAW);
	
	//Build mesh face normals
	std::vector<math::float3> face_normals;
	size = indices.size();
	for (uint k = 0; k < size - 2; k++)
	{
		math::float3 V(vertices[k + 1].position.x - vertices[k].position.x, vertices[k + 1].position.y - vertices[k].position.y, vertices[k + 1].position.z - vertices[k].position.z);
		math::float3 W(vertices[k + 2].position.x - vertices[k].position.x, vertices[k + 2].position.y - vertices[k].position.y, vertices[k + 2].position.z - vertices[k].position.z);
		math::float3 normal(V.Cross(W));
		normal.Normalize();
		math::float3 center_point((vertices[k].position.x + vertices[k + 1].position.x + vertices[k + 2].position.x) / 3, (vertices[k].position.y + vertices[k + 1].position.y + vertices[k + 2].position.y) / 3, (vertices[k].position.z + vertices[k + 1].position.z + vertices[k + 2].position.z) / 3);
		face_normals.push_back(center_point);
		face_normals.push_back(center_point + normal);
	}

	glGenBuffers(1, &face_normalsID);
	glBindBuffer(GL_ARRAY_BUFFER, face_normalsID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(math::float3) * 2, &face_normals.data()[0], GL_STATIC_DRAW);

}

// Game Loop ====================================
void Mesh_::Draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for (int i = 0; i < textures.size(); i++)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i].id);

	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), NULL);
	glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normals));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBufferObject);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);


	//Draw vertex normals
	glDisable(GL_LIGHTING);
	glBindBuffer(GL_ARRAY_BUFFER, normalsID);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glColor4f(App->geometry->vertex_normals_color[0], App->geometry->vertex_normals_color[1], App->geometry->vertex_normals_color[2], App->geometry->vertex_normals_color[3]);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, vertices.size());
	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	App->renderer3D->EnableGLRenderFlags();

	//Draw face normals
	glDisable(GL_LIGHTING);
	glBindBuffer(GL_ARRAY_BUFFER, face_normalsID);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glColor4f(App->geometry->face_normals_color[0], App->geometry->face_normals_color[1], App->geometry->face_normals_color[2], App->geometry->face_normals_color[3]);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, vertices.size());

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	App->renderer3D->EnableGLRenderFlags();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

const char * Mesh_::GetName() const
{
	return name.c_str();
}

// Functionality ================================
void Mesh_::SetTransformation(aiMatrix4x4 mat)
{
	transformation = mat;
	mat.Decompose(scale, rotation, position);
}

void Mesh_::BlitInfo() const
{
	//Header of the mesh
	if (ImGui::CollapsingHeader(("%s", name.c_str()), NULL))
	{
		//Show mesh position
		ImGui::Text("Position	");
		ImGui::SameLine();
		ImGui::Text("X %.1f		", position.x);
		ImGui::SameLine();
		ImGui::Text("Y %.1f		", position.y);
		ImGui::SameLine();
		ImGui::Text("Z %.1f", position.z);

		float eX = atan2(-2 * (rotation.y*rotation.z - rotation.w*rotation.x), rotation.w*rotation.w - rotation.x*rotation.x - rotation.y*rotation.y + rotation.z*rotation.z);
		float eY = asin(2 * (rotation.x*rotation.z + rotation.w*rotation.y));
		float eZ = atan2(-2 * (rotation.x*rotation.y - rotation.w*rotation.z), rotation.w*rotation.w + rotation.x*rotation.x - rotation.y*rotation.y - rotation.z*rotation.z);

		//Show mesh rotation
		ImGui::Text("Rotation	");
		ImGui::SameLine();
		ImGui::Text("X %.1f		", eX);
		ImGui::SameLine();
		ImGui::Text("Y %.1f		", eY);
		ImGui::SameLine();
		ImGui::Text("Z %.1f", eZ);

		//Show mesh scale
		ImGui::Text("Scale	");
		ImGui::SameLine();
		ImGui::Text("X %.1f		", scale.x);
		ImGui::SameLine();
		ImGui::Text("Y %.1f		", scale.y);
		ImGui::SameLine();
		ImGui::Text("Z %.1f", scale.z);

		for (std::vector<Texture>::const_iterator it = textures.begin(); it != textures.end(); it++)
		{
			ImGui::Image((void*)(*it).id, ImVec2(100, 100));
			ImGui::Text("Size: %ix%i", (*it).tex_width, (*it).tex_height);
		}

		ImGui::NewLine();

	}
}
