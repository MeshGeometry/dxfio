#include "Core/Context.h"
#include "Core/Object.h"
#include "Container/Vector.h"
#include "Container/Str.h"
#include "Core/Variant.h"
#include "IO/Deserializer.h"
#include "IO/File.h"
#include "IO/FileSystem.h"

using namespace Urho3D;

URHO3D_API class DxfReader : public Object
{
	URHO3D_OBJECT(DxfReader, Object);

public:
	DxfReader(Context* context, String path);
	~DxfReader() {};

	int ReadGroupCode();
	String ReadGroupName();

protected:

	File* source_;

	VariantMap header_;
	VariantMap classes_;
	VariantMap tables_;
	VariantMap blocks_;
	VariantMap entities_;
};