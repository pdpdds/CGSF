#include "GUIManager.h"
#include "CEGUI.h"
#include "CEGUIRenderingRoot.h"
#include "CEGUIDirect3D9Renderer.h"
#include "CEGUISystem.h"
#include "CEGUIDefaultResourceProvider.h"
#include "CEGUIImageset.h"
#include "CEGUIFont.h"
#include "CEGUIScheme.h"
#include "CEGUIWindowManager.h"
#include "falagard/CEGUIFalWidgetLookManager.h"
#include "CEGUIScriptModule.h"
#include "CEGUIXMLParser.h"
#include "CEGUIAnimationManager.h"

#include "GUIFSM.h"
#include "GUILobby.h"

GUIManager::GUIManager(void)
{
}

GUIManager::~GUIManager(void)
{
	delete m_pGUIFSM;
}

bool GUIManager::Create( IDirect3DDevice9* pDevice )
{
	ShowCursor( false );

	CEGUI::Direct3D9Renderer& myRenderer = CEGUI::Direct3D9Renderer::create( pDevice );
	CEGUI::System::create(myRenderer);

		// load scheme and set up defaults
	// initialise the required dirs for the DefaultResourceProvider
	CEGUI::DefaultResourceProvider* rp =
		static_cast<CEGUI::DefaultResourceProvider*>
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
	CEGUI::Imageset::setDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	// setup default group for validation schemas
	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", "schemas");  

	CEGUI::SchemeManager::getSingleton().create("TaharezLook.scheme");

	using namespace CEGUI;

	// Get window manager which we wil use for a few jobs here.
	WindowManager& winMgr = WindowManager::getSingleton();
	// Load the scheme to initialse the VanillaSkin which we use in this sample
	SchemeManager::getSingleton().create("VanillaSkin.scheme");
	// set default mouse image
	System::getSingleton().setDefaultMouseCursor("Vanilla-Images", "MouseArrow");

	// load an image to use as a background
	ImagesetManager::getSingleton().createFromImageFile("BackgroundImage", "lobby.jpg");

	// here we will use a StaticImage as the root, then we can use it to place a background image
	Window* background = winMgr.createWindow("Vanilla/StaticImage");
	// set area rectangle`
	background->setArea(URect(cegui_reldim(0), cegui_reldim(0), cegui_reldim(1), cegui_reldim(1)));
	// disable frame and standard background
	background->setProperty("FrameEnabled", "false");
	background->setProperty("BackgroundEnabled", "false");
	// set the background image
	background->setProperty("Image", "set:BackgroundImage image:full_image");
	// install this as the root GUI sheet
	System::getSingleton().setGUISheet(background);

	//FontManager::getSingleton().create("DejaVuSans-10.font");
	FontManager::getSingleton().create("Batang-26.font");

	if(FontManager::getSingleton().isDefined("Batang-26"))
	{
		System::getSingleton().setDefaultFont("Batang-26");
	}

	// load some demo windows and attach to the background 'root'
	//background->addChildWindow(winMgr.loadWindowLayout("VanillaWindows.layout"));

	// create an instance of the console class.
	//	d_console = new DemoConsole("Demo");

	 background->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUIManager::handleRootKeyDown, this));

	 m_pGUIFSM = new GUIFSM(GUI_STATE_NONE);
	 m_pGUIFSM->Initialize();

	// activate the background window
	background->activate();

	return true;

}

bool GUIManager::Render()
{
	CEGUI::System::getSingleton().renderGUI();

	return true;
}	


bool GUIManager::handleRootKeyDown(const CEGUI::EventArgs& args)
{
	using namespace CEGUI;

	const KeyEventArgs& keyArgs = static_cast<const KeyEventArgs&>(args);

	switch (keyArgs.scancode)
	{
	case Key::F12:
		break;

	default:
		return false;
	}

	return true;
}

bool GUIManager::ChangeState( eGUIState State )
{
	return m_pGUIFSM->ChangeState(State);
}

bool GUIManager::ProcessInput( int InputParam )
{
	return m_pGUIFSM->ProcessInput(InputParam);
}

bool GUIManager::Notify(BasePacket* pPacket)
{
	return m_pGUIFSM->Notify(pPacket);
}