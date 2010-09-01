/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    RTP clip metaheader write/read functionalities.*
*/




#ifndef CRTPMETAHEADER_H
#define CRTPMETAHEADER_H

// INCLUDES
#include <ipvideo/CRtpUtil.h>
#include <f32file.h>

// CONSTANTS
const TInt KStringLengthBytes( 1 );
const TInt KMaxMetaHeaderLength( 2048 );
const TInt KMetaLengthPoint( 0 );
const TInt KAttributesPoint( KMetaLengthPoint + KIntegerBytes ); // 4
const TInt KPlayCountPoint( KAttributesPoint + KIntegerBytes );	 // 8
const TInt KPlaySpotPoint( KPlayCountPoint + KIntegerBytes );	 // 12
const TInt KReservedPoint1( KPlaySpotPoint + KIntegerBytes );	 // 16
const TInt KReservedPoint2( KReservedPoint1 + KIntegerBytes );	 // 20
const TInt KReservedPoint3( KReservedPoint2 + KIntegerBytes );	 // 24
const TInt KReservedPoint4( KReservedPoint3 + KIntegerBytes );	 // 28
const TInt KStartTimePoint( KReservedPoint4 + KIntegerBytes );	 // 32
const TInt KEndTimePoint( KStartTimePoint + 2 * KIntegerBytes ); // 40
const TInt KDurationPoint( KEndTimePoint + 2 * KIntegerBytes );	 // 48
const TInt KSeekArrayPoint( KDurationPoint + KIntegerBytes );	 // 52
const TInt KUserIdPoint( KSeekArrayPoint + KIntegerBytes );		 // 56
const TInt KDeviceInfoPoint( KUserIdPoint + KStringLengthBytes + // 72
                             KUserIdLength );
// Metaheader attributes
const TInt KOngoingFlagShift( 0 );
const TInt KCompletedFlagShift( 1 );
const TInt KProtectedFlagShift( 2 );
const TInt KFailedFlagShift( 3 );
const TInt KVersionFieldShift( 4 );
const TInt KQualityFieldShift( 8 );
const TInt KPostRuleFieldShift( 16 );
const TInt KParentalFieldShift( 24 );

// MACROS
// None

// DATA TYPES
// None

// FORWARD DECLARATIONS
// None

// CLASS DECLARATION

/**
*  Meta header handling for RTP file format.
*
*  @lib CommonRecordingEngine.lib
*  @since Series 60 3.0
*/
class CRtpMetaHeader : public CBase
    {

public: // Enumeration

    // Defines mode
    enum TMetaMode
        {
        EMetaRead = 200,
        EMetaWrite,
        EMetaUpdate
        };

public: // Data types
    
    // Defines clip attributes struct
    class SAttributes
        {
        public: // Data

            /**
            * Recording ongoing.
            */
            TBool iOngoing;
            
            /**
            * Recording completed.
            */
            TBool iCompleted;
            
            /**
            * Clip protected.
            */
            TBool iProtected;
            
            /**
            * Recording failed.
            */
            TBool iFailed;
            
            /**
            * Clip version.
            */
            TUint8 iVersion;
            
            /**
            * Clip quality.
            */
            TUint8 iQuality;

            /**
            * Clip's post accuisition rule.
            */
            TUint8 iPostRule;
            
            /**
            * Parental rate.
            */
            TUint8 iParental;
            
            /**
            * Viewed counter.
            */
            TInt iPlayCount;
            
            /**
            * Point where vieving stopped.
            */
            TInt iPlaySpot;
            
        };

public: // Constructors and destructor

    /**
    * Two-phased constructor.
    * @param aFile a reference to file operations.
    * @param aMode a meta data read or write mode.
    */
    static CRtpMetaHeader* NewL( RFile& aFile,
                                 const TMetaMode& aMode );

    /**
    * Two-phased constructor.
    * @param aFile a reference to file operations.
    * @param aMode a meta data read or write mode.
    */
    static CRtpMetaHeader* NewLC( RFile& aFile,
                                  const TMetaMode& aMode );

    /**
    * Destructor.
    */
    virtual ~CRtpMetaHeader();

public: // New functions

    /**
    * Getter for seek header point.
    * @since Series 60 3.0
    * @return seek header point.
    */
    TInt SeekHeaderPoint();

    /**
    * Writes whole meta header to the clip.
    * Note, need use only with node: EMetaWrite.
    * @since Series 60 3.0
    * @return None.
    */
    void CommitL();

    /**
    * Writes clip's attributes of meta data header to a file.
    * @since Series 60 3.0
    * @param aAtt a struct of attributes to write.
    * @return None.
    */
    void WriteAttributesL( const SAttributes& aAtt );

    /**
    * Writes start date/time to meta data header of the clip.
    * @since Series 60 3.0
    * @param aTime a date/time value to write
    * @return None.
    */
    void WriteStartTimeL( const TTime& aTime );

    /**
    * Writes start date/time to meta data header of the clip.
    * @since Series 60 3.0
    * @param aTime a date/time value to write
    * @return None.
    */
    void WriteEndTimeL( const TTime& aTime );

    /**
    * Writes duration to meta data header of the clip.
    * @since Series 60 3.0
    * @param aDuration a duration value to write
    * @return None.
    */
    void WriteDurationL( const TInt aDuration );

    /**
    * Writes seek array point to meta data header of the clip.
    * @since Series 60 3.0
    * @param aPoint a value to write
    * @return None.
    */
    void WriteSeekArrayPointL( const TInt aPoint );

    /**
    * Writes device info to meta data header of the clip.
    * @since Series 60 3.0
    * @param aId a id to write
    * @return None.
    */
    void WriteUserIdL( const TDesC& aId );

    /**
    * Writes device info to meta data header of the clip.
    * @since Series 60 3.0
    * @param aInfo a device info to write
    * @return None.
    */
    void WriteDeviceInfoL( const TDesC& aInfo );

    /**
    * Writes ESG to meta data header of the clip.
    * @since Series 60 3.0
    * @param aService a name of the service
    * @param aProgram a name of the program
    * @return None.
    */
    void WriteEsgDataL( const TDesC& aService,
                        const TDesC& aProgram );

    /**
    * Writes SRTP data to meta data header of the clip.
    * @since Series 60 3.0
    * @param aSrtpData a SRTP data to write.
    * @return None.
    */
    void WriteSrtpDataL( const TDesC8& aSrtpData );

    /**
    * Writes SDP file data to meta data header of the clip.
    * @since Series 60 3.0
    * @param aSdpData a SDP file data.
    * @return None.
    */
    void WriteSdpDataL( const TDesC8& aSdpData );

    /**
    * Reads clip's attributes of meta data header from a file.
    * @since Series 60 3.0
    * @param aAtt a struct of attributes to read.
    * @return None.
    */
    void ReadAttributesL( SAttributes& aAtt );

    /**
    * Reads start date/time of meta data header from the clip.
    * @since Series 60 3.0
    * @param aTime a readed date/time value
    * @return None.
    */
    void ReadStartTimeL( TTime& aTime );

    /**
    * Reads end date/time of meta data header from the clip.
    * @since Series 60 3.0
    * @param aTime a readed date/time value
    * @return None.
    */
    void ReadEndTimeL( TTime& aTime );

    /**
    * Reads duration in seconds of meta data header from the clip.
    * @since Series 60 3.0
    * @param aDuration a readed duration value
    * @return None.
    */
    void ReadDurationL( TInt& aDuration );

    /**
    * Reads seek array point of meta data header from the clip.
    * @since Series 60 3.0
    * @param aPoint a readed seek array point value
    * @return None.
    */
    void ReadSeekArrayPointL( TInt& aPoint );

    /**
    * Reads user id of meta data header from the clip.
    * @since Series 60 3.0
    * @param a buffer for info to read
    * @return None.
    */
    void ReadUserIdL( TDes& aId );

    /**
    * Reads device info of meta data header from the clip.
    * @since Series 60 3.0
    * @param a buffer for info to read
    * @return None.
    */
    void ReadDeviceInfoL( TDes& aInfo );

    /**
    * Reads ESG of meta data header from the clip.
    * @since Series 60 3.0
    * @param a buffer for service name to read
    * @param a buffer for program name to read
    * @return None.
    */
    void ReadEsgDataL( TDes& aService, TDes& aProgram );

    /**
    * Reads SRTP data of meta data header from the clip.
    * @since Series 60 3.0
    * @return pointer to readed SRTP data.
    */
    HBufC8* ReadSrtpDataL();

    /**
    * Reads SDP file data of meta data header from the clip.
    * @since Series 60 3.0
    * @return pointer to readed SDP file data.
    */
    HBufC8* ReadSdpDataL();

private: // Constructors and destructor

    /**
    * C++ default constructor.
    * @param aFile a reference to file operations
    * @param aMode a meta data read or write mode
    */
    CRtpMetaHeader( RFile& aFile,
                    const TMetaMode& aMode );

    /**
    * Symbian 2nd phase constructor can leave and is private by default.
    */
    void ConstructL();
    
private: // New functions

    /**
    * Writes any string with one byte len info to meta header.
    * @since Series 60 3.0
    * @param aPosition a position to write
    * @param aData a data to write
    * @return None.
    */
    void WriteStringDataL( const TInt aPosition,
                           const TDesC& aData );

    /**
    * Reads any string with one byte len info from meta header.
    * @since Series 60 3.0
    * @param aPosition a position from to read
    * @param aData a data to read
    * @return None.
    */
    void ReadStringDataL( const TInt aPosition,
                          TDes& aData );

    /**
    * Writes time info to meta data header of the clip.
    * @since Series 60 3.0
    * @param aPosition a position to append
    * @param aTime a time value to write
    * @return None.
    */
    void AddTimeL( const TInt aPosition,
                   const TTime& aTime );

    /**
    * Reads time info of meta data header from the clip.
    * @since Series 60 3.0
    * @param aPosition a position to read
    * @param aTime a readed time value
    * @return None.
    */
    void GetTimeL( const TInt aPosition,
                   TTime& aTime );

    /**
    * Adds intehger data to meta buffer.
    * @since Series 60 3.0
    * @param aPosition a position to add
    * @param aValue a integer value to add
    * @return None.
    */
    void AddIntegerL( const TInt aPosition,
                      const TInt aValue );

    /**
    * Adds new data to meta buffer.
    * @since Series 60 3.0
    * @param aPosition a position to add
    * @param aData a data to append
    * @return None.
    */
    void AddDataL( const TInt aPosition,
                   const TDesC8& aData );

    /**
    * Reads 32 bits (TInt) from a file from certain position.
    * @since Series 60 3.0
    * @param aPosition a position from to read
    * @param aValue a value to update
    * @return None.
    */
    void ReadTintFromFileL( const TInt& aPosition,
                            TInt& aValue );

private: // Data

    /**
    * File operations.
    */
    RFile& iFile;

    /**
    * Mode.
    */
    TMetaMode iMode;
    
    /**
    * File data buffer.
    */
    HBufC8* iMetaData;
    
    /**
    * File data buffer pointer.
    */
    TPtr8 iDataPtr;
    
    /**
    * ESG data point.
    */
    TInt iEsgDataPoint;
    
    /**
    * SRTP data point.
    */
    TInt iSrtpDataPoint;

    /**
    * SDP file data point.
    */
    TInt iSdpDataPoint;
    
    /**
    * Meta total length.
    */
    TInt iMetaTotal;
    
  };

#endif // CRTPMETAHEADER_H

// End of File
