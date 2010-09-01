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
* Description:    RTSP command/response common part*
*/




// INCLUDE FILES
#include "CRRtspCommon.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TReal KRealZero( 0.0 ); 
const TReal KRealMinusOne( -1.0 ); 

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtspCommon::CCRRtspCommon
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRRtspCommon::CCRRtspCommon ()
  : iContentLen( KErrNotFound ),
    iContent( NULL, 0 ),
    iCSeq( KErrNotFound ),
    iSessionId( NULL, 0 ),
    iClientPort( KErrNotFound ),
    iLowerRange( KRealZero ),
    iUpperRange( KRealMinusOne ),
    iIsLiveStream( EFalse )
    {  
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::~CCRRtspCommon
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtspCommon::~CCRRtspCommon()
    {
    LOG( "CCRRtspCommon::~CCRRtspCommon()" );

    delete iRtspText; 
    delete iRealm;
    delete iOpaque;
    delete iNonce;
    delete iUserName;
    delete iUri; 
    delete iPassword;
    delete iAuthType;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::CSeq
// 
// -----------------------------------------------------------------------------
//
TInt CCRRtspCommon::CSeq( void ) 
    {
    return iCSeq;   
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::SetCSeq
// 
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::SetCSeq( TInt aCSeq ) 
    {
    iCSeq = aCSeq;  
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindCSeqL
// 
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::FindCSeqL( void ) 
    {
    TInt offSet( iRtspText->FindC( KCRCSeq() ) ); 
    if ( offSet == KErrNotFound ) 
        {
        LOG( "CCRRtspCommon::FindCSeqL FAILED, 'CSeq' not found [1]" );
        User::Leave ( KErrNotSupported ); 
        }
    
    offSet += KCRCSeq().Length();       
    TInt eolOffSet = iRtspText->Right ( iRtspText->Length() - 
                                        offSet ).Find ( KCRNewLine() ); 
    if ( eolOffSet == KErrNotFound ) 
        {
        LOG1( "CCRRtspCommon::FindCSeqL FAILED, 'CSeq' not found [2], offset=%d", offSet );
        User::Leave ( KErrNotSupported ); 
        }
    TPtrC8 CSeqString ( iRtspText->Mid ( offSet, eolOffSet ) );     
    TLex8 CSeqNumberLex( CSeqString ); 
    User::LeaveIfError ( CSeqNumberLex.Val( iCSeq ) ) ;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindSessionIdL
//
// method that finds session id from RTSP response
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::FindSessionIdL( void ) 
    {
    iSessionId.Set( NULL, 0 );
    TInt sessionStringOffset( iRtspText->FindC( KCRNlAndSession() ) );
    if ( sessionStringOffset > 0 )
        {
        TPtrC8 beginningFromSession( iRtspText->Right( iRtspText->Length() - 
                                   ( sessionStringOffset + 11 ) ) );
        TInt lfStringOffset( beginningFromSession.Find( KCRNewLine ) );
        if ( lfStringOffset != KErrNotFound )
            {
            iSessionId.Set( beginningFromSession.Mid( 0, lfStringOffset ) );
            // with some servers there is;Timeout=xxx after the session id; 
            // lets strip that out:
            TInt semicolonOffset( iSessionId.Locate( ';' ) ); 
            if ( semicolonOffset > 0 )
                {
                iSessionId.Set( iSessionId.Mid( 0, semicolonOffset ) ); 
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::SessionId
//
// method that returns session id string
// -----------------------------------------------------------------------------
//  
TInt CCRRtspCommon::SessionId( TPtrC8& aId ) 
    {
    if ( iSessionId.Ptr() != NULL )     
        {
        aId.Set( iSessionId ); 
        return KErrNone;
        }
    else
        {
        return KErrNotFound; 
        }       
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::SetSessionId
//
// 
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetSessionId( TPtrC8& aId ) 
    {
    iSessionId.Set( aId ); 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindContentL
//
// 
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::FindContentL( void ) 
    {
#ifdef _DEBUG       
    _LIT(KPanicStr, "RTSPCommon"); 
    __ASSERT_DEBUG( iRtspText, 
                    User::Panic( KPanicStr, KErrBadHandle ) );
#endif                    
    // find possible content:
    //
    TInt replyEndOffSet( iRtspText->Find( KCR2NewLines ) );
    if ( replyEndOffSet > KErrNotFound )
        {
        replyEndOffSet += KCR2NewLines().Length();
        }
    
    // next thing to check if there is content-length-header, 
    // it seems like it may follow about any response..
    //
    TInt contentLenOffset( KErrNotFound ); 
    iContentLen = KErrNotFound; // this will hold (usually SDP) length
    if ( ( contentLenOffset = iRtspText->Des().FindC( 
                              KCRRTSPContentLength() ) ) != KErrNotFound )
        {
        TLex8 contentLenLex( iRtspText->Des().Mid( contentLenOffset + 16, 5 ) );
        User::LeaveIfError( contentLenLex.Val( iContentLen ) ); 
        }
    
    LOG1( "CCRRtspCommon::FindContentL(), iContentLen: %d", iContentLen );                       

    // then set content in place    
    if ( replyEndOffSet > iRtspText->Length() )
        {
        LOG( "CCRRtspCommon::FindContentL() out because too litle data !" );
        User::Leave( KErrUnderflow ); 
        }
    if ( iContentLen > KErrNotFound )
        {   
        iContent.Set( iRtspText->Des().Mid( replyEndOffSet, iContentLen ) );
        }
    else
        {
        iContent.Set( NULL, 0 );
        }
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommon::ContentLen
// 
// -----------------------------------------------------------------------------
//
TInt CCRRtspCommon::ContentLen( void ) 
    {
    return iContentLen; 
    }
        
// -----------------------------------------------------------------------------
// CCRRtspCommon::Content
// 
// -----------------------------------------------------------------------------
//
TPtrC8& CCRRtspCommon::Content( void ) 
    {
    return iContent ; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindClientPorts
//
// method that finds client port numeric value
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::FindClientPorts( void )
    {
    iClientPort = KErrNotFound;

    // Bend concept of client port to mean also the interleaved channel for TCP streaming
    const TDesC8& portdes( ( iTransport == ERTPOverTCP )? KCRInterleaved():
                           ( iTransport == ERTPOverMulticast )? KCRPort():
                            KCRClient_Port() );

    TInt portNumberOffset( iRtspText->FindC( portdes ) ); 
    if ( portNumberOffset != KErrNotFound )
        {
        TPtrC8 portNumberStr( iRtspText->Mid( portNumberOffset + portdes.Length() ) );
        TInt endoffset = portNumberStr.FindC( KCRDash() );
        if ( endoffset >= 0 )
            {
            portNumberStr.Set( portNumberStr.Left( endoffset ) );
            }
        else
            {
            endoffset = portNumberStr.FindC( KCRSemiColon() );
            if ( endoffset >= 0 )
                {
                portNumberStr.Set( portNumberStr.Left( endoffset ) );
                }
            }
        TLex8 portNumberLex( portNumberStr );
        if ( portNumberLex.Val( iClientPort ) != KErrNone )
            { // something wrong? 
            LOG( "CCRRtspCommon::FindClientPorts val != KErrNone" );
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::ClientPort
//
// method that returns client port numeric value
// -----------------------------------------------------------------------------
//  
TInt CCRRtspCommon::ClientPort( void )
    {
    return iClientPort; 
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommon::SetClientPort
//
// method that sets client port numeric value
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetClientPort( TInt aPort )
    {
    iClientPort = aPort; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindTransport
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::FindTransport()
    {
    // User TCP streaming if 'RTP/AVP/TCP' found
    if ( iRtspText->FindC( KCRSDPRTPAVPTCP ) != KErrNotFound )
        {
        iTransport = ERTPOverTCP;
        }
    // multicast UDP if 'multicast' present
    else if ( iRtspText->FindC( KCRSDPMulticast ) != KErrNotFound ||
              iRtspText->FindC( KCRSDPRTPAVPUDP ) != KErrNotFound &&
              iRtspText->FindC( KCRDestination )  != KErrNotFound )
        {
        iTransport = ERTPOverMulticast;
        }
    // otherwise, plain unicast UDP
    else
        {
        iTransport = ERTPOverUDP;
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::Transport
// -----------------------------------------------------------------------------
//
TCRRTPTransport CCRRtspCommon::Transport()
    {
    return iTransport;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::SetTransport
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::SetTransport( TCRRTPTransport aTransport )
    {
    iTransport = aTransport; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::FindDestination
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::FindDestination()
    {
    // resets destination to KAFUnspec
    iDestination = TInetAddr();

    // search for destination
    TInt startoffset = iRtspText->FindC( KCRDestination() );
    if ( startoffset >= 0 )
        {
        startoffset += KCRDestination().Length();
        TPtrC8 destdes = iRtspText->Mid( startoffset );
        TInt endoffset = destdes.FindC( KCRSemiColon() );
        if ( endoffset >= 0 )
            {
            destdes.Set( destdes.Left(endoffset) );
            }

        TBuf<128> destbuf;
        destbuf.Copy( destdes.Left(128) );
        TInt err = iDestination.Input( destbuf );
        if ( err != KErrNone )
            {
            LOG1( "CCRRtspCommon::FindDestination(), INVALID destination address '%S'", &destbuf );
            }
        else
            {
            LOG1( "CCRRtspCommon::FindDestination(), multicast address '%S'", &destbuf );
            iDestination.ConvertToV4Mapped();
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::Destination
// -----------------------------------------------------------------------------
//
const TInetAddr& CCRRtspCommon::Destination()
    {
    return iDestination;
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::SetRange
//
// method that sets Range: header values
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetRange( TReal aLower , TReal aUpper ) 
    {
    iLowerRange = aLower; 
    iUpperRange = aUpper; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::GetRange
//
// method that gets Range: header values
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::GetRange( TReal& aLower , TReal& aUpper ) 
    {
    aLower = iLowerRange; 
    aUpper = iUpperRange; 
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::ParseRange
//
// method that tries to parse contents of possible Range: header 
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::ParseRange( void ) 
    {
    if ( iRtspText )
        {
        TInt rangeHeaderOffset( iRtspText->Des().FindC( KCRRangeHeader ) );
        if ( rangeHeaderOffset > 0 )
            {
            // check for live-stream specific range "Range: npt=now-"
            if ( iRtspText->Des().FindC( KCRRangeHeaderLiveStream ) > KErrNotFound )
                { // yes, this is a live stream
                iLowerRange = KRealZero; 
                iUpperRange = KRealMinusOne; 
                iIsLiveStream = ETrue; 
                LOG( "CCRRtspCommon::ParseRange(), Found livestream range" );
                }
            else
                {
                TPtrC8 rangeHeader = iRtspText->Des().Mid(
                                     rangeHeaderOffset + KCRRangeHeader().Length() ); 
                TInt minusSignOffset( rangeHeader.Locate( '-' ) ); 
                if ( minusSignOffset > 0 ) 
                    {
                    TPtrC8 startPosStr = rangeHeader.Left( minusSignOffset ); 
                    TLex8 startPosLex ( startPosStr ); 
                    startPosLex.Val ( iLowerRange ) ; // if .Val fails, value will just remain zero
                    LOG1( "CCRRtspCommon::ParseRange(), start: %f", iLowerRange );
                    }

                TInt lineFeedOffset( rangeHeader.Locate( '\n' ) );
                if ( lineFeedOffset > ( minusSignOffset + 2 ) )
                    {
                    TPtrC8 endPosStr = rangeHeader.Mid( minusSignOffset + 1,
                                       lineFeedOffset - ( minusSignOffset + 1 ) ); 
                    TLex8 endPosLex ( endPosStr ); 
                    endPosLex.Val ( iUpperRange ) ; // if .Val fails, value will just remain -1.0
                    LOG1( "CCRRtspCommon::ParseRange(), end: %f", iUpperRange );
                    }
                else
                    {
                    iUpperRange = KRealMinusOne; 
                    LOG( "CCRRtspCommon::ParseRange(), Setting end range to -1.0" );
                    }
                }
            }
        }   
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::AuthenticationTypeL
//
// method that returns authentication type
// -----------------------------------------------------------------------------
//  

TDesC8& CCRRtspCommon::AuthenticationTypeL( void )
    {
    if ( !iAuthType )
        {
        iAuthType = HBufC8::NewL( 0 );
        }
    
    return *iAuthType;
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::NonceL
//
// method that returns session nonce
// -----------------------------------------------------------------------------
//  
TDesC8& CCRRtspCommon::NonceL( void )
    {   
    if ( !iNonce )
        {           
        iNonce = HBufC8::NewL( 0 );
        }   
    
    return *iNonce;
    }
    
// -----------------------------------------------------------------------------
// CCRRTSPResponse::RealmL
//
// method that returns session realm
// -----------------------------------------------------------------------------
//  
TDesC8& CCRRtspCommon::RealmL( void )
    {   
    if ( !iRealm )
        {           
        iRealm = HBufC8::NewL( 0 );
        }
    
    return *iRealm;
    }
    
// -----------------------------------------------------------------------------
// CCRRTSPResponse::OpaqueL
//
// method that returns session opaque value
// -----------------------------------------------------------------------------
//  
TDesC8& CCRRtspCommon::OpaqueL( void )
    {
    if ( !iOpaque )
        {           
        iOpaque = HBufC8::NewL( 0 );
        }
    
    return *iOpaque;
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetNonce
//
// method that sets session nonce
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetNonceL( const TDesC& aNonce )
    {
    delete iNonce; iNonce = NULL;
    iNonce = HBufC8::NewL( aNonce.Length() ); 
    iNonce->Des().Copy( aNonce );   
    }
    
// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetNonce
//
// method that sets session nonce
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetNonceL( const TDesC8& aNonce )
    {
    delete iNonce; iNonce = NULL;
    iNonce = aNonce.AllocL();
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetAuthenticationTypeL
//
// method that sets authentication type
// -----------------------------------------------------------------------------
//
void CCRRtspCommon::SetAuthenticationTypeL( const TDesC8& aAuthType )
    {
    delete iAuthType;
    iAuthType = NULL;
    iAuthType = aAuthType.AllocL();
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetRealmL
//
// method that sets session realm
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetRealmL( const TDesC& aRealm )
    {
    delete iRealm; iRealm = NULL;
    iRealm = HBufC8::NewL( aRealm.Length()); 
    iRealm->Des().Copy( aRealm );   
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetRealmL
//
// method that sets session realm
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetRealmL( const TDesC8& aRealm )
    {
    delete iRealm; iRealm = NULL;
    iRealm = aRealm.AllocL();   
    }

// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetOpaqueL
//
// method that sets session opaque
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetOpaqueL( const TDesC& aOpaque )
    {
    delete iOpaque; iOpaque = NULL;
    iOpaque = HBufC8::NewL( aOpaque.Length()); 
    iOpaque->Des().Copy( aOpaque ); 
    }
    
// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetOpaqueL
//
// method that sets session opaque
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetOpaqueL( const TDesC8& aOpaque )
    {
    delete iOpaque; iOpaque = NULL;
    iOpaque = aOpaque.AllocL(); 
    }
    
// -----------------------------------------------------------------------------
// CCRRTSPResponse::SetUserNameL
//
// method that sets session user name
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetUserNameL( const TDesC& aUserName )
    {
    delete iUserName; iUserName = NULL;
    iUserName = HBufC8::NewL( aUserName.Length() ); 
    iUserName->Des().Copy( aUserName );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommon::SetPassWdL
//
// method that sets session password
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetPassWdL( const TDesC& aPassWd )
    {
    delete iPassword; iPassword = NULL;
    iPassword = HBufC8::NewL( aPassWd.Length() ); 
    iPassword->Des().Copy( aPassWd );
    }
    
// -----------------------------------------------------------------------------
// CCRRtspCommon::SetRtspUriL
//
// method that sets session uri
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::SetRtspUriL( const TDesC& aUri )
    {
    delete iUri; iUri = NULL;
    iUri = HBufC8::NewL( aUri.Length() ); 
    iUri->Des().Copy( aUri );
    }

// -----------------------------------------------------------------------------
// CCRRtspCommon::IsLiveStream
//
// getter method for livelihood of a stream
// -----------------------------------------------------------------------------
//  
TBool CCRRtspCommon::IsLiveStream( void )
    {
    return iIsLiveStream; 
    }
 
// -----------------------------------------------------------------------------
// CCRRtspCommon::FindContentBase
//
// -----------------------------------------------------------------------------
//  
void CCRRtspCommon::FindContentBase( void )
    {
    iContentBase.Set( NULL, 0 );
    TInt cbStringOffset( iRtspText->FindC( KCRContentBaseHeader() ) );
    if ( cbStringOffset > 0 )
        {
        TPtrC8 beginningFromCb( iRtspText->Right( iRtspText->Length() - 
                              ( cbStringOffset + KCRContentBaseHeader().Length() ) ) );
        TInt lfStringOffset( beginningFromCb.Find( KCRNewLine ) );
        if ( lfStringOffset != KErrNotFound )
            {
            LOG1( "### setting content-base, len = %d", lfStringOffset - 1 );
            iContentBase.Set( beginningFromCb.Mid( 0, lfStringOffset - 1 ) ); // -1 to cut trailing /
            }
        }
    }
// -----------------------------------------------------------------------------
// CCRRtspCommon::ContentBase
//
// -----------------------------------------------------------------------------
//  
TPtrC8 CCRRtspCommon::ContentBase( void )
    {
    return iContentBase;
    }

//  End of File

