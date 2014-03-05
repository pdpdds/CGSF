#include "StdAfx.h"
//#include "dxstdafx.h"
#include "DirectXSystem.h"
#include "CommonStructure.h"
//#include <atlbase.h>
#include "CommDlg.h"
#include <stdlib.h>
#include <string>
#include <limits.h>
#include "SGManager.h"
#include "SGTable.h"
#include "SGUser.h"
#include "TextureMgr.h"
#include "PictureBox.h"
#include "psystem.h"
#include <cstdlib>
#include <ctime>
#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTenum.h"
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#include "DXUTCamera.h"
#include "SDKMisc.h"
#include "DXUTguiIME.h"
#include "DXUTRes.h"
#include "CasualGameManager.h"
#include "SevenGameMain.h"
#include "SevenGameProtocol.h"
#include "SFPacketProtocol.h"

CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CModelViewerCamera      g_Camera;
D3DXMATRIXA16           g_mView;
D3DXMATRIXA16           g_mWorld;

psys::PSystem* g_pFireEffect = 0;
Camera TheCamera(Camera::AIRCRAFT);

CasualGameManager* g_pCasualGameManager = 0;

#ifdef _DEBUG
#pragma comment(lib, "CEGUIBase-0_d.lib")
#pragma comment(lib, "CEGUIDirect3D9Renderer-0_d.lib")
#else
#pragma comment(lib, "CEGUIBase-0.lib")
#pragma comment(lib, "CEGUIDirect3D9Renderer-0.lib")
#endif

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "liblzf.lib")
#pragma comment(lib, "CasualGameGUI.lib")
#pragma comment(lib, "EngineLayer.lib")
#pragma comment(lib, "BaseLayer.lib")
#pragma comment(lib, "DatabaseLayer.lib")
#pragma comment(lib, "libprotobuf.lib")

IGameMain* g_pGameMain = 0;
//
bool Setup()
{
	// seed random number generator
	srand((unsigned int)time(0));

	d3d::BoundingBox boundingBox;
	boundingBox._min = D3DXVECTOR3(-10.0f, -10.0f, -10.0f);
	boundingBox._max = D3DXVECTOR3( 10.0f,  10.0f,  10.0f);

	D3DXVECTOR3 origin(0.0f, 10.0f, 50.0f);
	g_pFireEffect = new psys::Firework(&origin, 6000);
	g_pFireEffect->init(CDirectXSystem::GetInstance()->GetD3DDevice(), L"./res/flare.bmp");

	g_pGameMain = new SevenGameMain();
	g_pCasualGameManager = new CasualGameManager();
	
	IPacketProtocol* pProtocol = new SFPacketProtocol<SevenGameProtocol>;
	g_pCasualGameManager->Initialize(CDirectXSystem::GetInstance()->GetD3DDevice(), pProtocol);

	return true;
}
//--------------------------------------------------------------------------------------
// Rejects any D3D9 devices that aren't acceptable to the app by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
								 D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
		return false;

	// Must support pixel shader 2.0
	if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
	assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps ) );

	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
	{
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if( s_bFirstTime )
	{
		s_bFirstTime = false;
		if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
			DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
	}

	return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D9 resources that will live through a device reset (D3DPOOL_MANAGED)
// and aren't tied to the back buffer size
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
								void* pUserContext )
{
	HRESULT hr;

	CDXUTIMEEditBox::Initialize( DXUTGetHWND() );

	V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );

	// Setup the camera's view parameters
	D3DXVECTOR3 vecEye( 0.0f, 1.5f, -7.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.2f, 0.0f );
	D3DXVECTOR3 vecUp ( 0.0f, 1.0f, 0.0f );
	g_Camera.SetViewParams( &vecEye, &vecAt );
	D3DXMatrixLookAtLH( &g_mView, &vecEye, &vecAt, &vecUp );

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D9 resources that won't live through a device reset (D3DPOOL_DEFAULT) 
// or that are tied to the back buffer size 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
							   const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	D3DXMATRIX  matView;

	D3DXVECTOR3 vEye = D3DXVECTOR3( 0.0f, -0.0f, -1.0f );
	D3DXVECTOR3 vAt  = D3DXVECTOR3( 0.0f, -0.0f, 0.0f ); 
	D3DXVECTOR3 vUp  = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
	D3DXMatrixLookAtLH( &matView, &vEye, &vAt, &vUp );  
	pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	pd3dDevice->SetTransform( D3DTS_WORLD, &g_mWorld );
	pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );  
	pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );

	CDirectXSystem::GetInstance()->SetWidth(pBackBufferSurfaceDesc->Width);
	CDirectXSystem::GetInstance()->SetHeight(pBackBufferSurfaceDesc->Height);

	CDirectXSystem::GetInstance()->ResetDevice(pd3dDevice);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
	g_pCasualGameManager->Update();

	CDirectXSystem::GetInstance()->OnMove(fTime, fElapsedTime);
}

//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9ResetDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{	
	g_DialogResourceManager.OnD3D9LostDevice();

	CDirectXSystem::GetInstance()->LostDevice();
}

//--------------------------------------------------------------------------------------
// Release D3D9 resources created in the OnD3D9CreateDevice callback 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
	CDXUTIMEEditBox::Uninitialize( );

	g_DialogResourceManager.OnD3D9DestroyDevice();
}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
						 void* pUserContext )
{
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
	if( *pbNoFurtherProcessing )
		return 0;

	CDirectXSystem::GetInstance()->ProcessMessage( hWnd, uMsg, wParam, lParam );

	// Pass all remaining windows messages to camera so it can respond to user input
	g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );	

	return 0;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D9 device
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
	HRESULT hr;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	D3DXMATRIXA16 mWorldViewProjection;    
	D3DXMATRIX  g_matView;

	D3DXVECTOR3 vEye = D3DXVECTOR3( 0.0f, -0.0f, -1.0f );
	D3DXVECTOR3 vAt  = D3DXVECTOR3( 0.0f, -0.0f, 0.0f ); 
	D3DXVECTOR3 vUp  = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );  
	D3DXMatrixLookAtLH( &g_matView, &vEye, &vAt, &vUp );  
	pd3dDevice->SetTransform( D3DTS_VIEW, &g_matView );


	// Clear the render target and the zbuffer 
	V( pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(0, 45, 50, 170), 1.0f, 0) );

	CDirectXSystem::GetInstance()->OnRender(fElapsedTime);

	// Render the scene
	if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{				
		g_pCasualGameManager->Render(fElapsedTime );

		V( pd3dDevice->EndScene() );
	}	

	

	pd3dDevice->Present(NULL,NULL,NULL,NULL);
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{

	
	
}


CDirectXSystem* CDirectXSystem::m_pDirectXSystem = NULL;

CDirectXSystem::CDirectXSystem(void)
{
	m_pTextureMgr = NULL;
	m_pBackGround = NULL;
}

CDirectXSystem::~CDirectXSystem(void)
{
	if(m_pDirectXSystem)
		delete m_pDirectXSystem;
}

BOOL CDirectXSystem::Initialize()
{
	// Set DXUT callbacks
	DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
	DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
	DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
	DXUTSetCallbackD3D9FrameRender( OnFrameRender );
	DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
	DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
	DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( KeyboardProc );
	DXUTSetCallbackFrameMove( OnFrameMove );
	DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

	// Show the cursor and clip it when in full screen
	DXUTSetCursorSettings( false, false );

	// Initialize the sample framework and create the desired Win32 window and Direct3D 
	// device for the application. Calling each of these functions is optional, but they
	// allow you to set several options which control the behavior of the framework.
	DXUTInit( true, false, false ); // Parse the command line, handle the default hotkeys, and show msgboxes

	m_iWidth = 1024;
	m_iHeight = 768;

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	///////////////////////////////////////////////////////////////////////////////////////////

	DXUTSetHotkeyHandling( true, true, true );
	DXUTCreateWindow( L"Seven Game" );
	DXUTCreateDevice( true, m_iWidth, m_iHeight );

	return TRUE;
}

int CDirectXSystem::Run()
{
	InitEnvironment();
	
	// Pass control to the sample framework for handling the message pump and 
	// dispatching render calls. The sample framework will call your FrameMove 
	// and FrameRender callback when there is idle time between handling window messages.
	DXUTMainLoop();

	// Perform any application-level cleanup here. Direct3D device resources are released within the
	// appropriate callback functions and therefore don't require any cleanup code here.

	return DXUTGetExitCode();
}


BOOL CDirectXSystem::InitEnvironment()
{
	CPictureBox* pPictureBox;;
	m_pBackGround  = new CPictureBox();

	m_pBackGround->m_Info.iPosX = 0;
	m_pBackGround->m_Info.iPosY = 0;
	m_pBackGround->m_Info.iWidth = GetWidth();
	m_pBackGround->m_Info.iHeight = GetHeight();
	_tcscpy(m_pBackGround->m_Info.szGUICaption, _T("./Image/Background.jpg"));


	//////////////////////////////////////////////////////////////////////
	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 362;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 302;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/NoPassTicket.png")); 
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_NOPASS, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 820;
	pPictureBox->m_Info.iPosY = 680;
	pPictureBox->m_Info.iWidth = 140;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/YourTurn.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_YOURTURN, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 442;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 140;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/AITurn.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_AITURN, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 442;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 140;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/Pass.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_PASS, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 372;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 282;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/YouWin.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_YOUWIN, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 372;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 282;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/YouLose.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_YOULOSE, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 372;
	pPictureBox->m_Info.iPosY = 300;
	pPictureBox->m_Info.iWidth = 282;
	pPictureBox->m_Info.iHeight = 80;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/GameStart.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_GAMESTART, pPictureBox);
	///////////////////////////////////////////////////////////////////////////////////

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 0;
	pPictureBox->m_Info.iPosY = 0;
	pPictureBox->m_Info.iWidth = 640;
	pPictureBox->m_Info.iHeight = 480;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/rGCard_86.bmp"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_CARDSET, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 0;
	pPictureBox->m_Info.iPosY = 0;
	pPictureBox->m_Info.iWidth = CARD_HEIGHT;
	pPictureBox->m_Info.iHeight = CARD_WIDTH;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/CardBack_Rotate.bmp"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_CARDHORIZON, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = 0;
	pPictureBox->m_Info.iPosY = 0;
	pPictureBox->m_Info.iWidth = CARD_WIDTH;
	pPictureBox->m_Info.iHeight = CARD_HEIGHT;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/CardBack.bmp"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_VERTICAL, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = PASSBUTTON_LEFT;
	pPictureBox->m_Info.iPosY = PASSBUTTON_TOP;
	pPictureBox->m_Info.iWidth = PASSBUTTON_RIGHT - PASSBUTTON_LEFT;
	pPictureBox->m_Info.iHeight = PASSBUTTON_BOTTOM - PASSBUTTON_TOP;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/Button1.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_BUTTON, pPictureBox);

	pPictureBox  = new CPictureBox();
	pPictureBox->m_Info.iPosX = PASSBUTTON_LEFT;
	pPictureBox->m_Info.iPosY = PASSBUTTON_TOP;
	pPictureBox->m_Info.iWidth = PASSBUTTON_RIGHT - PASSBUTTON_LEFT;
	pPictureBox->m_Info.iHeight = PASSBUTTON_BOTTOM - PASSBUTTON_TOP;
	_tcscpy_s(pPictureBox->m_Info.szGUICaption, _T("./Image/Button2.png"));
	m_PictureBoxMgr.AddPictureBox(PICTUREBOX_BUTTON_OVERPPED, pPictureBox);

	m_PictureBoxMgr.OnResetDevice();
	m_pBackGround->OnResetDevice();

	D3DXFONT_DESC desc;
	desc.CharSet = HANGEUL_CHARSET;
	desc.Height = 15;
	desc.Width = 8;
	desc.Weight = 100;
	desc.Quality = ANTIALIASED_QUALITY;
	desc.MipLevels = 1;
	desc.Italic = 0;
	desc.OutputPrecision = OUT_DEFAULT_PRECIS;
	desc.PitchAndFamily = FF_DONTCARE;
	_tcscpy_s(desc.FaceName, _T("Arial"));

	m_Font.Initialize(&desc);

	if(!Setup())
	{
		::MessageBox(0, L"Setup() - FAILED", 0, 0);
		return 0;
	}

	
	return TRUE;
}

BOOL CDirectXSystem::OnRender(float fElapsedTime)
{
	HRESULT hr;

	IDirect3DDevice9* pDevice = CDirectXSystem::GetInstance()->GetD3DDevice();

	if( SUCCEEDED( pDevice->BeginScene() ) )
	{				
		//	CEGUI::System::getSingleton().renderGUI();

		//m_pBackGround->OnRender(fElapsedTime);
		//m_SevenGameManger->OnRender(0);

		V( pDevice->EndScene() );
	}	

	return TRUE;
}

BOOL CDirectXSystem::OnRender( float fElapsedTime, int iImageIndex, float fPosX, float fPosY )
{
	CPictureBox* pPictureBox = m_PictureBoxMgr.GetPictureBox(iImageIndex);

	if(pPictureBox)
		pPictureBox->OnRender(fElapsedTime, fPosX, fPosY);

	return TRUE;
}

BOOL CDirectXSystem::OnRender( float fElapsedTime, int iImageIndex)
{
	CPictureBox* pPictureBox = m_PictureBoxMgr.GetPictureBox(iImageIndex);

	if(pPictureBox)
		pPictureBox->OnRender(fElapsedTime);

	return TRUE;
}

BOOL CDirectXSystem::ProcessMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(g_pCasualGameManager)
	g_pCasualGameManager->ProcessInput(uMsg, wParam, lParam);

	return TRUE;
}

BOOL CDirectXSystem::ResetDevice( IDirect3DDevice9* pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;

	m_PictureBoxMgr.OnResetDevice();

	if(m_pBackGround)
		m_pBackGround->OnResetDevice();

	m_Font.OnResetDevice();
	
	//if(m_pGUIManager)
		//m_pGUIManager->OnResetDevice(pd3dDevice);

	return TRUE;
}

HRESULT CDirectXSystem::Create3DSprite(LPDIRECT3DTEXTURE9 &Test_Texture,LPDIRECT3DVERTEXBUFFER9 &Test_Buffer,LPCTSTR lpstr,int alpha)
{
	IDirect3DDevice9* pd3dDevice = m_pd3dDevice;
	TCHAR szFilename[MAX_PATH];
	WCHAR wFileName[MAX_PATH];
	HRESULT hr;
	if(Test_Buffer==NULL)
	{ 
		pd3dDevice->CreateVertexBuffer(
			6 * sizeof(CUSTOMVERTEX), 
			D3DUSAGE_WRITEONLY,
			D3DFVF_CUSTOMVERTEX,
			D3DPOOL_MANAGED,
			&Test_Buffer,
			0);
	}

	CUSTOMVERTEX* v;	
	if( FAILED( hr = Test_Buffer->Lock( 0, 0, (void **)&v, 0 ) ) )
		return hr;

	// first triangle
	v[0].position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	v[0].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[0].tu       = 0.0f;
	v[0].tv       = 0.0f;

	v[1].position = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
	v[1].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[1].tu       = 1.0f;
	v[1].tv       = 0.0f;

	v[2].position = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
	v[2].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[2].tu       = 1.0f;
	v[2].tv       = 1.0f;

	// second triangle
	v[3].position = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
	v[3].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[3].tu       = 0.0f;
	v[3].tv       = 0.0f;

	v[4].position = D3DXVECTOR3(1.0f, -1.0, 0.0f);
	v[4].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[4].tu       = 1.0f;
	v[4].tv       = 1.0f;

	v[5].position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	v[5].color    = D3DCOLOR_ARGB(alpha,0,0,0);
	v[5].tu       = 0.0f;
	v[5].tv       = 1.0f;

	if( FAILED( hr = Test_Buffer->Unlock() ) )	
		return hr;


	(void)StringCchCopy(szFilename, sizeof(szFilename),lpstr);
	//USES_CONVERSION;

	//(void)StringCchCopyW(wFileName, MAX_PATH, T2W(szFilename));

	/*if (FAILED(hr = D3DXCreateTextureFromFileEx(m_pd3dDevice, wFileName, 
	0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
	0, NULL, NULL, &Test_Texture))) {
	return(hr);

	}*/

	D3DXCreateTextureFromFile(
		pd3dDevice,
		szFilename,
		&Test_Texture);  	

	return hr;
}

BOOL CDirectXSystem::GetFullFilePath(TCHAR** szFileName, TCHAR* szFileExt) 
{ 
	static TCHAR strFileName[MAX_PATH] = TEXT("");
	static TCHAR strPath[MAX_PATH] = TEXT("");

	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);


	OPENFILENAME ofn = { sizeof(OPENFILENAME), DXUTGetHWND(), NULL, szFileExt, NULL,
		0, 1, strFileName, MAX_PATH, NULL, 0, strPath,
		TEXT("Open Audio File"),
		OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, 0, 0,
		TEXT(".mp3"), 0, NULL, NULL };

	if( TRUE != GetOpenFileName( &ofn ) )
	{
		SetCurrentDirectory(buffer);
		return FALSE;
	}

	_tcsncpy_s( strPath, strFileName, MAX_PATH );
	strPath[MAX_PATH-1]=0;
	TCHAR* strLastSlash = _tcsrchr( strPath, _T('\\') );
	if( strLastSlash )
		strLastSlash[0] = '\0';

	*szFileName = strFileName;
	SetCurrentDirectory(buffer);

	return TRUE;
}

void CDirectXSystem::OnMove( double fTime, float fElapsedTime)
{

}

void CDirectXSystem::LostDevice()
{
	m_Font.OnLostDevice();
}

BOOL CDirectXSystem::DrawCard(float fElapsedTime) 
{
	//Sprite Global Table
	SevenGameMain* pGameMain = static_cast<SevenGameMain*>(g_pGameMain);
	SGManager* pManager = pGameMain->GetSevenGameManager();
	SGTable *pTable = pManager->GetTable();
	int *paTable = pTable->GetSpadeTableArray();

	CPictureBox* pCardSet = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_CARDSET);
	CPictureBox* pCardHorizon = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_CARDHORIZON);
	CPictureBox* pCardVertical = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_VERTICAL);

	for( int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paTable[i] != -1)	 
		{
			pCardSet->OnRenderRegion(fElapsedTime, 
				(i-1)*CARD_LOOKABLE_PART + DEFAULT_TABLE_CARD_POS_X,
				DEFAULT_TABLE_CARD_POS_Y,
				((float)((i-1)))/CARDNUM_CARDSET_WIDTH,  
				((float)(3))/CARDNUM_CARDSET_HEIGHT,
				CARDRATIO_CARDSET_WIDTH,
				CARDRATIO_CARDSET_HEIGHT,
				CARD_WIDTH,
				CARD_HEIGHT);
		}  			
	}

	paTable = pTable->GetHeartTableArray();
	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paTable[i] != -1)	   
		{
			pCardSet->OnRenderRegion(fElapsedTime, 
				(i-1)*CARD_LOOKABLE_PART+DEFAULT_TABLE_CARD_POS_X,
				DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE,
				((float)((i-1)))/CARDNUM_CARDSET_WIDTH,  
				((float)(1))/CARDNUM_CARDSET_HEIGHT,
				CARDRATIO_CARDSET_WIDTH,
				CARDRATIO_CARDSET_HEIGHT,
				CARD_WIDTH,
				CARD_HEIGHT);
		}  								
	}

	paTable = pTable->GetDiamondTableArray();
	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paTable[i] != -1)	   
		{
			pCardSet->OnRenderRegion(fElapsedTime, 
				(i-1)*CARD_LOOKABLE_PART+DEFAULT_TABLE_CARD_POS_X,
				DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE * 2,
				((float)((i-1)))/CARDNUM_CARDSET_WIDTH,  
				((float)(2))/CARDNUM_CARDSET_HEIGHT,
				CARDRATIO_CARDSET_WIDTH,
				CARDRATIO_CARDSET_HEIGHT,
				CARD_WIDTH,
				CARD_HEIGHT);
		}  		  									
	}

	paTable = pTable->GetCloverTableArray();
	for(int i=1; i<=MAX_CARD_NUM; i++ )
	{
		if(paTable[i] != -1)	   
		{
			pCardSet->OnRenderRegion(fElapsedTime, 
				(i-1)*CARD_LOOKABLE_PART+DEFAULT_TABLE_CARD_POS_X,
				DEFAULT_TABLE_CARD_POS_Y + CARD_DISTANCE * 3,
				((float)((i-1)))/CARDNUM_CARDSET_WIDTH,  
				((float)(0))/CARDNUM_CARDSET_HEIGHT,
				CARDRATIO_CARDSET_WIDTH,
				CARDRATIO_CARDSET_HEIGHT,
				CARD_WIDTH,
				CARD_HEIGHT);
		}  						
	}
	//End

	SGUser* pMe = NULL;
	//Draw User Card	
	int count = 0;
	for(auto& userID : pManager->m_vecDisplayOrder)
	{
		SGUser* pUser = pManager->FindUser(userID);
		int CardNum = pUser->GetRemainCard();				
		int iPassTicket = pUser->GetPassTicketNum();

		if( count == 0 )	
		{
			pMe = pUser;

			if( iPassTicket >= 0 )				
			{
				TCHAR strPass[20];
				_itot_s(iPassTicket,strPass,10);
				RECT rc;

				SetRect( &rc, 700, 710, 300, 0 );
				m_Font.Print(_T("남은 패스 : "), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 790, 710, 300, 0 );
				m_Font.Print(strPass, rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));	
			}

		}	

		if( count == 1 )	
		{
			for(int i=0; i < CardNum;i++)							
				pCardHorizon->OnRender(fElapsedTime, SILENCE_PLAYER_CARD_POS_X, SILENCE_PLAYER_CARD_POS_Y + (i-((CardNum)/2)) * CARD_LOOKABLE_PART);

			if( iPassTicket >= 0 )				
			{
				TCHAR strPass[20];
				_itot_s(iPassTicket,strPass,10);
				RECT rc;

				SetRect( &rc, 10, 150, 300, 0 );
				m_Font.Print(_T("플레이어3"), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 10, 170, 300, 0 );
				m_Font.Print(_T("남은 패스 : "), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 100, 170, 300, 0 );
				m_Font.Print(strPass, rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));
			}
		}
		if( count == 2 )	
		{			

			for(int i=0;i<CardNum;i++)	
				pCardVertical->OnRender(fElapsedTime, GENIUS_PLAYER_POS_X + (i-((CardNum)/2))*CARD_LOOKABLE_PART, GENIUS_PLAYER_POS_Y);


			if( iPassTicket >= 0 )				
			{
				TCHAR strPass[20];
				_itot_s(iPassTicket,strPass,10);
				RECT rc;

				SetRect( &rc, 200, 20, 300, 0 );
				m_Font.Print(_T("플레이어2"), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 200, 40, 300, 0 );
				m_Font.Print(_T("남은 패스 : "), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 290, 40, 300, 0 );
				m_Font.Print(strPass, rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));	


			}				
		}

		if( count == 3 )	
		{
			for(int i=0;i<CardNum;i++)			
				pCardHorizon->OnRender(fElapsedTime, DETECTIVE_PLAYER_POS_X, DETECTIVE_PLAYER_POS_Y + (i-((CardNum)/2))*CARD_LOOKABLE_PART);

			if( iPassTicket >= 0 )				
			{
				TCHAR strPass[20];
				_itot_s(iPassTicket,strPass,10);
				RECT rc;

				SetRect( &rc, 910, 150, 300, 0 );
				m_Font.Print(_T("플레이어1"), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 910, 170, 300, 0 );
				m_Font.Print(_T("남은 패스 : "), rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));

				SetRect( &rc, 1000, 170, 300, 0 );
				m_Font.Print(strPass, rc, DT_NOCLIP, D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ));	
			}

		}
		
		
		

		count++;
			
	} 


	//Draw User Card
	int iCount = 0;
	int iCardCnt = 0;
	int iPassticket = 0;

	if(pMe)
	{						
		iCardCnt = pMe->GetRemainCard();
		iPassticket = pMe->GetPassTicketNum();
		pManager->ReArrangeUserCard(pMe,iCount,iCardCnt);
	}

	if( iPassticket >= 0 )
	{
		for(int i=0;i<iCount;i++)
		{	
			SPRITE_CARD_INFO* pCardInfo = pManager->GetUserCard();


			pCardSet->OnRenderRegion(fElapsedTime, 
				HUMAN_PLAYER_POS_X + (i-((iCardCnt)/2))*CARD_LOOKABLE_PART, 
				HUMAN_PLAYER_POS_Y,
				((float)(pCardInfo[i].iCol))/CARDNUM_CARDSET_WIDTH,  
				((float)(pCardInfo[i].iRow))/CARDNUM_CARDSET_HEIGHT,
				CARDRATIO_CARDSET_WIDTH,
				CARDRATIO_CARDSET_HEIGHT,
				CARD_WIDTH,
				CARD_HEIGHT);

		}
	}
	//End	

	return TRUE;
}

BOOL CDirectXSystem::RenderRegion(int iTextureIdentifer,int x,int y,int align, int Col, int Row, int _x, int _y, int border, BOOL Transparent)
{
	LPDIRECT3DTEXTURE9 pTexture = m_pTextureMgr->GetTexture(iTextureIdentifer);
	if (pTexture)
	{	
		RECT rcSrcRect;
		D3DXVECTOR3 vecCenter;
		D3DXVECTOR3 vecPosition;

		rcSrcRect.left = Col*_x;
		rcSrcRect.top = Row*_y;
		rcSrcRect.right = _x;
		rcSrcRect.bottom = _y;

		vecCenter.x = .0f;
		vecCenter.y = .0f;
		vecCenter.z = .0f;

		vecPosition.x = 50;
		vecPosition.y = 50;
		vecPosition.z = .0f;

		m_pD3DXSprite->Begin(D3DXSPRITE_ALPHABLEND);	
		m_pD3DXSprite->Draw(pTexture, &rcSrcRect, &vecCenter, &vecPosition, 0xffffffff);
		m_pD3DXSprite->End();

		return TRUE;
	}
	return FALSE;
}

HRESULT CDirectXSystem::CreateTexture(LPDIRECT3DTEXTURE9 &Test_Texture, LPCTSTR szTextureName)
{
	HRESULT hr;
	if (FAILED(hr = D3DXCreateTextureFromFileEx(m_pd3dDevice, szTextureName, 
		0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0, NULL, NULL, &Test_Texture))) {
			return(hr);
	}

	return hr;
}

BOOL CDirectXSystem::DrawCardTurn( float fElapsedTime )
{
	SevenGameMain* pGameMain = static_cast<SevenGameMain*>(g_pGameMain);
	SGManager* pManager = pGameMain->GetSevenGameManager();

	SGUser *pUser = pManager->FindUser(pManager->GetMyID());
	CPictureBox* pYourTurn = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_YOURTURN);
	CPictureBox* pAITurn = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_AITURN);

	if(pUser->GetID() == pManager->GetMyID())
		pYourTurn->OnRender(fElapsedTime);

	return TRUE;
}

BOOL CDirectXSystem::DrawWinLose( float fElapsedTime )
{
	SevenGameMain* pGameMain = static_cast<SevenGameMain*>(g_pGameMain);
	SGManager* pManager = pGameMain->GetSevenGameManager();

	CPictureBox* pYouWin = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_YOUWIN);
	CPictureBox* pYouLose = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_YOULOSE);

	if(pManager->m_bIsVictory == TRUE)
	{
		pYouWin->OnRender(fElapsedTime);

		D3DXMATRIX proj;
		D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI / 4.0f, // 45 - degree
			(float)GetWidth() / (float)GetHeight(),
			1.0f,
			5000.0f);
		m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &proj);

		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		m_pd3dDevice->SetTransform(D3DTS_VIEW, &V);

		g_pFireEffect->update(fElapsedTime);

		m_pd3dDevice->SetRenderState(D3DRS_ZENABLE,FALSE);
		//
		// Draw the scene:
		//

		D3DXMATRIX I;
		D3DXMatrixIdentity(&I);

		// order important, render snow last.
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &I);
		g_pFireEffect->render();
	}
	else
	{
		pYouLose->OnRender(fElapsedTime);
	}

	return TRUE;
}


BOOL CDirectXSystem::ResetFireEffect()
{
	g_pFireEffect->reset();
	return TRUE;
}

BOOL CDirectXSystem::DrawPassButton( float fElapsedTime )
{
	SevenGameMain* pGameMain = static_cast<SevenGameMain*>(g_pGameMain);
	SGManager* pManager = pGameMain->GetSevenGameManager();

	CPictureBox* pButton = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_BUTTON);
	CPictureBox* pButtonOverpped = m_PictureBoxMgr.GetPictureBox(PICTUREBOX_BUTTON_OVERPPED);

	if(pGameMain->GetPassButtonOverapped())
		pButtonOverpped->OnRender(fElapsedTime);
	else pButton->OnRender(fElapsedTime);

	return TRUE;
}

BOOL CDirectXSystem::OnRenderRegion( float fElapsedTime, int iImageIndex, float fPosX, float fPosY, float fColRatio, float fRowRatio, float fColUnitRatio, float fRowUnitRatio, float fRegionWidth, float fRegionHeight )
{
	CPictureBox* pPictureBox = m_PictureBoxMgr.GetPictureBox(iImageIndex);
	pPictureBox->OnRenderRegion(fElapsedTime, 
		fPosX,
		fPosY,
		fColRatio,  
		fRowRatio,
		fColUnitRatio,
		fRowUnitRatio,
		fRegionWidth,
		fRegionHeight);

	return TRUE;
}
