#include "stdafx.h"
#include "QuitMessageBox.h"

QuitMessageBox::QuitMessageBox(void)
	: d_root(CEGUI::WindowManager::getSingleton().loadLayoutFromFile("MessageBox.layout"))
{
	using namespace CEGUI;

	CEGUI::Window* parent = NULL;

	// we will destroy the console box windows ourselves
	d_root->setDestroyedByParent(false);

	d_root->getChild("Button")->
		subscribeEvent(PushButton::EventClicked, Event::Subscriber(&QuitMessageBox::handleSubmit, this));

	parent = parent ? parent : CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow();

	// attach this window if parent is valid
	if (parent)
		parent->addChild(d_root);
}


QuitMessageBox::~QuitMessageBox(void)
{
}

bool QuitMessageBox::handleSubmit(const CEGUI::EventArgs&)
{
	PostQuitMessage(0);
	return true;
}
