/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:   Definitions for video type*
*/


// Version : %version: 7 %


#ifndef CMPXVIDEOPLAYBACKDEFS_H
#define CMPXVIDEOPLAYBACKDEFS_H

#include <e32base.h>
#include <mpxattribute.h>

/**
*  Content ID identifying video category of content provided
*  in the media object and associated attributes.
*/
const TInt KMPXMediaIdVideoPlayback = 0x200159B1;

//
//  TInt for Command Id
//
const TMPXAttributeData KMPXMediaVideoPlaybackCommand = { KMPXMediaIdVideoPlayback, 0x01 };

//
//  Video Region
//
const TMPXAttributeData KMPXMediaVideoPlaybackDSARegion = { KMPXMediaIdVideoPlayback, 0x02 };

//
//  TRect
//
const TMPXAttributeData KMPXMediaVideoPlaybackTRect = { KMPXMediaIdVideoPlayback, 0x04 };

//
//  File Name
//
const TMPXAttributeData KMPXMediaVideoPlaybackFileName = { KMPXMediaIdVideoPlayback, 0x08 };

//
//  TMPXVideoMode
//
const TMPXAttributeData KMPXMediaVideoMode = { KMPXMediaIdVideoPlayback, 0x10 };

//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoError = { KMPXMediaIdVideoPlayback, 0x20 };


//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoAspectRatio = { KMPXMediaIdVideoPlayback, 0x40 };

//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoMovePdlFile = { KMPXMediaIdVideoPlayback, 0x80 };

//
//  TBool
//
const TMPXAttributeData KMPXMediaVideoAppForeground = { KMPXMediaIdVideoPlayback, 0x100 };

//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoTvOutConnected = { KMPXMediaIdVideoPlayback, 0x200 };

//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoTvOutPlayAllowed = { KMPXMediaIdVideoPlayback, 0x400 };

//
//  MimeType returned by the recognizer
//
const TMPXAttributeData KMPXMediaVideoRecognizedMimeType = { KMPXMediaIdVideoPlayback, 0x800 };

//
//  TInt
//
const TMPXAttributeData KMPXMediaVideoBufferingPercentage = { KMPXMediaIdVideoPlayback, 0x1000 };


enum TMPXVideoPlaybackCommand
{
    EPbCmdInitView,
    EPbCmdAbortDSA,
    EPbCmdRestartDSA,
    EPbCmdSetDisplayWindow,
    EPbCmdStartVideoSeekingForward,
    EPbCmdStartVideoSeekingBackward,
    EPbCmdStopVideoSeeking,
    EPbCmdHandleForeground,
    EPbCmdHandleBackground,
    EPbCmdNaturalAspectRatio,
    EPbCmdZoomAspectRatio,
    EPbCmdStretchAspectRatio,
    EPbCmdSetDefaultAspectRatio,
    EPbCmdTvOutEvent,
    EPbCmdUpdateSeekable,
    EPbCmdPluginError,
    EPbCmdEndofClipReached,
    EPbCmdHandleIncreaseVolume,
    EPbCmdHandleDecreaseVolume,
    EPbCmdCustomPause,
    EPbCmdCustomPlay,
    EPbCmdRetrieveBufferingPercentage,
    EPbCmdSurfaceRemoved
};

enum TMPXGeneralError
{
    KMPXVideoCallOngoingError = 1,
    KMPXVideoPlayOver2GDuringVoiceCallError,
    KMPXVideoTvOutPlaybackNotAllowed,
    KMPXVideoTvOutPlaybackNotAllowedClose,

    KMPXPluginError
};


//
//  Content ID identifying video display window category of content provided
//  in the media object and associated attributes.
//
const TInt KMPXMediaIdVideoDisplaySyncMessage = 0x20024333;

//
//  TInt for Command Id
//
const TMPXAttributeData KMPXMediaVideoDisplayCommand = { KMPXMediaIdVideoDisplaySyncMessage, 0x01 };

//
//  TSurfaceId
//
const TMPXAttributeData KMPXMediaVideoDisplayTSurfaceId = { KMPXMediaIdVideoDisplaySyncMessage, 0x02 };

//
//  TRect
//
const TMPXAttributeData KMPXMediaVideoDisplayCropRect = { KMPXMediaIdVideoDisplaySyncMessage, 0x04 };

//
//  TVideoAspectRatio
//
const TMPXAttributeData KMPXMediaVideoDisplayAspectRatio = { KMPXMediaIdVideoDisplaySyncMessage, 0x08 };


enum TMPXVideoDisplayCommand
{
    EPbMsgVideoSurfaceCreated,
    EPbMsgVideoSurfaceChanged,
    EPbMsgVideoSurfaceRemoved,
    EPbMsgVideoRemoveDisplayWindow
};


#endif

// EOF
