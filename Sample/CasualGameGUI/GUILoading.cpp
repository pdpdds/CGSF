#include "stdafx.h"
#include "GUILoading.h"
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

GUILoading::GUILoading(eGUIState State)
: GUIState(State)
, d_root(WindowManager::getSingleton().loadLayoutFromFile("Loading.layout"))
{
	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	d_root->getRootWindow()->subscribeEvent(CEGUI::Window::EventUpdated, Event::Subscriber(&GUILoading::handleUpdate, this));
	
}

GUILoading::~GUILoading(void)
{
	CEGUI::WindowManager::getSingleton().destroyWindow(d_root);
}

bool GUILoading::ProcessInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//CEGUI::System::getSingleton().getDefaultGUIContext().injectChar((CEGUI::utf32)InputParam);
	return true;
}

bool GUILoading::OnEnter()
{
	CEGUI::Window* parent = NULL;
	m_loadingTime = 0.0f;
	m_bReportLoadingComplete = false;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);

	CEGUI::ProgressBar* progressBar = dynamic_cast<CEGUI::ProgressBar*>(d_root);
    if(progressBar != 0)
		progressBar->setProgress(0.0f);
      

	return true;
}

bool GUILoading::OnLeave()
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->removeChild(d_root);

	return true;
}

bool GUILoading::Notify(BasePacket* pPacket)
{

	return true;
}

#define MAX_LOADING_TIME 3.0f
void GUILoading::OnRender(float fElapsedTime)
{
	//m_loadingTime += fElapsedTime;

	CEGUI::ProgressBar* progressBar = dynamic_cast<CEGUI::ProgressBar*>(d_root);
    if(progressBar != 0)
    {
        float newProgress = progressBar->getProgress() + fElapsedTime * 0.4f;
        if(newProgress < 1.0f)
            progressBar->setProgress(newProgress);

		if(newProgress >= 1.0f && m_bReportLoadingComplete == false)
		{
			SFProtobufPacket<SFPacketStore::LoadingComplete> request(CGSF::LoadingComplete);		
			g_pCasualGameManager->GetNetwork()->TCPSend(&request);

			m_bReportLoadingComplete = true;
		}	
	}
}

bool GUILoading::handleUpdate(const CEGUI::EventArgs& args)
{
    const CEGUI::UpdateEventArgs& updateArgs = static_cast<const CEGUI::UpdateEventArgs&>(args);
    float passedTime = updateArgs.d_timeSinceLastFrame;

	CEGUI::ProgressBar* progressBar = dynamic_cast<CEGUI::ProgressBar*>(d_root->getChild("LoadingProgressBar"));
    if(progressBar != 0)
    {
        float newProgress = progressBar->getProgress() + passedTime * 0.2f;
        if(newProgress < 1.0f)
            progressBar->setProgress(newProgress);
    }

    return true;
}