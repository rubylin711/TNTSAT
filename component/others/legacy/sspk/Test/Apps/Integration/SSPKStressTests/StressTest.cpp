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
#include "DispatchTimer.h"
#include "SSPKHelpers.h"
#include "STUtilities.h"
#include "CEvent.h"
#include <time.h>
#include <algorithm>

using namespace SSPK;
using namespace SSPKTest;

typedef std::vector< AutoRefPtr<IManifestStream> > VectorOfStreams;

namespace SSPKTest
{
    enum Actions
        {
            Play = 0,
            Pause,
            Close,
            SeekToLive,
            SeekToBegin,
            NextSourceWithClose,
            NextSourceWithoutClose,
            ResetSSPK,
            FastForward,
            Rewind,
            SkipBack,
            SkipForward,
            SeekAny,
            SeekBack,
            SeekFront,
            DeleteAndOpenNextUrl,
            DeleteWithOutCloseAndNextUrl,
            TrackSelection,
            AudioSwitch,
            RandomStreamSelection,
            AddTextStream,
            ChannelSwitch,
            SetPlaybackRangePast,
            SetPlaybackRangeFuture,
            MaxActions
        };

    enum ActionFlow
        {
            Random = 0,
            Sequential,
            Loop,
            RandomRace
        };

    class StressTest : public DispatchTimer
    {
        STWrapper* _smoothObject;
        CXMLElementsList _xManifests;
        CManifestReadyCallback  _sink;
        int64_t _durationToRun;
        int64_t _duration;
        int32_t _actionInterval;
        string _currentUrl;
        time_t _startTime;
        time_t _currentTime;
        ActionFlow _flowType;
        size_t _currentActionIndex;
        int64_t _sameActionInterval;
        vector<Actions> _actionVector;

        CEvent* _testDoneEvent;

    public:

        StressTest(ActionFlow flowType) : _duration(0), _durationToRun(0), _actionInterval(2), _startTime(0), _flowType(flowType), _currentActionIndex(0), _sameActionInterval(0), _testDoneEvent(NULL)
        {
            CXMLElement xTestData = PKTest_GetDataTable( "StressTestSources" );
            CXMLElement xTestProps = PKTest_GetDataTable( "Properties" );

            _durationToRun = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"DurationToRunInSeconds")[0].Attributes()[L"value"].Value())));
            _actionInterval = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"ActionIntervalInSeconds")[0].Attributes()[L"value"].Value())));

            if(_flowType == Sequential)
            {
                _sameActionInterval = _durationToRun / MaxActions;
            }

            _xManifests = xTestData.Elements(L"source");

            _smoothObject = NEW_NO_THROW STWrapper();

            _testDoneEvent = NEW_NO_THROW CEvent(CEvent::eResetModeAuto,false);

        }

        ~StressTest()
        {
            if(NULL != _testDoneEvent)
            {
                delete _testDoneEvent;
            }
            if(!_smoothObject->IsClosed() )
            {
                _smoothObject->Close();
            }
            delete _smoothObject;
        }

        void StartTest()
        {
            vector<Actions> actionVector;
            for(int32_t index=0; index < MaxActions; index++)
            {
                actionVector.push_back(Actions(index));
            }
            
            StartTest(actionVector);
        }

        void StartTest(vector<Actions> actionVector, int64_t durationToRun = 0, int32_t actionInterval = 0 )
        {
            if(durationToRun != 0)
            {
                _durationToRun = durationToRun;
            }
            if(actionInterval != 0)
            {
                _actionInterval = actionInterval;
            }

            _actionVector = actionVector;
            _currentUrl = GetRandomSource();
            LogTestWarning("Stress Test starting with url : %s", _currentUrl.c_str());
            _smoothObject->OpenVideo(_currentUrl, true, false);
            _startTime = time (NULL);

            Init(_actionInterval * MILLISECONDS_PER_SECOND);
            Start();

        }

        void OnTick()
        {
            _currentTime = time (NULL);
            if ( difftime(_currentTime, _startTime) >= _durationToRun )
            {
                _testDoneEvent->Set();
                return;
            }
            else if ( _smoothObject->IsMediaFailed() || _smoothObject->IsMediaEnded() || _smoothObject->IsClosed() )
            {
                InvokeStressAction(NextSourceWithClose);
                _currentActionIndex = std::find(_actionVector.begin(), _actionVector.end(), NextSourceWithClose) - _actionVector.begin();
                _currentActionIndex = _currentActionIndex % (_actionVector.size()) ;
            }
            else
            {
                if(_actionVector[_currentActionIndex] == Close)
                {
                    InvokeStressAction(NextSourceWithClose);
                    _currentActionIndex = std::find(_actionVector.begin(), _actionVector.end(), NextSourceWithClose) - _actionVector.begin();
                    _currentActionIndex = _currentActionIndex % (_actionVector.size()) ;
                }
                else
                {
                    switch (_flowType)
                    {
                        case Random:
                            {
                                int32_t randomindex = STUtilities::RandomIndex(_actionVector.size());
                                InvokeStressAction(_actionVector[randomindex]);
                                _currentActionIndex = randomindex;
                            }
                            break;

                        case Sequential:
                            if(difftime(_currentTime, _startTime) >= (_currentActionIndex + 1) * _sameActionInterval)
                            {
                                _currentActionIndex = ( ++_currentActionIndex ) % _actionVector.size();
                            }
                            InvokeStressAction(_actionVector[_currentActionIndex]);
                            break;

                        case Loop:
                            _currentActionIndex = ( ++_currentActionIndex ) % _actionVector.size();
                            InvokeStressAction(_actionVector[_currentActionIndex]);
                            break;

                        case RandomRace:
                            {
                                int32_t randomindex = STUtilities::RandomIndex(_actionVector.size());
                                InvokeStressAction(_actionVector[randomindex]);
                                randomindex = STUtilities::RandomIndex(_actionVector.size());
                                InvokeStressAction(_actionVector[randomindex]);

                                _currentActionIndex = randomindex;
                            }
                            break;
                    }
                }
            }

            return;
        }

        void InvokeStressAction(Actions currentAction)
        {
            time_t rawtime;
            struct tm * timeinfo;

            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            string currentTime = asctime (timeinfo);
            currentTime = currentTime.substr(0,currentTime.length() - 1);

            VectorOfStreams proposedSelection;
            VectorOfStreams availableStreams;
            VectorOfStreams selectedStreams;
            VectorOfStreams unSelectedTextStreams;
            IManifestStream* parentStream ;
            vector< AutoRefPtr<IManifestTrack> > availableTracks;
            vector< AutoRefPtr<IManifestTrack> > proposedTracks;
            HRESULT hr2;
            bool textSelected;
            int32_t chunklistDuration = 0;
            int64_t proposedStartTime = 0;

            switch(currentAction)
            {
                case Play :
                    LogTestWarning("%s:::Stress Action: Play", currentTime.c_str());
                    _smoothObject->Play();
                    break;

                case Pause :
                    LogTestWarning("%s:::Stress Action: Pause", currentTime.c_str());
                    _smoothObject->Pause(0);
                    break;

                case Close :
                    LogTestWarning("%s:::Stress Action: Close", currentTime.c_str());
                    _smoothObject->Close();
                    break;

                case SeekToLive :
                    LogTestWarning("%s:::Stress Action: SeekToLive", currentTime.c_str());
                    _smoothObject->Skip(TOTAL_SECONDS_IN_A_DAY);
                    break;

                case SeekToBegin :
                    LogTestWarning("%s:::Stress Action: SeekToBegin", currentTime.c_str());
                    _smoothObject->Skip(-TOTAL_SECONDS_IN_A_DAY);
                    break;

                case ResetSSPK :
                    LogTestWarning("%s:::Stress Action: Reset SSPK : %s", currentTime.c_str(), _currentUrl.c_str());
                    {
                        _smoothObject->Close();

                        _smoothObject->OpenVideo(_currentUrl, ( STUtilities::Rand(2) % 2 ) != 0 , false);
                    }
                    break;

                case NextSourceWithClose :
                    _currentUrl = GetRandomSource();
                    LogTestWarning("%s:::Stress Action: Next Source with close : %s", currentTime.c_str(), _currentUrl.c_str());

                    _smoothObject->Close();

                    _smoothObject->OpenVideo(_currentUrl, ( STUtilities::Rand(2) % 2 ) != 0, false);
                    break;

                case NextSourceWithoutClose:
                    _currentUrl = GetRandomSource();
                    LogTestWarning("%s:::Stress Action: Next Source witout close : %s", currentTime.c_str(), _currentUrl.c_str());
                    
                    _smoothObject->OpenVideo(_currentUrl, ( STUtilities::Rand(2) % 2 ) != 0, false);
                    break;

                case DeleteAndOpenNextUrl:
                    _smoothObject->Close();
                    delete _smoothObject;

                    _smoothObject = NEW_NO_THROW STWrapper();
                    _currentUrl = GetRandomSource();
                    LogTestWarning("%s:::Stress Action: DeleteAndOpenNextUrl : %s", currentTime.c_str(), _currentUrl.c_str());
                    
                    _smoothObject->OpenVideo(_currentUrl, true, false);
                    break;
                
                case DeleteWithOutCloseAndNextUrl:
                    delete _smoothObject;
                    _smoothObject = NEW_NO_THROW STWrapper();
                    _currentUrl = GetRandomSource();
                    LogTestWarning("%s:::Stress Action: DeleteWithOutCloseAndNextUrl : %s", currentTime.c_str(), _currentUrl.c_str());
                    
                    _smoothObject->OpenVideo(_currentUrl, true, false);
                    break;

                case ChannelSwitch : 
                    _currentUrl = GetRandomSource();
                    LogTestWarning("%s:::Stress Action: ChannelSwitch : %s", currentTime.c_str(), _currentUrl.c_str());
                    _smoothObject->OpenVideo(_currentUrl, true, false);
                    break;

                case SkipBack :
                    LogTestWarning("%s:::Stress Action: SkipBack ", currentTime.c_str());
                    {
                        int32_t skipTime = STUtilities::Rand(RAND_SKIP_SECONDS);
                        _smoothObject->Skip(-skipTime);
                    }
                    break;

                case SkipForward :
                    LogTestWarning("%s:::Stress Action: SkipForward ", currentTime.c_str());
                    {
                        int32_t skipTime = STUtilities::Rand(RAND_SKIP_SECONDS);
                        _smoothObject->Skip(skipTime);
                    }
                    break;

                case SeekAny :
                    {
                        int64_t proposedPlayBackTime = _smoothObject->GetRandomSeekablePosition();
                        LogTestWarning("%s:::Stress Action: SeekAny to : %.3f ", currentTime.c_str(), (double)NTP_UINT64TO10MHZ(proposedPlayBackTime)/ TIMESCALE_10MHZ);
                        _smoothObject->Seek( NTP_UINT64TO10MHZ(proposedPlayBackTime) );
                    }
                    break;

                case SeekBack :
                    {
                        LogTestWarning("%s:::Stress Action: SeekBack ", currentTime.c_str());
                        int64_t currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
                        {
                            int32_t seekTime = STUtilities::Rand(RAND_SKIP_SECONDS) * TIMESCALE_10MHZ;
                    
                            int64_t proposedPlayBackTime = 0;
                            if(seekTime < currentPlayBackTime)
                            {
                                proposedPlayBackTime = currentPlayBackTime - seekTime;
                            }
                            _smoothObject->Seek( proposedPlayBackTime );
                        }
                    }
                    break;

                case SeekFront :
                    {
                        LogTestWarning("%s:::Stress Action: SeekFront ", currentTime.c_str());
                        int64_t currentPlayBackTime = _smoothObject->GetCurrentPlayBackTime();
                        {
                            int32_t seekTime = STUtilities::Rand(RAND_SKIP_SECONDS) * TIMESCALE_10MHZ;
                            int64_t proposedPlayBackTime = currentPlayBackTime + (seekTime);
                            _smoothObject->Seek( proposedPlayBackTime );
                        }
                    }
                    break;
            
                case FastForward :
                    LogTestWarning("%s:::Stress Action: FastForward", currentTime.c_str());
                    _smoothObject->FastForward();
                    break;

                case Rewind :
                    LogTestWarning("%s:::Stress Action: Rewind", currentTime.c_str());
                    _smoothObject->Rewind();
                    break;

                case SetPlaybackRangePast:
                    LogTestWarning("%s:::Stress Action: SetPlaybackRange Past", currentTime.c_str());
                    chunklistDuration = 8 * 60 * SECONDS_PER_MINUTE; //In seconds. Assuming the chunksize is 2 seconds, which means the chunksize 10800 chunks == 6 hours.

                    proposedStartTime = STUtilities::Rand(_smoothObject->GetCurrentEndTime() - TimeSpan_hns::ConvertFrom(TimeSpan_s::FromTicks(chunklistDuration)).Ticks(), _smoothObject->GetCurrentStartTime());

                    _smoothObject->SetPlaybackRangeAsync(proposedStartTime);
                    break;

                case SetPlaybackRangeFuture:
                    LogTestWarning("%s:::Stress Action: SetPlaybackRange Future", currentTime.c_str());
                    _smoothObject->SetPlaybackRangeAsync(_smoothObject->GetRandomSeekablePosition());
                    break;

                case AudioSwitch :
                    LogTestWarning("%s:::Stress Action: AudioSwitch", currentTime.c_str());
                    {
                        bool newAudioSelected = false;
                        selectedStreams.clear();
                        availableTracks.clear();
                        proposedTracks.clear();

                        PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

                        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
                        PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

                        proposedSelection = SSPKHelpers::ToggledStreamByMediaType(availableStreams, selectedStreams, MediaStreamTypeAudio);
                    
                        for(uint32_t i = 0; i < proposedSelection.size(); i++)
                        {
                            if( proposedSelection[i]->Type() == MediaStreamTypeAudio)
                            {
                                newAudioSelected = true;
                            }
                        }
                        if(newAudioSelected)
                        {
                            _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
                        }
                    }

                    break;

                case RandomStreamSelection:
                    LogTestWarning("%s:::Stress Action: RandomStreamSelection", currentTime.c_str());

                    availableStreams.clear();
                    proposedSelection.clear();

                    PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

                    PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );

                    proposedSelection = availableStreams;

                    for(uint32_t i = 0; i < STUtilities::RandomIndex(availableStreams.size() + 1); i++) // size can be anything from 0 to size
                    {
                        uint32_t streamIndexToRemove = STUtilities::Rand( proposedSelection.size() );
                        proposedSelection.erase( proposedSelection.begin() + streamIndexToRemove );
                    }

                    _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
                    
                    break;

                case TrackSelection :
                    LogTestWarning("%s:::Stress Action: TrackSelection", currentTime.c_str());

                    selectedStreams.clear();
                    availableTracks.clear();
                    proposedTracks.clear();

                    PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

                    PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

                    for(uint32_t i =0; i<selectedStreams.size(); i++)
                    {
                        if(selectedStreams[i]->Type() == MediaStreamTypeVideo)
                        {
                            PKTEST_HRESULT_EXIT( selectedStreams[i]->GetAvailableTracks( &availableTracks ) );

                            proposedTracks = availableTracks;

                            int32_t countToErase = static_cast<int>(STUtilities::Rand((size_t)1, availableTracks.size()));
                            for(int32_t j = 0; j < countToErase; j++)
                            {
                                int32_t iTrackToRemove = STUtilities::Rand( proposedTracks.size() );
                                proposedTracks.erase( proposedTracks.begin() + iTrackToRemove );
                            }
                            PKTEST_HRESULT_EXIT( selectedStreams[i]->SelectTracks( proposedTracks ) );
                            break;
                        }
                    }
                    break;

                case AddTextStream :
                    LogTestWarning("%s:::Stress Action: AddTextStream", currentTime.c_str());
                    availableStreams.clear();
                    selectedStreams.clear();
                    unSelectedTextStreams.clear();
                    proposedSelection.clear();
                    parentStream = NULL ;
                    textSelected = false;

                    PKTEST_ASSERT_MSG_EXIT(NULL != _smoothObject->GetManifest(), "manifest is null. Probably ManifestReady event never occurred. Please check the content Url" );

                    PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetAvailableStreams(&availableStreams) );
                    PKTEST_HRESULT_EXIT( _smoothObject->GetManifest()->GetSelectedStreams(&selectedStreams) );

                    for(uint32_t j=0; j<selectedStreams.size(); j++)
                    {
                        proposedSelection.push_back(selectedStreams[j]);
                    
                    }
                    for( uint32_t i = 0; i<availableStreams.size(); i++ )
                    {
                        if( availableStreams[i]->Type() == MediaStreamTypeText )
                        {
                            VectorOfStreams::const_iterator it = std::find( selectedStreams.begin(), selectedStreams.end(), availableStreams[i] );
                            if ( it == selectedStreams.end() )
                            {
                                unSelectedTextStreams.push_back(availableStreams[i]);
                            }
                        }
                    }
                    if(!unSelectedTextStreams.empty())
                    {
                        while (!textSelected)
                        {
                            int32_t textIndexSelected = STUtilities::Rand(unSelectedTextStreams.size());
                            unSelectedTextStreams[textIndexSelected]->GetParentStream(&parentStream);
                            if ( !parentStream )
                            {
                                proposedSelection.push_back(unSelectedTextStreams[textIndexSelected]);
                                textSelected = true;
                            }
                            else
                            {
                                VectorOfStreams::const_iterator parentit = std::find( selectedStreams.begin(), selectedStreams.end(), parentStream );
                                if ( parentit != selectedStreams.end() )
                                {
                                    proposedSelection.push_back(unSelectedTextStreams[textIndexSelected]);
                                    textSelected = true;
                                }
                            }
                        }
                    }
                    if(textSelected)
                    {
                        hr2 = _smoothObject->GetManifest()->SelectStreamsAsync( _smoothObject, proposedSelection);
                    }
                    break;

                default :
                    LogTestWarning("%s:::Stress Action: Not Defined Yet", currentTime.c_str());
                    break;
            }
        exit:
            return;
        }

        string GetRandomSource()
        {
            int32_t randomIndex = STUtilities::RandomIndex(_xManifests.Length() - 1);
            CXMLAttributesList currentElementAttributes = _xManifests[randomIndex].Attributes();

            return escapeSpaces( wstring_to_string( currentElementAttributes[L"url"].Value() ) );
        }
        
        bool IsDispatchTimerActive()
        {
            return IsTimerActive();
        }
        
        bool WaitForTestDoneEvent(DWORD dwMsTimeout = INFINITE)
        {
            CEvent::EWaitResult hr = _testDoneEvent->Wait(dwMsTimeout);
            return (hr == CEvent::eWaitSignaled)? true : false;
        }

    };
};

PKTEST_GROUP( StressTest1 )
{
    PKTEST_METHOD( StressWithRandomActions )
    {
        StressTest* sTest = NEW_NO_THROW StressTest(Random);
        sTest->StartTest();
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }

    PKTEST_METHOD( StressWithSequentialActions )
    {
        StressTest* sTest = NEW_NO_THROW StressTest(Sequential);
        sTest->StartTest();
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }

    PKTEST_METHOD( ChannelSwitchStress )
    {
        vector<Actions> actionVector;
        actionVector.push_back(NextSourceWithClose);
        actionVector.push_back(NextSourceWithoutClose);
        actionVector.push_back(DeleteAndOpenNextUrl);

        StressTest* sTest = NEW_NO_THROW StressTest(Random);
        sTest->StartTest(actionVector);
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }

    //Run 2 random actions sequentially without any wait
    PKTEST_METHOD( StressWithRandomRaceAction )
    {
        StressTest* sTest = NEW_NO_THROW StressTest(RandomRace);
        sTest->StartTest();
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }

    PKTEST_METHOD( SeekStress )
    {
        int32_t durationToRun = 0;
        int32_t actionInterval = 0;
        CXMLElement xTestProps = PKTest_GetDataTable( "SeekTestProperties" );

        durationToRun = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"DurationToRunInSeconds")[0].Attributes()[L"value"].Value())));
        actionInterval = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"ActionIntervalInSeconds")[0].Attributes()[L"value"].Value())));

        vector<Actions> actionVector;
        actionVector.push_back(SeekAny);

        StressTest* sTest = NEW_NO_THROW StressTest(Random);
        sTest->StartTest(actionVector, durationToRun, actionInterval);
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }

    PKTEST_METHOD( PlayPauseStress )
    {
        int32_t durationToRun = 0;
        int32_t actionInterval = 0;
        CXMLElement xTestProps = PKTest_GetDataTable( "NoTuneTestProperties" );

        durationToRun = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"DurationToRunInSeconds")[0].Attributes()[L"value"].Value())));
        actionInterval = toInt(escapeSpaces( wstring_to_string(xTestProps.Elements(L"ActionIntervalInSeconds")[0].Attributes()[L"value"].Value())));

        vector<Actions> actionVector;
        actionVector.push_back(Play);
        actionVector.push_back(Pause);

        StressTest* sTest = NEW_NO_THROW StressTest(Loop);
        sTest->StartTest(actionVector, durationToRun, actionInterval);
        sTest->WaitForTestDoneEvent();

        sTest->Stop();
        delete sTest;
    }
};
