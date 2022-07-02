#include "../pbd/line.h"
#include "../pbd/pbd.h"
#include "../pbd/triangle.h"


PXR_NAMESPACE_OPEN_SCOPE

PBDLine::PBDLine()
{
	_restitutionCoeff = static_cast<Real>(0.6);
	_frictionCoeff = static_cast<Real>(0.2);
}

PBDLine::~PBDLine(void)
{

}

PBDLine::Edges& PBDLine::GetEdges()
{
	return _edges;
}

void PBDLine::InitMesh(const unsigned int nPoints, const unsigned int nQuaternions, 
    const unsigned int indexOffset, const unsigned int indexOffsetQuaternions, 
    unsigned int* indices, unsigned int* indicesQuaternions)
{
	_nPoints = nPoints;
	_nQuaternions = nQuaternions;
	_indexOffset = indexOffset;
	_indexOffsetQuaternions = indexOffsetQuaternions;

	_edges.resize(nPoints - 1);

	for (unsigned int i = 0; i < nPoints-1; i++)
	{
		_edges[i] = OrientedEdge(indices[2*i], indices[2*i + 1], indicesQuaternions[i]);
	}
}

unsigned int PBDLine::GetIndexOffset() const
{
	return _indexOffset;
}

unsigned PBDLine::GetIndexOffsetQuaternions() const
{
	return _indexOffsetQuaternions;
}

PXR_NAMESPACE_CLOSE_SCOPE