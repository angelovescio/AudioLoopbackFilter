//------------------------------------------------------------------------------
// File: AudioLoopback.cpp
//
//
// Copyright (c) Corey Auger.  All rights reserved.
//------------------------------------------------------------------------------

//Incorporates adds by vesh to work with ffmpeg as a DirectShow filter
#include <windows.h>
#include <streams.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>

#include <math.h>
#include <mmreg.h>
#include <msacm.h>
#include <iostream>
#include <fstream>


#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#define _AUDIOSYNTH_IMPLEMENTATION_

#include "DynSrc.h"
#include "IAudioLoopbackFilter.h"
#include "AudioLoopbackFilter.h"

#include <cstdio>
#include <cstdarg>

// Quick and dirty debug..
static FILE* f = NULL;
void Log( const char* frmt, ... ){
#ifdef _DEBUG
    char buf[2048];
    va_list ptr;
    va_start(ptr,frmt);
    vsprintf_s(buf,frmt,ptr);
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
    if (f == NULL)
    {
        f  = fopen("_aloop.txt", "w");
    }
    fprintf(f,"%s",buf);
    fflush(f);
#endif
}



// setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{ &MEDIATYPE_Audio      // clsMajorType
, &MEDIASUBTYPE_NULL }; // clsMinorType

const AMOVIESETUP_PIN sudOpPin =
{ L"Output"          // strName
, FALSE              // bRendered
, TRUE               // bOutput
, FALSE              // bZero
, FALSE              // bMany
, &CLSID_NULL        // clsConnectsToFilter
, L"Input"           // strConnectsToPin
, 1                  // nTypes
, &sudOpPinTypes };  // lpTypes

const AMOVIESETUP_FILTER sudSynth =
{ &CLSID_AudioLoopbackFilter     // clsID
, L"Audio Loopback" // strName
, MERIT_UNLIKELY       // dwMerit
, 1                    // nPins
, &sudOpPin };         // lpPin

// -------------------------------------------------------------------------
// g_Templates
// -------------------------------------------------------------------------
// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {

    { L"Audio Loopback"
    , &CLSID_AudioLoopbackFilter
    , CAudioLoopbackFilter::CreateInstance
    , NULL
    , &sudSynth }
  ,
    //{ L"Audio Loopback Property Page"
    //, &CLSID_AudioLoopbackPropertyPage
    //, CAudioLoopbackProperties::CreateInstance }

};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// There are 8 bits in a byte.
const DWORD BITS_PER_BYTE = 8;


// -------------------------------------------------------------------------
// CSynthFilter, the main filter object
// -------------------------------------------------------------------------
//
// CreateInstance
//
// The only allowed way to create Synthesizers

CUnknown * WINAPI CAudioLoopbackFilter::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr) 
{
    ASSERT(phr);
    Log("Printing: %d\n", __LINE__);
    CUnknown *punk = new CAudioLoopbackFilter(lpunk, phr);
    if (punk == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return punk;
}


CAudioLoopbackFilter::CAudioLoopbackFilter(LPUNKNOWN lpunk, HRESULT *phr)
    : CDynamicSource(NAME("Audio Loopback Filter"),lpunk, CLSID_AudioLoopbackFilter, phr)
    , CPersistStream(lpunk, phr)
{
    Log("Printing: %d\n", __LINE__);
    m_paStreams = (CDynamicSourceStream **) new CSynthStream*[1];
    if (m_paStreams == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }

    m_paStreams[0] = new CAudioLoopbackPin(phr, this, L"Audio Loopback Pin");
    if (m_paStreams[0] == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
        return;
    }
 
}


CAudioLoopbackFilter::~CAudioLoopbackFilter(void) 
{
    //
    //  Base class will free our pins
    //
}


//
// NonDelegatingQueryInterface
//
// Reveal our property page, persistance, and control interfaces
STDMETHODIMP CAudioLoopbackFilter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    Log("Printing: %d\n", __LINE__);
    if (riid == IID_IAudioLoopbackFilter) {
        return GetInterface((IAudioLoopbackFilter *) this, ppv);
    }
    else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);
    }
    else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } 
    else {
        return CDynamicSource::NonDelegatingQueryInterface(riid, ppv);
    }
}


//
// GetPages
//
STDMETHODIMP CAudioLoopbackFilter::GetPages(CAUUID * pPages) 
{
    Log("Printing: %d\n", __LINE__);
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_AudioLoopbackPropertyPage;

    return NOERROR;
}


// -------------------------------------------------------------------------
// --- IPersistStream ---
// -------------------------------------------------------------------------
#define WRITEOUT(var)   hr = pStream->Write(&var, sizeof(var), NULL); \
                        if (FAILED(hr)) return hr;

#define READIN(var)     hr = pStream->Read(&var, sizeof(var), NULL); \
                        if (FAILED(hr)) return hr;


STDMETHODIMP CAudioLoopbackFilter::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);
}


int CAudioLoopbackFilter::SizeMax ()
{
    return sizeof (int) * 8;
}


HRESULT CAudioLoopbackFilter::WriteToStream(IStream *pStream)
{
    Log("Printing: %d\n", __LINE__);
    CheckPointer(pStream,E_POINTER);
    HRESULT hr;
    int i, k;
    return hr;
}


HRESULT CAudioLoopbackFilter::ReadFromStream(IStream *pStream)
{
    Log("Printing: %d\n", __LINE__);
    CheckPointer(pStream,E_POINTER);
    if (GetSoftwareVersion() != mPS_dwFileVersion)
        return E_FAIL;

    HRESULT hr;
    int i, k;  

    return hr;
}


DWORD CAudioLoopbackFilter::GetSoftwareVersion(void)
{
    return 1;
}

//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CAudioLoopbackPin::SetFormat(AM_MEDIA_TYPE* pmt)
{
    Log("Printing: %d\n", __LINE__);
    // you "must" use this type now...
    // they call this...

    m_mt = *pmt;

    IPin* pin;
    ConnectedTo(&pin);
    if (pin)
    {
        IFilterGraph* pGraph = m_pParent->GetGraph();
        pGraph->Reconnect(this);
    }
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CAudioLoopbackPin::GetFormat(AM_MEDIA_TYPE** ppmt)
{
    Log("Printing: %d\n", __LINE__);
    *ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CAudioLoopbackPin::GetNumberOfCapabilities(int* piCount, int* piSize)
{
    Log("Printing: %d\n", __LINE__);
    *piCount = 1; // only allow one type currently...
    *piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);
    return S_OK;
}

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

HRESULT STDMETHODCALLTYPE CAudioLoopbackPin::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppMediaType, BYTE* pSCC)
{
    Log("Printing: %d\n", __LINE__);
    if (iIndex < 0)
        return E_INVALIDARG;
    if (iIndex > 0)
        return S_FALSE;
    if (pSCC == NULL)
        return E_POINTER;

    //WAVEFORMATEX* pAudioFormat = (WAVEFORMATEX*) CoTaskMemAlloc(sizeof(WAVEFORMATEX));

    *ppMediaType = CreateMediaType(&m_mt);
    if (*ppMediaType == NULL) return E_OUTOFMEMORY;

    DECLARE_PTR(WAVEFORMATEX, pAudioFormat, (*ppMediaType)->pbFormat);

    pAudioFormat->cbSize = 0;
    pAudioFormat->wFormatTag = WAVE_FORMAT_PCM;		// This is the wave format (needed for more than 2 channels)
    pAudioFormat->nSamplesPerSec = 44100;	// This is in hertz
    pAudioFormat->nChannels = 2;		// 1 for mono, 2 for stereo, and 4 because the camera puts out dual stereo
    pAudioFormat->wBitsPerSample = 16;	// 16-bit sound
    pAudioFormat->nBlockAlign = 2 * 16 / 8;
    pAudioFormat->nAvgBytesPerSec = 44100 * pAudioFormat->nBlockAlign;

    AM_MEDIA_TYPE* pm = *ppMediaType;

    pm->majortype = MEDIATYPE_Audio;
    pm->subtype = MEDIASUBTYPE_PCM;
    pm->formattype = FORMAT_WaveFormatEx;
    pm->bTemporalCompression = FALSE;
    pm->bFixedSizeSamples = TRUE;
    pm->lSampleSize = pAudioFormat->nBlockAlign;//appears reasonable http://github.com/tap/JamomaDSP/blob/2c80c487c6e560d959dd85e7d2bcca3a19ce9b26/src/os/win/DX/BaseClasses/mtype.cpp
    pm->cbFormat = sizeof(WAVEFORMATEX);
    pm->pUnk = NULL;

    AUDIO_STREAM_CONFIG_CAPS* pASCC = (AUDIO_STREAM_CONFIG_CAPS*)pSCC;
    ZeroMemory(pSCC, sizeof(AUDIO_STREAM_CONFIG_CAPS));

    // Set the audio capabilities
    pASCC->guid = MEDIATYPE_Audio;
    pASCC->ChannelsGranularity = 1;
    pASCC->MaximumChannels = 2;
    pASCC->MinimumChannels = 2;
    pASCC->MaximumSampleFrequency = 44100;
    pASCC->BitsPerSampleGranularity = 16;
    pASCC->MaximumBitsPerSample = 16;
    pASCC->MinimumBitsPerSample = 16;
    pASCC->MinimumSampleFrequency = 44100;
    pASCC->SampleFrequencyGranularity = 11025;

    return S_OK;
}
//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////


HRESULT CAudioLoopbackPin::Set(REFGUID guidPropSet, DWORD dwID, void* pInstanceData,
    DWORD cbInstanceData, void* pPropData, DWORD cbPropData)
{// Set: Cannot set any properties.
    Log("Printing: %d\n", __LINE__);
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CAudioLoopbackPin::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void* pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void* pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD* pcbReturned     // Return the size of the property.
)
{
    Log("Printing: %d\n", __LINE__);
    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;

    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.

    *(GUID*)pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CAudioLoopbackPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET;
    return S_OK;
}


// -------------------------------------------------------------------------
// IAudioLoopbackFilter, the control interface for the synthesizer
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// CAudioLoopbackPin, the output pin
// -------------------------------------------------------------------------

//
// CAudioLoopbackPin::Constructor
//
CAudioLoopbackPin::CAudioLoopbackPin(HRESULT *phr, CAudioLoopbackFilter *pParent, LPCWSTR pName)
    : CDynamicSourceStream(NAME("AudioLoopback Pin"),phr, pParent, pName)
    , m_hPCMToMSADPCMConversionStream(NULL)
    , m_dwTempPCMBufferSize(0)
    , m_fFirstSampleDelivered(FALSE)
    , m_llSampleMediaTimeStart(0) 
	, m_dwAdviseToken(NULL)
{
    Log("Printing: %d\n", __LINE__);
    ASSERT(phr);

	m_Loopback = new CAudioLoopback(pParent->pStateLock());

	pParent->m_Loopback = m_Loopback;
    if (m_Loopback == NULL) {
        *phr = E_OUTOFMEMORY;
        return;
    }

    m_pParent = pParent;
}


//
// CAudioLoopbackPin::Destructor
CAudioLoopbackPin::~CAudioLoopbackPin(void) 
{
    Log("Printing: %d\n", __LINE__);
	delete m_Loopback;
}
HRESULT CAudioLoopbackPin::QueryInterface(REFIID riid, void** ppv)
{
    Log("Printing: %d\n", __LINE__);
    // Standard OLE stuff
    if (riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*)this;
    else if (riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*)this;
    else
        return CDynamicSourceStream::QueryInterface(riid, ppv);

    AddRef();
    return S_OK;
}
//
// Notify
// Ignore quality management messages sent from the downstream filter
STDMETHODIMP CAudioLoopbackPin::Notify(IBaseFilter* pSender, Quality q)
{
    return E_NOTIMPL;
} // Notify
//
// FillBuffer
//
// Stuffs the buffer with data
HRESULT CAudioLoopbackPin::FillBuffer(IMediaSample *pms) 
{
    Log("Printing: %d\n", __LINE__);
	// Try to enter the semaphore gate.
    DWORD dwWaitResult = WaitForSingleObject( m_hSemaphore, INFINITE);

    CheckPointer(pms,E_POINTER);

    BYTE *pData;

    HRESULT hr = pms->GetPointer(&pData);
    if (FAILED(hr)) {
        return hr;
    }

    // This function must hold the state lock because it calls
    // FillPCMAudioBuffer().
    CAutoLock lStateLock(m_pParent->pStateLock());
    
    // This lock must be held because this function uses
    // m_dwTempPCMBufferSize, m_hPCMToMSADPCMConversionStream,
    // m_rtSampleTime, m_fFirstSampleDelivered and
    // m_llSampleMediaTimeStart.
    CAutoLock lShared(&m_cSharedState);

    WAVEFORMATEX* pwfexCurrent = (WAVEFORMATEX*)m_mt.Format();

    if (WAVE_FORMAT_PCM == pwfexCurrent->wFormatTag) 
    {
        //m_Synth->FillPCMAudioBuffer(*pwfexCurrent, pData, pms->GetSize());
		long size = pms->GetSize();
		m_Loopback->FillPCMAudioBuffer(*pwfexCurrent, pData, size);

		//if( size == 0 )return NOERROR;

        hr = pms->SetActualDataLength(size);
        if (FAILED(hr))
            return hr;

    } 
    else 
    {
        Log("Printing: %s\n", "WTF is this shit?");

        // This filter only supports two audio formats: PCM and ADPCM. 
        /*
		ASSERT(WAVE_FORMAT_ADPCM == pwfexCurrent->wFormatTag);

        // Create PCM audio samples and then compress them.  We use the
        // Audio Compression Manager (ACM) API to convert the samples to 
        // the ADPCM format. 

        ACMSTREAMHEADER ACMStreamHeader;

        ACMStreamHeader.cbStruct = sizeof(ACMStreamHeader);
        ACMStreamHeader.fdwStatus = 0;
        ACMStreamHeader.dwUser = 0;
        ACMStreamHeader.cbSrcLength = m_dwTempPCMBufferSize;
        ACMStreamHeader.cbSrcLengthUsed = 0;
        ACMStreamHeader.dwSrcUser = 0; 
        ACMStreamHeader.pbDst = pData; 
        ACMStreamHeader.cbDstLength = pms->GetSize(); 
        ACMStreamHeader.cbDstLengthUsed = 0; 
        ACMStreamHeader.dwDstUser = 0; 
        ACMStreamHeader.pbSrc = new BYTE[m_dwTempPCMBufferSize];
        if (NULL == ACMStreamHeader.pbSrc) {
            return E_OUTOFMEMORY;
        }

        WAVEFORMATEX wfexPCMAudio;

        DerivePCMFormatFromADPCMFormatStructure(*pwfexCurrent, &wfexPCMAudio);

        m_Synth->FillPCMAudioBuffer(wfexPCMAudio,
                                    ACMStreamHeader.pbSrc,
                                    ACMStreamHeader.cbSrcLength);

        MMRESULT mmr = acmStreamPrepareHeader(m_hPCMToMSADPCMConversionStream,   
                                              &ACMStreamHeader,
                                              0);
  
        // acmStreamPrepareHeader() returns 0 if no errors occur.
        if (mmr != 0) {
            delete [] ACMStreamHeader.pbSrc;
            return E_FAIL;
        }

        mmr = acmStreamConvert(m_hPCMToMSADPCMConversionStream,
                               &ACMStreamHeader,
                               ACM_STREAMCONVERTF_BLOCKALIGN);

        MMRESULT mmrUnprepare = acmStreamUnprepareHeader(m_hPCMToMSADPCMConversionStream,   
                                                         &ACMStreamHeader,
                                                         0);

        delete [] ACMStreamHeader.pbSrc;

        // acmStreamConvert() andacmStreamUnprepareHeader() returns 0 if no errors occur.
        if ((mmr != 0) || (mmrUnprepare != 0)) {
            return E_FAIL;
        }

        hr = pms->SetActualDataLength(ACMStreamHeader.cbDstLengthUsed);
        if (FAILED(hr)) {
            return hr;
        }
		*/
    }

    // Set the sample's time stamps.  
    CRefTime rtStart = m_rtSampleTime;

    m_rtSampleTime = rtStart + (REFERENCE_TIME)(UNITS * pms->GetActualDataLength()) / 
                     (REFERENCE_TIME)pwfexCurrent->nAvgBytesPerSec;

    hr = pms->SetTime((REFERENCE_TIME*)&rtStart, (REFERENCE_TIME*)&m_rtSampleTime);

    if (FAILED(hr)) {
        return hr;
    }

    // Set the sample's properties.
    hr = pms->SetPreroll(FALSE);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pms->SetMediaType(NULL);
    if (FAILED(hr)) {
        return hr;
    }
   
    hr = pms->SetDiscontinuity(!m_fFirstSampleDelivered);
    if (FAILED(hr)) {
        return hr;
    }
    
    hr = pms->SetSyncPoint(!m_fFirstSampleDelivered);
    if (FAILED(hr)) {
        return hr;
    }

    LONGLONG llMediaTimeStart = m_llSampleMediaTimeStart;
    
    DWORD dwNumAudioSamplesInPacket = (pms->GetActualDataLength() * BITS_PER_BYTE) /
                                      (pwfexCurrent->nChannels * pwfexCurrent->wBitsPerSample);

    LONGLONG llMediaTimeStop = m_llSampleMediaTimeStart + dwNumAudioSamplesInPacket;

    hr = pms->SetMediaTime(&llMediaTimeStart, &llMediaTimeStop);
    if (FAILED(hr)) {
        return hr;
    }

    m_llSampleMediaTimeStart = llMediaTimeStop;
    m_fFirstSampleDelivered = TRUE;

    return NOERROR;
}


//
// Format Support
//

//
// GetMediaType
//
HRESULT CAudioLoopbackPin::GetMediaType(CMediaType *pmt) 
{
    Log("Printing: %d\n", __LINE__);
    CheckPointer(pmt,E_POINTER);

    // The caller must hold the state lock because this function
    // calls get_OutputFormat() and GetPCMFormatStructure().
    // The function assumes that the state of the m_Synth
    // object does not change between the two calls.  The
    // m_Synth object's state will not change if the 
    // state lock is held.
    ASSERT(CritCheckIn(m_pParent->pStateLock()));

    //WAVEFORMATEX *pwfex;
	WAVEFORMATEX *pwfex = m_pParent->m_Loopback->Format();

    SYNTH_OUTPUT_FORMAT ofCurrent = SYNTH_OF_PCM;    
    if(SYNTH_OF_PCM == ofCurrent)
    {
        pwfex = (WAVEFORMATEX *) pmt->AllocFormatBuffer(sizeof(WAVEFORMATEX));
        if(NULL == pwfex)
        {
            return E_OUTOFMEMORY;
        }

        m_Loopback->GetPCMFormatStructure(pwfex);
    }
    else if(SYNTH_OF_MS_ADPCM == ofCurrent)
    {
        DWORD dwMaxWAVEFORMATEXSize;

        MMRESULT mmr = acmMetrics(NULL, ACM_METRIC_MAX_SIZE_FORMAT, 
                                 (void*)&dwMaxWAVEFORMATEXSize);

        // acmMetrics() returns 0 if no errors occur.
        if(0 != mmr)
        {
            return E_FAIL;
        }

        pwfex = (WAVEFORMATEX *) pmt->AllocFormatBuffer(dwMaxWAVEFORMATEXSize);
        if(NULL == pwfex)
        {
            return E_OUTOFMEMORY;
        }

        WAVEFORMATEX wfexSourceFormat;
        m_Loopback->GetPCMFormatStructure(&wfexSourceFormat);

        ZeroMemory(pwfex, dwMaxWAVEFORMATEXSize);
        pwfex->wFormatTag = WAVE_FORMAT_ADPCM;
        pwfex->cbSize = (USHORT)(dwMaxWAVEFORMATEXSize - sizeof(WAVEFORMATEX));
        pwfex->nChannels = wfexSourceFormat.nChannels;
        pwfex->nSamplesPerSec = wfexSourceFormat.nSamplesPerSec;

        mmr = acmFormatSuggest(NULL,
                               &wfexSourceFormat,
                               pwfex,
                               dwMaxWAVEFORMATEXSize,
                               ACM_FORMATSUGGESTF_WFORMATTAG | 
                                    ACM_FORMATSUGGESTF_NSAMPLESPERSEC | 
                                    ACM_FORMATSUGGESTF_NCHANNELS);
        // acmFormatSuggest() returns 0 if no errors occur.
        if(0 != mmr)
        {
            return E_FAIL;
        }

    }
    else
    {
        return E_UNEXPECTED;
    }

    return CreateAudioMediaType(pwfex, pmt, FALSE);
}


HRESULT CAudioLoopbackPin::CompleteConnect(IPin *pReceivePin)
{
    Log("Printing: %d\n", __LINE__);
    // This lock must be held because this function uses
    // m_hPCMToMSADPCMConversionStream, m_fFirstSampleDelivered 
    // and m_llSampleMediaTimeStart.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr;
    WAVEFORMATEX *pwfexCurrent = (WAVEFORMATEX*)m_mt.Format();

    if(WAVE_FORMAT_PCM == pwfexCurrent->wFormatTag)
    {
       
    }
    else if(WAVE_FORMAT_ADPCM == pwfexCurrent->wFormatTag)
    {
        WAVEFORMATEX wfexSourceFormat;

        DerivePCMFormatFromADPCMFormatStructure(*pwfexCurrent, &wfexSourceFormat);
        
        MMRESULT mmr = acmStreamOpen(&m_hPCMToMSADPCMConversionStream,
                                    NULL,
                                    &wfexSourceFormat,
                                    pwfexCurrent,
                                    NULL,
                                    0,
                                    0,
                                    ACM_STREAMOPENF_NONREALTIME);
        // acmStreamOpen() returns 0 if an no errors occur.                              
        if(mmr != 0)
        {
            return E_FAIL;
        }
    }
    else
    {
        ASSERT(NULL == m_hPCMToMSADPCMConversionStream);

    }

    hr = CDynamicSourceStream::CompleteConnect(pReceivePin);
    if(FAILED(hr))
    {
        if(WAVE_FORMAT_ADPCM == pwfexCurrent->wFormatTag)
        {
            // acmStreamClose() should never fail because m_hPCMToMSADPCMConversionStream
            // holds a valid ACM stream handle and all operations using the handle are 
            // synchronous.
            EXECUTE_ASSERT(0 == acmStreamClose(m_hPCMToMSADPCMConversionStream, 0));
            m_hPCMToMSADPCMConversionStream = NULL;
        }

        return hr;
    }

    m_fFirstSampleDelivered = FALSE;
    m_llSampleMediaTimeStart = 0;

    return S_OK;
}


void CAudioLoopbackPin::DerivePCMFormatFromADPCMFormatStructure(const WAVEFORMATEX& wfexADPCM, 
                                                           WAVEFORMATEX* pwfexPCM)
{
    Log("Printing: %d\n", __LINE__);
    ASSERT(pwfexPCM);
    if (!pwfexPCM)
        return;

    pwfexPCM->wFormatTag = WAVE_FORMAT_PCM; 
    pwfexPCM->wBitsPerSample = 16;
    pwfexPCM->cbSize = 0;

    pwfexPCM->nChannels = wfexADPCM.nChannels;
    pwfexPCM->nSamplesPerSec = wfexADPCM.nSamplesPerSec;

    pwfexPCM->nBlockAlign = (WORD)((pwfexPCM->nChannels * pwfexPCM->wBitsPerSample) / BITS_PER_BYTE);
    pwfexPCM->nAvgBytesPerSec = pwfexPCM->nBlockAlign * pwfexPCM->nSamplesPerSec;
}


HRESULT CAudioLoopbackPin::BreakConnect(void)
{
    Log("Printing: %d\n", __LINE__);
    // This lock must be held because this function uses
    // m_hPCMToMSADPCMConversionStream and m_dwTempPCMBufferSize.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr = CDynamicSourceStream::BreakConnect();
    if(FAILED(hr))
    {
        return hr;
    }

    if(NULL != m_hPCMToMSADPCMConversionStream)
    {
        // acmStreamClose() should never fail because m_hPCMToMSADPCMConversionStream
        // holds a valid ACM stream handle and all operations using the handle are 
        // synchronous.
        EXECUTE_ASSERT(0 == acmStreamClose(m_hPCMToMSADPCMConversionStream, 0));
        m_hPCMToMSADPCMConversionStream = NULL;
        m_dwTempPCMBufferSize = 0;
    }
	if(m_dwAdviseToken){
		m_pClock->Unadvise(m_dwAdviseToken);
		m_dwAdviseToken = NULL;
	}

    return S_OK;
}


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what format we agreed to.
// Then we can ask for buffers of the correct size to contain them.
HRESULT CAudioLoopbackPin::DecideBufferSize(IMemAllocator *pAlloc,
                                       ALLOCATOR_PROPERTIES *pProperties)
{
    Log("Printing: %d\n", __LINE__);
    // The caller should always hold the shared state lock 
    // before calling this function.  This function must hold 
    // the shared state lock because it uses m_hPCMToMSADPCMConversionStream
    // m_dwTempPCMBufferSize.
    ASSERT(CritCheckIn(&m_cSharedState));

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    WAVEFORMATEX *pwfexCurrent = (WAVEFORMATEX*)m_mt.Format();
	//WAVEFORMATEX *pwfexCurrent = m_pParent->m_Loopback->Format();


    if(WAVE_FORMAT_PCM == pwfexCurrent->wFormatTag)
    {
        pProperties->cbBuffer = WaveBufferSize;
    }
    else
    {
        // This filter only supports two formats: PCM and ADPCM. 
        ASSERT(WAVE_FORMAT_ADPCM == pwfexCurrent->wFormatTag);

        pProperties->cbBuffer = pwfexCurrent->nBlockAlign;

        MMRESULT mmr = acmStreamSize(m_hPCMToMSADPCMConversionStream,
                                     pwfexCurrent->nBlockAlign,
                                     &m_dwTempPCMBufferSize,
                                     ACM_STREAMSIZEF_DESTINATION);

        // acmStreamSize() returns 0 if no error occurs.
        if(0 != mmr)
        {
            return E_FAIL;
        }
    }

    int nBitsPerSample = pwfexCurrent->wBitsPerSample;
    int nSamplesPerSec = pwfexCurrent->nSamplesPerSec;
    int nChannels = pwfexCurrent->nChannels;

    pProperties->cBuffers = (nChannels * nSamplesPerSec * nBitsPerSample) / 
                            (pProperties->cbBuffer * BITS_PER_BYTE);

    // Get 1/2 second worth of buffers
    pProperties->cBuffers /= 2;
    if(pProperties->cBuffers < 1)
        pProperties->cBuffers = 1 ;

    // Ask the allocator to reserve us the memory
    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    // Is this allocator unsuitable
    if(Actual.cbBuffer < pProperties->cbBuffer)
    {
        return E_FAIL;
    }

    return NOERROR;
}


//
// Active
//
HRESULT CAudioLoopbackPin::Active(void)
{
    Log("Printing: %d\n", __LINE__);
    // This lock must be held because the function
    // uses m_rtSampleTime, m_fFirstSampleDelivered
    // and m_llSampleMediaTimeStart.
    CAutoLock lShared(&m_cSharedState);

    HRESULT hr = CDynamicSourceStream::Active();
    if(FAILED(hr))
    {
        return hr;
    }

    m_rtSampleTime = 0;
    m_fFirstSampleDelivered = FALSE;
    m_llSampleMediaTimeStart = 0;

    return NOERROR;
}


HRESULT	CAudioLoopbackPin::Run(REFERENCE_TIME tStart)
{	
	if( !m_dwAdviseToken ){
		HRESULT hr = S_OK;
		m_hSemaphore = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
		const REFERENCE_TIME rtPeriodTime = m_Loopback->m_hnsDefaultDevicePeriod;

		hr = m_pClock->AdvisePeriodic(tStart+rtPeriodTime, rtPeriodTime,  (HSEMAPHORE)m_hSemaphore, &m_dwAdviseToken);		
		//hr = m_pClock->AdvisePeriodic(1, rtPeriodTime,  (HSEMAPHORE)m_hSemaphore, &m_dwAdviseToken);
		if( hr != S_OK ){
			// TODO: log this.
			return hr;
		}
	}
	//return CBaseOutputPin::Run(0);
	return CBaseOutputPin::Run(tStart);
}


HRESULT CAudioLoopbackPin::SetSyncSource(IReferenceClock *pClock)
{
    Log("Printing: %d\n", __LINE__);
	m_pClock = pClock;
	return S_OK;
}


// -------------------------------------------------------------------------
// CAudioLoopback
// -------------------------------------------------------------------------
HRESULT get_default_device(IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pMMDeviceEnumerator;
    Log("Printing: %d\n", __LINE__);
    // activate a device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
		Log( "CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x",hr);
        return hr;
    }

    // get the default render endpoint
    hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, ppMMDevice);
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
		Log("IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: hr = 0x%08x",hr);
        return hr;
    }

    return S_OK;
}



CAudioLoopback::CAudioLoopback(CCritSec* pStateLock)
	:pnFrames(0)
	,bFirstPacket(true)
{
    Log("Printing: %d\n", __LINE__);
	Initialize();	// should take out of constructor
    
}
CAudioLoopback::~CAudioLoopback(){
    Log("Printing: %d\n", __LINE__);
	AvRevertMmThreadCharacteristics(m_hTask);
	m_pAudioClient->Release();
	m_pAudioCaptureClient->Release();
	CoTaskMemFree(m_pwfx);
	delete[] m_silenceBuf;
}

void CAudioLoopback::GetPCMFormatStructure(WAVEFORMATEX* pwfex)
{
    Log("Printing: %d\n", __LINE__);
	ASSERT(pwfex);
    if (!pwfex)
        return;
    pwfex->wFormatTag = WAVE_FORMAT_PCM;
	pwfex->nChannels = m_pwfx->nChannels;
	pwfex->nSamplesPerSec = m_pwfx->nSamplesPerSec;
	pwfex->wBitsPerSample = m_pwfx->wBitsPerSample;        
	pwfex->nBlockAlign = m_pwfx->nBlockAlign;
    pwfex->nAvgBytesPerSec = pwfex->nBlockAlign * pwfex->nSamplesPerSec;
	pwfex->cbSize = m_pwfx->cbSize;
}

HRESULT CAudioLoopback::Initialize()
{
    Log("Printing: %d\n", __LINE__);
	IMMDevice *pMMDevice;
	HRESULT hr;

    hr = get_default_device(&pMMDevice);
    if (FAILED(hr)) {
        return hr;
    }
	
	// activate an IAudioClient    
	hr = pMMDevice->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL, NULL,
        (void**)&m_pAudioClient
    );
    if (FAILED(hr)) {
        Log("IMMDevice::Activate(IAudioClient) failed: hr = 0x%08x",hr);
        return hr;
    }
    
    // get the default device periodicity
    
    hr = m_pAudioClient->GetDevicePeriod(&m_hnsDefaultDevicePeriod, NULL);
    if (FAILED(hr)) {
		Log("IAudioClient::GetDevicePeriod failed: hr = 0x%08x", hr);
        m_pAudioClient->Release();
        return hr;
    }
	Log("GetDevicePeriod set %d\n",	m_hnsDefaultDevicePeriod);	
	Log("Milliseconds beween fire %d\n", (int)(m_hnsDefaultDevicePeriod / 2 / (10 * 1000))); // convert to milliseconds

    // get the default device format
    
    hr = m_pAudioClient->GetMixFormat(&m_pwfx);
    if (FAILED(hr)) {
        Log("IAudioClient::GetMixFormat failed: hr = 0x%08x",hr);        
        return hr;
    }

    //if (bInt16) {
	if( true ){
        // coerce int-16 wave format
        // can do this in-place since we're not changing the size of the format
        // also, the engine will auto-convert from float to int for us
        switch (m_pwfx->wFormatTag) {
            case WAVE_FORMAT_IEEE_FLOAT:
                m_pwfx->wFormatTag = WAVE_FORMAT_PCM;
                m_pwfx->wBitsPerSample = 16;
                m_pwfx->nBlockAlign = m_pwfx->nChannels * m_pwfx->wBitsPerSample / 8;
                m_pwfx->nAvgBytesPerSec = m_pwfx->nBlockAlign * m_pwfx->nSamplesPerSec;
                break;

            case WAVE_FORMAT_EXTENSIBLE:
                {
                    // naked scope for case-local variable
                    PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(m_pwfx);
                    if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) {
                        pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                        pEx->Samples.wValidBitsPerSample = 16;
                        m_pwfx->wBitsPerSample = 16;
                        m_pwfx->nBlockAlign = m_pwfx->nChannels * m_pwfx->wBitsPerSample / 8;
                        m_pwfx->nAvgBytesPerSec = m_pwfx->nBlockAlign * m_pwfx->nSamplesPerSec;
                    } else {
                        Log("Don't know how to coerce mix format to int-16\n");
                        return E_UNEXPECTED;
                    }
                }
                break;

            default:
                Log("Don't know how to coerce WAVEFORMATEX with wFormatTag = 0x%08x to int-16",m_pwfx->wFormatTag);
                return E_UNEXPECTED;
        }
    }

	Log("wFormatTag: %d\n",m_pwfx->wFormatTag);
	Log("wBitsPerSampl: %d\n",m_pwfx->wBitsPerSample);
	Log("nChannels: %d\n",m_pwfx->nChannels);
	Log("wBitsPerSample: %d\n",m_pwfx->wBitsPerSample);
	Log("nBlockAlign: %d\n",m_pwfx->nBlockAlign);
	Log("nSamplesPerSec: %d\n",m_pwfx->nSamplesPerSec);
	Log("nAvgBytesPerSec: %d\n",m_pwfx->nAvgBytesPerSec);

	m_silenceBuf = new BYTE[SILENCE_BUF_SIZE];	
	memset(m_silenceBuf, 128, SILENCE_BUF_SIZE);
    

    UINT32 nBlockAlign = m_pwfx->nBlockAlign;
    
    // call IAudioClient::Initialize
    // note that AUDCLNT_STREAMFLAGS_LOOPBACK and AUDCLNT_STREAMFLAGS_EVENTCALLBACK
    // do not work together...
    // the "data ready" event never gets set
    // so we're going to do a timer-driven loop
    hr = m_pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0, m_pwfx, 0
    );
    if (FAILED(hr)) {
		Log( "IAudioClient::Initialize failed: hr = 0x%08x", hr);
        m_pAudioClient->Release();
        return hr;
    }
    
    // activate an IAudioCaptureClient
    hr = m_pAudioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**)&m_pAudioCaptureClient
    );
    if (FAILED(hr)) {
		Log( "IAudioClient::GetService(IAudioCaptureClient) failed: hr 0x%08x", hr);
        return hr;
    }
    
    // register with MMCSS
    DWORD nTaskIndex = 0;
    m_hTask = AvSetMmThreadCharacteristicsW(L"Capture", &nTaskIndex);
    if (NULL == m_hTask) {
        DWORD dwErr = GetLastError();
        Log("AvSetMmThreadCharacteristics failed: last error = %u\n", dwErr);
        return HRESULT_FROM_WIN32(dwErr);
    }    

    // call IAudioClient::Start
    hr = m_pAudioClient->Start();
    if (FAILED(hr)) {
        Log("IAudioClient::Start failed: hr = 0x%08x\n", hr);
        return hr;
    } 
    Log("Ending: %s\n","CAudioLoopback::Initialize()");
}




HRESULT CAudioLoopback::FillPCMAudioBuffer(const WAVEFORMATEX& wfex, BYTE pBuf[], long& iSize)
{
    Log("Printing: %d\n", __LINE__);
	HRESULT hr = S_OK;
    // grab next audio chunk...	
    UINT32 nNextPacketSize;
    hr = m_pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize); // get next packet, if one is ready...
    if (FAILED(hr)) {
        Log("IAudioCaptureClient::GetNextPacketSize failed after %u frames: hr = 0x%08x\n", pnFrames, hr);
        m_pAudioClient->Stop();            
        return hr;
    }
       		
    // get the captured data
    BYTE *pData;
    UINT32 nNumFramesToRead;
    DWORD dwFlags;

	hr = m_pAudioCaptureClient->GetBuffer(
        &pData,
        &nNumFramesToRead,
        &dwFlags,
        NULL,
        NULL
    );         
        
    if (FAILED(hr)) {
        Log("IAudioCaptureClient::GetBuffer failed after %u frames: hr = 0x%08x\n", pnFrames, hr);
        m_pAudioClient->Stop();
        return hr;            
    }		

	if (0 == nNumFramesToRead) {
		// We have silence in this case...
		iSize = SILENCE_BUF_SIZE;
		memcpy(pBuf, m_silenceBuf, iSize);  // (CA) - inject silence buffer..
		return S_OK;
	}
	pnFrames += nNumFramesToRead; // increment total count...		

	// lBytesToWrite typically 1792 bytes...
	LONG lBytesToWrite = nNumFramesToRead * m_pwfx->nBlockAlign; // nBlockAlign is "audio block size" or frame size, for one audio segment...	
	iSize = min(iSize, lBytesToWrite);	// (CA) NOTE: this sets the size of the refrence var
	memcpy(pBuf, pData, iSize);		
		        
    hr = m_pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
    if (FAILED(hr)) {
        Log("IAudioCaptureClient::ReleaseBuffer failed after %u frames: hr = 0x%08x\n", pnFrames, hr);
        m_pAudioClient->Stop();                 
        return hr;            
    }        
	return S_OK;	
}
// straight call to here on init, instead of to
// AMovieDllRegisterServer2 which is what the other fella does...
// which I assume is similar to this...maybe?
#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);
STDAPI AMovieSetupRegisterServer(CLSID   clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType = L"InprocServer32");
STDAPI AMovieSetupUnregisterServer(CLSID clsServer);
STDAPI RegisterFilters(BOOL bRegister)
{
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);

    if (0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp)))
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1,
        achFileName, NUMELMS(achFileName));

    hr = CoInitialize(0);
    if (bRegister)
    {
        hr = AMovieSetupRegisterServer(CLSID_AudioLoopbackFilter, L"DirectShow Loopback Adapter", achFileName, L"Both", L"InprocServer32");
    }

    if (SUCCEEDED(hr))
    {
        IFilterMapper2* fm = 0;
        hr = CreateComObject(CLSID_FilterMapper2, IID_IFilterMapper2, fm);
        if (SUCCEEDED(hr))
        {
            if (bRegister)
            {
                IMoniker* pMoniker = 0;
                REGFILTER2 rf2;
                rf2.dwVersion = 1;
                rf2.dwMerit = MERIT_DO_NOT_USE;
                rf2.cPins = 1;
                rf2.rgPins = &sudOpPin;
                hr = fm->RegisterFilter(CLSID_AudioLoopbackFilter, L"DirectShow Loopback Adapter", &pMoniker, &CLSID_AudioInputDeviceCategory, NULL, &rf2);
            }
            else
            {
                hr = fm->UnregisterFilter(&CLSID_AudioInputDeviceCategory, 0, CLSID_AudioLoopbackFilter);
            }
        }

        // release interface
        //
        if (fm)
            fm->Release();
    }

    if (SUCCEEDED(hr) && !bRegister)
        hr = AMovieSetupUnregisterServer(CLSID_AudioLoopbackFilter);

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
    return RegisterFilters(TRUE);
}

STDAPI DllUnregisterServer()
{
    return RegisterFilters(FALSE);
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


