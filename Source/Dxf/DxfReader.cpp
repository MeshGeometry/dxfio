#include "DxfReader.h"
#include "Core/StringUtils.h"
#include "IO/Log.h"

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

	LinePair nextPair = GetNextLinePair();

	//create a block and push it to list
	VariantMap block;
	block["Name"] = "$GENERIC_BLOCK_NAME";
	blocks_.Push(block);

	//proceed
	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDSEC")) {

		if (Is(nextPair, 0, "POLYLINE")) {
			ParsePolyLine();
			continue;
		}

		else if (Is(nextPair, 0, "INSERT")) {
			ParseInsertion();
			continue;
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

		//make sure to add empty insertion vector. This gets filled in ParseInsertions
		//do the same with the lines.
		block["Insertions"] = VariantVector();
		block["Lines"] = VariantVector();

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

	LinePair nextPair = GetNextLinePair();

	//insertions are par to of the block structure
	VariantMap* currBlock = blocks_.Back().GetVariantMapPtr();

	//we store all insertion info in variant map
	VariantMap insertion;

	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDBLK")) {

		//get the info
		switch (nextPair.first_) {
		case 2:
			insertion["Name"] = nextPair.second_.GetString();
			break;
			//translation
		case 10:
			insertion["Position_X"] = nextPair.second_.GetFloat();
			break;
		case 20:
			insertion["Position_Y"] = nextPair.second_.GetFloat();
			break;
		case 30:
			insertion["Position_Z"] = nextPair.second_.GetFloat();
			break;
			// scaling
		case 41:
			insertion["Scale_X"] = nextPair.second_.GetFloat();
			break;
		case 42:
			insertion["Scale_Y"] = nextPair.second_.GetFloat();
			break;
		case 43:
			insertion["Scale_Z"] = nextPair.second_.GetFloat();
			break;
			// rotation angle
		case 50:
			insertion["Angle"] = nextPair.second_.GetFloat();
			break;
		}

		//recurse
		nextPair = GetNextLinePair();
	}

	//done with parsing the insertion. Push to stack
	VariantVector* insertions = (*currBlock)["Insertions"].GetVariantVectorPtr();
	assert(insertions);
	insertions->Push(insertion);
}

void DxfReader::ParsePolyLine()
{
	URHO3D_LOGINFO("Parsing polyline...");

	LinePair nextPair = GetNextLinePair();

	//get reference to last block
	VariantMap* currBlock = blocks_.Back().GetVariantMapPtr();

	//store polyline line data in variantmap
	VariantMap polyline;
	polyline["Vertices"] = VariantVector();
	polyline["Faces"] = VariantVector();

	while (!IsEnd(nextPair) && !Is(nextPair, 0, "ENDSEC")) {


		if (Is(nextPair, 0, "VERTEX")) {

			ParsePolyLineVertex(polyline);

			//not exactly sure what to do here...
			if (Is(nextPair, 0, "SEQEND")) {
				break;
			}

			continue;
		}

		switch (nextPair.first_)
		{
			// flags --- important that we know whether it is a
			// polyface mesh or 'just' a line.
		case 70:
			
			polyline["Flags"] = nextPair.second_.GetUInt();
			break;

			// optional number of vertices
		case 71:
			polyline["NumVerticesHint"] = nextPair.second_.GetUInt();
			break;

			// optional number of faces
		case 72:
			polyline["NumFacesHint"] = nextPair.second_.GetUInt();
			break;

			// 8 specifies the layer on which this line is placed on
		case 8:
			polyline["Layer"] = nextPair.second_.GetString();
			break;
		}

		//recurse
		nextPair = GetNextLinePair();
	}

	//add to line list
	VariantVector* lines = (*currBlock)["Lines"].GetVariantVectorPtr();
	assert(lines);
	lines->Push(polyline);
}

void DxfReader::ParsePolyLineVertex(VariantMap& polyline)
{
	URHO3D_LOGINFO("Parsing polyline vertex...");

	LinePair nextPair = GetNextLinePair();

	unsigned int flags = 0;
	VariantVector indices;
	Vector3 v;

	while (!IsEnd(nextPair)) {

		if (nextPair.first_ == 0) { // SEQEND or another VERTEX
			break;
		}

		switch (nextPair.first_)
		{
		case 8:
			// layer to which the vertex belongs to - assume that
			// this is always the layer the top-level polyline
			// entity resides on as well.
			break;

		case 70:
			flags = nextPair.second_.GetUInt();
			break;

			// VERTEX COORDINATES
		case 10: 
			v.x_ = nextPair.second_.GetFloat();
			break;

		case 20: 
			v.y_ = nextPair.second_.GetFloat();
			break;

		case 30: 
			v.z_ = nextPair.second_.GetFloat();
			break;

			// POLYFACE vertex indices
		case 71:
		case 72:
		case 73:
		case 74:
			if (indices.Size() == 4) {
				URHO3D_LOGERROR("DXF: more than 4 indices per face not supported; ignoring");
				break;
			}
			indices.Push(nextPair.second_.GetUInt());
			break;

			// color
		case 62:
			URHO3D_LOGERROR("Ignoring vertex color...");
			break;
		};

		//add to vertex list
		VariantVector* verts = polyline["Vertices"].GetVariantVectorPtr();
		assert(verts);
		verts->Push(v);

		//add to face list
		VariantVector* faces = polyline["Faces"].GetVariantVectorPtr();
		assert(faces);
		faces->Push(indices);

		//recurse
		nextPair = GetNextLinePair();
	}
}

void DxfReader::Parse3DFace()
{
	URHO3D_LOGINFO("Parsing 3D face...");

	LinePair nextPair = GetNextLinePair();

	//get reference to last block
	VariantMap* currBlock = blocks_.Back().GetVariantMapPtr();

	//store polyline line data in variantmap
	VariantMap polyline;
	polyline["Vertices"] = VariantVector();
	polyline["Faces"] = VariantVector();

	//some data
	Vector3 vip[4];
	Color clr = Color::BLACK;
	bool b[4] = { false,false,false,false };

	while (!IsEnd(nextPair)) {

		// next entity with a groupcode == 0 is probably already the next vertex or polymesh entity
		if (nextPair.first_ == 0) {
			break;
		}
		switch (nextPair.first_)
		{

			// 8 specifies the layer
		case 8:
			polyline["Layer"] = nextPair.second_.GetString();
			break;

			// x position of the first corner
		case 10: 
			vip[0].x_ = nextPair.second_.GetFloat();
			b[2] = true;
			break;

			// y position of the first corner
		case 20: 
			vip[0].y_ = nextPair.second_.GetFloat();
			b[2] = true;
			break;

			// z position of the first corner
		case 30: 
			vip[0].z_ = nextPair.second_.GetFloat();
			b[2] = true;
			break;

			// x position of the second corner
		case 11: 
			vip[1].x_ = nextPair.second_.GetFloat();
			b[3] = true;
			break;

			// y position of the second corner
		case 21: 
			vip[1].y_ = nextPair.second_.GetFloat();
			b[3] = true;
			break;

			// z position of the second corner
		case 31: 
			vip[1].z_ = nextPair.second_.GetFloat();
			b[3] = true;
			break;

			// x position of the third corner
		case 12:
			vip[2].x_ = nextPair.second_.GetFloat();
			b[0] = true;
			break;

			// y position of the third corner
		case 22: 
			vip[2].y_ = nextPair.second_.GetFloat();
			b[0] = true;
			break;

			// z position of the third corner
		case 32: 
			vip[2].z_ = nextPair.second_.GetFloat();
			b[0] = true;
			break;

			// x position of the fourth corner
		case 13: 
			vip[3].x_ = nextPair.second_.GetFloat();
			b[1] = true;
			break;

			// y position of the fourth corner
		case 23: 
			vip[3].y_ = nextPair.second_.GetFloat();
			b[1] = true;
			break;

			// z position of the fourth corner
		case 33: 
			vip[3].z_ = nextPair.second_.GetFloat();
			b[1] = true;
			break;

			// color
		case 62:
			break;
		};

		//recurse
		nextPair = GetNextLinePair();
	}

	//fill the data
	VariantVector pVerts;
	pVerts.Resize(4);

	for (int i = 0; i < 4; i++)
	{
		pVerts[i] = vip[i];
	}

	polyline["Vertices"] = pVerts;

	//add to line list
	VariantVector* lines = (*currBlock)["Lines"].GetVariantVectorPtr();
	assert(lines);
	lines->Push(polyline);

	//probably need to add some stuff here...
}




