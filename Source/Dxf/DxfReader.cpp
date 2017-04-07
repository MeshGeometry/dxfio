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

bool DxfReader::IsEnd(LinePair pair)
{
	bool res = false;

	//check for actual file end
	if (pair.first_ == -100 || source_->IsEof())
	{
		res = true;
	}

	//check end of file return code
	if (pair.first_ == 0 && pair.second_ == "EOF")
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
		else if (IsEnd(nextPair)) {
			URHO3D_LOGINFO("---END OF DXF FILE---");
			break;
		}

	}

	return true;
}

void DxfReader::SkipSection()
{
	URHO3D_LOGINFO("Skipping section...");

	LinePair nextPair = GetNextLinePair();

	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDSEC"))
	{
		nextPair = GetNextLinePair();
	}
}

void DxfReader::ParseHeader()
{
	URHO3D_LOGINFO("Parsing header...");

	SkipSection();
}

void DxfReader::ParseEntities()
{
	URHO3D_LOGINFO("Parsing entities...");

	SkipSection();
}

void DxfReader::ParseBlocks()
{
	URHO3D_LOGINFO("Parsing blocks...");

	LinePair nextPair = GetNextLinePair();

	//call individual block parsing loop
	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDSEC")) {
		if (Is(nextPair, 0, "BLOCK")) {
			ParseBlock();
			continue;
		}
	}
}

void DxfReader::ParseBlock()
{
	URHO3D_LOGINFO("Parsing single block...");

	LinePair nextPair = GetNextLinePair();

	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDBLK")) {

		//we store all block info in variant map
		VariantMap block;
		
		//get the info
		switch (nextPair.first_) {
		case 2:
			block["Name"] = nextPair.second_.GetString();
			break;
		case 10:
			block["Base_X"] = nextPair.second_.GetFloat();
			break;
		case 20:
			block["Base_Y"] = nextPair.second_.GetFloat();
			break;
		case 30:
			block["Base_Z"] = nextPair.second_.GetFloat();
			break;
		}

		//done with parsing the block. Push to stack
		blocks_.Push(block);

		//continue with parsing rest of content
		if (Is(nextPair, 0, "POLYLINE")) {
			ParsePolyLine();
			continue;
		}

		//skipping this case
		if (Is(nextPair, 0, "INSERT")) {
			URHO3D_LOGERROR("DXF: INSERT within a BLOCK not currently supported; skipping");
			while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDBLK"))
			{
				nextPair = GetNextLinePair();
			}
			break;
		}

		//parse these types
		else if (Is(nextPair, 0, "3DFACE") || Is(nextPair, 0, "LINE") || Is(nextPair, 0, "3DLINE")) {
			//http://sourceforge.net/tracker/index.php?func=detail&aid=2970566&group_id=226462&atid=1067632
			Parse3DFace();
			continue;
		}
	
		//recurse
		nextPair = GetNextLinePair();
	}
}

void DxfReader::ParseInsertion()
{
	URHO3D_LOGINFO("Parsing insertion...");

	SkipSection();
}

void DxfReader::ParsePolyLine()
{
	URHO3D_LOGINFO("Parsing polyline...");

	SkipSection();
}

void DxfReader::ParsePolyLineVertex()
{
	URHO3D_LOGINFO("Parsing polyline vertex...");

	SkipSection();
}

void DxfReader::Parse3DFace()
{
	URHO3D_LOGINFO("Parsing 3D face...");

	SkipSection();
}




