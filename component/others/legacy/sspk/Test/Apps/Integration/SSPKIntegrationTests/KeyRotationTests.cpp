///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PKTestSuite.h"
#include "STWrapper.h"
#include "ManifestReadyCallbackImpl.h"
#include "SSPKHelpers.h"
#include "IXDrm.h"//For Drm callback
#include "PKTestSuiteUtils.h"//For CTextReader

#include <fstream>
#include <iomanip>
using namespace SSPK;
using namespace SSPKTest;

PKTEST_GROUP( KeyRotationTests )
{

    STWrapper* _smoothObject;
    CXMLElementsList _xManifests;
    static const int32_t KEYROTATION_WAIT_FOR_MEDIA_COMPLETED = 120000;//2 minutes
    
    KeyRotationTests()
    {
    }

    bool TestSetup()
    {
        CXMLElement xTestData = PKTest_GetDataTable( "KeyRotationSources" );
        PKTEST_ASSERT_EXIT( !xTestData.IsNull() );

        _xManifests = xTestData.Elements(L"source");

    exit:
        return true;
    }

    bool TestCleanup()
    {        	
        return true;
    }
    /*
    //Setup and cleanup
    */
    bool StartSmoothObject( wstring sourceName, bool autoPlay)
    {
        _smoothObject = NEW_NO_THROW STWrapper();
        if( !_smoothObject )
        {
            return false;
        }
        
        string urlString = SSPKHelpers::GetUrlFromList( _xManifests, sourceName.c_str());
        _smoothObject->OpenVideo(urlString, autoPlay);
        
        return true;
    }
    
    void StopSmoothObject( )
    {
        if(_smoothObject)
        {
            _smoothObject->Close();
            delete _smoothObject;
            _smoothObject = NULL;
        }
    }

    /*
    //Key rotation test specific types and methods
    */
    enum StreamIndex
    {
        StreamIndex_Audio = 0,
        StreamIndex_Video,
        StreamIndex_Count
    };

    /*
    //Represents the data that is emitted by CXDrm for key rotation
    */
    class KeyRotationLine
    {
        //////////////////////////////////////
        //Helper classes
    public:
        class KeyIDStruct
        {
        public:
            static const int32_t KEY1_COUNT = 2;
            static const int32_t KEY2_COUNT = 8;

            uint32_t Key0;
            uint16_t Key1[KEY1_COUNT];
            uint8_t Key2[KEY2_COUNT];

        public:
            bool operator!=(const KeyIDStruct& rhs)
            {
                return !(*this == rhs);
            }
            bool operator==(const KeyIDStruct& rhs)
            {
                bool isEqual = (Key0 == rhs.Key0);

                for(int32_t i = 0; i < KEY1_COUNT; ++i)
                {
                    isEqual &= (Key1[i] == rhs.Key1[i]);
                }
                                
                for(int32_t i = 0; i < KEY2_COUNT; ++i)
                {
                    isEqual &= (Key2[i] == rhs.Key2[i]);
                }
                
                return isEqual;
            }
        };
        //////////////////////////////////////		
        //Values used in parsing string that represents the expected
        static const int32_t SAMPLE_ID_STRING_LENGTH = 18;
        static const int32_t KEY0_STRING_LENGTH = 8;
        static const int32_t KEY1_STRING_LENGTH = 4;
        static const int32_t KEY2_STRING_LENGTH = 2;
        static const int32_t CRC_STRING_LENGTH = 10;

        static const int32_t KEY1_BUFFER_POSITION = 4;
        static const int32_t KEY2_BUFFER_POSITION = 8;

        static const int32_t DECRYPT_BUFFER_CHAIN_LINE_LENGTH = 80;
    public:		
        //////////////////////////////////////
        //Methods
        ~KeyRotationLine()
        {
            delete[] m_data.pbKeyID;
        }

        KeyRotationLine(string inLine):
            Line(inLine)
        {
            BuildData();
        }

        KeyRotationLine(const IXDrmDiagDelegate::SDecryptInfo& inData):
            Line("")
        {
            m_data = inData;
            BuildString();
        }
 
        void BuildString(void)
        {
            int32_t IDStringLength = SAMPLE_ID_STRING_LENGTH - 2; //-2 for '0x', which is hardcoded here
            int32_t CRCStringLength = CRC_STRING_LENGTH - 2; //-2 for '0x', which is hardcoded here

            //Format to look like current expected files, which ignores qwOffset
            stringstream stream;
            stream<< "0x"<< 
                setw(IDStringLength)<<setfill('0')<<setbase(IDStringLength)<<
                m_data.qwSampleID<<" "<<
                setw(1)<<//1 for true or 0 false
                (int32_t)(m_data.fIsAES ? 1 : 0)<<" "<<
                setw(KEY0_STRING_LENGTH)<<
                (unsigned int) *((uint32_t*)m_data.pbKeyID)<<"-"<<
                setw(KEY1_STRING_LENGTH)<<
                (unsigned int) *(uint16_t*)(m_data.pbKeyID + KEY1_BUFFER_POSITION )<<"-"<<
                setw(KEY1_STRING_LENGTH)<<
                (unsigned int) *(uint16_t*)(m_data.pbKeyID + KEY1_BUFFER_POSITION + sizeof(uint16_t) )<<"-"<<
                setw(KEY2_STRING_LENGTH)<<				
                (unsigned int) m_data.pbKeyID[KEY2_BUFFER_POSITION]<<
                setw(KEY2_STRING_LENGTH)<<
                (unsigned int) m_data.pbKeyID[KEY2_BUFFER_POSITION + 1]<<"-";
                for(int32_t i = 2; i < KeyIDStruct::KEY2_COUNT; ++i)
                {
                    stream<<setw(KEY2_STRING_LENGTH)<<
                    (unsigned int) m_data.pbKeyID[KEY2_BUFFER_POSITION + i];
                }

                stream<<" 0x"<<
                setw(CRCStringLength)<<
                m_data.crc32PSSH;

            //Use uppercase as in expected files
            Line = stream.str();
        }

        //Sets the data in SDecryptInfo by parsing the string representation which looks like this:
        //0x0000000000000000 1 c394a553-74cd-4803-9acb-00fc3e18e7c3 0xBFF097B9
        void BuildData(void)
        {		
            //Counters
            int32_t dataOffset = 0;
            int32_t stringOffset = 0;

            m_data.qwSampleID = toUInt64(Line.substr(stringOffset, SAMPLE_ID_STRING_LENGTH));
            stringOffset += SAMPLE_ID_STRING_LENGTH + 1; 

            m_data.fIsAES = (toInt(Line.substr(stringOffset, 1)) == 1) ? true: false; //1 for true, 0 for false
            stringOffset += 2; //1 for value plus 1 for the space

            //Parse data in string format from expected files
            m_data.pbKeyID = new uint8_t[sizeof(KeyIDStruct)];

            //First part of KeyID
            *((uint32_t*) m_data.pbKeyID) = toULongFromHex(Line.substr(stringOffset, KEY0_STRING_LENGTH));
            stringOffset += KEY0_STRING_LENGTH + 1;
            dataOffset += sizeof(uint32_t);

            //Second part of KeyID
            for(int32_t i = 0; i < KeyIDStruct::KEY1_COUNT; ++i)
            {
                *((uint16_t*)(m_data.pbKeyID + dataOffset)) = convertFromHex<uint16_t>(Line.substr(stringOffset, KEY1_STRING_LENGTH));
                dataOffset += sizeof(uint16_t);
                stringOffset += KEY1_STRING_LENGTH + 1;
            }
            
            //The third part of the KeyID, composed of unsigned shorts parsed from 2-character strings.  
            //The first two are separate and so are outside of the loop
            *((uint8_t*)(m_data.pbKeyID + dataOffset)) = convertFromHex<uint8_t>(Line.substr(stringOffset, KEY2_STRING_LENGTH));
            dataOffset += sizeof(uint8_t);
            stringOffset += KEY2_STRING_LENGTH;
            *((uint8_t*)(m_data.pbKeyID + dataOffset)) = convertFromHex<uint8_t>(Line.substr(stringOffset, KEY2_STRING_LENGTH));
            dataOffset += sizeof(uint8_t);
            stringOffset += KEY2_STRING_LENGTH + 1;
            //Start at 2 because the first 2 are separated in the string and read above
            for(int32_t i = 2; i < KeyIDStruct::KEY2_COUNT; ++i)
            {
                *((uint8_t*)(m_data.pbKeyID + dataOffset)) = convertFromHex<uint8_t>(Line.substr(stringOffset, KEY2_STRING_LENGTH));
                dataOffset += sizeof(uint8_t);
                stringOffset += KEY2_STRING_LENGTH;
            }
            stringOffset += 1; //For the space

            m_data.crc32PSSH = toULongFromHex( Line.substr(stringOffset, CRC_STRING_LENGTH) );
            m_data.qwOffset = 0; //Not used in comparison
            m_data.cbKeyID = 0x10;//This is the value that is set in the code when an SDecryptInfo is created in CXDrm.cpp; sizeof(_DRM_DECRYPT_CONTEXT.oKID)
        }

        //Accessors for important values
        uint64_t GetSampleID()
        {
            return m_data.qwSampleID;
        }

        uint32_t GetCRC()
        {
            return m_data.crc32PSSH;
        }

        bool operator!=(const KeyRotationLine& rhs)
        {
            return !(*this == rhs);
        }

        bool operator==(const KeyRotationLine& rhs)
        {
            bool isEqual = true;
            //Compare normal fields m_data.qwOffset == rhs.m_data.qwOffset is excluded because it is not part of the expected files
            isEqual =	(m_data.fIsAES == rhs.m_data.fIsAES) &&
                        (m_data.qwSampleID == rhs.m_data.qwSampleID);

            //Compare Key IDs
            if(isEqual)
            {
                KeyIDStruct *left = (KeyIDStruct*)m_data.pbKeyID, 
                            *right = (KeyIDStruct*)rhs.m_data.pbKeyID;
                isEqual = (*left == *right);
            }

            return isEqual;
        }

        //////////////////////////////////////
        //Fields
        string Line;
        IXDrmDiagDelegate::SDecryptInfo m_data;
    };//KeyRotationLine

    //Typedefs
    typedef vector<KeyRotationLine*> KeyRotationVector;

    //Used to get Key Rotation 
    class DrmDecryptDelegate : public IXDrmDiagDelegate
    {
    public:
        DrmDecryptDelegate(string expectedAudioFilepath, string expectedVideoFilepath )
            : m_expectedFileErrorOccurred(false)
        {
            for(int32_t i = 0; i < StreamIndex_Count; ++i)
            {
                m_endOfFile[i] = false;
                m_expectedLines[i] = NULL;
                m_expectedFilesTR[i] = NULL;
            }

            expectedAudioFilepath = unescape(expectedAudioFilepath);
            expectedVideoFilepath = unescape(expectedVideoFilepath);
            bool success = false;

            PKTEST_MSG("%s Audio expected file", expectedAudioFilepath.c_str());
            m_expectedFilesTR[StreamIndex_Audio] = NEW_NO_THROW CTextFileReader();
            if(m_expectedFilesTR[StreamIndex_Audio])
            {
                success = pkS_OK == m_expectedFilesTR[StreamIndex_Audio]->OpenExisting(expectedAudioFilepath.c_str());
            }
            if(!success)
            {
                PKTEST_ASSERT_MSG_EXIT(success, "Unable to open file %s", expectedAudioFilepath.c_str() );
            }

            PKTEST_MSG("%s Video expected file", expectedVideoFilepath.c_str());
            m_expectedFilesTR[StreamIndex_Video] = NEW_NO_THROW CTextFileReader();
            if(m_expectedFilesTR[StreamIndex_Video])
            {
                success = pkS_OK == m_expectedFilesTR[StreamIndex_Video]->OpenExisting(expectedVideoFilepath.c_str());
            }
            if(!success)
            {
                PKTEST_ASSERT_MSG_EXIT(success, "Unable to open file %s", expectedVideoFilepath.c_str() );
            }
                                
        exit:
            m_expectedFileErrorOccurred = !success;
            
            return;
        }

        virtual ~DrmDecryptDelegate()
        {
            for(int32_t i = 0; i < StreamIndex_Count; ++i)
            {
                if(m_expectedFilesTR[i])
                {
                    m_expectedFilesTR[i]->Close();
                    delete m_expectedFilesTR[i];
                }
                if(m_expectedLines[i])
                {
                    delete m_expectedLines[i];
                }
            }
        }

        virtual void OnDecryptBufferChain( const SDecryptInfo& sDecryptInfo )
        {
            KeyRotationLine* newKRL = NULL;
            char lineText[KeyRotationLine::DECRYPT_BUFFER_CHAIN_LINE_LENGTH];
            bool foundMatch = false;
            bool allFilesEnded = true;

            //Make sure the expected files are loaded
            PKTEST_ASSERT_MSG_EXIT(!m_expectedFileErrorOccurred, "One or both of the files with the expected results are missing.");
                                        
            //There is no information about whether this is for video or audio, so just
            //check both.  If it matches neither, the test has failed.  It is conceivable (though perhaps not likely) 
            //that an incorrect line from one stream could match the other stream, among other possible mix ups.
            newKRL = NEW_NO_THROW KeyRotationLine(sDecryptInfo);
            for(int32_t i=0; i < StreamIndex_Count; ++i)
            {
                //Read another line if necessary
                memset(lineText,0,KeyRotationLine::DECRYPT_BUFFER_CHAIN_LINE_LENGTH);
                if(NULL == m_expectedLines[i])
                {
                    uint32_t characterCount = 0;
                    m_expectedFilesTR[i]->ReadLine(lineText, KeyRotationLine::DECRYPT_BUFFER_CHAIN_LINE_LENGTH, &characterCount);
                    m_endOfFile[i] = (strlen(lineText) == 0);
                    if(!m_endOfFile[i])
                    {
                        m_expectedLines[i] = NEW_NO_THROW KeyRotationLine(lineText);
                    }
                }
                
                //Skip if there are no more lines
                if(!m_endOfFile[i])
                {
                    if(*newKRL == *m_expectedLines[i])
                    {
                        foundMatch = true;
                        //Reset expected so that next time another read from the file is required.
                        delete m_expectedLines[i];
                        m_expectedLines[i] = NULL;
                    }
                    allFilesEnded = false;
                }
                else
                {
                    m_endOfFile[i] = true;
                }
                
            }

            //Report error if one occurred.
            if(allFilesEnded)
            {
                LogTestError("The end of the expected lines has been reached.  There should be no more key rotation");
            }
            else if(!foundMatch)
            {
                //There is a problem.  Try to figure out what it is.  This analysis is somewhat limited by the 
                //lack of knowledge of what stream the incoming line is supposed to be from
                KeyRotationLine* expected = NULL;
                for(int32_t i=0; i < StreamIndex_Count; ++i)
                {
                    if(m_endOfFile[i])
                    {
                        m_expectedLines[i] = NULL;
                        continue;
                    }

                    expected = m_expectedLines[i];
                    //This check is a replacement for knowing which stream we should be looking at.  
                    //Thrown off by errors that occur when this value changes between samples
                    if(newKRL->GetCRC() == expected->GetCRC()) 
                    {
                        //Skipped samples
                        if(newKRL->GetSampleID() > expected->GetSampleID() )
                        {
                            LogTestError("Skipped %016llx samples", newKRL->GetSampleID() - expected->GetSampleID());
                        }
                        //Duplicated a sample
                        else if (newKRL->GetSampleID() < expected->GetSampleID() )
                        {
                            LogTestError("Duplicated this sample %016llx", newKRL->GetSampleID());
                        }
                    }
                    else
                    {
                        expected = NULL;
                    }

                }
                stringstream failedMessage;
                //Found which one it supposed to be
                if(expected)
                {
                    failedMessage<<"The key rotation message ";
                    failedMessage<<"\t"<<newKRL->Line;
                    failedMessage<<"\t does not match the expected value: ";
                    failedMessage<<"\t"<<expected->Line;
                }
                //Do not know which stream it should have been
                else if (!allFilesEnded)
                {
                    failedMessage<<"The key rotation message does not match any of the expected values.";
                    failedMessage<<"Expected one of these: ";
                    for(int32_t i=0; i < StreamIndex_Count; ++i)
                    {
                        if( !(m_endOfFile[i]) && 
                            m_expectedLines[i])
                        {
                            failedMessage<<(*m_expectedLines[i]).Line<<" ";
                        }
                    }
                    failedMessage<<" But found this: "<<endl<<newKRL->Line<<endl;
                    LogTestError(failedMessage.str().c_str());
                }
                //No more lines with which to compare
                else
                {
                    failedMessage << " Extra key rotation after expected values ";
                }
                LogTestError(failedMessage.str().c_str());
            }
            
            PKTEST_ASSERT_MSG_EXIT(foundMatch, "Match not found");
        exit:
            return;
        }

    protected:
        //Expected
        CTextFileReader* m_expectedFilesTR[StreamIndex_Count];
        KeyRotationLine* m_expectedLines[StreamIndex_Count];
        bool m_endOfFile[StreamIndex_Count];
        bool m_expectedFileErrorOccurred;
    };//DrmDecryptDelegate

    /////////////////////// Key Rotation Tests /////////////////////
    ///////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////
    PKTEST_METHOD_EX(KeyRotationsTests_CompareWithExpected,
                        PKTEST_PROPERTY("Data:Blackout_After_10_Seconds", "KR_Blackout_After_10_Seconds")
                        PKTEST_PROPERTY("Data:No_Key_Rotation", "KR_No_Key_Rotation")
                        PKTEST_PROPERTY("Data:Protected_Clear_Protected", "KR_Protected_Clear_Protected")
                        PKTEST_PROPERTY("Data:Rotate_Key_Every_Fragment", "KR_Rotate_Key_Every_Fragment")
                        PKTEST_PROPERTY("Data:RotateVideo9Seconds_RotateAudio13Seconds", "KR_RotateVideo9Seconds_RotateAudio13Seconds")
                        PKTEST_PROPERTY("Data:LongVideoChunkAndLongAudioChunk_Rotate_Key_Every_60_Seconds", "KR_LongVideoChunkAndLongAudioChunk_Rotate_Key_Every_60_Seconds")
                        PKTEST_PROPERTY("Bug", "26470")
                        )
    {
        wstring urlString = Str2WStr(PKTest_GetTestData());

        //Read golden files into memory
        KeyRotationVector audioLog;
        KeyRotationVector videoLog;
        string logPath = SSPKHelpers::GetAttributeValueFromList(_xManifests, urlString.c_str(), L"logPath");

        //Register callback
        DrmDecryptDelegate callback(logPath + SSPKHelpers::GetAttributeValueFromList(_xManifests, urlString.c_str(), L"soundLog"), 
                                    logPath + SSPKHelpers::GetAttributeValueFromList(_xManifests, urlString.c_str(), L"videoLog"));
        IXDrm* pXDrm = NULL;
        XDRM_CreateInstance(&pXDrm);
        pXDrm->SetDiagDelegate(&callback);

        //Play the file and wait for callbacks
        PKTEST_ASSERT_MSG_EXIT(StartSmoothObject(urlString, false), "Unable to start  the streaming objects" );

        _smoothObject->Play();
        //Play for 2 minutes (or less if the video is shorter)
        _smoothObject->WaitForMediaEnded(KEYROTATION_WAIT_FOR_MEDIA_COMPLETED);
        
        
        exit:
        //Cleanup
        if(pXDrm)
        {
            pXDrm->SetDiagDelegate(NULL);
        }
        
        StopSmoothObject();        
    
        return;
    }    
};
