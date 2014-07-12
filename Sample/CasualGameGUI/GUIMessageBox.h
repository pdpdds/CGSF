#pragma once

class GUIMessageBox
{
public:
	GUIMessageBox(void);
	virtual ~GUIMessageBox(void);

	bool handleSubmit(const CEGUI::EventArgs&);
	CEGUI::Window* d_root;
};

