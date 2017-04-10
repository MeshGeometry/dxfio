#include "DxfWriter.h"
#include "Core/StringUtils.h"
#include "IO/Log.h"

DxfWriter::DxfWriter(Context* context, String path) : Object(context)
{

}

bool DxfWriter::Save(String path)
{

	FileSystem* fs = new FileSystem(GetContext());

	//create the file
	dest_ = new File(GetContext(), path, FILE_WRITE);

	//double check
	assert(dest_);

	//create the log
	GetContext()->RegisterSubsystem(new Log(GetContext()));

	//oepn
	WriteHeader();

	//write the objects
	WriteEntities();

	//close
	WriteLinePair(0, "EOF");

	dest_->Close();

	return false;
}

//writers
bool DxfWriter::WriteLinePair(int code, String value)
{

	if (!dest_)
		return false;

	dest_->WriteLine(String(code));
	dest_->WriteLine(value);

	return true;
}


void DxfWriter::WriteHeader()
{
	//opener
	WriteLinePair(0, "SECTION");
	WriteLinePair(2, "HEADER");

	WriteLinePair(9, "$ACADVER");
	WriteLinePair(1, "AC1018");


	WriteLinePair(9, "$INSBASE");
	WriteLinePair(10, String(0.0));
	WriteLinePair(20, String(0.0));
	WriteLinePair(30, String(0.0));

	//closer
	WriteLinePair(0, "ENDSEC");
}

void DxfWriter::WriteEntities()
{
	//ENTITIES opener
	WriteLinePair(0, "SECTION");
	WriteLinePair(2, "ENTITIES");

	//write meshes
	for (int i = 0; i < meshes_.Size(); i++)
	{
		WriteMesh(i);
	}

	//write
	for (int i = 0; i < polylines_.Size(); i++)
	{
		WritePolyline(i);
	}

	for (int i = 0; i < points_.Size(); i++)
	{
		WritePoint(i);
	}

	//ENTITIES closer
	WriteLinePair(0, "ENDSEC");
}

void DxfWriter::WriteMesh(int id)
{

}

void DxfWriter::WritePolyline(int id)
{

}

void DxfWriter::WritePoint(int id)
{
	if (id >= points_.Size())
		return;

	VariantMap pMap = points_[id].GetVariantMap();

	//check that this is a point structure
	if (pMap.Keys().Contains("000_TYPE")) {
		if (pMap["000_TYPE"].GetString() == "POINT")
		{
			Vector3 v = pMap.Keys().Contains("Position") ? pMap["Position"].GetVector3() : Vector3::ZERO;
			String layer = pMap.Keys().Contains("Layer") ? pMap["Layer"].GetString() : "Default";

			//actually write
			WriteLinePair(0, "POINT");
			WriteLinePair(8, layer);
			WriteLinePair(10, String(v.x_));
			WriteLinePair(20, String(v.y_));
			WriteLinePair(30, String(v.z_));
		}
	}
}

//setters
void DxfWriter::SetMesh(VariantMap mesh)
{

}

void DxfWriter::SetMesh(VariantVector meshes)
{

}

void DxfWriter::SetPolyline(VariantMap polyline)
{

}
void DxfWriter::SetPolyline(VariantVector polylines)
{

}

void DxfWriter::SetPoint(Vector3 point, String layer)
{
	VariantMap pMap;
	pMap["000_TYPE"] = "POINT";
	pMap["Position"] = point;
	pMap["Layer"] = layer;

	points_.Push(pMap);
}

void DxfWriter::SetPoints(Vector<Vector3> points)
{

}