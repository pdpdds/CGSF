#include "stdafx.h"
#include "GUILogin.h"
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"
#include "CommonStructure.h"

using namespace google;

using namespace CEGUI;

#define SF_GETPACKET_ARG(a,b,c) memcpy(a,b.c_str(), sizeof(c));

extern CasualGameManager* g_pCasualGameManager;

GUILogin::GUILogin(eGUIState State)
: GUIState(State)
, d_root(WindowManager::getSingleton().loadLayoutFromFile("LoginBox.layout"))
{
	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	// Do events wire-up
	d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUILogin::handleKeyDown, this));
	d_root->getChild("Submit")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILogin::handleSubmit, this));
	
}

GUILogin::~GUILogin(void)
{
	CEGUI::WindowManager::getSingleton().destroyWindow(d_root);
}

bool GUILogin::isVisible() const
{
	return d_root->isVisible();
}

bool GUILogin::handleKeyDown(const CEGUI::EventArgs& args)
{
	using namespace CEGUI;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Network Transport 20120618(¿ù)
///////////////////////////////////////////////////////////////////////////////////////////////
bool GUILogin::handleSubmit(const CEGUI::EventArgs&)
{
	using namespace CEGUI;

	// get the text entry editbox
	Editbox* editboxName = static_cast<Editbox*>(d_root->getChild("GameIDInput"));
	// get text out of the editbox
	String edit_text(editboxName->getText());

	if(edit_text.empty())
		return false;

	Editbox* editboxPassword = static_cast<Editbox*>(d_root->getChild("PasswordInput"));
	// get text out of the editbox
	String edit_text2(editboxPassword->getText());

	if(edit_text.empty())
		return false;

	if(edit_text2.empty())
		return false;

	SFProtobufPacket<SFPacketStore::Login> PktLogin(CGSF::Login);
//	PktLogin.SetOwnerSerial(g_engine->GetLocalID());
	PktLogin.GetData().set_username(edit_text.c_str());
	PktLogin.GetData().set_password(edit_text2.c_str());

	g_pCasualGameManager->GetNetwork()->TCPSend(&PktLogin);

	editboxName->setText("");
	editboxPassword->setText("");

	return true;
}

bool GUILogin::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//CEGUI::System::getSingleton().getDefaultGUIContext().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUILogin::OnEnter()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	return true;
}

bool GUILogin::OnLeave()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->removeChild(d_root);

	return true;
}

bool GUILogin::Notify(BasePacket* pPacket)
{
	int PacketID = pPacket->GetPacketID();

	if (PacketID == CGSF::LoginSuccess)	
	{
		SFProtobufPacket<SFPacketStore::LoginSuccess>* pLoginSuccess = (SFProtobufPacket<SFPacketStore::LoginSuccess>*)pPacket;
		_UserInfo info;
		SF_GETPACKET_ARG(&info, pLoginSuccess->GetData().userinfo(), _UserInfo);
		g_pCasualGameManager->SetMyID(info.Serial);

		SFProtobufPacket<SFPacketStore::EnterLobby> request(CGSF::EnterLobby);		
		g_pCasualGameManager->GetNetwork()->TCPSend(&request);
	}
	else if(PacketID == CGSF::LoginFail)
	{
		g_pCasualGameManager->GetGUI()->PopupMessage("Login Fail!!");
	}

	return true;
}