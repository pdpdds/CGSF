//-----------------------------------------------------------------------------
// DeviceEnumeration.h implementation.
// Refer to the DeviceEnumeration.h interface for more details.
//
// Programming a Multiplayer First Person Shooter in DirectX
// Copyright (c) 2004 Vaughan Young
//-----------------------------------------------------------------------------
#include "Engine.h"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
DeviceEnumeration *g_deviceEnumeration = NULL;

//-----------------------------------------------------------------------------
// A callback director for the graphics settings dialog's message handler.
//-----------------------------------------------------------------------------
BOOL CALLBACK SettingsDialogProcDirector( HWND hDlg, UINT uiMsg, WPARAM wParam, LPARAM lParam )
{
	return g_deviceEnumeration->SettingsDialogProc( hDlg, uiMsg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Enumerates available Direct3D devices on the default adapter.
//-----------------------------------------------------------------------------
INT_PTR DeviceEnumeration::Enumerate( IDirect3D9 *d3d )
{
	// Create the display modes linked list.
	m_displayModes = new LinkedList< DisplayMode >;

	// Load the settings script.
	m_settingsScript = new Script( "DisplaySettings.txt" );

	// Get the details of the default adapter.
	d3d->GetAdapterIdentifier( D3DADAPTER_DEFAULT, 0, &m_adapter );

	// Build a list of the allowable pixel formats.
	D3DFORMAT allowedFormats[6];
	allowedFormats[0] = D3DFMT_X1R5G5B5;
	allowedFormats[1] = D3DFMT_A1R5G5B5;
	allowedFormats[2] = D3DFMT_R5G6B5;
	allowedFormats[3] = D3DFMT_X8R8G8B8;
	allowedFormats[4] = D3DFMT_A8R8G8B8;
	allowedFormats[5] = D3DFMT_A2R10G10B10;

	// Go through the list of allowable pixel formats.
	for( char af = 0; af < 6; af++ )
	{
		// Get the number of adapter modes and go through them.
		unsigned long totalAdapterModes = d3d->GetAdapterModeCount( D3DADAPTER_DEFAULT, allowedFormats[af] );
		for( unsigned long m = 0; m < totalAdapterModes; m++ )
		{
			// Get the display mode details.
			D3DDISPLAYMODE mode;
			d3d->EnumAdapterModes( D3DADAPTER_DEFAULT, allowedFormats[af], m, &mode );

			// Reject small display modes.
			if( mode.Height < 480 )
				continue;

			// Create the new display mode.
			DisplayMode *displayMode = new DisplayMode;
			memcpy( &displayMode->mode, &mode, sizeof( D3DDISPLAYMODE ) );
			if( af < 3 )
				strcpy( displayMode->bpp, "16 bpp" );
			else
				strcpy( displayMode->bpp, "32 bpp" );

			// Add this display mode to the list.
			m_displayModes->Add( displayMode );
		}
	}

	return DialogBox( NULL, MAKEINTRESOURCE( IDD_GRAPHICS_SETTINGS ), NULL, SettingsDialogProcDirector );
}

//-----------------------------------------------------------------------------
// Handles window messages for the graphics settings dialog.
//-----------------------------------------------------------------------------
INT_PTR DeviceEnumeration::SettingsDialogProc( HWND dialog, UINT msg, WPARAM wparam, LPARAM lparam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			// Display the adapter details and its driver version.
			char version[16];
			sprintf( version, "%d", LOWORD( m_adapter.DriverVersion.LowPart ) );
			Edit_SetText( GetDlgItem( dialog, IDC_DISPLAY_ADAPTER ), m_adapter.Description );
			Edit_SetText( GetDlgItem( dialog, IDC_DRIVER_VERSION ), version );

			// Check if the settings script has anything in it.
			if( m_settingsScript->GetBoolData( "windowed" ) == NULL )
			{
				// The settings script is empty, so default to windowed mode.
				CheckDlgButton( dialog, IDC_WINDOWED, m_windowed = true );
			}
			else
			{
				// Load the window mode state.
				CheckDlgButton( dialog, IDC_WINDOWED, m_windowed = *m_settingsScript->GetBoolData( "windowed" ) );
				CheckDlgButton( dialog, IDC_FULLSCREEN, !m_windowed );

				// Check if running in fullscreen mode.
				if( m_windowed == false )
				{
					// Enable all the fullscreen controls.
					EnableWindow( GetDlgItem( dialog, IDC_VSYNC ), true );
					EnableWindow( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ), true );
					EnableWindow( GetDlgItem( dialog, IDC_RESOLUTION ), true );
					EnableWindow( GetDlgItem( dialog, IDC_REFRESH_RATE ), true );

					// Load the vsync state.
					CheckDlgButton( dialog, IDC_VSYNC, m_vsync = *m_settingsScript->GetBoolData( "vsync" ) );

					// Fill in the display formats combo box.
					ComboBox_ResetContent( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ) );
					m_displayModes->Iterate( true );
					while( m_displayModes->Iterate() )
						if( !ComboBoxContainsText( dialog, IDC_DISPLAY_FORMAT, m_displayModes->GetCurrent()->bpp ) )
							ComboBoxAdd( dialog, IDC_DISPLAY_FORMAT, (void*)m_displayModes->GetCurrent()->mode.Format, m_displayModes->GetCurrent()->bpp );
					ComboBoxSelect( dialog, IDC_DISPLAY_FORMAT, *m_settingsScript->GetNumberData( "bpp" ) );

					char text[16];

					// Fill in the resolutions combo box.
					ComboBox_ResetContent( GetDlgItem( dialog, IDC_RESOLUTION ) );
					m_displayModes->Iterate( true );
					while( m_displayModes->Iterate() )
					{
						if( m_displayModes->GetCurrent()->mode.Format == (D3DFORMAT)PtrToUlong( ComboBoxSelected( dialog, IDC_COLOUR_DEPTH ) ) )
						{
							sprintf( text, "%d x %d", m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height );
							if (!ComboBoxContainsText( dialog, IDC_RESOLUTION, text ) )
								ComboBoxAdd( dialog, IDC_RESOLUTION, (void*)MAKELONG( m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height ), text );
						}
					}
					ComboBoxSelect( dialog, IDC_RESOLUTION, *m_settingsScript->GetNumberData( "resolution" ) );

					// Fill in the refresh rates combo box.
					ComboBox_ResetContent( GetDlgItem( dialog, IDC_REFRESH_RATE ) );
					m_displayModes->Iterate( true );
					while( m_displayModes->Iterate() )
					{
						if( (DWORD)MAKELONG( m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height ) == (DWORD)PtrToUlong( ComboBoxSelected( dialog, IDC_RESOLUTION ) ) )
						{
							sprintf( text, "%d Hz", m_displayModes->GetCurrent()->mode.RefreshRate );
							if (!ComboBoxContainsText( dialog, IDC_REFRESH_RATE, text ) )
								ComboBoxAdd( dialog, IDC_REFRESH_RATE, (void*)m_displayModes->GetCurrent()->mode.RefreshRate, text );
						}
					}
					ComboBoxSelect( dialog, IDC_REFRESH_RATE, *m_settingsScript->GetNumberData( "refresh" ) );
				}
			}

			return true;
		}

		case WM_COMMAND:
		{
			switch( LOWORD(wparam) )
			{
				case IDOK:
				{
					// Store the details of the selected display mode.
					m_selectedDisplayMode.Width = LOWORD( PtrToUlong( ComboBoxSelected( dialog, IDC_RESOLUTION ) ) );
					m_selectedDisplayMode.Height = HIWORD( PtrToUlong( ComboBoxSelected( dialog, IDC_RESOLUTION ) ) );
					m_selectedDisplayMode.RefreshRate = PtrToUlong( ComboBoxSelected( dialog, IDC_REFRESH_RATE ) );
					m_selectedDisplayMode.Format = (D3DFORMAT)PtrToUlong( ComboBoxSelected( dialog, IDC_DISPLAY_FORMAT ) );
					m_windowed = IsDlgButtonChecked( dialog, IDC_WINDOWED ) ? true : false;
					m_vsync = IsDlgButtonChecked( dialog, IDC_VSYNC ) ? true : false;

					// Destroy the display modes list.
					SAFE_DELETE( m_displayModes );

					// Get the selected index from each combo box.
					long bpp = ComboBox_GetCurSel( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ) );
					long resolution = ComboBox_GetCurSel( GetDlgItem( dialog, IDC_RESOLUTION ) );
					long refresh = ComboBox_GetCurSel( GetDlgItem( dialog, IDC_REFRESH_RATE ) );

					// Check if the settings script has anything in it.
					if( m_settingsScript->GetBoolData( "windowed" ) == NULL )
					{
						// Add all the settings to the script.
						m_settingsScript->AddVariable( "windowed", VARIABLE_BOOL, &m_windowed );
						m_settingsScript->AddVariable( "vsync", VARIABLE_BOOL, &m_vsync );
						m_settingsScript->AddVariable( "bpp", VARIABLE_NUMBER, &bpp );
						m_settingsScript->AddVariable( "resolution", VARIABLE_NUMBER, &resolution );
						m_settingsScript->AddVariable( "refresh", VARIABLE_NUMBER, &refresh );
					}
					else
					{
						// Set all the settings.
						m_settingsScript->SetVariable( "windowed", &m_windowed );
						m_settingsScript->SetVariable( "vsync", &m_vsync );
						m_settingsScript->SetVariable( "bpp", &bpp );
						m_settingsScript->SetVariable( "resolution", &resolution );
						m_settingsScript->SetVariable( "refresh", &refresh );
					}

					// Save all the settings out to the settings script.
					m_settingsScript->SaveScript();

					// Destroy the settings script.
					SAFE_DELETE( m_settingsScript );

					// Close the dialog.
					EndDialog( dialog, IDOK );

					return true;
				}

				case IDCANCEL:
				{
					// Destroy the display modes list.
					SAFE_DELETE( m_displayModes );

					// Destroy the settings script.
					SAFE_DELETE( m_settingsScript );

					EndDialog( dialog, IDCANCEL );

					return true;
				}

				case IDC_COLOUR_DEPTH:
				{
					if( CBN_SELCHANGE == HIWORD(wparam) )
					{
						char res[16];
						DWORD selectedRes = (DWORD)PtrToUlong( ComboBoxSelected( dialog, IDC_RESOLUTION ) );

						// Update the resolution combo box.
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_RESOLUTION ) );
						m_displayModes->Iterate( true );
						while( m_displayModes->Iterate() )
						{
							if( m_displayModes->GetCurrent()->mode.Format == (D3DFORMAT)PtrToUlong( ComboBoxSelected( dialog, IDC_COLOUR_DEPTH ) ) )
							{
								sprintf( res, "%d x %d", m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height );
								if( !ComboBoxContainsText( dialog, IDC_RESOLUTION, res ) )
								{
									ComboBoxAdd( dialog, IDC_RESOLUTION, (void*)MAKELONG( m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height ), res );
									if( selectedRes == (DWORD)MAKELONG( m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height ) )
										ComboBoxSelect( dialog, IDC_RESOLUTION, (void*)selectedRes );
								}
							}
						}
						if( ComboBoxSelected( dialog, IDC_RESOLUTION ) == NULL )
							ComboBoxSelect( dialog, IDC_RESOLUTION, 0 );
					}

					return true;
				}

				case IDC_RESOLUTION:
				{
					if( CBN_SELCHANGE == HIWORD(wparam) )
					{
						char refresh[16];
						DWORD selectedRefresh = (DWORD)PtrToUlong( ComboBoxSelected( dialog, IDC_REFRESH_RATE ) );

						// Update the refresh rate combo box.
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_REFRESH_RATE ) );
						m_displayModes->Iterate( true );
						while( m_displayModes->Iterate() )
						{
							if( (DWORD)MAKELONG( m_displayModes->GetCurrent()->mode.Width, m_displayModes->GetCurrent()->mode.Height ) == (DWORD)PtrToUlong( ComboBoxSelected( dialog, IDC_RESOLUTION ) ) )
							{
								sprintf( refresh, "%d Hz", m_displayModes->GetCurrent()->mode.RefreshRate );
								if( !ComboBoxContainsText( dialog, IDC_REFRESH_RATE, refresh ) )
								{
									ComboBoxAdd( dialog, IDC_REFRESH_RATE, (void*)m_displayModes->GetCurrent()->mode.RefreshRate, refresh );
									if( selectedRefresh == m_displayModes->GetCurrent()->mode.RefreshRate )
										ComboBoxSelect( dialog, IDC_REFRESH_RATE, (void*)selectedRefresh );
								}
							}
						}
						if( ComboBoxSelected( dialog, IDC_REFRESH_RATE ) == NULL )
							ComboBoxSelect( dialog, IDC_REFRESH_RATE, 0 );
					}

					return true;
				}

				case IDC_WINDOWED:
				case IDC_FULLSCREEN:
				{
					// Check if the user has change to windowed or fullscreen mode.
					if( IsDlgButtonChecked( dialog, IDC_WINDOWED ) )
					{
						// Clear and disable all the fullscreen controls.
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ) );
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_RESOLUTION ) );
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_REFRESH_RATE ) );
						CheckDlgButton( dialog, IDC_VSYNC, false );
						EnableWindow( GetDlgItem( dialog, IDC_VSYNC ), false );
						EnableWindow( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ), false );
						EnableWindow( GetDlgItem( dialog, IDC_RESOLUTION ), false );
						EnableWindow( GetDlgItem( dialog, IDC_REFRESH_RATE ), false );
					}
					else
					{
						// Enable all the fullscreen controls.
						EnableWindow( GetDlgItem( dialog, IDC_VSYNC ), true );
						EnableWindow( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ), true );
						EnableWindow( GetDlgItem( dialog, IDC_RESOLUTION ), true );
						EnableWindow( GetDlgItem( dialog, IDC_REFRESH_RATE ), true );

						// Fill in the display formats combo box.
						ComboBox_ResetContent( GetDlgItem( dialog, IDC_DISPLAY_FORMAT ) );
						m_displayModes->Iterate( true );
						while( m_displayModes->Iterate() )
						{
							if( !ComboBoxContainsText( dialog, IDC_DISPLAY_FORMAT, m_displayModes->GetCurrent()->bpp ) )
								ComboBoxAdd( dialog, IDC_DISPLAY_FORMAT, (void*)m_displayModes->GetCurrent()->mode.Format, m_displayModes->GetCurrent()->bpp );
						}
						ComboBoxSelect( dialog, IDC_DISPLAY_FORMAT, 0 );
					}

					return true;
				}
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Returns the selected display mode.
//-----------------------------------------------------------------------------
D3DDISPLAYMODE *DeviceEnumeration::GetSelectedDisplayMode()
{
	return &m_selectedDisplayMode;
}

//-----------------------------------------------------------------------------
// Indicates if the display is windowed or fulscreen.
//-----------------------------------------------------------------------------
bool DeviceEnumeration::IsWindowed()
{
	return m_windowed;
}

//-----------------------------------------------------------------------------
// Indicates if the display is v-synced.
//-----------------------------------------------------------------------------
bool DeviceEnumeration::IsVSynced()
{
	return m_vsync;
}

//-----------------------------------------------------------------------------
// Adds an entry to the combo box.
//-----------------------------------------------------------------------------
void DeviceEnumeration::ComboBoxAdd( HWND dialog, int id, void *data, char *desc )
{
	HWND control = GetDlgItem( dialog, id );
	int i = ComboBox_AddString( control, desc );
	ComboBox_SetItemData( control, i, data );
}

//-----------------------------------------------------------------------------
// Selects an entry in the combo box by index.
//-----------------------------------------------------------------------------
void DeviceEnumeration::ComboBoxSelect( HWND dialog, int id, int index )
{
	HWND control = GetDlgItem( dialog, id );
	ComboBox_SetCurSel( control, index );
	PostMessage( dialog, WM_COMMAND, MAKEWPARAM( id, CBN_SELCHANGE ), (LPARAM)control );
}

//-----------------------------------------------------------------------------
// Selects an entry in the combo box by data.
//-----------------------------------------------------------------------------
void DeviceEnumeration::ComboBoxSelect( HWND dialog, int id, void *data )
{
	HWND control = GetDlgItem( dialog, id );
	for( int i = 0; i < ComboBoxCount( dialog, id ); i++ )
	{
		if( (void*)ComboBox_GetItemData( control, i ) == data )
		{
			ComboBox_SetCurSel( control, i );
			PostMessage( dialog, WM_COMMAND, MAKEWPARAM( id, CBN_SELCHANGE ), (LPARAM)control );
			return;
		}
	}
}

//-----------------------------------------------------------------------------
// Returns the data for the selected entry in the combo box.
//-----------------------------------------------------------------------------
void *DeviceEnumeration::ComboBoxSelected( HWND dialog, int id )
{
	HWND control = GetDlgItem( dialog, id );
	int index = ComboBox_GetCurSel( control );
	if( index < 0 )
		return NULL;
	return (void*)ComboBox_GetItemData( control, index );
}

//-----------------------------------------------------------------------------
// Checks if a valid entry in the combo box is selected.
//-----------------------------------------------------------------------------
bool DeviceEnumeration::ComboBoxSomethingSelected( HWND dialog, int id )
{
	HWND control = GetDlgItem( dialog, id );
	int index = ComboBox_GetCurSel( control );
	return ( index >= 0 );
}

//-----------------------------------------------------------------------------
// Returns the number of entries in the combo box.
//-----------------------------------------------------------------------------
int DeviceEnumeration::ComboBoxCount( HWND dialog, int id )
{
	HWND control = GetDlgItem( dialog, id );
	return ComboBox_GetCount( control );
}

//-----------------------------------------------------------------------------
// Checks if the combo box contains the given text.
//-----------------------------------------------------------------------------
bool DeviceEnumeration::ComboBoxContainsText( HWND dialog, int id, char *text )
{
	char item[MAX_PATH];
	HWND control = GetDlgItem( dialog, id );
	for( int i = 0; i < ComboBoxCount( dialog, id ); i++ )
	{
		ComboBox_GetLBText( control, i, item );
		if( lstrcmp( item, text ) == 0 )
			return true;
	}
	return false;
}