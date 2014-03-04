#include "StdAfx.h"
#include "MP3Player.h"
#include <dxerr8.h>
#include "../DirectXUtil.h"
#include <malloc.h>

CMP3Player* CMP3Player::m_pMP3Manager = NULL;

CMP3Player::CMP3Player(void)
: m_pMediaControl(NULL)
, m_pME(NULL)
, m_pSourceNext(NULL)
, m_pSourceCurrent(NULL)
, m_pMediaSeeking(NULL)
, m_pGraphBuilder(NULL)
{
}

CMP3Player::~CMP3Player(void)
{
	Finally();
}

BOOL CMP3Player::Initialize( HWND hWnd )
{
	HRESULT hr;
	m_hWnd = hWnd;

	if (FAILED (hr = CoInitialize(NULL)) )
		return FALSE;

	// Create DirectShow Graph
	if (FAILED (hr = CoCreateInstance(CLSID_FilterGraph, NULL,
		CLSCTX_INPROC, IID_IGraphBuilder,
		reinterpret_cast<void **>(&m_pGraphBuilder))) )
		return FALSE;

	// Get the IMediaControl Interface
	if (FAILED (m_pGraphBuilder->QueryInterface(IID_IMediaControl,
		reinterpret_cast<void **>(&m_pMediaControl))))
		return FALSE;

	// Get the IMediaControl Interface
	if (FAILED (m_pGraphBuilder->QueryInterface(IID_IMediaSeeking,
		reinterpret_cast<void **>(&m_pMediaSeeking))))
		return FALSE;
	
	m_pGraphBuilder->QueryInterface(IID_IMediaEventEx, (void **)&m_pME);

	m_pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0);  

	return TRUE;
}

BOOL CMP3Player::Finally()
{
	// Stop playback
	if (m_pMediaControl)
		m_pMediaControl->Stop();

	// Release all remaining pointers
	SAFE_RELEASE(m_pME);
	SAFE_RELEASE( m_pSourceNext);
	SAFE_RELEASE( m_pSourceCurrent);
	SAFE_RELEASE( m_pMediaSeeking);
	SAFE_RELEASE( m_pMediaControl);
	SAFE_RELEASE( m_pGraphBuilder);

	// Clean up COM
	CoUninitialize();

	return TRUE;
}

BOOL CMP3Player::OnPlayAudio( TCHAR* szName, BOOL bLooped /*= FALSE */ )
{					
	//WCHAR wstrFileName[MAX_PATH];    
	HRESULT hr;
	IPin *pPin = NULL;

	// Make sure that this file exists
	DWORD dwAttr = GetFileAttributes(szName);
	if (dwAttr == (DWORD) -1)
	{
		return FALSE;
	}

	// OPTIMIZATION OPPORTUNITY
	// This will open the file, which is expensive. To optimize, this
	// should be done earlier, ideally as soon as we knew this was the
	// next file to ensure that the file load doesn't add to the
	// filter swapping time & cause a hiccup.
	//
	// Add the new source filter to the graph. (Graph can still be running)
	hr = m_pGraphBuilder->AddSourceFilter(szName,szName, &m_pSourceNext);

	// Get the first output pin of the new source filter. Audio sources 
	// typically have only one output pin, so for most audio cases finding 
	// any output pin is sufficient.
	if (SUCCEEDED(hr)) {
		hr = m_pSourceNext->FindPin(L"Output", &pPin);  
	}

	// Stop the graph
	if (SUCCEEDED(hr)) {
		hr = m_pMediaControl->Stop();
	}

	// Break all connections on the filters. You can do this by adding 
	// and removing each filter in the graph
	if (SUCCEEDED(hr)) {
		IEnumFilters *pFilterEnum = NULL;

		if (SUCCEEDED(hr = m_pGraphBuilder->EnumFilters(&pFilterEnum))) {
			int iFiltCount = 0;
			int iPos = 0;

			// Need to know how many filters. If we add/remove filters during the
			// enumeration we'll invalidate the enumerator
			while (S_OK == pFilterEnum->Skip(1)) {
				iFiltCount++;
			}

			// Allocate space, then pull out all of the 
			IBaseFilter **ppFilters = reinterpret_cast<IBaseFilter **>
				(_alloca(sizeof(IBaseFilter *) * iFiltCount));
			pFilterEnum->Reset();

			while (S_OK == pFilterEnum->Next(1, &(ppFilters[iPos++]), NULL));
			SAFE_RELEASE(pFilterEnum);

			for (iPos = 0; iPos < iFiltCount; iPos++) {
				m_pGraphBuilder->RemoveFilter(ppFilters[iPos]);
				// Put the filter back, unless it is the old source
				if (ppFilters[iPos] != m_pSourceCurrent) {
					m_pGraphBuilder->AddFilter(ppFilters[iPos], NULL);
				}
				SAFE_RELEASE(ppFilters[iPos]);
			}
		}
	}

	// We have the new ouput pin. Render it
	if (SUCCEEDED(hr)) {
		hr = m_pGraphBuilder->Render(pPin);
		m_pSourceCurrent = m_pSourceNext;
		m_pSourceNext = NULL;
	}

	SAFE_RELEASE(pPin);
	SAFE_RELEASE(m_pSourceNext); // In case of errors

	// Re-seek the graph to the beginning
	if (SUCCEEDED(hr)) {
		LONGLONG llPos = 0;
		hr = m_pMediaSeeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,
			&llPos, AM_SEEKING_NoPositioning);
	} 

	// Start the graph
	if (SUCCEEDED(hr)) {
		hr = m_pMediaControl->Run();
	}

	// Release the old source filter.
	SAFE_RELEASE(m_pSourceCurrent);

	return TRUE;
}

BOOL CMP3Player::HandleGraphEvent()
{
	LONG evCode, evParam1, evParam2;
	HRESULT hr=S_OK;

	// Make sure that we don't access the media event interface
	// after it has already been released.
	if (!m_pME)
		return S_OK;

	// Process all queued events
	while(SUCCEEDED(m_pME->GetEvent(&evCode, (LONG_PTR *) &evParam1,
		(LONG_PTR *) &evParam2, 0)))
	{
		// Free memory associated with callback, since we're not using it
		hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);

		// If this is the end of the clip, reset to beginning
		/*if(EC_COMPLETE == evCode)
		{                           

			LONGLONG llPos = 0;        
			hr = m_pMediaSeeking->SetPositions(&llPos, AM_SEEKING_AbsolutePositioning,                               
				&llPos, AM_SEEKING_NoPositioning);       
			if(SUCCEEDED(hr))
			{        
				hr = m_pMediaSeeking->Run();    
			}
		}*/
	}

	return TRUE;
}