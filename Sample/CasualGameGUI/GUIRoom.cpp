#include "stdafx.h"
#include "GUIRoom.h"
#include "PacketID.h"
#include "SFPacketStore.pb.h"
#include "BasePacket.h"
#include "SFProtobufPacket.h"
#include "CasualGameManager.h"
#include "NetworkSystem.h"
#include "CasualGameGUI.h"

using namespace google;
using namespace CEGUI;

extern CasualGameManager* g_pCasualGameManager;

class MemberListItem : public ListboxTextItem
{
public:
	MemberListItem(const String& text, CEGUI::uint item_id = 0) :
		ListboxTextItem(text, item_id)
	{
		setSelectionBrushImage("Vanilla-Images/GenericBrush");
		playerIndex = -1;
	}

	int playerIndex;
};

GUIRoom::GUIRoom(eGUIState State)
: GUIState(State)
, d_root(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("Room.layout"))
, d_historyPos(0)
{
	d_root->setDestroyedByParent(false);
	d_root->getChild("LeaveRoom")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleLeave, this));
	//d_root->subscribeEvent(Window::EventKeyDown, Event::Subscriber(&GUIRoom::handleKeyDown, this));
	d_root->getChild("SendButton")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleSend, this));
	d_root->getChild("GameStart")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleStart, this));

	
	/*
	d_root->getChild(EntryBoxID)->
		subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUIRoom::handleSend, this));
	d_root->getChild(TeamChangeButtonID)->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIRoom::handleTeamChange, this));
	*/
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
	// get the text entry editbox
	Editbox* editbox = static_cast<Editbox*>(d_root->getChild("SendButton"));

	switch (static_cast<const KeyEventArgs&>(args).scancode)
	{
	case Key::ArrowUp:
		//d_historyPos = max(d_historyPos - 1, -1);
		//if (d_historyPos >= 0)
		{
			editbox->setText(d_history[d_historyPos]);
			editbox->setCaretIndex(static_cast<size_t>(-1));
		}
	//	else
	//	{
	//		editbox->setText("");
	//	}

		editbox->activate();
		break;

	case Key::ArrowDown:
		//d_historyPos = min(d_historyPos + 1, static_cast<int>(d_history.size()));
	//	if (d_historyPos < static_cast<int>(d_history.size()))
		{
			editbox->setText(d_history[d_historyPos]);
			editbox->setCaretIndex(static_cast<size_t>(-1));
		}
	//	else
	//	{
	//		editbox->setText("");
	//	}

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
	
	// get the text entry editbox
	Editbox* editbox = static_cast<Editbox*>(d_root->getChild("Editbox"));
	// get text out of the editbox
	String edit_text(editbox->getText());

	SFProtobufPacket<SFPacketStore::ChatReq> pktChatReq(CGSF::ChatReq);
	pktChatReq.GetData().set_message(edit_text.c_str());
	
	g_pCasualGameManager->GetNetwork()->TCPSend(&pktChatReq);

	// re-activate the text entry box
	editbox->setText("");
	editbox->activate();
	
	return true;
}

bool GUIRoom::handleStart( const CEGUI::EventArgs& args )
{
	SFProtobufPacket<SFPacketStore::LoadingStart> pktLoadingStart(CGSF::LoadingStart);
	g_pCasualGameManager->GetNetwork()->TCPSend(&pktLoadingStart);

	return true;
}

bool GUIRoom::handleTeamChange( const CEGUI::EventArgs& args )
{
	return true;
}

bool GUIRoom::handleLeave( const CEGUI::EventArgs& args )
{
	
	SFProtobufPacket<SFPacketStore::LeaveRoom> pktLeaveRoom(CGSF::LeaveRoom);
	g_pCasualGameManager->GetNetwork()->TCPSend(&pktLeaveRoom);

	return true;
}

bool GUIRoom::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//	CEGUI::System::getSingleton().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUIRoom::OnEnter()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild("Conversation"));
	history->setText("");

	return true;
}

bool GUIRoom::OnLeave()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->removeChild(d_root);

	CEGUI::Listbox* pListBox = static_cast<CEGUI::Listbox*>(d_root->getChild("MemberList"));
	pListBox->resetList();

	return true;
}

bool GUIRoom::Notify(BasePacket* pPacket)
{
	switch(pPacket->GetPacketID())
	{
	case CGSF::EnterTeamMember:
		{
			CEGUI::Listbox* pListBox = static_cast<CEGUI::Listbox*>(d_root->getChild("MemberList"));

			SFProtobufPacket<SFPacketStore::EnterTeamMember>* pEnterTeamMember = (SFProtobufPacket<SFPacketStore::EnterTeamMember>*)pPacket;

			MemberListItem* item = new MemberListItem(pEnterTeamMember->GetData().member());
			item->setSelectionColours(CEGUI::Colour(0.3f, 0.7f, 1.0f, 1.0f));
			item->playerIndex = pEnterTeamMember->GetData().playerindex();
			pListBox->addItem(item);

			AddMessage(pEnterTeamMember->GetData().member() + " entered.");
		}
		break;
	case CGSF::LeaveTeamMember:
		{
			CEGUI::Listbox* pListBox = static_cast<CEGUI::Listbox*>(d_root->getChild("MemberList"));

			SFProtobufPacket<SFPacketStore::LeaveTeamMember>* pLeaveTeamMember = (SFProtobufPacket<SFPacketStore::LeaveTeamMember>*)pPacket;

			MemberListItem* pItem = (MemberListItem*)pListBox->findItemWithText(pLeaveTeamMember->GetData().member().c_str(), NULL);
			pListBox->removeItem(pItem);

			AddMessage(pLeaveTeamMember->GetData().member() + " left.");
		}
		break;
	case CGSF::RoomMember:
		{
			CEGUI::Listbox* pListBox = static_cast<CEGUI::Listbox*>(d_root->getChild("MemberList"));

			SFProtobufPacket<SFPacketStore::RoomMember>* pRoomMember = (SFProtobufPacket<SFPacketStore::RoomMember>*)pPacket;

			for(int i=0; i < pRoomMember->GetData().info_size(); i++)
			{
				const SFPacketStore::RoomMember::Member& info = pRoomMember->GetData().info(i);
	
				MemberListItem* item = new MemberListItem(info.member().c_str());
				item->setSelectionColours(CEGUI::Colour(0.3f, 0.7f, 1.0f, 1.0f));
				item->playerIndex = info.playerindex();
				
				pListBox->addItem(item);

				if(pListBox->getType().compare("WindowsLook/Listbox") == 0)
				{
					item->setTextColours(CEGUI::Colour(0.0f, 0.0f, 0.0f, 1.0f));
				}
			}
		}
		break;

	case CGSF::ChatRes:
		{
			SFProtobufPacket<SFPacketStore::ChatRes>* pPktChatRes = (SFProtobufPacket<SFPacketStore::ChatRes>*)pPacket;

			std::string name = pPktChatRes->GetData().sender();
			std::string szMessage = pPktChatRes->GetData().message();

			MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild("Conversation"));
			// append new text to history output
			history->setText(history->getText() + name + " : " + szMessage);
			// scroll to bottom of history output
			history->setCaretIndex(static_cast<size_t>(-1));

		}
		break;
	}
	return true;

}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
bool GUIRoom::AddMessage(std::string& message)
{
	MultiLineEditbox* history = static_cast<MultiLineEditbox*>(d_root->getChild("Conversation"));
	
	history->setText(history->getText() + message);
	history->setCaretIndex(static_cast<size_t>(-1));

	return true;
}