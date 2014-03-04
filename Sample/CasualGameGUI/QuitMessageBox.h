#pragma once
class QuitMessageBox
{
public:
	QuitMessageBox(void);
	virtual ~QuitMessageBox(void);

	bool handleSubmit(const CEGUI::EventArgs&);
	CEGUI::Window* d_root;
};

