#include "GUIRoom.h"
#include "PacketID.h"
#include "SFPacketStore.pb.h"
#include "Engine.h"
#include "BasePacket.h"
#include "SFProtobufPacket.h"

using namespace google;

extern Engine *g_engine;

const unsigned int GUIRoom::SendButtonID = 1;
const unsigned int GUIRoom::EntryBoxID = 2;
const unsigned int GUIRoom::HistoryID = 3;
const unsigned int GUIRoom::StartButtonID = 4;
const unsigned int GUIRoom::TeamChangeButtonID = 5;
const unsigned int GUIRoom::LeaveButtonID = 6;

GUIRoom::GUIRoom(eGUIState State)
: GUIState(State)
, d_root(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("Room.layout", "Room"))
, d_historyPos(0)
{


	using namespace CEGUI;

	CEGUI::Window* parent = NULL;

	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	// Do events wire-up
	d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUIRoom::handleKeyDown, this));

	d_root->getChild(SendButtonID)->
		subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleSend, this));

	d_root->getChild(EntryBoxID)->
		subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUIRoom::handleSend, this));


	d_root->getChild(StartButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleStart, this));
	d_root->getChild(TeamChangeButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleTeamChange, this));
	d_root->getChild(LeaveButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleLeave, this));


	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	d_root->hide();
}

GUIRoom::~GUIRoom()
{
	// destroy the windows that we loaded earlier
	CEGUI::WindowManager::getSingleton().destroyWindow(d_root);
}

bool GUIRoom::isVisible() const
{
	return d_root->isVisible();
}

bool GUIRoom::handleKeyDown(const CEGUI::EventArgs& args)
{
	using namespace CEGUI;

	// get the text entry editbox
	Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));

	switch (static_cast<const KeyEventArgs&>(args).scancode)
	{
	case Key::ArrowUp:
		d_historyPos = max(d_historyPos - 1, -1);
		if (d_historyPos >= 0)
		{
			editbox->setText(d_history[d_historyPos]);
			editbox->setCaretIndex(static_cast<size_t>(-1));
		}
		else
		{
			editbox->setText("");
		}

		editbox->activate();
		break;

	case Key::ArrowDown:
		d_historyPos = min(d_historyPos + 1, static_cast<int>(d_history.size()));
		if (d_historyPos < static_cast<int>(d_history.size()))
		{
			editbox->setText(d_history[d_historyPos]);
			editbox->setCaretIndex(static_cast<size_t>(-1));
		}
		else
		{
			editbox->setText("");
		}

		editbox->activate();
		break;

	default:
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////
//Network Transport 20120614(¸ñ)
///////////////////////////////////////////////////////////////////////////////////////////////
bool GUIRoom::handleSend(const CEGUI::EventArgs&)
{
	using namespace CEGUI;

	// get the text entry editbox
	Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));
	// get text out of the editbox
	String edit_text(editbox->getText());
	// if the string is not empty
	if (!edit_text.empty())
	{
		// add this entry to the command history buffer
		d_history.push_back(edit_text);
		// reset history position
		d_historyPos = d_history.size();
		// append newline to this entry
		edit_text += '\n';
		// get history window
		MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild(HistoryID));
		// append new text to history output
		history->setText(history->getText() + edit_text);
		// scroll to bottom of history output
		history->setCaretIndex(static_cast<size_t>(-1));
		// erase text in text entry box.
		editbox->setText("");
	}

	SFProtobufPacket<SFPacketStore::ChatReq> PktChatReq(CGSF::ChatReq);
	PktChatReq.SetOwnerSerial(g_engine->GetLocalID());

	PktChatReq.GetData().set_message(edit_text.c_str());
	
	g_engine->GetNetwork()->TCPSend(&PktChatReq);

	// re-activate the text entry box
	editbox->activate();

	return true;
}

bool GUIRoom::handleStart( const CEGUI::EventArgs& args )
{
	SFProtobufPacket<SFPacketStore::StartGame> PktStartGame(CGSF::StartGame);
	PktStartGame.SetOwnerSerial(g_engine->GetLocalID());

	g_engine->GetNetwork()->TCPSend(&PktStartGame);

	return true;
}

bool GUIRoom::handleTeamChange( const CEGUI::EventArgs& args )
{
	return true;
}

bool GUIRoom::handleLeave( const CEGUI::EventArgs& args )
{
	
	SFProtobufPacket<SFPacketStore::LeaveRoom> PktLeaveRoom(CGSF::LeaveRoom);
	PktLeaveRoom.SetOwnerSerial(g_engine->GetLocalID());

	g_engine->GetNetwork()->TCPSend(&PktLeaveRoom);

	return true;
}

bool GUIRoom::ProcessInput( int InputParam )
{
//	CEGUI::System::getSingleton().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUIRoom::OnEnter()
{
	d_root->show();

	return true;
}

bool GUIRoom::OnLeave()
{
	d_root->hide();

	return true;
}

bool GUIRoom::Notify(BasePacket* pPacket)
{
	if(pPacket->GetPacketID() == CGSF::ChatRes)
	{
		SFProtobufPacket<SFPacketStore::ChatRes>* pPktChatRes = (SFProtobufPacket<SFPacketStore::ChatRes>*)pPacket;

		std::string name = pPktChatRes->GetData().sender();
		std::string szMessage = pPktChatRes->GetData().message();

		using namespace CEGUI;

		// get the text entry editbox
		Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));

		// get history window
		MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild(HistoryID));
		// append new text to history output
		history->setText(history->getText() + szMessage);
		// scroll to bottom of history output
		history->setCaretIndex(static_cast<size_t>(-1));

	}

	return true;

}