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
	DxfWriter(Context* context);
	~DxfWriter() {};

	/**************************************************************************
	Dxf info comes in pairs of lines, eg:
	---- 0         <- code id
	---- HEADER    <- value for this code
	***************************************************************************/
	bool WriteLinePair(int code, String value);

	//main loop for writing
	bool Save(String path);

	//setters
	void SetMesh(VariantMap mesh, String layer = "Default");
	void SetMesh(VariantVector meshes);
	void SetPolyline(Vector<Vector3> vertices, String layer = "Default");
	void SetPolyline(VariantVector polylines);
	void SetPoint(Vector3 point, String layer = "Default");
	void SetPoints(Vector<Vector3> points);


protected:

	File* dest_;

	//These are the things we want. 
	VariantVector meshes_;
	VariantVector polylines_;
	VariantVector points_;

	//writers
	void WriteHeader();
	void WriteEntities();
	void WriteMesh(int id);
	void WritePolyline(int id);
	void WritePoint(int id);
	void WriteVertex(Vector3 vertex);
	void WriteIndices(int a, int b, int c);

};