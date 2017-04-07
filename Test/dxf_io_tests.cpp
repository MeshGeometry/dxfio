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



//utils
void SaveVariantVector(File* dest, const VariantVector& vector, String indent);
void SaveVariantMap(File* dest, const VariantMap& map, String indent);
void SaveRaw(String path, VariantVector value);


void SaveVariantVector(File* dest, const VariantVector& vector, String indent)
{
	if (!dest)
	{
		return;
	}

	for (VariantVector::ConstIterator i = vector.Begin(); i != vector.End(); i++)
	{
		VariantType type = i->GetType();

		switch (type)
		{
		case VAR_VARIANTMAP:
			indent += "   ";
			SaveVariantMap(dest, i->GetVariantMap(), indent);
			break;
		case VAR_VARIANTVECTOR:
			indent += "   ";
			SaveVariantVector(dest, i->GetVariantVector(), indent);
			break;
		default:
			String currLine = i->ToString();
			dest->WriteLine(currLine);
			break;
		}

	}
}

void SaveVariantMap(File* dest, const VariantMap& map, String indent)
{
	if (!dest)
	{
		return;
	}

	for (VariantMap::ConstIterator i = map.Begin(); i != map.End(); i++)
	{
		VariantType type = i->second_.GetType();

		switch (type)
		{
		case VAR_VARIANTMAP:
			indent += "   ";
			SaveVariantMap(dest, i->second_.GetVariantMap(), indent);
			break;
		case VAR_VARIANTVECTOR:
			indent += "   ";
			SaveVariantVector(dest, i->second_.GetVariantVector(), indent);
			break;
		default:
			String currLine = i->second_.ToString();
			dest->WriteLine(currLine);
			break;
		}

	}
}

void SaveRaw(String path, VariantVector value)
{
	FileSystem* fs = new FileSystem(ctx);
	File* dest = new File(ctx, path, FILE_WRITE);

	if (!dest)
	{
		return;
	}

	String indent = "";
	for (VariantVector::ConstIterator i = value.Begin(); i != value.End(); i++)
	{
		VariantType type = i->GetType();

		switch (type)
		{
		case VAR_VARIANTMAP:
			indent += "   ";
			SaveVariantMap(dest, i->GetVariantMap(), indent);
			break;
		case VAR_VARIANTVECTOR:
			indent += "   ";
			SaveVariantVector(dest, i->GetVariantVector(), indent);
			break;
		default:
			String currLine = i->ToString();
			dest->WriteLine(currLine);
			break;
		}

	}

	dest->Close();

}


TEST(Basic, CheckTestFiles)
{
	bool res = fs->FileExists(baseTestFile);
	EXPECT_EQ(res, true);
	res = fs->FileExists(multiObject);
	EXPECT_EQ(res, true);

}

TEST(Basic, ReadLines)
{
	DxfReader* reader = new DxfReader(ctx, multiObject);

	//int res = reader->ReadGroupCode();
	int res = reader->Parse();

	String pathOut = "../../Test/output.txt";
	SaveRaw(pathOut, reader->GetBlocks());

}