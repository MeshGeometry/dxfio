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

URHO3D_API class DxfReader : public Object
{
	URHO3D_OBJECT(DxfReader, Object);

public:
	DxfReader(Context* context, String path);
	~DxfReader() {};

	/**************************************************************************
	Dxf info comes in pairs of lines, eg:
	 ---- 0         <- code id
	 ---- HEADER    <- value for this code 

	 NOTE: a code can have more than one value, therefore the full pair must be considered.

	This is the most basic example, other codes have more complex values.
	Therefore, we define a method that reads in pairs of lines.
	***************************************************************************/
	LinePair GetNextLinePair();

	//main loop for parsing
	bool Parse();

	//individual parsers
	void SkipSection();
	void ParseHeader();
	void ParseEntities();
	void ParseBlocks();
	void ParseBlock();
	void ParseInsertion();
	void ParsePolyLine();
	void ParsePolyLineVertex();
	void Parse3DFace();

	//some helpers
	bool Is(LinePair pair, int code, String name);
	bool IsType(LinePair pair, int code, VariantType type);

protected:

	File* source_;

	VariantMap header_;
	VariantMap classes_;
	VariantMap tables_;
	VariantMap blocks_;
	VariantMap entities_;
};