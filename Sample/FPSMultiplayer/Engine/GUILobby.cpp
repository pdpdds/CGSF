#include "GUILobby.h"
#include "PacketID.h"
#include "SFPacketStore.pb.h"
#include "Engine.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>


using namespace google;

extern Engine *g_engine;

// these must match the IDs assigned in the layout
const unsigned int GUILobby::SubmitButtonID = 1;
const unsigned int GUILobby::EntryBoxID     = 2;
const unsigned int GUILobby::HistoryID      = 3;
const unsigned int GUILobby::CreateButtonID  = 4;
const unsigned int GUILobby::JoinButtonID    = 5;


GUILobby::GUILobby(eGUIState State)
: GUIState(State)
, d_root(CEGUI::WindowManager::getSingleton().loadWindowLayout("VanillaConsole.layout", "Lobby"))
, d_historyPos(0)
{
	
	
	using namespace CEGUI;

	CEGUI::Window* parent = NULL;

    // we will destroy the console box windows ourselves
    d_root->setDestroyedByParent(false);

    // Do events wire-up
    d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUILobby::handleKeyDown, this));

    d_root->getChild(SubmitButtonID)->
        subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleSubmit, this));

    d_root->getChild(EntryBoxID)->
        subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUILobby::handleSubmit, this));


	d_root->getChild(CreateButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleRoomCreate, this));
	d_root->getChild(JoinButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleRoomJoin, this));


    // decide where to attach the console main window
    parent = parent ? parent : CEGUI::System::getSingleton().getGUISheet();

    // attach this window if parent is valid
    if (parent)
        parent->addChildWindow(d_root);

	d_root->hide();
}

GUILobby::~GUILobby()
{
    // destroy the windows that we loaded earlier
    CEGUI::WindowManager::getSingleton().destroyWindow(d_root);
}

bool GUILobby::isVisible() const
{
    return d_root->isVisible();
}

bool GUILobby::handleKeyDown(const CEGUI::EventArgs& args)
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
            editbox->setCaratIndex(static_cast<size_t>(-1));
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
            editbox->setCaratIndex(static_cast<size_t>(-1));
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
bool GUILobby::handleSubmit(const CEGUI::EventArgs&)
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
		history->setCaratIndex(static_cast<size_t>(-1));
		// erase text in text entry box.
		editbox->setText("");
	}

	SFPacketStore::ChatReq PktChatReq;
	PktChatReq.set_message(edit_text.c_str());
	int BufSize = PktChatReq.ByteSize();

	char Buffer[2048] = {0,};

	if(BufSize != 0)
	{
		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		PktChatReq.SerializeToZeroCopyStream(&os);
	}

	g_engine->GetNetwork()->TCPSend(g_engine->GetLocalID(), CGSF::ChatReq, Buffer, BufSize);

	// re-activate the text entry box
	editbox->activate();

	return true;
}

bool GUILobby::handleRoomCreate( const CEGUI::EventArgs& args )
{
	SFPacketStore::CreateRoom PktCreateRoom;
	PktCreateRoom.set_gamemode(3);
	int BufSize = PktCreateRoom.ByteSize();

	char Buffer[2048] = {0,};

	if(BufSize != 0)
	{
		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		PktCreateRoom.SerializeToZeroCopyStream(&os);
	}

	g_engine->GetNetwork()->TCPSend(g_engine->GetLocalID(), CGSF::CreateRoom, Buffer, BufSize);
	return true;
}

bool GUILobby::handleRoomJoin( const CEGUI::EventArgs& args )
{
	SFPacketStore::EnterRoom PktEnterRoom;
	PktEnterRoom.set_roomindex(0);
	PktEnterRoom.set_gamemode(3);
	int BufSize = PktEnterRoom.ByteSize();

	char Buffer[2048] = {0,};

	if(BufSize != 0)
	{
		::google::protobuf::io::ArrayOutputStream os(Buffer, BufSize);
		PktEnterRoom.SerializeToZeroCopyStream(&os);
	}


	g_engine->GetNetwork()->TCPSend(g_engine->GetLocalID(), CGSF::EnterRoom, Buffer, BufSize);

	return true;
}

bool GUILobby::ProcessInput( int InputParam )
{
	CEGUI::System::getSingleton().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUILobby::OnEnter()
{
	d_root->show();

	return true;
}

bool GUILobby::OnLeave()
{
	d_root->hide();

	return true;
}

bool GUILobby::Notify( int Msg, char* pBuffer, int BufferSize )
{
	if(Msg == CGSF::ChatRes)
	{

		SFPacketStore::ChatRes PktChatRes;
		protobuf::io::ArrayInputStream is(pBuffer, BufferSize);
		PktChatRes.ParseFromZeroCopyStream(&is);
		int BufSize = PktChatRes.ByteSize();

		std::string name = PktChatRes.sender();
		std::string szMessage = PktChatRes.message();

		using namespace CEGUI;

		// get the text entry editbox
		Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));

		// get history window
		MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild(HistoryID));
		// append new text to history output
		history->setText(history->getText() + szMessage);
		// scroll to bottom of history output
		history->setCaratIndex(static_cast<size_t>(-1));

	}
	
	return true;

}