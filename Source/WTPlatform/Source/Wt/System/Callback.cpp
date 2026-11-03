#include "WT/stdafx.h"

#include "WT/System/Callback.h"

namespace WT
{
namespace System
{

CStaticCallback0* CreateStaticCallback0 ( void (* StaticMethod) () )
{
	return new CStaticCallback0 ( StaticMethod );
}

}
}

