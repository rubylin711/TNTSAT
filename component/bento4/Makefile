include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include
INCLUDE_PATH += -I$(EXTERNAL_BINARY_PREFIX_DIR)/drm

ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
EXTRA_CFLAGS +=  -DMT_DRM_SUPPORT
endif

ifeq ($(CONFIG_MT_VMX_OTT_SVP_SUPPORT),y)
EXTRA_CFLAGS += -DVMX_OTT_SVP
endif

INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc

EXTRA_CFLAGS += -D_REENTRANT -DAP4_PLATFORM_BYTE_ORDER=AP4_PLATFORM_BYTE_ORDER_LITTLE_ENDIAN

INCLUDE_PATH += -ISource/Config -ISource/C++/Core -ISource/C++/Crypto -ISource/C++/MetaData -ISource/C++/Codecs 

OBJS := Source/C++/Core/Ap4Stz2Atom.o Source/C++/Core/Ap4String.o Source/C++/Core/Ap4.o Source/C++/Core/Ap4TrexAtom.o Source/C++/Core/Ap4OhdrAtom.o Source/C++/Core/Ap4CommandFactory.o \
   Source/C++/Core/Ap4TimsAtom.o Source/C++/Core/Ap4FragmentSampleTable.o Source/C++/Core/Ap4SegmentBuilder.o Source/C++/Core/Ap4TrunAtom.o Source/C++/Core/Ap4Debug.o Source/C++/Core/Ap4StszAtom.o \
   Source/C++/Core/Ap4Ipmp.o Source/C++/Core/Ap4FtypAtom.o Source/C++/Core/Ap4Co64Atom.o Source/C++/Core/Ap4Protection.o Source/C++/Core/Ap4DescriptorFactory.o Source/C++/Core/Ap4Descriptor.o \
   Source/C++/Core/Ap4TfdtAtom.o Source/C++/Core/Ap4RtpHint.o Source/C++/Core/Ap4SchmAtom.o Source/C++/Core/Ap4HintTrackReader.o Source/C++/Core/Ap4IproAtom.o Source/C++/Core/Ap4Track.o \
   Source/C++/Core/Ap4IsltAtom.o Source/C++/Core/Ap4StscAtom.o Source/C++/Core/Ap4TrakAtom.o Source/C++/Core/Ap4UrlAtom.o Source/C++/Core/Ap4StssAtom.o Source/C++/Core/Ap4IodsAtom.o \
   Source/C++/Core/Ap4GrpiAtom.o Source/C++/Core/Ap4Command.o Source/C++/Core/Ap4Sample.o Source/C++/Core/Ap4SgpdAtom.o Source/C++/Core/Ap4Results.o Source/C++/Core/Ap4FrmaAtom.o \
   Source/C++/Core/Ap4Movie.o Source/C++/Core/Ap4UuidAtom.o Source/C++/Core/Ap4StsdAtom.o Source/C++/Core/Ap4FileCopier.o Source/C++/Core/Ap4OddaAtom.o Source/C++/Core/Ap4HmhdAtom.o \
   Source/C++/Core/Ap4EsDescriptor.o Source/C++/Core/Ap4AtomFactory.o Source/C++/Core/Ap4SmhdAtom.o Source/C++/Core/Ap4DataBuffer.o Source/C++/Core/Ap4OmaDcf.o Source/C++/Core/Ap4CttsAtom.o \
   Source/C++/Core/Ap4MovieFragment.o Source/C++/Core/Ap4SidxAtom.o Source/C++/Core/Ap4Marlin.o Source/C++/Core/Ap4ByteStream.o Source/C++/Core/Ap4TfraAtom.o Source/C++/Core/Ap4Expandable.o \
   Source/C++/Core/Ap4HdlrAtom.o Source/C++/Core/Ap4AinfAtom.o Source/C++/Core/Ap4ObjectDescriptor.o Source/C++/Core/Ap4PsshAtom.o Source/C++/Core/Ap4MoovAtom.o Source/C++/Core/Ap4MvhdAtom.o \
   Source/C++/Core/Ap4SampleDescription.o Source/C++/Core/Ap4DvccAtom.o Source/C++/Core/Ap4TrefTypeAtom.o Source/C++/Core/Ap4DecoderSpecificInfoDescriptor.o Source/C++/Core/Ap4Piff.o \
   Source/C++/Core/Ap4LinearReader.o Source/C++/Core/Ap4PdinAtom.o Source/C++/Core/Ap4OdheAtom.o Source/C++/Core/Ap48bdlAtom.o Source/C++/Core/Ap4Atom.o Source/C++/Core/Ap4HvccAtom.o \
   Source/C++/Core/Ap4Utils.o Source/C++/Core/Ap4IsfmAtom.o Source/C++/Core/Ap4SampleTable.o Source/C++/Core/Ap4SttsAtom.o Source/C++/Core/Ap4FileWriter.o Source/C++/Core/Ap4StcoAtom.o \
   Source/C++/Core/Ap4SthdAtom.o Source/C++/Core/Ap4RtpAtom.o Source/C++/Core/Ap4MdhdAtom.o Source/C++/Core/Ap4OdafAtom.o Source/C++/Core/Ap4AtomSampleTable.o Source/C++/Core/Ap4AvccAtom.o \
   Source/C++/Core/Ap4IsmaCryp.o Source/C++/Core/Ap4File.o Source/C++/Core/Ap4SaioAtom.o Source/C++/Core/Ap4SbgpAtom.o Source/C++/Core/Ap4TkhdAtom.o Source/C++/Core/Ap4SdpAtom.o \
   Source/C++/Core/Ap4SampleEntry.o Source/C++/Core/Ap4DrefAtom.o Source/C++/Core/Ap4MfroAtom.o Source/C++/Core/Ap4BlocAtom.o Source/C++/Core/Ap4SencAtom.o Source/C++/Core/Ap4IkmsAtom.o \
   Source/C++/Core/Ap4CommonEncryption.o Source/C++/Core/Ap4SaizAtom.o Source/C++/Core/Ap4ContainerAtom.o Source/C++/Core/Ap4DecoderConfigDescriptor.o Source/C++/Core/Ap4TfhdAtom.o \
   Source/C++/Core/Ap4Processor.o Source/C++/Core/Ap4Mpeg2Ts.o Source/C++/Core/Ap4ElstAtom.o Source/C++/Core/Ap4SampleSource.o Source/C++/Core/Ap4SyntheticSampleTable.o Source/C++/Core/Ap4MfhdAtom.o \
   Source/C++/Core/Ap4VmhdAtom.o Source/C++/Core/Ap4SLConfigDescriptor.o Source/C++/Core/Ap4EsdsAtom.o Source/C++/Core/Ap4TencAtom.o Source/C++/Core/Ap4NmhdAtom.o Source/C++/Core/Ap4MehdAtom.o \
   Source/C++/Core/Ap4Dec3Atom.o Source/C++/Crypto/Ap4StreamCipher.o Source/C++/Crypto/Ap4AesBlockCipher.o Source/C++/Crypto/Ap4KeyWrap.o Source/C++/Crypto/Ap4Hmac.o Source/C++/MetaData/Ap4MetaData.o \
   Source/C++/Codecs/Ap4HevcParser.o Source/C++/Codecs/Ap4BitStream.o Source/C++/Codecs/Ap4AvcParser.o Source/C++/Codecs/Ap4Mp4AudioInfo.o Source/C++/Codecs/Ap4NalParser.o \
   Source/C++/Codecs/Ap4AdtsParser.o Source/C++/System/StdC/Ap4StdCFileByteStream.o Source/C++/System/Posix/Ap4PosixRandom.o Source/C++/Core/Ap4Av1cAtom.o Source/C++/Core/Ap4VpccAtom.o \
   Source/C++/Core/Ap4Ac4Utils.o Source/C++/Core/Ap4Dac3Atom.o Source/C++/Core/Ap4Dac4Atom.o

LIB := libBento4

#HEADER_FILES=$(KWARE_DIR)/include/mtos

include ${SDK_DIR}/build/script/Makefile-lib.rule
