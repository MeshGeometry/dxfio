#include <stdio.h>
#include "gtest/gtest.h"

#include "Core/Context.h"
#include "Core/Object.h"
#include "Container/Vector.h"
#include "Container/Str.h"
#include "Core/Variant.h"
#include "IO/File.h"
#include "IO/FileSystem.h"

#include "Dxf/DxfReader.h"

using namespace Urho3D;

//commmon elements
Context* ctx = new Context();
FileSystem* fs = new FileSystem(ctx);

//testing files
String multiObject = "../../Test/Test_Dxf.dxf";
String baseTestFile = "../../Test/most_basic.dxf";
String box = "../../Test/2D_Box.dxf";

//utils
void SaveVariantVector(File* dest, const VariantVector& vector, String indent);
void SaveVariantMap(File* dest, const VariantMap& map, String indent);
void SaveRaw(String path, Variant value);


void SaveVariantVector(File* dest, const VariantVector& vector, String indent)
{
	if (!dest)
	{
		return;
	}

	indent += "==";

	dest->WriteLine(indent + "VECTOR " + String(indent.Length()) + " START");

	for (VariantVector::ConstIterator i = vector.Begin(); i != vector.End(); i++)
	{
		VariantType type = i->GetType();

		switch (type)
		{
		case VAR_VARIANTMAP:
			SaveVariantMap(dest, i->GetVariantMap(), indent);
			break;
		case VAR_VARIANTVECTOR:
			SaveVariantVector(dest, i->GetVariantVector(), indent);
			break;
		default:
			String currLine = indent + i->ToString();
			dest->WriteLine(currLine);
			break;
		}

	}

	dest->WriteLine(indent + "VECTOR " + String(indent.Length()) + " END");
}

void SaveVariantMap(File* dest, const VariantMap& map, String indent)
{
	if (!dest)
	{
		return;
	}

	indent += "==";

	dest->WriteLine(indent + "MAP " + String(indent.Length()) + " START");

	for (VariantMap::ConstIterator i = map.Begin(); i != map.End(); i++)
	{
		VariantType type = i->second_.GetType();

		switch (type)
		{
		case VAR_VARIANTMAP:
			SaveVariantMap(dest, i->second_.GetVariantMap(), indent);
			break;
		case VAR_VARIANTVECTOR:
			SaveVariantVector(dest, i->second_.GetVariantVector(), indent);
			break;
		default:
			String currLine = indent + i->second_.ToString();
			dest->WriteLine(currLine);
			break;
		}

	}

	dest->WriteLine(indent + "MAP " + String(indent.Length()) + " END");
}

void SaveRaw(String path, Variant value)
{
	FileSystem* fs = new FileSystem(ctx);
	File* dest = new File(ctx, path, FILE_WRITE);

	if (!dest)
	{
		return;
	}

	String indent = "";
	VariantType type = value.GetType();

	switch (type)
	{
	case VAR_VARIANTMAP:
		SaveVariantMap(dest, value.GetVariantMap(), indent);
		break;
	case VAR_VARIANTVECTOR:
		SaveVariantVector(dest, value.GetVariantVector(), indent);
		break;
	default:
		String currLine = indent + value.ToString();
		dest->WriteLine(currLine);
		break;
	}



	dest->Close();

}


TEST(Basic, CheckTestFiles)
{
	bool res = fs->FileExists(baseTestFile);
	EXPECT_EQ(res, true);
	res = fs->FileExists(multiObject);
	EXPECT_EQ(res, true);
	res = fs->FileExists(box);
	EXPECT_EQ(res, true);

}

TEST(Basic, Output)
{
	VariantVector test;
	test.Push("Start");
	test.Push(1.0);
	test.Push(2.0);
	test.Push(3.0);

	VariantVector vec;
	vec.Push(4.0);
	vec.Push(5.0);
	vec.Push(6.0);

	VariantMap map;
	map["A"] = "my";
	map["B"] = "var";
	map["C"] = "map";

	vec.Push(map);

	test.Push(vec);

	test.Push("End");


	SaveRaw("../../Test/VarTest.txt", test);
}

TEST(Basic, ReadLines)
{
	DxfReader* reader = new DxfReader(ctx, multiObject);

	//int res = reader->ReadGroupCode();
	int res = reader->Parse();

	String pathOut = "../../Test/blocks_out.txt";
	SaveRaw(pathOut, reader->GetBlocks());

	pathOut = "../../Test/meshes_out.txt";
	SaveRaw(pathOut, reader->GetMeshes());

	pathOut = "../../Test/polys_out.txt";
	SaveRaw(pathOut, reader->GetPolylines());

	pathOut = "../../Test/points_out.txt";
	SaveRaw(pathOut, reader->GetPoints());

}