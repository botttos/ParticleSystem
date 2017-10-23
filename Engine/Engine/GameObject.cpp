#include "GameObject.h"

#include "Glew/include/glew.h"
#include "SDL/include/SDL_opengl.h"

#include "Component.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GeometryManager.h"
#include "ModuleAudio.h"
#include "ModuleRenderer3D.h"

// Constructors =================================
GameObject::GameObject()
{

}

GameObject::GameObject(const GameObject & cpy) :actived(cpy.actived), name(cpy.name), parent(cpy.parent), bounding_box(cpy.bounding_box), draw_bounding_box(cpy.draw_bounding_box)
{
	//Clone all the components
	uint size = cpy.components.size();
	for (uint k = 0; k < size; k++)
	{
		components.push_back(CloneComponent(cpy.components[k]));
	}

	//Clone all the childs
	size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		childs.push_back(new GameObject(*cpy.childs[k]));
	}

}

// Destructors ==================================
GameObject::~GameObject()
{
	//Delete components
	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		RELEASE(components[k]);
	}
	components.clear();

	//Delete childs
	size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		RELEASE(childs[k]);
	}
	childs.clear();

	//Reset parent
	parent = nullptr;
}

// Game Loop ====================================
bool GameObject::Update()
{
	bool ret = true;

	/* This update*/
	if (draw_bounding_box)DrawBoundingBox();

	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		if (components[k]->GetActive())
		{
			ret = components[k]->Update();
		}
	}

	size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		if (childs[k]->GetActive())
		{
			ret = childs[k]->Update();
		}
	}

	return ret;
}

// Set Methods ==================================
void GameObject::SetActiveState(bool act)
{
	actived = act;
}

void GameObject::SetName(const char * str)
{
	name = str;
}

void GameObject::SetParent(GameObject * target)
{
	if (parent != nullptr)
	{
		parent->PopChild(this);
	}

	if (target != nullptr)
	{
		target->AddChild(this);
	}

	parent = (GameObject*)target;
}

// Get Methods ==================================
bool GameObject::GetActive()
{
	return actived;
}

float GameObject::GetBoundingBoxDiagonalSize() const
{
	math::float3 vec = bounding_box.maxPoint - bounding_box.minPoint;
	return abs(vec.Length());;
}

const GameObject * GameObject::GetParent() const
{
	return parent;
}

bool GameObject::IsRoot() const
{
	return App->scene->IsRoot(this);
}

bool GameObject::IsSelectedObject() const
{
	return (this == App->scene->GetSelectedGameObject());
}

// Functionality ================================
Component * GameObject::CreateComponent(COMPONENT_TYPE c_type)
{
	Component* comp = nullptr;

	switch (c_type)
	{
	case COMP_TRANSFORMATION:	comp = new ComponentTransform();	break;
	case COMP_MESH:				comp = new ComponentMesh();			break;
	case COMP_MATERIAL:			comp = new ComponentMaterial();		break;
	case COMP_MESH_RENDERER:	comp = new ComponentMeshRenderer();	break;
	case COMP_CAMERA:			comp = new ComponentCamera();		break;
	}

	if (comp != nullptr)
	{
		comp->SetParent(this);
		comp->Start();
		components.push_back(comp);
	}

	return comp;
}

bool GameObject::RemoveComponent(Component * cmp)
{
	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		if (components[k] == cmp)
		{
			RELEASE(components[k]);

			for (uint h = k; h < size - 1; h++)
			{
				components[h] == components[h + 1];
			}

			components.pop_back();

			return true;
		}
	}

	return false;
}

bool GameObject::FindComponent(Component * cmp) const
{
	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		if (components[k] == cmp)return true;

	}

	return false;
}

Component * GameObject::FindComponent(COMPONENT_TYPE type) const
{
	Component* cmp = nullptr;
	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		if (components[k]->GetType() == type)
		{
			cmp = components[k];
			break;
		}
	}

	return cmp;
}

Component * GameObject::CloneComponent(const Component * target) const
{
	Component* new_c = nullptr;

	switch (target->GetType())
	{
	case COMP_TRANSFORMATION:		new_c = new ComponentTransform(*(const ComponentTransform*)target);			break;
	case COMP_MESH:					new_c = new ComponentMesh(*(const ComponentMesh*)target);					break;
	case COMP_MATERIAL:				new_c = new ComponentMaterial(*(const ComponentMaterial*)target);			break;
	case COMP_MESH_RENDERER:		new_c = new ComponentMeshRenderer(*(const ComponentMeshRenderer*)target);	break;
	}

	return new_c;
}

void GameObject::AddChild(const GameObject * child)
{
	if (child != nullptr)childs.push_back((GameObject*)child);
}

bool GameObject::RemoveChild(GameObject * child, bool search_in)
{
	bool ret = false;
	uint size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		if (childs[k] == child)
		{
			RELEASE(childs[k]);

			for (uint h = k; h < size - 1; h++)
			{
				childs[h] == childs[h + 1];
			}

			childs.pop_back();

			ret = true;
			break;
		}
		else if (search_in)
		{
			ret = childs[k]->RemoveChild(child, search_in);
			if (ret)break;
		}
	}

	return ret;
}

bool GameObject::PopChild(GameObject * child, bool search_in)
{
	bool ret = false;
	uint size = childs.size();
	for (uint k = 0; k < size; k++)
	{
		if (childs[k] == child)
		{
			for (uint h = k; h < size - 1; h++)
			{
				childs[h] == childs[h + 1];
			}

			childs.pop_back();

			ret = true;
			break;
		}
		else if (search_in)
		{
			ret = childs[k]->PopChild(child, search_in);
			if (ret)break;
		}
	}

	return ret;
}

std::vector<GameObject*>* GameObject::GetChilds()
{
	return &childs;
}

void GameObject::BlitGameObjectHierarchy(uint index)
{
	char name_str[40];
	sprintf_s(name_str, 40, "%s##%i", name.c_str(), index);
	bool op = ImGui::TreeNodeEx(name_str, ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnDoubleClick);

	if (ImGui::IsItemClicked() && ImGui::IsItemHovered())
	{
		App->scene->EnableInspector();
		App->scene->SetSelectedGameObject(this);
	}

	if(op)
	{
		uint size = childs.size();
		for (uint k = 0; k < size; k++)
		{
			childs[k]->BlitGameObjectHierarchy(k);
		}
		ImGui::TreePop();
	}

}

void GameObject::BlitGameObjectInspector()
{
	//Blit game object base data
	//Enable / disable
	ImGui::Checkbox("##object_active", &actived);
	ImGui::SameLine();
	
	//Name
	if (ImGui::InputText("Name", (char*)name.c_str(), 20, ImGuiInputTextFlags_::ImGuiInputTextFlags_EnterReturnsTrue))
	{
		App->audio->PlayFxForInput(FX_ID::APPLY_FX);
	}
	
	//Bounding box
	ImGui::Checkbox("Bounding Box", &draw_bounding_box);
	
	//Components inspectors
	uint size = components.size();
	for (uint k = 0; k < size; k++)
	{
		components[k]->BlitComponentInspector();
	}

	//Add a margin to scroll
	ImGui::NewLine();ImGui::NewLine();ImGui::NewLine();ImGui::NewLine();
}

void GameObject::DrawBoundingBox()
{
	App->renderer3D->DisableGLRenderFlags();

	this->GetTranslatedBoundingBox().Draw(4.0f, App->geometry->bounding_box_color);

	App->renderer3D->EnableGLRenderFlags();
}

math::AABB * GameObject::GetBoundingBox()
{
	return &bounding_box;
}

math::AABB GameObject::GetTranslatedBoundingBox()
{
	math::AABB tmp = bounding_box;
	ComponentTransform* trans = (ComponentTransform*)FindComponent(COMPONENT_TYPE::COMP_TRANSFORMATION);
	if(trans != nullptr)tmp.Translate(trans->GetInheritedTransform().TranslatePart());
	return tmp;
}

std::pair<math::float3, math::float3> GameObject::AdjustBoundingBox(bool all_childs)
{
	uint ch_size = childs.size();
	
	std::vector<std::pair<math::float3, math::float3>> childs_bb_vertex;

	for (uint k = 0; k < ch_size; k++)
	{
		childs_bb_vertex.push_back(childs[k]->AdjustBoundingBox(all_childs));
	}

	if (App->scene->IsRoot(parent) || all_childs)
	{
		//Reset bounding box
		bounding_box.SetNegativeInfinity();

		//Collect childs bb corners
		std::vector<math::float3> v_pos;
		uint pairs_size = childs_bb_vertex.size();
		for (uint k = 0; k < pairs_size; k++)
		{
			v_pos.push_back(childs_bb_vertex[k].first);
			v_pos.push_back(childs_bb_vertex[k].second);
		}

		ComponentMesh* mesh = (ComponentMesh*)FindComponent(COMPONENT_TYPE::COMP_MESH);
		
		if (mesh != nullptr)
		{
			//Collect all the vertex and bb corners
			v_pos = mesh->GetVertexPositions();
		}

		//Build bounding box
		bounding_box.Enclose(v_pos.data(), v_pos.size());

	}

	//Build the pair of this game object bounding box
	std::pair<math::float3, math::float3> pair;
	pair.first = bounding_box.minPoint;
	pair.second = bounding_box.maxPoint;

	return pair;
}
