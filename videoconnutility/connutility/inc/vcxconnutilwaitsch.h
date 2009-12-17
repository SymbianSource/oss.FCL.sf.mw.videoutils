/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:    CCVcxConnUtilWaitSch class declaration file*
*/




#ifndef __CVCXCONNUTILWAITSCH_H
#define __CVCXCONNUTILWAITSCH_H

// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <e32hashtab.h>


class CVcxConnUtilWait;
class CActiveSchedulerWait;

// CLASS DECLARATION

/**
 * CVcxConnUtilWaitSch is used to manage CActiveSchedulerWait objects
 * within videoconnutility.
 * Basically, when client calls wait, a new CActiveSchedulerWait is created 
 * and being put to list. Different types for waits are managed by different
 * wait -wrapper object, which type is defined as a private class for this class.
 * 
 * Currently videoconnutility puts active objects to wait in following situations:
 * 
 * (1) When there is connection creation ongoing and other active objects try to 
 * create connection: only the one active object that is creating connection is runnig, 
 * other a-objects waits as long as connection is ready
 * 
 * (2) Master waiting for roaming status from slave instances: Master's roaming active object is 
 * waiting for roaming response status change.
 * 
 * (3) Connection creation during roaming: All active objects requesting connection during roaming
 * are put to wait for the roaming to finish, so that correct iap id can be returned.
 * 
 * (4) Disconnecting during roaming. Master's disconnecting a-object(s) is put to wait for
 * roaming to finish
 * 
 * Waiting is released by the client by calling EndWait. CVcxConnUtilWaitSch basicallly 
 * stops wait for every wait object, put those whose type is not the one provided 
 * by the parameter of EndWait are being put back to wait. This solution is needed, to 
 * prevent deadlocks: we might have different kind of waits at the same time, but not all
 * are to be released at the same time, but because of the nature of the ActiveScheduler,
 * all wait objects are needed to release once, before anyone can proceed.
 * 
 * 
 */
NONSHARABLE_CLASS( CVcxConnUtilWaitSch ) : public CBase
    {
    public: // Constructors and destructor

        /**
         * Destructor.
         */
        ~CVcxConnUtilWaitSch();
    
        /**
         * Two-phased constructor.
         */
        static CVcxConnUtilWaitSch* NewL();
    
        /**
         * Two-phased constructor.
         */
        static CVcxConnUtilWaitSch* NewLC();

    private:

        /**
         * Constructor for performing 1st stage construction
         */
        CVcxConnUtilWaitSch();
    
        /**
         * EPOC default constructor for performing 2nd stage construction
         */
        void ConstructL();
    
    public:
  
        /**
         * Method creates a new CVcxConnUtilWait object in case
         * one with provided type does not already exist, then
         * creates a CActiveSchedulerWait object for the current
         * active object and puts it to wait by calling 
         * CActiveSchedulerWait::Start. After call returns (wait has 
         * released) checks if CVcxConnUtilWait's iCanStop is ETrue.
         * In case it is not, the object is being put back to wait. 
         * 
         * After wait object is really released, it is being deallocated. 
         * 
         * Method leaves with systemwide error code, in case object 
         * creation or saving to array fails.
         * 
         * @param aWaitId wait type id for this wait
         */
        void WaitL( TUint32 aWaitId );
        
        
        /**
         * Method loops all CVcxConnUtilWait objects and sets their
         * iCanStop to ETrue, in case aWaitId is same as CVcxConnUtilWait's
         * iType. 
         * All CActiveSchedulerWait's for each CVcxConnUtilWait is being 
         * released by calling CActiveSchedulerWait::AsyncStop.
         * 
         * @param aWaitId wait type id for this wait
         */
        void EndWait( TUint32 aWaitId );
        
    
    private:
                         
        /**
         * Internal helper method for getting corresponding 
         * CVcxConnUtilWait object based in the aID. 
         * 
         * In case aID -type object is not found from the 
         * iWaits -array, it is being created and appended to array.
         * 
         * @param aID wait type id for this wait
         */
        CVcxConnUtilWait* GetWaitL( TUint32 aID );
        
        /**
         * Internal helper method for PrepareWaitObjectL for 
         * getting next not started Active scheduler wait 
         * object for given CVcxConnUtilWait.
         * 
         * If there is no existing not started object or given
         * CVcxConnUtilWait is NULL a new  CActiveSchedulerWait
         * object is created and appended to the aWait object's
         * waitarray.
         * 
         * 
         * @param aWait wait wait object from where to search
         */
        CActiveSchedulerWait* GetActiveSWaitL( CVcxConnUtilWait* aWait );
        
    private:       
        /**
         * Array containing all CVcxConnUtilWait objects maintained
         * by this object  
         */
        RPointerArray< CVcxConnUtilWait > iWaits;

    };

#endif // __CVCXCONNUTILWAITSCH_H
