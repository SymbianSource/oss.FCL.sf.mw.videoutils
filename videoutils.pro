#
# Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description:   Definitions for video type*
#

TEMPLATE = subdirs
CONFIG += ordered
symbian: {
BLD_INF_RULES.prj_mmpfiles += $$LITERAL_HASH"include \"videoutils_plat/videoplayer_constants_api/group/bld.inf\""
}
