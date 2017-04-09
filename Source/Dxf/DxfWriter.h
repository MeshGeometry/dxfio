#include "Core/Context.h"
#include "Core/Object.h"
#include "Container/Vector.h"
#include "Container/Str.h"
#include "Core/Variant.h"
#include "IO/Deserializer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"

using namespace Urho3D;

typedef Pair<int, Variant> LinePair;

URHO3D_API class DxfWriter : public Object
{
	URHO3D_OBJECT(DxfWriter, Object);

public:
	DxfWriter(Context* context, String path);
	~DxfWriter() {};

	/**************************************************************************
	Dxf info comes in pairs of lines, eg:
	---- 0         <- code id
	---- HEADER    <- value for this code
	***************************************************************************/
	LinePair WriteLinePair();

	//main loop for writing
	bool Save(String path);

	//writers
	void WriteTemplate();
	void WriteMesh();
	void WritePolyline();
	void WritePoint();

	//setters
	void SetMesh(VariantMap mesh);
	void SetMesh(VariantVector meshes);
	void SetPolyline(VariantMap polyline);
	void SetPolyline(VariantVector polylines);
	void SetPoint(Vector3 point);
	void SetPoints(Vector<Vector3> points);


protected:

	File* dest;

	//These are the things we want. 
	VariantVector meshes_;
	VariantVector polylines_;
	VariantVector points_;

};