#include "DxfReader.h"
#include "Core/StringUtils.h"
#include "IO/Log.h"

//these codes signal that a new group is starting or ending
static const char* topLevelCodes[] = 
{
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9"
};


DxfReader::DxfReader(Context* context, String path) : Object(context)
{
	FileSystem* fs = new FileSystem(GetContext());
	bool res = fs->FileExists(path);

	//make sure that this file exists
	assert(res);

	//create the file
	source_ = new File(GetContext(), path, FILE_READ);

	//double check
	assert(source_);

	//create the log
	GetContext()->RegisterSubsystem(new Log(GetContext()));

}

LinePair DxfReader::GetNextLinePair()
{
	LinePair nextPair;
	
	//initialize with error code:
	nextPair.first_ = -100; //use this as error since DXF has some negative codes...
	nextPair.second_ = Variant();

	//read the code
	if (!source_->IsEof())
	{
		nextPair.first_ = ToInt(source_->ReadLine().Trimmed());
	}

	//read the value
	if (!source_->IsEof())
	{
		nextPair.second_ = source_->ReadLine().Trimmed();
	}

	return nextPair;
}

bool DxfReader::Is(LinePair pair, int code, String name)
{
	bool res = false;

	if (pair.first_ == code && pair.second_ == name)
	{
		res = true;
	}

	return res;
}

bool DxfReader::IsType(LinePair pair, int code, VariantType type)
{
	bool res = false;

	if (pair.first_ == code && pair.second_.GetType() == type)
	{
		res = true;
	}

	return res;
}

bool DxfReader::Parse()
{
	while (!source_->IsEof())
	{
		LinePair nextPair = GetNextLinePair();

		//debug
		URHO3D_LOGINFO("Line Pair: " + String(nextPair.first_) + " : " + nextPair.second_.GetString());

		// blocks table - these 'build blocks' are later (in ENTITIES)
		// referenced an included via INSERT statements.
		if (Is(nextPair, 2, "BLOCKS")) {
			ParseBlocks();
			continue;
		}

		// primary entity table
		if (Is(nextPair, 2, "ENTITIES")) {
			ParseEntities();
			continue;
		}

		// skip unneeded sections entirely to avoid any problems with them
		// alltogether.
		else if (Is(nextPair, 2, "CLASSES") || Is(nextPair, 2, "TABLES")) {
			SkipSection();
			continue;
		}

		else if (Is(nextPair, 2, "HEADER")) {
			ParseHeader();
			continue;
		}

		// comments
		else if (nextPair.first_ == 999) {
		URHO3D_LOGINFO("DXF comment");
		}

		// don't read past the official EOF sign
		else if (Is(nextPair, 0, "EOF")) {
			URHO3D_LOGINFO("---END OF DXF FILE---");
			break;
		}

	}

	return true;
}

void DxfReader::SkipSection()
{
	URHO3D_LOGINFO("Skipping section...");
}

void DxfReader::ParseHeader()
{
	URHO3D_LOGINFO("Parsing header...");
}

void DxfReader::ParseEntities()
{
	URHO3D_LOGINFO("Parsing entities...");
}

void DxfReader::ParseBlocks()
{
	URHO3D_LOGINFO("Parsing blocks...");
}

void DxfReader::ParseBlock()
{
	URHO3D_LOGINFO("Parsing single block...");
}

void DxfReader::ParseInsertion()
{
	URHO3D_LOGINFO("Parsing insertion...");
}

void DxfReader::ParsePolyLine()
{
	URHO3D_LOGINFO("Parsing polyline...");
}

void DxfReader::ParsePolyLineVertex()
{
	URHO3D_LOGINFO("Parsing polyline vertex...");
}

void DxfReader::Parse3DFace()
{
	URHO3D_LOGINFO("Parsing 3D face...");
}




