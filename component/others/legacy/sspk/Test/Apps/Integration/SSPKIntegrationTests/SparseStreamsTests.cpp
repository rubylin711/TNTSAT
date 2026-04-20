///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PKTestSuite.h"
#include "PKTestSuiteUtils.h"
#include "STWrapper.h"
#include "ISmoothTransport.h"
#include "ManifestReadyCallbackImpl.h"
#include "StringUtils.h"
#include "SSPKHelpers.h"
#include <sstream>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( SparseStreamsTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;

    SparseStreamsTests()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "IntegrationTestSources" );
        if( !xTestData.IsNull() )
        {
             _xManifests = xTestData.Elements(L"source");
        }
    }

    bool TestSetup()
    {
        PKTEST_ASSERT_MSG_EXIT(_xManifests.Length() > 0,"Couldn't find any elements with name 'source'");

        _smoothObject = NEW_NO_THROW STWrapper();
        PKTEST_ASSERT_EXIT(NULL != _smoothObject);
        return true;

    exit:
        return false;
    }

    bool TestCleanup()
    {
        pkRESULT pkResult = pkS_OK;

        _smoothObject->SetManifestCallback(NULL);
        if(!_smoothObject->IsClosed() )
        {
            pkResult = _smoothObject->Close();
        }
        delete _smoothObject;
        return SUCCEEDED(pkResult);
    }

    /////////////////// Sparse Stream Tests ///////////////////////
    ///////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_GetFirstChunkatZero,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse chunk at the zero ");
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime == 0, "We are expecting a sparse chunk at the zero ");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_ReverseRange,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator( 5000, 2000 );
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(FAILED( hr ), "The TryGetChunkInfo should fail when the range is reversed in the iterator ");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_OutOfRange,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator( MAX_TIME64 - 2, MAX_TIME64 );
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS , "The TryGetChunkInfo should fail when the iterator is out of range ");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_GetFirstChunkOutsideIteratorRange,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkNotAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator(0,0);
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS , "The TryGetChunkInfo should fail when the iterator is out of range ");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_MoveNextOrPrevWithoutResolve,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkNotAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        PKTEST_HRESULT_EXIT(itChunk.MoveNext());
        PKTEST_HRESULT_EXIT(itChunk.MoveNext());
        PKTEST_HRESULT_EXIT(itChunk.MovePrev());

        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        PKTEST_HRESULT_EXIT(itChunk.MoveNext());
        PKTEST_HRESULT_EXIT(itChunk.MoveNext());
        PKTEST_HRESULT_EXIT(itChunk.MovePrev());


        itChunk = sparseStreams[0]->GetIterator(0, MAX_TIME64);
        hr = itChunk.MoveNext();
        PKTEST_ASSERT_MSG_EXIT( hr == pkE_INVALID_REQUEST , "Iterator should not be able to move next, without resolving current.");
        hr = itChunk.MovePrev();
        PKTEST_ASSERT_MSG_EXIT( hr == pkE_INVALID_REQUEST , "Iterator should not be able to move prev, without resolving current.");

exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_NonSelectedStreams,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        ChunkIterator itChunk;
        bool nonSelectedTextFound = false;

        VectorOfStreams selectedStreams;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveWithTwoSparseStreamsInSameHeader");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllAvailableSparseStreams();
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams available.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), sparseStreams[i]);
            if ( it == selectedStreams.end() )
            {
                nonSelectedTextFound = true;
                itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
                hr = sparseStreams[i]->GetChunkInfoAsync( itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT( hr == pkE_INVALID_REQUEST, "Should fail to get sparse chunk info for non-selected streams" );
            }
        }
        PKTEST_ASSERT_MSG_EXIT(nonSelectedTextFound, "No non selected text streams found" );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_GetLastChunkInsideIteratorRange,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse last Chunk inside the range ");
        // the check below uses a hardcoded timestamp to compare the lastchunk timestamp. when source changes, this value should also change.
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime == 650000000, "The chunk timestamp should be 650000000 but observed : %d ", chunkInfo.chunkTime);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_GetLastChunkInsideIteratorRangeUsingGetIteratorVOD,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator( 0, MAX_TIME64 );
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse last Chunk inside the range ");
        // the check below uses a hardcoded timestamp to compare the lastchunk timestamp. when source changes, this value should also change.
        PKTEST_ASSERT_MSG_EXIT(chunkInfo.chunkTime == 650000000, "The chunk timestamp should be 650000000 but observed : %d ", chunkInfo.chunkTime);

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_WalkThroughTheListAscendingOD,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = -1;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            currentTime = -1;
            itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                LogTestComment( "  Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                // Chunks must progress in time as the iterator moves to the next chunk
                PKTEST_ASSERT_EXIT( chunkInfo.chunkTime > currentTime );

                currentTime = chunkInfo.chunkTime;
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS, "For OnDemand content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at end of stream side." );
        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_WalkThroughTheListAscendingLive,
        PKTEST_PROPERTY("Data:LiveSparseStreamDefault", "LiveSparseStreamDefault")
        PKTEST_PROPERTY("Data:LiveSparseStreamInPragmaHeader", "LiveSparseStreamInPragmaHeader")
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = -1;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData());
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            currentTime = -1;
            itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                LogTestComment( "  Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                // Chunks must progress in time as the iterator moves to the next chunk
                PKTEST_ASSERT_EXIT( chunkInfo.chunkTime > currentTime );

                currentTime = chunkInfo.chunkTime;
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks on the live side." );
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_LiveToVod_ChunksBeforeWindow,
        PKTEST_PROPERTY( "Priority", "1" )
        PKTEST_PROPERTY( "Bug", "31762" )
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = MAX_TIME64;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveToVodMultiSegments" );
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            //Get first chunk and record time
            itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
            PKTEST_HRESULT_EXIT( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) );
            currentTime = chunkInfo.chunkTime;
            LogTestComment( "Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );

            //Move to previous chunk 
            PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MovePrev() ) );
            hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo );
            LogTestComment( "Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );

            //Make sure the correct pkResult is returned from TryGetChunkInfo and that the time is correct
            PKTEST_ASSERT_EXIT( chunkInfo.chunkTime < currentTime );
            PKTEST_ASSERT_MSG_EXIT( SUCCEEDED(hr) , "For LiveToVod content, we should see no error getting the chunk before the first in the chunk list" );
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_WalkThroughTheListDescendingOD,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = MAX_TIME64;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamWithFirstChunkAtZero");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            currentTime = MAX_TIME64;
            itChunk = sparseStreams[i]->GetLastInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                LogTestComment( "Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MovePrev() ) );
                // Chunks must progress in time as the iterator moves to the next chunk
                PKTEST_ASSERT_EXIT( chunkInfo.chunkTime < currentTime );

                currentTime = chunkInfo.chunkTime;
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS, "For OnDemand content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at the left side of the DVR window." );
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        SparseStreams_TryGetChunkInfo_WalkThroughTheListDescendingLive,
        PKTEST_PROPERTY( "Priority", "1" )
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = MAX_TIME64;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            currentTime = MAX_TIME64;
            itChunk = sparseStreams[i]->GetLastInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                LogTestComment( "  Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MovePrev() ) );
                // Chunks must progress in time as the iterator moves to the next chunk
                PKTEST_ASSERT_EXIT( chunkInfo.chunkTime < currentTime );

                currentTime = chunkInfo.chunkTime;
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_BEFORE_VALID_RANGE, "For Live content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at the left side of the DVR window." );
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_TryGetChunkInfo_StreamParentedToTextStream,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t currentTime = -1;

        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamParentedWithTextStream");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_FUNC_EXIT(proposedSelection = SSPKHelpers::AddTextStream(availableStreams, selectedStreams, false));
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->SelectStreamsAsync(_smoothObject, proposedSelection) );
        PKTEST_FUNC_EXIT(_smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            currentTime = -1;
            itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                LogTestComment( "  Stream '%ls' chunkTime %lld ", sparseStreams[i]->Name().c_str(), chunkInfo.chunkTime );
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                // Chunks must progress in time as the iterator moves to the next chunk
                PKTEST_ASSERT_EXIT( chunkInfo.chunkTime > currentTime );

                currentTime = chunkInfo.chunkTime;
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS, "For OnDemand content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at end of stream side." );

        }

    exit:
        return;
    }

    ////////////////// Get ChunkInfo Async tests /////////////////
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_GetChunkInfoAsync_Live,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT chunkInfoResult, chunkInfoCallBackResult;
        CHUNK_INFO chunkInfo;
        ChunkIterator itChunk;
        int64_t chunkTimestamp;
        int32_t expectedChunkIntervalTimeOut = 0, firstLiveSparseChunkTimeOut = 0;
        int64_t lastChunkTimestamp = 0;
        int32_t numberOfLiveChunksforTesting = 2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

                chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );

                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );

                if(0 == lastChunkTimestamp )
                {
                    lastChunkTimestamp = chunkTimestamp;
                }
                else if(0 == expectedChunkIntervalTimeOut)
                {
                    expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp) )).Ticks() * 2; 
                }

                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for already available sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );

            firstLiveSparseChunkTimeOut = expectedChunkIntervalTimeOut + (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(LIVE_BACKOFF_TIME)).Ticks() + VALIDATION_DELAY ;//Allow more time for the first one that was not in the manifest

            for( int32_t j =0; j<numberOfLiveChunksforTesting; j++ )
            {
                chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

                if(j == 0)
                {
                    chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(firstLiveSparseChunkTimeOut, chunkTimestamp );
                }
                else
                {
                    chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(expectedChunkIntervalTimeOut, chunkTimestamp );
                }

                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() < chunkTimestamp, "We should be getting the text chunk ahead of playback time.");

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_GetChunkInfoAsync_OutOfRange,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT chunkInfoResult, chunkInfoCallBackResult;
        ChunkIterator itChunk;
        int64_t chunkTimestamp;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator( MAX_TIME64 - 2, MAX_TIME64 );

        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( chunkInfoCallBackResult == pkE_TIMEOUT, "Should not receive the callback with in timeout." );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX( SparseStreams_GetChunkInfoAsync_MultipleAsyncCalls,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr, chunkInfoResult, chunkInfoCallBackResult;
        ChunkIterator itChunk;
        CHUNK_INFO chunkInfo;
        int64_t chunkTimestamp;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( hr ), "We are expecting a sparse last Chunk inside the range ");
        PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );

        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        // while waiting for the last chunk, make another chunk download call.
        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        PKTEST_ASSERT_MSG_EXIT( FAILED( chunkInfoResult ), "We should not be allowing multiple GetChunkInfo async calls at the same time." );

        // this wait is for the first GetChunkInfoAsync operation.
        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT + (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(LIVE_BACKOFF_TIME)).Ticks(), chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_GetChunkInfoAsync_AbortAsyncCall,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr, chunkInfoResult, chunkInfoCallBackResult, chunkAbortResult;
        ChunkIterator itChunk;
        int64_t chunkTimestamp;
        CHUNK_INFO chunkInfo;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseNotFrequentChunks");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );

        chunkInfoResult = sparseStreams[0]->GetChunkInfoAsync(itChunk, _smoothObject );
        chunkAbortResult = sparseStreams[0]->AbortChunkInfoAsync();
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED(chunkAbortResult), "AbortChunkInfoAsync should not fail." );

        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
        PKTEST_ASSERT_MSG_EXIT( chunkInfoCallBackResult == E_ABORT, "Should get a abort callback from Async call." );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_GetChunkInfoAsync_ForAllAvailableSparseStreams,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT chunkInfoResult, chunkInfoCallBackResult;
        CHUNK_INFO chunkInfo;
        int64_t chunkTimestamp;
        VectorOfStreams selectedStreams;
        int32_t expectedChunkIntervalTimeOut = 0, firstLiveSparseChunkTimeOut = 0;
        int64_t lastChunkTimestamp = 0;

        // number of live chunksinfo's for the testing.
        int32_t numberOfLiveChunksforTesting = 2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveWithTwoSparseStreamsInSameHeader");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllAvailableSparseStreams();
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams available.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), sparseStreams[i]);
            if ( it != selectedStreams.end() )
            {
                ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

                while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
                {
                    chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                    PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

                    chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
                    PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "The GetChunkInfoAsync callback should not fail for already available sparse chunks" );
                    if(0 == lastChunkTimestamp )
                    {
                        lastChunkTimestamp = chunkTimestamp;
                    }
                    else if(0 == expectedChunkIntervalTimeOut)
                    {
                        expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp))).Ticks() * 2; 
                    }

                    PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                }

                PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );

                firstLiveSparseChunkTimeOut = expectedChunkIntervalTimeOut + (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(LIVE_BACKOFF_TIME)).Ticks() + VALIDATION_DELAY ;//Allow more time for the first one that was not in the manifest
                for( int32_t j =0; j<numberOfLiveChunksforTesting; j++ )
                {
                    chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                    PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail for live sparse chunks");

                    if(j == 0)
                    {
                        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(firstLiveSparseChunkTimeOut, chunkTimestamp );
                    }
                    else
                    {
                        chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(expectedChunkIntervalTimeOut, chunkTimestamp );
                    }

                    PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );
                    PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
                }
            }
            else
            {
                ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
                hr = sparseStreams[i]->GetChunkInfoAsync( itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT( hr == pkE_INVALID_REQUEST, "Should fail to get sparse chunk info for non-selected streams" );
            }
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_GetChunkInfoAsync_DVR,
        PKTEST_PROPERTY("Priority", "0")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT chunkInfoResult, chunkInfoCallBackResult;
        CHUNK_INFO chunkInfo;
        int64_t chunkTimestamp;
        int32_t expectedChunkIntervalTimeOut = 0, firstLiveSparseChunkTimeOut = 0;
        int64_t lastChunkTimestamp = 0;
        // number of live chunksinfo's for the testing.
        int32_t numberOfLiveChunksforTesting = 2;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);
        // bringing the playback from live to DVR.
        PKTEST_HRESULT_EXIT( _smoothObject->Rewind() );
        _smoothObject->Delay(VALIDATION_DELAY);
        PKTEST_HRESULT_EXIT( _smoothObject->Play());
        _smoothObject->Delay(VALIDATION_DELAY);


        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

                chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, chunkTimestamp );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "WaitForFragmentInfoAsync should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );
                if(0 == lastChunkTimestamp )
                {
                    lastChunkTimestamp = chunkTimestamp;
                }
                else if(0 == expectedChunkIntervalTimeOut)
                {
                    expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp))).Ticks() * 2;
                }

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );

            firstLiveSparseChunkTimeOut = expectedChunkIntervalTimeOut + (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks(LIVE_BACKOFF_TIME)).Ticks() + VALIDATION_DELAY ;//Allow more time for the first one that was not in the manifest
            for( int32_t j =0; j<numberOfLiveChunksforTesting; j++ )
            {
                chunkInfoResult = sparseStreams[i]->GetChunkInfoAsync(itChunk, _smoothObject );
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( chunkInfoResult ), "The GetChunkInfoAsync call should not fail ");

                if(j == 0)
                {
                    chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(firstLiveSparseChunkTimeOut, chunkTimestamp );
                }
                else
                {
                    chunkInfoCallBackResult = _smoothObject->WaitForFragmentInfoAsync(expectedChunkIntervalTimeOut, chunkTimestamp );
                }

                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( chunkInfoCallBackResult ), "FragmentInfo download should not fail for live sparse chunks.  Return value is 0x%X", chunkInfoCallBackResult );

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }
        }

    exit:
        return;
    }

    /////////////// Download Fragment Async tests ////////////////
    //////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_ManifestOutputTrueOD,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT downloadResult, downloadCallBackResult;
        CHUNK_INFO chunkInfo;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamManifestOutputTrue");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
            sparseStreams[i]->GetSelectedTracks(&selectedTracks);

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS, "For OnDemand content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at end of stream side." );
        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_ManifestOutputFalseOD,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT downloadResult, downloadCallBackResult;
        CHUNK_INFO chunkInfo;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamManifestOutputFalse");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
            sparseStreams[i]->GetSelectedTracks(&selectedTracks);

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_NO_MORE_ITEMS, "For OnDemand content, we should see pkE_NO_MORE_ITEMS, if there are no more chunks at end of stream side." );
        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_ManifestOutputTrueLive,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT downloadResult, downloadCallBackResult;
        CHUNK_INFO chunkInfo;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        int32_t expectedChunkIntervalTimeOut = 0;
        int64_t lastChunkTimestamp = 0;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveSparseStreamManifestOutputTrue");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
            sparseStreams[i]->GetSelectedTracks(&selectedTracks);

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

                if(0 == lastChunkTimestamp )
                {
                    lastChunkTimestamp = chunkTimestamp;
                }
                else if(0 == expectedChunkIntervalTimeOut)
                {
                    expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp) )).Ticks() * 2; 
                }

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );

            // number of live chunks downloading for the testing.
            int32_t numberOfLiveChunksforTesting = 2;
            for( int32_t j =0; j<numberOfLiveChunksforTesting; j++ )
            {
                downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( expectedChunkIntervalTimeOut + SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize ); //* 2 to allow time for the first one that was not in the manifest
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() < chunkTimestamp, "We should be getting the text chunk ahead of playback time.");

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_DeselectStreamDuringDownloadAsync,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT downloadResult, downloadCallBackResult;
        CHUNK_INFO chunkInfo;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        int32_t expectedChunkIntervalTimeOut = 0;
        int64_t lastChunkTimestamp = 0;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, L"LiveSparseStreamManifestOutputTrue");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetLastInCurrentChunkList();
            sparseStreams[i]->GetSelectedTracks(&selectedTracks);

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

                if(0 == lastChunkTimestamp )
                {
                    lastChunkTimestamp = chunkTimestamp;
                }
                else if(0 == expectedChunkIntervalTimeOut)
                {
                    expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp) )).Ticks() * 2; 
                }

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );
            
            downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
            PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

            downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
            
            PKTEST_HRESULT_EXIT(itChunk.MoveNext());
            downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
            PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

            PKTEST_FUNC_EXIT( DeSelectAllSparseStream());

            downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( expectedChunkIntervalTimeOut + SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize ); //* 2 to allow time for the first one that was not in the manifest
            PKTEST_ASSERT_MSG_EXIT( pkE_INVALID_REQUEST == downloadCallBackResult , "The Downloadfragment callback should fail, when the stream is deselected." );

        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_ManifestOutputFalseLive,
        PKTEST_PROPERTY("Data:LiveSparseStreamManifestOutputFalse", "LiveSparseStreamManifestOutputFalse")
        PKTEST_PROPERTY("Data:LiveSparseStreamInPragmaHeader", "LiveSparseStreamInPragmaHeader")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        HRESULT downloadResult, downloadCallBackResult;
        CHUNK_INFO chunkInfo;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        int32_t expectedChunkIntervalTimeOut = 0;
        int64_t lastChunkTimestamp = 0;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, PKTest_GetTestData() );
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            ChunkIterator itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
            sparseStreams[i]->GetSelectedTracks(&selectedTracks);

            while( SUCCEEDED( hr = sparseStreams[i]->TryGetChunkInfo( itChunk, &chunkInfo ) ) )
            {
                downloadResult = sparseStreams[i]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

                if(0 == lastChunkTimestamp )
                {
                    lastChunkTimestamp = chunkTimestamp;
                }
                else if(0 == expectedChunkIntervalTimeOut)
                {
                    expectedChunkIntervalTimeOut = (int32_t)TimeSpan_ms::ConvertFrom(TimeSpan_hns::FromTicks((chunkTimestamp - lastChunkTimestamp))).Ticks() * 2; 
                }

                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

            PKTEST_ASSERT_MSG_EXIT( hr == pkE_PENDING, "For Live content, we should see pkE_PENDING, if there are no more chunks at end of stream side." );

            // number of live chunks downloading for the testing.
            int32_t numberOfLiveChunksforTesting = 2;
            for( int32_t j =0; j<numberOfLiveChunksforTesting; j++ )
            {
                downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
                PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

                downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( expectedChunkIntervalTimeOut + SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );//* 2 to allow time for the first one that was not in the manifest
                PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
                PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
                PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");
                PKTEST_ASSERT_MSG_EXIT(_smoothObject->GetCurrentPlayBackTime() < chunkTimestamp, "We should be getting the text chunk ahead of playback time.");
                PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );
            }

        }
    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_MultipleAsyncCalls,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr, downloadResult, downloadCallBackResult;
        ChunkIterator itChunk;
        bool isChunkComplete;
        CHUNK_INFO chunkInfo;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, true));
        _smoothObject->Delay(VALIDATION_DELAY);

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        //get the last chunk.
        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        // Iterator can't go forward, unless the current chunk is resolved. so doing a tryGetChunkInfo to resolve it.
        hr = sparseStreams[0]->TryGetChunkInfo( itChunk, &chunkInfo );
        PKTEST_ASSERT_EXIT( SUCCEEDED( itChunk.MoveNext() ) );

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);

        // while waiting for the last chunk, make another chunk download call.
        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT( FAILED( downloadResult ), "We should not be allowing multiple Download Chunk async calls at the same time." );

        //This wait is for the first downloadFragmentAsync operation.
        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( LIVE_SPARSE_STREAM_DEFAULT_TIMEOUT + SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );
        PKTEST_ASSERT_MSG_EXIT( isChunkComplete, "We are expecting to get the complete chunk, but did not.");
        PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize == totalChunkSize, "We are expecting the downloaded size is equal to the total chunk size" );
        PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= DEFAULT_CHUNK_DOWNLOAD_LIMIT, "The downloaded buffer size is greater than the download limit provided");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        SparseStreams_DownloadFragmentAsync_ChunkLargerThanTheDownloadLimit,
        PKTEST_PROPERTY( "Bug", "27387" )
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT downloadResult, downloadCallBackResult;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        ChunkIterator itChunk;
        int32_t smallChunkDownloadLimit = 10;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;

        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamManifestOutputTrue");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], smallChunkDownloadLimit, _smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( LIVE_SPARSE_STREAM_DEFAULT_TIMEOUT + SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( SUCCEEDED( downloadCallBackResult ), "The Downloadfragment callback should not fail" );

        PKTEST_ASSERT_MSG_EXIT(!isChunkComplete, "When the chunk is bigger than the downloadlimit, fFinalBuffer should be false.");
        PKTEST_ASSERT_MSG_EXIT( downloadedBufferSize <= smallChunkDownloadLimit, "The downloaded buffer size is greater than the download limit provided");

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_OutOfRange,
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT downloadResult, downloadCallBackResult;
        bool isChunkComplete;
        ChunkIterator itChunk;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetIterator( MAX_TIME64 - 2, MAX_TIME64 );
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp,totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( downloadCallBackResult == pkE_TIMEOUT, "Should not receive the callback with in timeout." );

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_Sparse404,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT downloadResult, downloadCallBackResult;
        bool isChunkComplete;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;
        string allSparseFragments404Body;
        ChunkIterator itChunk;
        
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"ODSparseStreamManifestOutputFalse");

        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        //ClientResponder post
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");

        allSparseFragments404Body = SSPKHelpers::GetCR_ReturnStatusCodeProfile(SSPKHelpers::GetLocalUrl(urlString), "fragments", WStr2Str(sparseStreams[0]->Name()), 404 );

        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + ADDPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), allSparseFragments404Body);

        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0],DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( FAILED(downloadCallBackResult), "Should receive a failure when 404 occurred on chunk download." );

        //try Again.
        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0],DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( FAILED(downloadCallBackResult), "Should receive a failure when 404 occurred on chunk download." );

        // skip this
        PKTEST_HRESULT_EXIT(itChunk.MoveNext());
        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0],DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( FAILED(downloadCallBackResult), "Should receive a failure when 404 occurred on chunk download." );

    exit:
        SSPKHelpers::ClientResponderPost(SSPKHelpers::GetLocalUrl(urlString) + DELETEPROFILE_STR, SSPKHelpers::GetServerNameFromUrl(urlString), "");
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(
        SparseStreams_DownloadFragmentAsync_AfterDetune,
        PKTEST_PROPERTY( "Bug", "27205" )
        PKTEST_PROPERTY("Priority", "1")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT downloadResult, downloadCallBackResult;
        bool isChunkComplete;
        ChunkIterator itChunk;
        int64_t chunkTimestamp;
        int64_t downloadedBufferSize;
        int64_t totalChunkSize;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveSparseStreamDefault");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams selected.");

        itChunk = sparseStreams[0]->GetLastInCurrentChunkList();
        sparseStreams[0]->GetSelectedTracks(&selectedTracks);

        PKTEST_HRESULT_EXIT( _smoothObject->Close() );

        downloadResult = sparseStreams[0]->DownloadFragmentAsync(itChunk,selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT,_smoothObject);
        PKTEST_ASSERT_MSG_EXIT(SUCCEEDED( downloadResult ), "The DownloadAsync call should not fail ");

        downloadCallBackResult = _smoothObject->WaitForDownloadFragmentAsync( SPARSE_CHUNK_DOWNLOAD_DEFAULT_TIMEOUT, isChunkComplete, downloadedBufferSize, chunkTimestamp, totalChunkSize );
        PKTEST_ASSERT_MSG_EXIT( downloadCallBackResult == pkE_TIMEOUT, "Should not receive the callback with in timeout." );

    exit:
        return;
    }
    
    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_NonSelectedStreams,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        ChunkIterator itChunk;

        VectorOfStreams selectedStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveWithTwoSparseStreamsInSameHeader");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllAvailableSparseStreams();
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams available.");

        for(size_t i = 0 ; i < sparseStreams.size() ; i++)
        {
            VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), sparseStreams[i]);
            if ( it == selectedStreams.end() )
            {
                itChunk = sparseStreams[i]->GetFirstInCurrentChunkList();
                sparseStreams[i]->GetSelectedTracks(&selectedTracks);

                hr = sparseStreams[i]->DownloadFragmentAsync(itChunk, selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT, _smoothObject);
                PKTEST_ASSERT_MSG_EXIT( hr == pkE_INVALID_REQUEST, "Should fail to get sparse chunk info for non-selected streams" );
            }
        }

    exit:
        return;
    }

    //////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(SparseStreams_DownloadFragmentAsync_StreamTrackMismatch,
        PKTEST_PROPERTY("Priority", "2")
        )
    {
        VectorOfStreams sparseStreams;
        HRESULT hr;
        ChunkIterator itChunk;
        AutoRefPtr<IManifestStream> nonSparseStream;

        VectorOfStreams selectedStreams;
        std::vector< AutoRefPtr<IManifestTrack> > selectedTracks;

        string urlString = SSPKHelpers::GetUrlFromList( _xManifests,L"LiveWithTwoSparseStreamsInSameHeader");
        PKTEST_FUNC_EXIT(_smoothObject->OpenVideoAndValidate(urlString, false));

        PKTEST_FUNC_EXIT( SelectASparseStream() );

        sparseStreams = GetAllSelectedSparseStreams();
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        PKTEST_ASSERT_MSG_EXIT(sparseStreams.size() > 0, "There are no sparse streams available.");

        for(size_t i = 0; i < selectedStreams.size(); i++)
        {
            if(selectedStreams[i]->Type() != MediaStreamTypeText)
            {
                nonSparseStream = selectedStreams[i];
                break;
            }
        }

        // Use sparsestream to request the DownloadFragment but use track of a different stream
        itChunk = sparseStreams[0]->GetFirstInCurrentChunkList();
        nonSparseStream->GetSelectedTracks(&selectedTracks);

        hr = sparseStreams[0]->DownloadFragmentAsync(itChunk, selectedTracks[0], DEFAULT_CHUNK_DOWNLOAD_LIMIT, _smoothObject);
        PKTEST_ASSERT_MSG_EXIT( hr == E_INVALIDARG, "Should fail to get sparse chunk info when a mismatched track is supplied." );

    exit:
        return;
    }
    

    //////////////////////////////////////////////////////////////

private:
    VectorOfStreams GetAllSelectedSparseStreams()
    {
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams);
        proposedSelection = SSPKHelpers::GetAllSparseStreams(selectedStreams);

    exit:
        return proposedSelection;
    }

    VectorOfStreams GetAllAvailableSparseStreams()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams);
        proposedSelection = SSPKHelpers::GetAllSparseStreams(availableStreams);

    exit:
        return proposedSelection;
    }

    void SelectASparseStream()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        if( GetAllSelectedSparseStreams().size() == 0)
        {
            PKTEST_FUNC_EXIT(proposedSelection = SSPKHelpers::AddTextStream(availableStreams, selectedStreams, true));
            PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->SelectStreamsAsync(_smoothObject, proposedSelection) );
            PKTEST_FUNC_EXIT(_smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT));
        }

    exit:
        return;
    }

    void DeSelectAllSparseStream()
    {
        VectorOfStreams availableStreams;
        VectorOfStreams selectedStreams;
        VectorOfStreams proposedSelection;

        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

        if( GetAllSelectedSparseStreams().size() != 0)
        {
            PKTEST_FUNC_EXIT(proposedSelection = SSPKHelpers::SelectFirstStreamOfAudioAndVideo(availableStreams));
            PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->SelectStreamsAsync(_smoothObject, proposedSelection) );
            PKTEST_FUNC_EXIT(_smoothObject->WaitForSelectedStreams(STREAMSELECTION_TIMEOUT));
        }

exit:
        return;
    }

};
