#include "stdafx.h"
#include <CEGUI/CEGUI.h>
#include "GUIMessageBox.h"


GUIMessageBox::GUIMessageBox(void)
	: d_root(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("MessageBox.layout"))
{
	using namespace CEGUI;

	CEGUI::Window* parent = NULL;

	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	// Do events wire-up
//	d_root->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIMessageBox::handleSubmit, this));

	d_root->getChild("Button")->
		subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUIMessageBox::handleSubmit, this));

	/*CEGUI::Window* d_fontNameEditbox = static_cast<CEGUI::Editbox*>(d_root->getChild("Login/Name"));
	d_fontNameEditbox->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILogin::handleSubmit, this));

	d_root->getChild("Login/PassText")->
		subscribeEvent(Editbox::EventTextAccepted, Event::Subscriber(&GUILogin::handleSubmit, this));


	d_root->getChild("Login/Submit")->subscribeEvent(PushButton::EventClicked, Event::Subscriber(&GUILogin::handleSubmit, this));
	*/
	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);
}


GUIMessageBox::~GUIMessageBox(void)
{
}

bool GUIMessageBox::handleSubmit(const CEGUI::EventArgs&)
{
	CEGUI::Window* parent = NULL;

	// decide where to attach the console main window
	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->removeChild(d_root);

	delete this;
	
	return true;
}