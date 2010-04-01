#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Symbian Foundation License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.symbianfoundation.org/legal/sfl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
# 
# Description: Project file for building Videoutils components
#
#

TEMPLATE = subdirs
CONFIG += ordered
symbian: {
BLD_INF_RULES.prj_mmpfiles += $$LITERAL_HASH"include \"videoutils_plat/group/bld.inf\""
BLD_INF_RULES.prj_mmpfiles += $$LITERAL_HASH"include \"videoconnutility/group/bld.inf\""
}
