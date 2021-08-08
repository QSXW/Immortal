#pragma once

#ifdef _MSC_VER

#	include <D3d12.h>

#   if defined( DEBUG ) | defined ( _DEBUG )
#   include <D3d12SDKLayers.h>
#   endif /* DEBUG */

#endif /* _MSC_VER */