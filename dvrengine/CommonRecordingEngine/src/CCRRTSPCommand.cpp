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
* Description:    RTSP command parser and producer*
*/




// INCLUDE FILES
#include "CCRRtspCommand.h"
#include "CCRSock.h"
#include <imcvcodc.h>
#include <Hash.h>
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TReal KRealZero( 0.0 ); 
const TReal KRealMinusOne( -1.0 ); 
// Length of a digest hash before converting to hex.
const TInt KCRRawHashLength( 16 );
// Length of a digest hash when represented in hex
const TInt KCRHashLength( 32 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtspCommand::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRRtspCommand* CCRRtspCommand::NewL()
    {
    CCRRtspCommand* self = new( ELeave ) CCRRtspCommand();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::CCRRtspCommand
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRRtspCommand::CCRRtspCommand()
  : iCommand( ERTSPCommandNOCOMMAND )
    {  
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::~CCRRtspCommand
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtspCommand::~CCRRtspCommand()
    {
    LOG( "CCRRtspCommand::~CCRRtspCommand" );
    
    // iRtspText is deleted in base class destructor
    delete iAuthHeader;
    delete iMD5Calculator;
    delete iUserAgent;
    delete iWapProfile;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::TryParseL
// 
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::TryParseL( const TDesC8 &aString ) 
    {
    // try to find out if end of the command has been received
    // "RTSP/1.0 XXX\r\n\r\n" at least..
    const TInt KDVRMinCommandLen( 14 ); 
    TInt replyEndOffSet( ( aString.Length() < KDVRMinCommandLen )?
                           KDVRMinCommandLen: aString.Find( KCR2NewLines() ) ); 
    if ( replyEndOffSet == KErrNotFound )
        {
        // need to have more, do nothing yet
        LOG( "CCRRtspCommand::TryParseL() out because response not complete" );
        User::Leave( KErrUnderflow ); 
        }

    // copy the stuff into local variable:      
    delete iRtspText; iRtspText = NULL; 
    iRtspText = aString.AllocL(); 
    iCommand = ERTSPCommandNOCOMMAND;

    // try each command in order: 
    if ( iRtspText->Find( KCROPTIONS() ) == 0 )
        {
        // it was OPTIONS command
        LOG( "CCRRtspCommand::TryParseL() -> OPTIONS" );
        iCommand = ERTSPCommandOPTIONS;
        }
    else if ( iRtspText->Find( KCRDESCRIBE() ) == 0 )
        {
        LOG( "CCRRtspCommand::TryParseL() -> DESCRIBE" );
        iCommand = ERTSPCommandDESCRIBE;
        }  
    else if ( iRtspText->Find( KCRTEARDOWN() ) == 0 )
        {
        LOG( "CCRRtspCommand::TryParseL() -> TEARDOWN" );
        iCommand = ERTSPCommandTEARDOWN;        
        }                   
    else if ( iRtspText->Find( KCRPAUSE() ) == 0 )
        {
        LOG( "CCRRtspCommand::TryParseL() -> PAUSE" );
        iCommand = ERTSPCommandPAUSE;       
        }                                       
    else if ( iRtspText->Find( KCRSETUP() ) == 0 )
        {
        LOG( "CCRRtspCommand::TryParseL() -> SETUP" );
        iCommand = ERTSPCommandSETUP;       
        }
    else if ( iRtspText->Find( KCRPLAY() ) == 0 )
        {
        LOG( "CCRRtspCommand::TryParseL() -> PLAY" );
        iCommand = ERTSPCommandPLAY;        
        }  
    else
        {
        User::Leave( KErrNotSupported ); 
        }

    // then find CSeq
    FindCSeqL();    
    // then find session id
    FindSessionIdL(); 
    // then find possible content. for commands it is usually not there
    FindContentL(); 
    // find URL
    FindURLL(); 
    // find possible transport method
    // IMPORTANT: should be done before parsing client port
    FindTransport();
    // find possible client port
    FindClientPorts();
    // find possible range-header
    ParseRange();
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::FindURLL
// in rtsp the URL is between first and second whitespace.
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::FindURLL( void ) 
    {
#ifdef _DEBUG   
    _LIT( KPanicStr, "RTSPCommon" ); 
    __ASSERT_DEBUG( iRtspText, 
                    User::Panic( KPanicStr, KErrBadHandle ) );
#endif                    
    
    iURL.Set( NULL, 0 );
    TInt spaceOffset( iRtspText->Locate( ' ' ) ); 
    if ( spaceOffset < 0 ) 
        {
        User::Leave( KErrNotSupported ); 
        }
        
    TPtrC8 beginningFromUrl( iRtspText->Right( iRtspText->Length() - 
                                             ( spaceOffset + 1 ) ) ) ;      
    spaceOffset = beginningFromUrl.Locate( ' ' ); 
    if ( spaceOffset < 0 ) 
        {
        User::Leave( KErrNotSupported ); 
        }
    
    iURL.Set( beginningFromUrl.Left( spaceOffset ) ); 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::URL
//
// method that returns URL
// -----------------------------------------------------------------------------
//  
TInt CCRRtspCommand::URL( TPtrC8& aURL ) 
    {
    if ( iURL.Ptr() != NULL )   
        {
        aURL.Set ( iURL ); 
        return KErrNone;
        }

    return KErrNotFound; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::SetURL
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetURL ( const TDesC8& aURL ) 
    {
    iURL.Set ( aURL ); 
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommand::SetAuthentication
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetAuthentication( TBool aAuth ) 
    {
    iAuthenticationNeeded = aAuth;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::SetUserAgentL
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetUserAgentL( const TDesC8& aUserAgent )
    {
    delete iUserAgent; iUserAgent = NULL;
    iUserAgent = aUserAgent.AllocL();
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::SetBandwidth
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetBandwidth( TInt aBandwidth )
    {
    iBandwidth = aBandwidth;
    iBandwidthAvailable = ETrue;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::SetWapProfileL
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetWapProfileL( const TDesC8& aWapProfile )
    {
    delete iWapProfile; iWapProfile = NULL;
    iWapProfile = aWapProfile.AllocL();
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::SetCommand
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommand::SetCommand( TCommand aCommand ) 
    {
    iCommand = aCommand; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::Command
//
// -----------------------------------------------------------------------------
//  
CCRRtspCommand::TCommand CCRRtspCommand::Command( void ) const 
    {
    return iCommand;    
    }       
            
// -----------------------------------------------------------------------------
// CCRRtspCommand::ProduceL
//
// -----------------------------------------------------------------------------
//  
TPtrC8& CCRRtspCommand::ProduceL( void ) 
    {
    // First common part for all commands, except actual command
    delete iRtspText; iRtspText = NULL;
    iRtspText = HBufC8::NewL( KMaxName );
    iRtspText->Des().Zero();
    
    switch ( iCommand )
        {
        case ERTSPCommandOPTIONS:
            AppendL( iRtspText, KCROPTIONS ); 
            break;
        
        case ERTSPCommandDESCRIBE:
            AppendL( iRtspText, KCRDESCRIBE ); 
            break;
        
        case ERTSPCommandTEARDOWN:     
            AppendL( iRtspText, KCRTEARDOWN ); 
            break;
        
        case ERTSPCommandPAUSE:
            AppendL( iRtspText, KCRPAUSE ); 
            break;
        
        case ERTSPCommandSETUP:        
            AppendL( iRtspText, KCRSETUP ); 
            break;
        
        case ERTSPCommandPLAY : 
            AppendL( iRtspText, KCRPLAY ); 
            break;
        
        default:
            User::Leave( KErrNotSupported ); 
            break;
        }
    
    AppendL( iRtspText, iURL ); 
    AppendL( iRtspText, KCRSpace ); 
    AppendL( iRtspText, KCRRTSP10 ); 
    AppendL( iRtspText, KCRNewLine ); 
    AppendL( iRtspText, KCRCSeq ); 
    AppendNumL( iRtspText, iCSeq );
    AppendL( iRtspText, KCRNewLine );

    if ( iUserAgent )
        {
        TPtrC8 useragent( iUserAgent->Des() );
        AppendFormatL( iRtspText, KCRRTSPUserAgentHeader, &useragent );
        }

    // then variable tail depending on command
    switch ( iCommand )
        {
        case ERTSPCommandOPTIONS:
            if ( iSessionId.Ptr() != NULL ) 
                {
                AppendL( iRtspText, KCRSessionStr() ); 
                AppendL( iRtspText, iSessionId ); // now only session number
                AppendL( iRtspText, KCRNewLine );                
                }
            break;
        
        case ERTSPCommandDESCRIBE:
            {
            AppendL( iRtspText, KCRAcceptSDP );
            if ( iWapProfile )
                {
                TPtrC8 profile = iWapProfile->Des();
                AppendFormatL( iRtspText, KCRRTSPXWapProfile, &profile );
                }
            if ( iBandwidthAvailable )
                {
                AppendFormatL( iRtspText, KCRRTSPBandwidth, iBandwidth );
                }
            }
            break;
        
        case ERTSPCommandTEARDOWN:     
            if ( iSessionId.Ptr() != NULL ) 
                {
                AppendL( iRtspText, KCRSessionStr() ); 
                AppendL( iRtspText, iSessionId ); // now only session number
                AppendL( iRtspText, KCRNewLine );                
                }
            break;

        case ERTSPCommandPAUSE:        
            if ( iSessionId.Ptr() != NULL ) 
                {
                AppendL( iRtspText, KCRSessionStr() ); 
                AppendL( iRtspText, iSessionId ); // now only session number
                AppendL( iRtspText, KCRNewLine );                
                }
            break;

        case ERTSPCommandSETUP:        
            {
            // build transport header according to chosen method
            switch ( iTransport )
                {
                case ERTPOverUDP:
                    AppendFormatL( iRtspText, KCRTransportHeaderUDP,
                                   iClientPort, iClientPort + 1 );
                    break;
                case ERTPOverTCP:
                    AppendFormatL( iRtspText, KCRTransportHeaderTCP,
                                   iClientPort, iClientPort + 1 );
                    break;
                
                case ERTPOverMulticast:
                    AppendL( iRtspText, KCRTransportHeaderMulticast );
                    break;
                }

            // Session: 5273458854096827704         
            if ( iSessionId.Ptr() != NULL ) 
                {
                AppendL( iRtspText, KCRSessionStr ); 
                AppendL( iRtspText, iSessionId ); // now only session number
                AppendL( iRtspText, KCRNewLine ); 
                }
            if ( iWapProfile )
                {
                TPtrC8 profile( iWapProfile->Des() );
                AppendFormatL( iRtspText, KCRRTSPXWapProfile, &profile );
                }
            }
            break;
            
        case ERTSPCommandPLAY: 
            {
            if ( !( iLowerRange == KRealZero && iUpperRange == KRealMinusOne ) )
                {
                // Range was something else than 0,-1
                TBuf8<KMaxName> buf( KCRRangeHeader );
                TRealFormat format( 10, 3 ); 
                format.iTriLen = 0; 
                format.iPoint = '.';
                buf.AppendNum( iLowerRange, format );
                buf.Append( '-' ); 
                if ( iUpperRange > KRealZero )
                    {
                    buf.AppendNum( iUpperRange, format );
                    }
                
                buf.Append( KCRNewLine );
                AppendL( iRtspText, buf );
                }
            if ( iSessionId.Ptr() != NULL ) 
                {
                AppendL( iRtspText, KCRSessionStr() ); 
                AppendL( iRtspText, iSessionId ); // now only session number
                AppendL( iRtspText, KCRNewLine ); 
                }
            }
            break;

        default:
            User::Leave( KErrNotSupported ); 
            break;
        }           
    
    if ( iAuthenticationNeeded )
        {
        TBool useDigest( EFalse );
        
        if ( iAuthType && ( iAuthType->FindC( KCRAuthDigest ) != KErrNotFound ) )
            {
            useDigest = ETrue;
            }
        
        switch ( iCommand )
            {
            case ERTSPCommandOPTIONS :
                useDigest ? CalculateDigestResponseL( KCROPTIONSNoSpace ) :
                            CalculateBasicResponseL( KCROPTIONSNoSpace );
                
                if ( iAuthHeader ) 
                    {
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            case ERTSPCommandDESCRIBE :
                useDigest ? CalculateDigestResponseL( KCRDESCRIBENoSpace ) :
                            CalculateBasicResponseL( KCRDESCRIBENoSpace );  
               
                if ( iAuthHeader ) 
                    {
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            case ERTSPCommandTEARDOWN :     
                useDigest ? CalculateDigestResponseL( KCRTEARDOWNNoSpace ) :
                            CalculateBasicResponseL( KCRTEARDOWNNoSpace );
                
                if ( iAuthHeader ) 
                    {               
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            case ERTSPCommandPAUSE :        
                useDigest ? CalculateDigestResponseL( KCRPAUSENoSpace ) :
                            CalculateBasicResponseL( KCRPAUSENoSpace );
                
                if ( iAuthHeader ) 
                    {           
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            case ERTSPCommandSETUP :        
                useDigest ? CalculateDigestResponseL( KCRSETUPNoSpace ) :
                            CalculateBasicResponseL( KCRSETUPNoSpace );
                
                if ( iAuthHeader ) 
                    {
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            case ERTSPCommandPLAY :  
                useDigest ? CalculateDigestResponseL( KCRPLAYNoSpace ) :
                            CalculateBasicResponseL( KCRPLAYNoSpace );
                
                if ( iAuthHeader ) 
                    {           
                    AppendL( iRtspText, iAuthHeader->Des() );
                    }
                break;
            
            default:
                User::Leave( KErrNotSupported ); 
                break;
            }       
        }
        
    AppendL( iRtspText, KCRNewLine ); 
    iProductDescriptor.Set( *iRtspText );
    return iProductDescriptor;
    }   
    
// -----------------------------------------------------------------------------
// CCRRtspCommand::AppendL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspCommand::AppendL( HBufC8*& aBuffer, const TDesC8& aStr )
    {
    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + aStr.Length() ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + aStr.Length() + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.Append( aStr );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommand::AppendNumL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspCommand::AppendNumL( HBufC8*& aBuffer, const TInt aNum )
    {
    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + KMaxInfoName ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + KMaxInfoName + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.AppendNum( aNum );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommand::AppendFormatL
//
// -----------------------------------------------------------------------------
//      
void CCRRtspCommand::AppendFormatL(
    HBufC8*& aBuffer,
    TRefByValue<const TDesC8> aFmt, ... )
    {
    VA_LIST list;
    VA_START( list, aFmt );
    HBufC8* buf = HBufC8::NewLC( KMaxDataSize );
    buf->Des().FormatList( aFmt, list );
    VA_END( list );

    TPtr8 ptr( aBuffer->Des() );
    if ( ( ptr.Length() + buf->Length() ) >= ptr.MaxLength() )
        {
        const TInt newLength( ptr.Length() + buf->Length() + KMaxName );
        aBuffer = aBuffer->ReAllocL( newLength );
        ptr.Set( aBuffer->Des() );
        }
    
    ptr.Append( *buf );
    CleanupStack::PopAndDestroy( buf );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommand::Hash
// Calculates hash value ( from S60 HttpFilters )
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::HashL( const TDesC8& aMessage, TDes8& aHash )
    {
    LOG( "CCRRtspCommand::HashL() in" );
    // check if md5 calculator is already constructed
    if ( !iMD5Calculator )
        { 
        iMD5Calculator = CMD5::NewL();
        }
    // Calculate the 128 bit (16 byte) hash
    iMD5Calculator->Reset();
    TPtrC8 hash = iMD5Calculator->Hash( aMessage );
    
    // Now print it as a 32 byte hex number
    aHash.Zero();
    _LIT8( formatStr, "%02x" );
    TBuf8<2> scratch;
    for ( TInt i( 0 ); i < KCRRawHashLength; i++ )
        {
        scratch.Zero();
        scratch.Format( formatStr, hash[i] );
        aHash.Append( scratch );
        }
    
    LOG( "CCRRtspCommand::HashL() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::CalculateBasicResponseL
//
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::CalculateBasicResponseL( const TDesC8& /*aMethod*/ )
    {
    LOG( "CCRRtspCommand::CalculateBasicResponseL() in" );
    
    if ( !( iUserName && iPassword  ) )
        {
        LOG( "CCRRtspCommand::CalculateBasicResponseL() out, username or password not set" );
        delete iAuthHeader; 
        iAuthHeader = NULL;
        return; // no can do
        }
    
    HBufC8* plainData = HBufC8::NewL( iUserName->Length() + 1 + // ':'
                                      iPassword->Length() );
    
    CleanupStack::PushL( plainData );
    
    plainData->Des().Append( *iUserName );
    plainData->Des().Append( ':' );
    plainData->Des().Append( *iPassword );
    
    // Max size = ((Bytes + 3 - (Bytes MOD 3)) /3) x 4 
    TInt base64MaxSize = ( ( plainData->Length() + 3 - 
            ( plainData->Length() % 3 ) ) / 3 ) * 4;
    
    HBufC8* encodedData = HBufC8::NewL( base64MaxSize );
    TPtr8 dataPrt( encodedData->Des() );   
    CleanupStack::PushL( encodedData );
    
    TImCodecB64 b64enc;
    b64enc.Initialise();
    b64enc.Encode( *plainData, dataPrt );
    
    delete iAuthHeader; 
    iAuthHeader = NULL;
    iAuthHeader = HBufC8::NewL( KCRAuthorizationBasicHeader().Length() + encodedData->Length() );           
    iAuthHeader->Des().Format( KCRAuthorizationBasicHeader, encodedData );
    
    CleanupStack::PopAndDestroy( encodedData );
    CleanupStack::PopAndDestroy( plainData );
    
    LOG( "CCRRtspCommand::CalculateBasicResponseL() out" );
    }

// -----------------------------------------------------------------------------
// CCRRtspCommand::CalculateDigestResponseL
//
// -----------------------------------------------------------------------------
//
void CCRRtspCommand::CalculateDigestResponseL( const TDesC8& aMethod )
    {
    LOG( "CCRRtspCommand::CalculateDigestResponseL() in" );

    if ( !( iUserName && iPassword && iNonce && iOpaque && iRealm && iUri ) )
        {
        LOG( "CCRRtspCommand::CalculateDigestResponseL() out, username or password not set" );
        delete iAuthHeader; iAuthHeader = NULL;
        return; // no can do
        }

    TBuf8<KCRHashLength> hash1;
    TBuf8<KCRHashLength> hash2;
    TBuf8<KCRHashLength> finalHash;

    // calculate the hash1 using "username:realm:password"
    HBufC8* hashPtr = HBufC8::NewL ( iUserName->Length() + 1 +  // ':'
                                     iRealm->Length() + 1 +      // ':' 
                                     iPassword->Length() );
    hashPtr->Des().Append( *iUserName );
    hashPtr->Des().Append( ':' );
    hashPtr->Des().Append( *iRealm );
    hashPtr->Des().Append( ':' );
    hashPtr->Des().Append( *iPassword );
    
    HashL( *hashPtr, hash1 );
    delete hashPtr; hashPtr = NULL;

    // calculate hash2 using "Method:uri"
    HBufC8* hashPtr2 = HBufC8::NewL(aMethod.Length() + 1 + iUri->Length() );
    hashPtr2->Des().Append( aMethod );
    hashPtr2->Des().Append( ':' );
    hashPtr2->Des().Append( *iUri );
    
    HashL( *hashPtr2, hash2 );
    delete hashPtr2; hashPtr2 = NULL;

    // calculate finalHash to be sent to remote server using
    // hash1 + ":" + nonce + ":" + hash2
    HBufC8* hashPtr3 = HBufC8::NewL( hash1.Length() + 1 + // ':'
                                     iNonce->Length() + 1 + // ':'
                                     hash2.Length() );
    hashPtr3->Des().Append( hash1 );
    hashPtr3->Des().Append( ':' );
    hashPtr3->Des().Append( *iNonce );
    hashPtr3->Des().Append( ':' );
    hashPtr3->Des().Append( hash2 );

    HashL( *hashPtr3, finalHash );
    delete hashPtr3; hashPtr3 = NULL;

    // generate the authentication header 
    if ( iOpaque->Length()  ) 
        {
        delete iAuthHeader; iAuthHeader = NULL;
        iAuthHeader = HBufC8::NewL( KCRAuthorizationHeader().Length() + 
            iUserName->Length() + iRealm->Length() + iNonce->Length() +
            iUri->Length() + finalHash.Length() + iOpaque->Length() );
        
        iAuthHeader->Des().Format( KCRAuthorizationHeader, iUserName,
            iRealm, iNonce, iUri, &finalHash, iOpaque );
        }
    else
        {
        delete iAuthHeader; iAuthHeader = NULL;
        iAuthHeader = HBufC8::NewL( KCRAuthorizationHeaderNoOpaque().Length() +
            iUserName->Length() + iRealm->Length() + iNonce->Length() +
            iUri->Length() + finalHash.Length() );
                                    
        iAuthHeader->Des().Format( KCRAuthorizationHeaderNoOpaque, iUserName,
            iRealm, iNonce, iUri, &finalHash );
        }

    LOG( "CCRRtspCommand::CalculateDigestResponseL() out" );   
    }

//  End of File
