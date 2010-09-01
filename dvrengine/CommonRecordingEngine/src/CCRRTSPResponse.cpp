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
* Description:    RTSP response parser and producer*
*/




// INCLUDE FILES
#include "CCRRtspResponse.h"
#include "CRRTSPCommon.h"
#include "videoserviceutilsLogger.h"

// CONSTANTS
const TInt KDVRMinRTSPResponseLen( 14 );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCRRtspResponse::CCRRtspResponse
// C++ default constructor can NOT contain any code, that might leave.
// -----------------------------------------------------------------------------
//
CCRRtspResponse::CCRRtspResponse()
  : iStatusCode( ERTSPRespContinue ),
    iServerPort( KErrNotFound ),
    iSSRC( NULL, 0 )
    {  
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCRRtspResponse* CCRRtspResponse::NewL()
    {
    CCRRtspResponse* self = new( ELeave ) CCRRtspResponse();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCRRtspResponse::ConstructL()
    {
    // None
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::~CCRRtspResponse
// Destructor.
// -----------------------------------------------------------------------------
//
CCRRtspResponse::~CCRRtspResponse()
    {
    LOG( "CCRRtspResponse::~CCRRtspResponse" );
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::TryParseL
// 
// -----------------------------------------------------------------------------
//
void CCRRtspResponse::TryParseL( const TDesC8 &aString ) 
    {
    if ( aString.Length() < KDVRMinRTSPResponseLen )
        {
        LOG( "CCRRtspResponse::TryParseL(), Length less than minimum, Leaved: KErrUnderflow" );
        User::Leave( KErrUnderflow ); 
        }
        
    // Copy the stuff into local variable:      
    delete iRtspText; iRtspText = NULL; 
    iRtspText = aString.AllocL(); 
    
    const TInt KReplyHeaderOffset( KCRRTSPReplyHeader().Length() );
    const TInt KStatusNumberLen( 5 );
    
    // Try to find out if end of the command has been received
    // "RTSP/1.0 XXX\r\n\r\n" at least..
    TInt replyEndOffSet = aString.FindC( KCR2NewLines() ); 
    if ( replyEndOffSet == KErrNotFound )
        {
        // Need to have more, do nothing yet.. except that some servers 
        // do not append 2 newlines to 404 or other error responses:
        if ( !aString.Left( KReplyHeaderOffset ).CompareC( KCRRTSPReplyHeader() ) )
            {
            TPtrC8 statusNumberString( 
                aString.Mid( KReplyHeaderOffset, KStatusNumberLen ) );
            TLex8 statusNumberLex( statusNumberString ); 
            TInt statusCodeInteger( KErrNotFound ); 
            User::LeaveIfError ( statusNumberLex.Val( statusCodeInteger ) ) ;
            iStatusCode = static_cast<TResponseCode>( statusCodeInteger ); 
            if ( iStatusCode != ERTSPRespOK &&
                 iStatusCode != ERTSPRespCreated &&
                 iStatusCode != ERTSPRespProxyAuthenticationRequired &&
                 iStatusCode != ERTSPRespUnauthorized )
                { 
                // Was something else than ok or unauthorized-401
                FindCSeqL();
                LOG1( "CCRRtspResponse::TryParseL() out, with rtsp error code: %d", iStatusCode );
                return; 
                }
            }
        
        LOG( "CCRRtspResponse::TryParseL() out, because response not complete" );
        User::Leave( KErrUnderflow ); 
        }
        
    LOG1( "CCRRtspResponse::TryParseL(), replyEndOffSet: %d", replyEndOffSet );

    // Find status code:    
    if ( iRtspText->Left( KReplyHeaderOffset ).CompareC( KCRRTSPReplyHeader() ) == 0 )
        {
        TPtrC8 statusNumberString( iRtspText->Mid(
            KReplyHeaderOffset, KStatusNumberLen ) );
        TLex8 statusNumberLex( statusNumberString ); 
        TInt statusCodeInteger( KErrNotFound ); 
        User::LeaveIfError ( statusNumberLex.Val( statusCodeInteger ) );
        iStatusCode = ( TResponseCode )( statusCodeInteger ); 
        }
    else
        {
        LOG( "CCRRtspResponse::TryParseL(), Statuscode integer not found !" );
        User::Leave( KErrNotSupported ); 
        }       
        
    LOG1( "CCRRtspResponse::TryParseL(), iStatusCode: %d", iStatusCode );

    FindContentL(); 
    // Then find CSeq
    FindCSeqL();    
    // Then find session id
    FindSessionIdL(); 
    // Then find server ports
    FindServerPorts();
    // Then find SSRC
    FindSSRC();  
    // Then try finding rtp-info header if it was play reply
    FindRTPInfoHeader();
    // Find possible transport method
    // IMPORTANT: should be done before parsing client port
    FindTransport();
    // Find possible client port
    FindClientPorts(); 
    if ( iServerPort == KErrNotFound && iClientPort != KErrNotFound )
        {
        LOG( "CCRRtspResponse::TryParseL(), using client port as server port -> UGLY!!" );
        iServerPort = iClientPort ; 
        }
    // Find destination address is existing
    FindDestination();
    // Try finding authentication
    FindRTPAuthenticationL();
    // Try to find range header
    ParseRange(); 
    // Try to find session-base header
    FindContentBase(); 
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::StatusCode
// 
// -----------------------------------------------------------------------------
//
CCRRtspResponse::TResponseCode CCRRtspResponse::StatusCode( void ) 
    {
    return iStatusCode;     
    }
    
// -----------------------------------------------------------------------------
// CCRRtspResponse::FindServerPorts
//
// method that finds server port numeric value
// -----------------------------------------------------------------------------
//  
void CCRRtspResponse::FindServerPorts( void )
    {
    iServerPort = KErrNotFound;                 
    TInt portNumberOffset( iRtspText->FindC( KCRServerPort ) ); 
    if ( portNumberOffset != KErrNotFound )
        {
        const TInt KDVRPortNumberMaxLen ( 5 ) ; 
        TPtrC8 portNumberStr( iRtspText->Mid( 
           portNumberOffset + KCRServerPort().Length() + 1, KDVRPortNumberMaxLen ) );
        TLex8 portNumberLex( portNumberStr );
        if ( portNumberLex.Val( iServerPort ) != KErrNone )
            { // something wrong? 
            LOG( "CCRRtspResponse::FindServerPorts(), portNumberLex.Val() != KErrNone" );
            iServerPort = KErrNotFound;             
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::ServerPort
//
// method that returns server port numeric value
// -----------------------------------------------------------------------------
//  
TInt CCRRtspResponse::ServerPort( void )
    {
    return iServerPort; 
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::FindSSRC
//
// -----------------------------------------------------------------------------
//
void CCRRtspResponse::FindSSRC( void ) 
    {
    LOG( "CCRRtspResponse::FindSSRC() in" );
    TInt SSRCOffset( KErrNotFound );
    iSSRC.Set( NULL, 0 );
        
    if ( ( SSRCOffset = iRtspText->FindC( KCRSSRC ) ) != KErrNotFound )
        {
        TPtrC8 SSRCStr( iRtspText->Right( 
                      ( iRtspText->Length() -  SSRCOffset ) - 6 ) );
        TInt SSRCLen( 0 ); 
        for ( TInt i( 0 ); i < SSRCStr.Length(); i++ )
            {
            if ( TChar( SSRCStr[i] ).IsAlphaDigit() ) 
                {
                SSRCLen++;
                }
            else
                {
                iSSRC.Set( SSRCStr.Mid( 0, SSRCLen ) );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCRRtspResponse::SSRC
//
// method that returns SSRC string
// -----------------------------------------------------------------------------
//  
TInt CCRRtspResponse::SSRC( TPtrC8& aSSRC ) 
    {
    if ( iSSRC.Ptr() != NULL )  
        {
        aSSRC.Set( iSSRC ); 
        return KErrNone;
        }
    else
        {
        return KErrNotFound; 
        }       
    }
    
// -----------------------------------------------------------------------------
// CCRRtspResponse::FindRTPInfoHeader
//
// method that partially parses rtp info header
// -----------------------------------------------------------------------------
//  
void CCRRtspResponse::FindRTPInfoHeader( void ) 
    {
    TInt offset = iRtspText->FindC( KCRRtpInfo ); // "Rtp-Info: "
    TInt url2OffSet( KErrNotFound );

    iRTPInfoHeader.iFirstURL.Set( NULL , 0 );
    iRTPInfoHeader.iFirstSeq = 0; 
    iRTPInfoHeader.iFirstTS = 0; 
    iRTPInfoHeader.iSecondURL.Set( NULL, 0 );
    iRTPInfoHeader.iSecondSeq = 0; 
    iRTPInfoHeader.iSecondTS = 0;   
    
    if ( offset > 0 ) 
        { 
        // Yes, there is header. That seems to be sucky to parse. We have delimiting 
        // characters ,; and nl and we may or may not find words url, seq and rtptime
        // and maybe something else. We  may be confident that there will be at max
        // 2 url  srings
        offset += KCRRtpInfo().Length(); // add the len of "RTP-Info: "
        TPtrC8 rtpInfoContent = iRtspText->Right( iRtspText->Length() - offset );
        TInt urlOffSet = rtpInfoContent.FindC( KCRUrlStr );
        
        for ( TInt i( 0 ); urlOffSet != KErrNotFound && i < 2; i ++ ) 
            { 
            // At least one url string found
            TPtrC8 urlContent( iRtspText->Right( iRtspText->Length() -
                             ( offset + urlOffSet + 4 ) ) );
            // Above string now contains rest of the PLAY commands RTSP OK response
            // so in practice there is 2 url-strings. If so, find the next one 
            // and cut our string 
            if ( ( url2OffSet = urlContent.FindC( KCRUrlStr ) ) > 0 )  
                {
                urlContent.Set( urlContent.Left( url2OffSet ) );
                }
            
            // Ok, now there is only one url string in urlContent. 
            // then just find seq and ts
            TInt seqOffSet = urlContent.FindC( KCRSeqStr ); 
            if ( seqOffSet != KErrNotFound ) 
                {
                TPtrC8 seqContent( urlContent.Right( urlContent.Length() -
                                 ( seqOffSet + KCRSeqStr().Length() ) ) );
                TLex8 seqLex( seqContent ); 
                if ( seqLex.Val( ( iRTPInfoHeader.iFirstURL.Length() == 0 )? 
                    iRTPInfoHeader.iFirstSeq : iRTPInfoHeader.iSecondSeq,
                    EDecimal ) == KErrNone )
                    {
                    TInt tsOffSet( urlContent.FindC( KCRRtptimeStr ) );
                    if ( tsOffSet != KErrNotFound ) 
                        {
                        TPtrC8 tsContent( urlContent.Right( urlContent.Length() -
                                        ( tsOffSet + KCRRtptimeStr().Length() ) ) );
                        TLex8 tsLex( tsContent ); 
                        tsLex.Val( ( iRTPInfoHeader.iFirstURL.Length() == 0 )? 
                                     iRTPInfoHeader.iFirstTS: 
                                     iRTPInfoHeader.iSecondTS, EDecimal );
                        }
                    }
                else
                    {
                    urlContent.Set ( NULL , 0 ) ;
                    }                   
                }    
            else
                {
                urlContent.Set ( NULL , 0 ) ;
                }           
            
            if ( urlContent.Length() > 0 ) 
                {
                TInt semicolonOffSet( urlContent.Locate(';') );  
                const TInt KDVRMinSemicolonOffset ( 5 ) ;
                if ( iRTPInfoHeader.iFirstURL.Length() == 0 && semicolonOffSet > KDVRMinSemicolonOffset )
                    {
                    iRTPInfoHeader.iFirstURL.Set(
                        urlContent.Mid( 0, semicolonOffSet ) ); 
                    }
                else
                    {
                    const TInt KDVRURLBeginOffset ( 4 ) ; 
                    iRTPInfoHeader.iSecondURL.Set( 
                        urlContent.Mid( KDVRURLBeginOffset,
                        semicolonOffSet - KDVRURLBeginOffset ) ); 
                    }
                }
            
            // Then continue with next url
            urlOffSet = url2OffSet;
            } 
        }   
    }   

// -----------------------------------------------------------------------------
// CCRRtspResponse::RTPInfoHeader
//
// method that returns rtp-info header content
// -----------------------------------------------------------------------------
//  
TInt CCRRtspResponse::RTPInfoHeader(
    CCRRtspResponse::SRTPInfoHeader &aRTPInfoHeader ) 
    {
    if ( iRTPInfoHeader.iFirstURL.Length() == 0  )
        {
        return KErrNotFound;
        }
    else
        {
        aRTPInfoHeader.iFirstURL.Set( iRTPInfoHeader.iFirstURL ); 
        aRTPInfoHeader.iFirstSeq = iRTPInfoHeader.iFirstSeq; 
        aRTPInfoHeader.iFirstTS = iRTPInfoHeader.iFirstTS; 
        aRTPInfoHeader.iSecondURL.Set( iRTPInfoHeader.iSecondURL ); 
        aRTPInfoHeader.iSecondSeq = iRTPInfoHeader.iSecondSeq; 
        aRTPInfoHeader.iSecondTS = iRTPInfoHeader.iSecondTS;
                
        return KErrNone; 
        }
    }
    
// -----------------------------------------------------------------------------
// CCRRtspResponse::FindRTPAuthenticationL
//
// method that partially parses rtp authentication header
// -----------------------------------------------------------------------------
//  
void CCRRtspResponse::FindRTPAuthenticationL( void ) 
    {
    TInt endPos( KErrNotFound );
    
    // First look for authorization method(basic / digest)
    TInt pos = iRtspText->FindC( KCRAuthDigest );
    
    // digest
    if ( pos != KErrNotFound ) 
        {       
        // Digest found, we can continue
        LOG( "CCRRtspResponse::FindRTPAuthenticationL() Digest found" );
        
        delete iAuthType;
        iAuthType = NULL;
        iAuthType = HBufC8::NewL( KCRAuthDigest().Length() );
        iAuthType->Des().Copy( KCRAuthDigest );
            
        // find "realm"
        pos = iRtspText->FindC( KCRAuthRealm );
        if ( pos != KErrNotFound ) 
            {
            LOG( "CCRRtspResponse::FindRTPAuthenticationL() realm found" );
            pos = pos + KCRAuthRealm().Length(); // realm
            endPos = iRtspText->Mid( pos ).LocateF( '"' );
            
            if ( endPos != KErrNotFound ) 
                {
                TPtrC8 data = iRtspText->Mid( pos ).Left( endPos );
                delete iRealm;
                iRealm = NULL;
                iRealm = HBufC8::NewL( data.Length() );
                iRealm->Des().Copy( data ); 
                }
            }
        
        // Find "nonce"
        pos = iRtspText->FindC( KCRAuthNonce() );
        if ( pos != KErrNotFound ) 
            {
            LOG( "CCRRtspResponse::FindRTPAuthenticationL() nonce found" );
            pos = pos + KCRAuthNonce().Length(); // nonce
            endPos = iRtspText->Mid( pos ).LocateF( '"' );
            if ( endPos != KErrNotFound ) 
                {
                TPtrC8 nonceData = iRtspText->Mid( pos ).Left( endPos );
                delete iNonce;
                iNonce = NULL;
                iNonce = HBufC8::NewL( nonceData.Length() );
                iNonce->Des().Copy( nonceData );
                }
            }
        
        // Find "opaque"(it seems that Darwin streaming server does not send this one)
        pos = iRtspText->FindC( KCRAuthOpaque() );
        if ( pos != KErrNotFound )
            {
            LOG( "CCRRtspResponse::FindRTPAuthenticationL() opaque found" );
            pos = pos + KCRAuthOpaque().Length(); // opaque
            endPos = iRtspText->Mid( pos ).LocateF( '"' );
            if ( endPos != KErrNotFound ) 
                {
                TPtrC8 opaqData = iRtspText->Mid( pos ).Left( endPos );
                delete iOpaque;
                iOpaque = NULL;
                iOpaque = HBufC8::NewL( opaqData.Length() );
                iOpaque->Des().Copy( opaqData );
                }
            }
        }
    
    // basic
    else
        {
        pos = iRtspText->FindC( KCRAuthBasic );
        
        if ( pos != KErrNotFound ) 
            {
            LOG( "CCRRtspResponse::FindRTPAuthenticationL() Basic found" );
            
            delete iAuthType;
            iAuthType = NULL;
            iAuthType = HBufC8::NewL( KCRAuthBasic().Length() );
            iAuthType->Des().Copy( KCRAuthBasic );
                      
            // find "realm"
            pos = iRtspText->FindC( KCRAuthRealm );
            
            if ( pos != KErrNotFound ) 
                {
                LOG( "CCRRtspResponse::FindRTPAuthenticationL() realm found" );
                pos = pos + KCRAuthRealm().Length(); // realm
                endPos = iRtspText->Mid( pos ).LocateF( '"' );
                        
                if ( endPos != KErrNotFound ) 
                    {
                    TPtrC8 data = iRtspText->Mid( pos ).Left( endPos );
                    delete iRealm;
                    iRealm = NULL;
                    iRealm = HBufC8::NewL( data.Length() );
                    iRealm->Des().Copy( data ); 
                    }
                }
            }
        }
    }

//  End of File
