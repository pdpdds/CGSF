#include "GUILogin.h"
#include "PacketID.h"
#include "SFPacketStore.pb.h"
#include "Engine.h"
#include "BasePacket.h"
#include "SFProtobufPacket.h"

using namespace google;

extern Engine *g_engine;

const unsigned int GUILogin::NameEditBoxID = 1;
const unsigned int GUILogin::PasswordEditBoxID = 2;
const unsigned int GUILogin::SubmitButtionID = 3;


GUILogin::GUILogin(eGUIState State)
: GUIState(State)
, d_root(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("Login.layout", "Login"))
{


	using namespace CEGUI;

	CEGUI::Window* parent = NULL;

	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	// Do events wire-up
	d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUILogin::handleKeyDown, this));

	d_root->getChild(NameEditBoxID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILogin::handleSubmit, this));

	d_root->getChild(PasswordEditBoxID)->
		subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUILogin::handleSubmit, this));


	d_root->getChild(SubmitButtionID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILogin::handleSubmit, this));

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	d_root->hide();
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
	Editbox* editboxName = static_cast<Editbox*>(d_root->getChild(NameEditBoxID));
	// get text out of the editbox
	String edit_text(editboxName->getText());

	if(edit_text.empty())
		return false;

	Editbox* editboxPassword = static_cast<Editbox*>(d_root->getChild(PasswordEditBoxID));
	// get text out of the editbox
	String edit_text2(editboxPassword->getText());

	if(edit_text.empty())
		return false;

	if(edit_text2.empty())
		return false;

	SFProtobufPacket<SFPacketStore::Login> PktLogin(CGSF::Login);
	PktLogin.SetOwnerSerial(g_engine->GetLocalID());
	PktLogin.GetData().set_username(edit_text.c_str());
	PktLogin.GetData().set_password(edit_text2.c_str());

	g_engine->GetNetwork()->TCPSend(&PktLogin);

	editboxName->setText("");
	editboxPassword->setText("");

	return true;
}

bool GUILogin::ProcessInput( int InputParam )
{
//	CEGUI::System::getSingleton().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUILogin::OnEnter()
{
	d_root->show();

	return true;
}

bool GUILogin::OnLeave()
{
	d_root->hide();

	return true;
}

bool GUILogin::Notify(BasePacket* pPacket)
{
	return true;
}