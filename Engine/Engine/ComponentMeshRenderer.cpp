#include "ComponentMeshRenderer.h"
#include "Glew/include/glew.h"
#include "SDL/include/SDL_opengl.h"
#include "Application.h"
#include "ModuleTextures.h"
#include "ComponentMaterial.h"
#include "ComponentTransform.h"
#include "GeometryManager.h"
#include "GameObject.h"
#include "ModuleRenderer3D.h"
#include "Serializer.h"

#include "ResourceMaterial.h"

// Constructors =================================
ComponentMeshRenderer::ComponentMeshRenderer():Component(COMP_MESH_RENDERER)
{

}

ComponentMeshRenderer::ComponentMeshRenderer(const ComponentMeshRenderer & cpy) : Component(cpy), render_flags(cpy.render_flags), target_mesh(cpy.target_mesh)
{

}

// Destructors ==================================
ComponentMeshRenderer::~ComponentMeshRenderer()
{
	target_mesh = nullptr;
}

// Game Loop ====================================
bool ComponentMeshRenderer::Update(float dt)
{

	if (target_mesh == nullptr || target_mesh->MeshResourceIsNull() || parent->GetHide())return false;

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//Transform to rotate/move/scale mesh
	ComponentTransform* tmp = (ComponentTransform*)parent->FindComponent(COMPONENT_TYPE::COMP_TRANSFORMATION);

	//Select between the textures
	if (App->textures->GetCheckMode())
	{
		glBindTexture(GL_TEXTURE_2D, App->textures->check_image);
	}

	else if (App->textures->GetCustomMode())
	{
		glBindTexture(GL_TEXTURE_2D, App->textures->custom_check_image);
	}

	else if (App->textures->GetMeshMode())
	{
		ComponentMaterial* mat = target_mesh->GetDrawMaterial();
		if (mat!= nullptr && mat->GetActive())
		{
			for (int i = 0; i < mat->textures.size(); i++)
			{
				glBindTexture(GL_TEXTURE_2D, mat->textures[i]->GetMaterialID());

			}
		}
	}

	//Transform the mesh before drawing
	if (tmp)
	{
		tmp->SetMatrixToDraw();		
	}

	//Draw the mesh
	glLineWidth(App->geometry->mesh_lines_width);
	glColor4f(App->geometry->mesh_color[0], App->geometry->mesh_color[1], App->geometry->mesh_color[2], App->geometry->mesh_color[3]);
	glBindBuffer(GL_ARRAY_BUFFER, target_mesh->GetVertexBufferObject());
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), NULL);
	glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normals));
	glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, target_mesh->GetElementBufferObject());
	glDrawElements(GL_TRIANGLES, target_mesh->GetIndexSize(), GL_UNSIGNED_INT, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	glBindTexture(GL_TEXTURE_2D, 0);

	

	//Draw mesh debug information
	App->renderer3D->DisableGLRenderFlags();
	if (render_flags & REND_FACE_NORMALS)DrawFaceNormals();
	if (render_flags & REND_VERTEX_NORMALS)DrawVertexNormals();
	App->renderer3D->EnableGLRenderFlags();


	if (tmp)
		tmp->QuitMatrixToDraw();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	return true;
}

// Functionality ================================
void ComponentMeshRenderer::DrawVertexNormals() const
{
	if (target_mesh->GetVertexNormalsID() == 0)return;

	//Draw vertex normals
	glBindBuffer(GL_ARRAY_BUFFER, target_mesh->GetVertexNormalsID());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glColor4f(App->geometry->vertex_normals_color[0], App->geometry->vertex_normals_color[1], App->geometry->vertex_normals_color[2], App->geometry->vertex_normals_color[3]);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, target_mesh->GetNumVertex() * 2);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ComponentMeshRenderer::DrawFaceNormals() const
{
	if (target_mesh->GetFaceNormalsID() == 0)return;

	//Draw face normals
	glBindBuffer(GL_ARRAY_BUFFER, target_mesh->GetFaceNormalsID());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glColor4f(App->geometry->face_normals_color[0], App->geometry->face_normals_color[1], App->geometry->face_normals_color[2], App->geometry->face_normals_color[3]);
	glLineWidth(2.f);
	glDrawArrays(GL_LINES, 0, target_mesh->GetNumTris() * 2);

	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Set Methods ==================================
void ComponentMeshRenderer::SetTargetMesh(const ComponentMesh * target)
{
	target_mesh = (ComponentMesh*)target;
}

// Functionality ================================
void ComponentMeshRenderer::BlitComponentInspector()
{
	ImGui::Separator();

	ImGui::Checkbox("##mesh_renderer_comp", &actived);
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 0.64f, 0.0f, 1.0f), "Mesh Renderer");
	
	//Show mesh render flags
	bool rend_bool = bool(render_flags & REND_FACE_NORMALS);
	char str_buff[30];

	sprintf_s(str_buff, "Face Normals");
	if (ImGui::Checkbox(str_buff, &rend_bool))
	{
		if (rend_bool)
		{
			render_flags |= REND_FACE_NORMALS;
		}
		else
		{
			render_flags &= ~REND_FACE_NORMALS;
		}

	}

	rend_bool = bool(render_flags & REND_VERTEX_NORMALS);
	sprintf_s(str_buff, "Vertex Normals");
	if (ImGui::Checkbox(str_buff, &rend_bool))
	{
		if (rend_bool)
		{
			render_flags |= REND_VERTEX_NORMALS;
		}
		else
		{
			render_flags &= ~REND_VERTEX_NORMALS;
		}

	}
}

bool ComponentMeshRenderer::Save(Serializer & array_root) const
{
	bool ret = false;

	//Serializer where all the data of the component is built
	Serializer comp_data;

	//Insert Component Type
	ret = comp_data.InsertString("type", ComponentTypeToStr(type));
	//Insert component id
	ret = comp_data.InsertInt("id", id);
	//Insert actived
	ret = comp_data.InsertBool("actived", actived);

	//Insert target mesh component id
	if (target_mesh != nullptr)ret = comp_data.InsertInt("target_mesh_id", target_mesh->GetID());

	//Insert render flags
	ret = comp_data.InsertBool("render_face_normals", (render_flags & REND_FACE_NORMALS));
	ret = comp_data.InsertBool("render_vertex_normals", (render_flags & REND_VERTEX_NORMALS));

	//Save the built data in the components array
	ret = array_root.InsertArrayElement(comp_data);

	return ret;
}

bool ComponentMeshRenderer::Load(Serializer & data, std::vector<std::pair<Component*, uint>>& links)
{
	bool ret = true;

	//Get component id
	id = data.GetInt("id");

	//Get actived
	actived = data.GetBool("actived");

	//Get target mesh component id
	uint target_mesh_id = data.GetInt("target_mesh_id");
	if (target_mesh_id != 0)links.push_back(std::pair<Component*, uint>(this, target_mesh_id));
	else ret = false;

	//Get render flags
	bool rend_bool = data.GetBool("render_face_normals");
	if (rend_bool)
	{
		render_flags |= REND_FACE_NORMALS;
	}
	else
	{
		render_flags &= ~REND_FACE_NORMALS;
	}
	rend_bool = data.GetBool("render_vertex_normals");
	if (rend_bool)
	{
		render_flags |= REND_VERTEX_NORMALS;
	}
	else
	{
		render_flags &= ~REND_VERTEX_NORMALS;
	}

	return ret;
}

void ComponentMeshRenderer::LinkComponent(const Component * target)
{
	target_mesh = (ComponentMesh*)target;
}
