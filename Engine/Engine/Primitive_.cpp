#include "Primitive_.h"

// Constructors =================================
Primitive_::Primitive_()
{

}

Primitive_::Primitive_(const Primitive_ & _cpy) :color(_cpy.color), axis(_cpy.axis), type(_cpy.type)
{

}

// Destructors ==================================
Primitive_::~Primitive_()
{

}

// Game Loop ====================================
bool Primitive_::Draw()
{
	return true;
}

// Set Methods ==================================
void Primitive_::SetColor(const Color & n_color)
{
	color = n_color;
}

void Primitive_::SetAxis(bool n_axis)
{
	axis = n_axis;
}

void Primitive_::SetType(PRIMITIVE_TYPE n_type)
{
	type = n_type;
}

// Get Methods ==================================
Color Primitive_::GetColor() const
{
	return color;
}

bool Primitive_::GetAxis() const
{
	return axis;
}

PRIMITIVE_TYPE Primitive_::GetType() const
{
	return type;
}

