#include "stdafx.h"
#include "CasualGameGUI.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Direct3D9/Renderer.h>
#include "GUIFSM.h"
#include "GUILobby.h"

CasualGameGUI::CasualGameGUI(void)
{
}


CasualGameGUI::~CasualGameGUI(void)
{
}

bool CasualGameGUI::Create(LPDIRECT3DDEVICE9 pDevice)
{
	//CEGUI::Direct3D9Renderer& myRenderer = CEGUI::Direct3D9Renderer::bootstrapSystem( pDevice );

	CEGUI::Direct3D9Renderer& myRenderer =
    CEGUI::Direct3D9Renderer::create( pDevice );
	CEGUI::System::create( myRenderer );

		// load scheme and set up defaults
	// initialise the required dirs for the DefaultResourceProvider
	CEGUI::DefaultResourceProvider* rp = static_cast<CEGUI::DefaultResourceProvider*>
		(CEGUI::System::getSingleton().getResourceProvider());

	const char* dataPathPrefix = "./datafiles";
	char resourcePath[260];

	// for each resource type, set a resource group directory
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "schemes/");
	rp->setResourceGroupDirectory("schemes", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "imagesets/");
	rp->setResourceGroupDirectory("imagesets", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "fonts/");
	rp->setResourceGroupDirectory("fonts", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "layouts/");
	rp->setResourceGroupDirectory("layouts", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "looknfeel/");
	rp->setResourceGroupDirectory("looknfeels", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "lua_scripts/");
	rp->setResourceGroupDirectory("lua_scripts", resourcePath);
	sprintf_s(resourcePath, "%s/%s", dataPathPrefix, "xml_schemas/");
	rp->setResourceGroupDirectory("schemas", resourcePath);   


	// set the default resource groups to be used
	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	// setup default group for validation schemas
	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");  
	
	using namespace CEGUI;

	//CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
    SchemeManager::getSingleton().createFromFile("AlfiskoSkin.scheme");
    SchemeManager::getSingleton().createFromFile("WindowsLook.scheme");
    SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");
    SchemeManager::getSingleton().createFromFile("OgreTray.scheme");
    CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("Vanilla-Images/MouseArrow");	

	// Get window manager which we wil use for a few jobs here.
	WindowManager& winMgr = WindowManager::getSingleton();
	// Load the scheme to initialse the VanillaSkin which we use in this sample
	SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");
	// set default mouse image
	//System::getSingleton().("Vanilla-Images", "MouseArrow");

	// load an image to use as a background

	if( !ImageManager::getSingleton().isDefined("BackgroundImage") )
        ImageManager::getSingleton().addFromImageFile("BackgroundImage", "lobby.jpg");

	// here we will use a StaticImage as the root, then we can use it to place a background image
	Window* background = winMgr.createWindow("Vanilla/StaticImage");
	// set area rectangle`
	background->setArea(URect(cegui_reldim(0), cegui_reldim(0), cegui_reldim(1), cegui_reldim(1)));
	// disable frame and standard background
	background->setProperty("FrameEnabled", "false");
	background->setProperty("BackgroundEnabled", "true");
	// set the background image
	background->setProperty("Image", "set:BackgroundImage image:full_image");
	// install this as the root GUI sheet
	System::getSingleton().getDefaultGUIContext().setRootWindow(background);

	//FontManager::getSingleton().create("DejaVuSans-10.font");
	//FontManager::getSingleton().createFromFile("Batang-18.font");
	
    Font& defaultFont = FontManager::getSingleton().createFromFile("DejaVuSans-10.font");
    
	if(FontManager::getSingleton().isDefined("DejaVuSans-10"))
	{
		System::getSingleton().getDefaultGUIContext().setDefaultFont(&defaultFont);
	}

	

	// create an instance of the console class.
	//	d_console = new DemoConsole("Demo");

	 background->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&CasualGameGUI::handleRootKeyDown, this));

	// CEGUI::Window* pMessageBox = winMgr.loadLayoutFromFile("MessageBox.layout");
	// background->addChild(pMessageBox);

	 // load some demo windows and attach to the background 'root'
	//background->addChild(winMgr.loadLayoutFromFile("VanillaWindows.layout"));

	//background->addChild(winMgr.loadLayoutFromFile("LoginBox.layout"));

	 //background->removeChild(pMessageBox);
	 m_pGUIFSM = new GUIFSM(GUI_STATE_NONE);
	 m_pGUIFSM->Initialize();

	// activate the background window
	background->activate();

	return true;

}

bool CasualGameGUI::Render(float fElapsedTime)
{
	m_pGUIFSM->OnRender(fElapsedTime);
	CEGUI::System::getSingleton().renderAllGUIContexts();

	return true;
}

bool CasualGameGUI::OnResetDevice(LPDIRECT3DDEVICE9 pDevice)
{
	CEGUI::Direct3D9Renderer* pRender = (CEGUI::Direct3D9Renderer*)CEGUI::System::getSingleton().getRenderer();
	

	//pRender->preD3DReset();

	//pRender->
	//if (SUCCEEDED(pRender->getDevice()->Reset(pRender->getp)))
    //{
        // re-build stuff now reset has been done.
       // pRender->postD3DReset();

      //  return true;
    //}

	return false;
}

bool CasualGameGUI::handleRootKeyDown(const CEGUI::EventArgs& args)
{
	using namespace CEGUI;

	const KeyEventArgs& keyArgs = static_cast<const KeyEventArgs&>(args);

	switch (keyArgs.scancode)
	{
	case Key::F12:
		break;

	default:
		{
			m_pGUIFSM->handleRootKeyDown(args);
		}
		return false;
	}

	return true;
}

bool CasualGameGUI::ChangeState( eGUIState State )
{
	return m_pGUIFSM->ChangeState(State);
}

bool CasualGameGUI::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	switch( uMsg )
	{
	case WM_MOUSEMOVE:
		{
		int iMouseButtons = wParam;
		int iMouseX       = LOWORD(lParam);
		int iMouseY       = HIWORD(lParam);

		CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition((float)iMouseX, (float)iMouseY);
		}
		break;

	case WM_LBUTTONUP:
		{

			int iPosX = (int)LOWORD(lParam);     
			int iPosY = (int)HIWORD(lParam);

			CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::LeftButton);	
		}
		break;

	case WM_LBUTTONDOWN:
		{
			int iPosX = (int)LOWORD(lParam);     
			int iPosY = (int)HIWORD(lParam);
			

			CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::LeftButton);
		}
		
		break;

		case WM_CHAR:
		{
			if(wParam == VK_BACK)
			{
				CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown(CEGUI::Key::Backspace);
				return 0;
			}
			if(wParam == VK_RETURN)
			{
				CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown(CEGUI::Key::Return);
				return 0;
			}

			CEGUI::System::getSingleton().getDefaultGUIContext().injectChar((CEGUI::utf32)wParam);

			/*static char MultiByte[2] = {0,};
			static bool isHaveMultiByte = false;

			if(isHaveMultiByte)
			{
				isHaveMultiByte = false;
				MultiByte[1] = (unsigned char) wParam;
				int iLengthUnicode = 0;
				wchar_t szUnicode;

				MultiByteToWideChar(CP_ACP, 0, (char*)MultiByte, 2, &szUnicode, 1);
				m_pGUIFSM->ProcessInput((CEGUI::utf32)szUnicode);
			}
			else
			{
				if(0X80 & wParam)
				{
					isHaveMultiByte = true;
					MultiByte[0] = (unsigned char)wParam;
				}
				else
				{
					m_pGUIFSM->ProcessInput(wParam);
				}		
			}*/
		}
		break;	
	}

	return m_pGUIFSM->ProcessInput(uMsg, wParam, lParam);
}

bool CasualGameGUI::Notify(BasePacket* pPacket)
{
	return m_pGUIFSM->Notify(pPacket);
}

#include "GUIMessageBox.h"
#include "QuitMessageBox.h"
void CasualGameGUI::PopupMessage(char* szMessage)
{
	GUIMessageBox* pMessageBox = new GUIMessageBox();
}

void CasualGameGUI::QuitMessage(char* szMessage)
{
	QuitMessageBox* pMessageBox = new QuitMessageBox();
}

eGUIState CasualGameGUI::GetGUIState()
{
	return m_pGUIFSM->GetGUIState();
}