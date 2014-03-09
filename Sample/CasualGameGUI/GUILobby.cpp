#include "stdafx.h"
#include "GUILobby.h"
#include <SFPacketStore/SFPacketID.h>
#include <SFPacketStore/SFPacketStore.pb.h>
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"

using namespace google;
using namespace CEGUI;

extern CasualGameManager* g_pCasualGameManager;

class MyListItem : public ListboxTextItem
{
public:
	MyListItem(const String& text, CEGUI::uint item_id = 0) :
		ListboxTextItem(text, item_id)
	{
		setSelectionBrushImage("Vanilla-Images/GenericBrush");
		roomIndex = -1;
	}

	int roomIndex;
};

GUILobby::GUILobby(eGUIState State)
	: GUIState(State)
	, d_root(WindowManager::getSingleton().loadLayoutFromFile("Lobby.layout"))
	, d_historyPos(0)
{

	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	d_root->getChild("CreateRoom")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleRoomCreate, this));
	d_root->getChild("Refresh")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleRoomListRefresh, this));
	d_root->getChild("RoomList")->subscribeEvent(Listbox::EventMouseClick, Event::Subscriber(&GUILobby::handleRoomClick, this));

	// Do events wire-up
	/* d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUILobby::handleKeyDown, this));

	d_root->getChild(SubmitButtonID)->
	subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILobby::handleSubmit, this));

	d_root->getChild(EntryBoxID)->
	subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUILobby::handleSubmit, this));*/
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
	/*    Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));

	switch (static_cast<const KeyEventArgs&>(args).scancode)
	{
	case Key::ArrowUp:
	//        d_historyPos = max(d_historyPos - 1, -1);
	//  if (d_historyPos >= 0)
	{
	editbox->setText(d_history[d_historyPos]);
	editbox->setCaretIndex(static_cast<size_t>(-1));
	}
	//    else
	//    {
	//       editbox->setText("");
	//   }

	editbox->activate();
	break;

	case Key::ArrowDown:
	//  d_historyPos = min(d_historyPos + 1, static_cast<int>(d_history.size()));
	//  if (d_historyPos < static_cast<int>(d_history.size()))
	{
	editbox->setText(d_history[d_historyPos]);
	editbox->setCaretIndex(static_cast<size_t>(-1));
	}
	//  else
	//   {
	//      editbox->setText("");
	//  }

	editbox->activate();
	break;

	default:
	return false;
	}
	*/
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Network Transport 20120614(¸ñ)
///////////////////////////////////////////////////////////////////////////////////////////////
bool GUILobby::handleSubmit(const CEGUI::EventArgs&)
{
	using namespace CEGUI;

	// get the text entry editbox
	/*Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));
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
	//MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild(HistoryID));
	// append new text to history output
	//history->setText(history->getText() + edit_text);
	// scroll to bottom of history output
	//history->setCaretIndex(static_cast<size_t>(-1));
	// erase text in text entry box.
	//editbox->setText("");
	}

	SFProtobufPacket<SFPacketStore::ChatReq> PktChatReq(CGSF::ChatReq);
	//PktChatReq.SetOwnerSerial(g_engine->GetLocalID());
	PktChatReq.GetData().set_message(edit_text.c_str());

	//	g_engine->GetNetwork()->TCPSend(&PktChatReq);


	// re-activate the text entry box
	editbox->activate();
	*/
	return true;
}

bool GUILobby::handleRoomCreate( const CEGUI::EventArgs& args )
{
	SFProtobufPacket<SFPacketStore::CreateRoom> pktCreateRoom(CGSF::CreateRoom);
	pktCreateRoom.GetData().set_gamemode(2);

	g_pCasualGameManager->GetNetwork()->TCPSend(&pktCreateRoom);
	return true;
}

bool GUILobby::handleRoomListRefresh( const CEGUI::EventArgs& args )
{
	SFProtobufPacket<SFPacketStore::RoomListRefresh> pktCreateRoom(CGSF::RoomListRefresh);
	g_pCasualGameManager->GetNetwork()->TCPSend(&pktCreateRoom);
	return true;
}

bool GUILobby::handleRoomClick( const CEGUI::EventArgs& args )
{
	CEGUI::Listbox* pListBox = static_cast<CEGUI::Listbox*>(d_root->getChild("RoomList"));

	if(pListBox->getFirstSelectedItem())
    {
        MyListItem* pItem = static_cast<MyListItem*>(pListBox->getFirstSelectedItem());
		
		SFProtobufPacket<SFPacketStore::EnterRoom> pktEnterRoom(CGSF::EnterRoom);

		pktEnterRoom.GetData().set_roomindex(pItem->roomIndex);
		pktEnterRoom.GetData().set_gamemode(3);
		g_pCasualGameManager->GetNetwork()->TCPSend(&pktEnterRoom);
	}

	return true;
}

bool GUILobby::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//	CEGUI::System::getSingleton().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUILobby::OnEnter()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	Listbox* lbox = static_cast<Listbox*>(d_root->getChild("RoomList"));
	lbox->resetList();

	return true;
}

bool GUILobby::OnLeave()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->removeChild(d_root);

	return true;
}

bool GUILobby::Notify(BasePacket* pPacket)
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::ChatRes:
		{
			SFProtobufPacket<SFPacketStore::ChatRes>* pChatRes = (SFProtobufPacket<SFPacketStore::ChatRes>*)pPacket;

			std::string name = pChatRes->GetData().sender();
			std::string szMessage = pChatRes->GetData().message();

			// get the text entry editbox
			//Editbox* editbox = static_cast<Editbox*>(d_root->getChild(EntryBoxID));

			// get history window
			//	MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild(HistoryID));
			// append new text to history output
			//history->setText(history->getText() + szMessage);
			// scroll to bottom of history output
			//history->setCaretIndex(static_cast<size_t>(-1));

		}
		break;

	case CGSF::RoomList:
		{
			SFProtobufPacket<SFPacketStore::RoomList>* pRoomList = (SFProtobufPacket<SFPacketStore::RoomList>*)pPacket;

			Listbox* lbox = static_cast<Listbox*>(d_root->getChild("RoomList"));
			lbox->resetList();

			for(int i=0; i < pRoomList->GetData().info_size(); i++)
			{
				const SFPacketStore::RoomList::RoomInfo& info = pRoomList->GetData().info(i);
				//_PeerInfo PeerInfo;
				//SF_GETPACKET_ARG(&PeerInfo, Peer.info(), _PeerInfo);
				MyListItem* item = new MyListItem(info.roomname().c_str());
				item->setSelectionColours(CEGUI::Colour(0.3f, 0.7f, 1.0f, 1.0f));
				item->roomIndex = info.roomindex(); 
				
				lbox->addItem(item);

				if(lbox->getType().compare("WindowsLook/Listbox") == 0)
				{

					item->setTextColours(CEGUI::Colour(0.0f, 0.0f, 0.0f, 1.0f));

				}
			}	

		}
		break;
		
	case CGSF::EnterRoom:
		{
			SFProtobufPacket<SFPacketStore::EnterRoom>* pEnterRoom = (SFProtobufPacket<SFPacketStore::EnterRoom>*)pPacket;		
		}
		break;
	}

	return true;

}

