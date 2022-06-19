#include "../pbd/triangle.h"
#include "../pbd/pbd.h"


PXR_NAMESPACE_OPEN_SCOPE

PBDTriangle::PBDTriangle() :
	_mesh(NULL)
{
	_restitutionCoeff = static_cast<float>(0.6);
	_frictionCoeff = static_cast<float>(0.2);
}

PBDTriangle::~PBDTriangle(void)
{
	CleanUp();
}

void PBDTriangle::CleanUp()
{
	if(_mesh)delete _mesh;
    _mesh = NULL;
}

void PBDTriangle::UpdateMeshNormals(const PBDParticle& p)
{
    _mesh->ComputeNormals();
}

void PBDTriangle::Init(const Mesh* mesh)
{
	_indexOffset = indexOffset;
	_mesh = mesh;
}

unsigned int PBDTriangle::GetIndexOffset() const
{
	return _indexOffset;
}


PXR_NAMESPACE_CLOSE_SCOPE