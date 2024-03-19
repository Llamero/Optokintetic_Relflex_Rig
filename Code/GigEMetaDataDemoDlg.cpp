//==============================================================================
// Module : GigEMetaDataDemoDlg.cpp
// Project: Sapera samples.
// Purpose: This module contains the methods that define the CGigEMetaDataDemoDlg class.
//==============================================================================
// Include files.

#include "stdafx.h"
#include "GigEMetaDataDemo.h"
#include "GigEMetaDataDemoDlg.h"
#include "GigEMetaDataDemoAbout.h"
#include "float.h"
#include "Windows.h"
#include "thread"
#include "chrono"

#define debug false

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::CGigEMetaDataDemoDlg
// Purpose   : Class constructor.
// Parameters:
//    pCParent                Parent window.
//==============================================================================
CGigEMetaDataDemoDlg::CGigEMetaDataDemoDlg(CWnd* pParent)
   : CDialog(CGigEMetaDataDemoDlg::IDD, pParent)
{
   //{{AFX_DATA_INIT(CGigEMetaDataDemoDlg)
   m_LiveFrameRate = _T("");
   m_BufferFrameRate = 0.0f;
   m_MaxTime = 0.0f;
   m_MinTime = 0.0f;
   m_ActiveBuffer = 0;
   m_BufferCount = 0;
   m_Slider = 0;
   selectedServer = 0; //Index of camera selected in dropdown - this will be the view camera
   serverIndex = 0; //Counter for current camera index
   masterServer = NULL; //Index of camera that sends trigger output, controlling all other cameras

   //Timer variables
   PCFreq = 0.0;
   CounterStart = 0;


   //}}AFX_DATA_INIT

   // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
   serverCount = SapManager::GetServerCount(SapManager::ResourceAcqDevice); //Only count cameras
   if (debug) {
       CString str;
       str.Format(_T("Cameras Found: %d"), serverCount);
       MessageBox(str);
   }
   m_AcqDevice.resize(serverCount);
   std::vector<SapAcqDevice*> m_AcqDevice(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_Feature.resize(serverCount);
   std::vector<SapAcqDevice*> m_Feature(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_Buffers.resize(serverCount);
   std::vector<SapAcqDevice*> m_Buffers(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_tempBuffers.resize(serverCount);
   std::vector<SapAcqDevice*> m_tempBuffers(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_Xfer.resize(serverCount);
   std::vector<SapAcqDevice*> m_Xfer(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_View.resize(serverCount);
   std::vector<SapAcqDevice*> m_View(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c
   m_Metadata.resize(serverCount);
   std::vector<SapAcqDevice*> m_Metadata(serverCount, NULL); //https://stackoverflow.com/questions/5887615/creating-an-array-of-object-pointers-c

   m_metadataType = SapMetadata::MetadataUnknown;

   m_IsSignalDetected = TRUE;
   m_IsAreaScan = TRUE;

   m_bRecordOn = FALSE;
   m_bPlayOn = FALSE;
   m_bPauseOn = FALSE;

   m_XferDisconnectDuringSetup = FALSE;
   m_IsSelectAvailable = FALSE;

   m_nFramesPerCallback = 1;
   m_MetadataFileIndex = 1;

   m_BufferIsValid = NULL;
   m_tempBufferIsValid = NULL;

   m_TimestampCurrent = _T("");
   m_TimestampBuffer = 0;
   m_TimestampBufferDelta = 0;
   m_PlayLastClock = 0;
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::DoDataExchange
// Purpose   : Data exchange.
// Parameters:
//    pDX
//==============================================================================
void CGigEMetaDataDemoDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CGigEMetaDataDemoDlg)
   DDX_Control(pDX, IDC_STOP, m_stopButton);
   DDX_Control(pDX, IDC_PAUSE, m_pauseButton);
   DDX_Control(pDX, IDC_PLAY, m_playButton);
   DDX_Control(pDX, IDC_RECORD, m_recordButton);
   DDX_Control(pDX, IDC_SLIDER, m_SliderCtrl);
   DDX_Control(pDX, IDC_VIEW_WND, m_ImageWnd1);
   DDX_Text(pDX, IDC_LIVE_FRAME_RATE, m_LiveFrameRate);
   DDX_Text(pDX, IDC_BUFFER_FRAME_RATE, m_BufferFrameRate);
   DDX_Text(pDX, IDC_MAX_TIME, m_MaxTime);
   DDX_Text(pDX, IDC_MIN_TIME, m_MinTime);
   DDX_Text(pDX, IDC_ACTIVE, m_ActiveBuffer);
   DDX_Text(pDX, IDC_AVAILABLE, m_BufferCount);
   DDX_Slider(pDX, IDC_SLIDER, m_Slider);
   DDX_Control(pDX, IDC_CHECKLIST_METADATA, m_checklistMetadata);
   DDX_Control(pDX, IDC_METADATA_ACTIVE_MODE, m_checkEnable);
   DDX_Control(pDX, IDC_LIST_METADATA_VIEW, m_listMetadataView);
   DDX_Control(pDX, IDC_LIST_METADATA_VIEW2, m_listMetadataView2);
   DDX_Text(pDX, IDC_TIMESTAMP_CURRENT, m_TimestampCurrent);
   DDX_Text(pDX, IDC_TIMESTAMP_BUFFER, m_TimestampBuffer);
   DDX_Text(pDX, IDC_TIMESTAMP_BUFFER_DELTA, m_TimestampBufferDelta);
   //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGigEMetaDataDemoDlg, CDialog)
   //{{AFX_MSG_MAP(CGigEMetaDataDemoDlg)
   ON_WM_SYSCOMMAND()
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_WM_DESTROY()
   ON_WM_MOVE()
   ON_WM_SIZE()
   ON_WM_HSCROLL()
   ON_WM_VSCROLL()
   ON_WM_TIMER()
   ON_BN_CLICKED(IDC_RECORD, OnBnClickedRecord)
   ON_BN_CLICKED(IDC_PLAY, OnBnClickedPlay)
   ON_BN_CLICKED(IDC_PAUSE, OnBnClickedPause)
   ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
   ON_BN_CLICKED(IDC_BUFFER_OPTIONS, OnBnClickedBufferOptions)
   ON_BN_CLICKED(IDC_LOAD_ACQ_CONFIG, OnBnClickedLoadAcqConfig)
   ON_BN_CLICKED(IDC_FILE_LOAD, OnBnClickedFileLoad)
   ON_BN_CLICKED(IDC_FILE_SAVE, OnBnClickedFileSave)
   ON_EN_KILLFOCUS(IDC_BUFFER_FRAME_RATE, OnKillfocusBufferFrameRate)
   ON_BN_CLICKED(IDC_FILE_LOAD_CURRENT, OnBnClickedFileLoadCurrent)
   ON_BN_CLICKED(IDC_FILE_SAVE_CURRENT, OnBnClickedFileSaveCurrent)
   ON_MESSAGE(WM_UPDATE_CONTROLS, OnUpdateControls)
   ON_BN_CLICKED(IDC_HIGH_FRAME_RATE, OnBnClickedHighFrameRate)
   ON_WM_ENDSESSION()
   ON_WM_QUERYENDSESSION()
   ON_BN_CLICKED(IDC_SAVE_METADATA, &CGigEMetaDataDemoDlg::OnBnClickedSaveMetadata)
   ON_BN_CLICKED(IDC_METADATA_ACTIVE_MODE, &CGigEMetaDataDemoDlg::OnBnClickedMetadataActiveMode)
   ON_CLBN_CHKCHANGE(IDC_CHECKLIST_METADATA, &CGigEMetaDataDemoDlg::OnClBnChkChangeCheckListMetadata)
   ON_BN_CLICKED(IDC_READ_CURRENT_TIMESTAMP, &CGigEMetaDataDemoDlg::OnBnClickedReadCurrentTimestamp)
   ON_BN_CLICKED(IDC_RESET_TIMESTAMP, &CGigEMetaDataDemoDlg::OnBnClickedResetTimestamp)
   //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::XferCallback
// Purpose   : Callback function for the transfer.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::XferCallback(SapXferCallbackInfo* pInfo)
{
    static int counter = 0;
    counter++;
    CGigEMetaDataDemoDlg* pDlg = (CGigEMetaDataDemoDlg*)pInfo->GetContext();
   SapBuffer::State bufState = SapBuffer::StateEmpty;
   int index = pDlg->selectedServer - 1;

   int bufIndex = pDlg->m_Buffers[index]->GetIndex();
   pDlg->m_Buffers[index]->GetState(bufIndex, &bufState);
   pDlg->m_BufferIsValid[bufIndex] = (bufState == SapBuffer::StateFull);

   // Measure real frame time
   pDlg->UpdateFrameRate();

   // Check if last frame is reached
   pDlg->CheckForLastFrame();

   // Refresh view
   if(index == 0) pDlg->m_View[0]->Show();

   // Refresh controls
   pDlg->PostMessage(WM_UPDATE_CONTROLS, 0, 0);
   if (counter % pDlg->serverCount != 0) {

   }
}

#pragma region image viewer event handlers

void CGigEMetaDataDemoDlg::PixelChanged(int x, int y)
{
   CString str = m_appTitle;
   str += "  " + m_ImageWnd1.GetPixelString(CPoint(x, y));
   SetWindowText(str);
}

void CGigEMetaDataDemoDlg::MarkerChanged()
{
   uint markerLine = m_ImageWnd1.GetMarkerLine();
   CString str;
   str.Format(_T("    [ marker at line %d ]"), markerLine);
   SetWindowText(m_appTitle + str);

   UpdateMetadataList();
}

void CGigEMetaDataDemoDlg::InteractiveMarkerChanged(int lineIndex)
{
   CString str;
   str.Format(_T("    [ interactive marker at line %d ]"), lineIndex);
   SetWindowText(m_appTitle + str);
}

#pragma endregion

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnInitDialog
// Purpose   : Dialog initialization.
// Parameters: None
//==============================================================================
BOOL CGigEMetaDataDemoDlg::OnInitDialog(void)
{
   CDialog::OnInitDialog();
   CDialog::SetDefID(IDC_METADATA_ACTIVE_MODE);

   // Add "About..." menu item to system menu.

   // IDM_ABOUTBOX must be in the system command range.
   ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
   ASSERT(IDM_ABOUTBOX < 0xF000);

   CMenu* pSysMenu = GetSystemMenu(FALSE);
   if (pSysMenu != NULL)
   {
      CString strAboutMenu;
      strAboutMenu.LoadString(IDS_ABOUTBOX);
      if (!strAboutMenu.IsEmpty())
      {
         pSysMenu->AppendMenu(MF_SEPARATOR);
         pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
      }

      pSysMenu->EnableMenuItem(SC_MAXIMIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
      pSysMenu->EnableMenuItem(SC_SIZE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
   }

   // Set the icon for this dialog.  The framework does this automatically
   //  when the application's main window is not a dialog
   SetIcon(m_hIcon, TRUE);       // Set big icon
   SetIcon(m_hIcon, FALSE);      // Set small icon

   // Initialize variables
   GetWindowText(m_originalAppTitle);

   // We must be operating on-line
   CAcqConfigDlg dlg(this, CAcqConfigDlg::ServerAcqDevice);
   if (dlg.DoModal() != IDOK)
   {
      MessageBox(_T("No device found"));
      EndDialog(TRUE);
      return FALSE;
   }

   // Define objects
   serverCount = SapManager::GetServerCount(SapManager::ResourceAcqDevice); //Only count cameras
   CString str;
   SapLocation location;
   int resourceIndex = dlg.GetLocation().GetResourceIndex(); //Get index of resource (normally 0)
   selectedServer = dlg.GetLocation().GetServerIndex(); //Get selected camera index
   str.Format(_T("Resource #%d, Selection #%d"), resourceIndex, selectedServer);
   if (debug) MessageBox(str);
   char serverName[CORSERVER_MAX_STRLEN];
   for (serverIndex = 0; serverIndex < serverCount; serverIndex++) {
       SapManager::GetServerName(serverIndex+1, serverName, sizeof(serverName)); //Get Server name
       location = SapLocation(CStringA(serverName), resourceIndex);
       str.Format(_T("Initializing device #%d - %s"), serverIndex, CString(serverName));
       if(debug) MessageBox(str);
       m_AcqDevice[serverIndex] = new SapAcqDevice(location, dlg.GetConfigFile());
       m_Feature[serverIndex] = new SapFeature(location);
       m_Buffers[serverIndex] = new SapBufferWithTrash(MAX_BUFFER);
       m_tempBuffers[serverIndex] = new SapBufferWithTrash(MAX_BUFFER);
       m_Metadata[serverIndex] = new SapMetadata(m_AcqDevice[serverIndex], m_Buffers[serverIndex]);
       if(true) m_Xfer[serverIndex] = new SapAcqDeviceToBuf(m_AcqDevice[serverIndex], m_Buffers[serverIndex], XferCallback, this);
       else m_Xfer[serverIndex] = new SapAcqDeviceToBuf(m_AcqDevice[serverIndex], m_Buffers[serverIndex], NULL, this); //Dont waste time doing call back for buffers that aren't displayed
   }

   // Attach sapview to image viewer to the selected camera
   str.Format(_T("Attach view to %d"), selectedServer);
   if (debug) MessageBox(str);
   m_View[0] = new SapView(m_Buffers[selectedServer-1]);
   m_ImageWnd1.AttachSapView(m_View[0]);

   // Create all objects
   serverCount = SapManager::GetServerCount(SapManager::ResourceAcqDevice); //Only count cameras
   for (int serverIndex = serverCount-1; serverIndex > -1; serverIndex--) {
       //createView = (*m_View[serverIndex] != NULL); //Build view object only if it isn't null
       str.Format(_T("Building device #%d."), serverIndex);
       if (debug) { MessageBox(str); }
       if (!CreateObjects(serverIndex))
       {
           EndDialog(TRUE);
           return FALSE;
       }
   }

   // Create image window
   m_ImageWnd1.AttachEventHandler(this);
   m_ImageWnd1.Reset();

   UpdateMenu();

   return TRUE;
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::CreateObjects
// Purpose   : Create sapera objects.
// Parameters: None
//==============================================================================
BOOL CGigEMetaDataDemoDlg::CreateObjects(int deviceIndex, bool createView, bool createAcqDevice)
{
   CString str;

   //Number of frames per callback retreived
   int nFramesPerCallback;

   // Create acquisition object
   if (createAcqDevice && m_AcqDevice[deviceIndex] && !*m_AcqDevice[deviceIndex] && !m_AcqDevice[deviceIndex]->Create())
   {
      DestroyObjects(deviceIndex);
      return FALSE;
   }

   // Make sure the acq device supports mandatory metadata features
   if (!SapMetadata::IsMetadataSupported(m_AcqDevice[deviceIndex]))
   {
      MessageBox(_T("This demo only supports Teledyne Dalsa and Teledyne Lumenera cameras with metadata features"),
                 _T("Incompatible camera"), MB_ICONERROR);
      DestroyObjects(deviceIndex);
      return FALSE;
   }

   // Create feature object
   if (m_Feature[deviceIndex] && !*m_Feature[deviceIndex] && !m_Feature[deviceIndex]->Create())
   {
      DestroyObjects(deviceIndex);
      return FALSE;
   }

   // Create metadata object
   if (m_Metadata[deviceIndex] && !*m_Metadata[deviceIndex] && !m_Metadata[deviceIndex]->Create())
   {
      DestroyObjects(deviceIndex);
      return FALSE;
   }

   m_IsAreaScan = (m_Metadata[deviceIndex]->GetMetadataType() == SapMetadata::MetadataPerFrame);

   // Check if metadata selectors can be enabled through application code
   m_IsSelectAvailable = FALSE;
   char* featurename = m_IsAreaScan ? "ChunkEnable" : "endOfLineMetadataContentActivationMode";
   if (m_AcqDevice[deviceIndex]->GetFeatureInfo(featurename, m_Feature[deviceIndex]))
   {
      SapFeature::AccessMode accessMode;
      if (m_Feature[deviceIndex]->GetAccessMode(&accessMode))
         m_IsSelectAvailable = (accessMode == SapFeature::AccessRW);
   }

   // Check if the transfer needs to remain disconnected when enabling / disabling / selecting metadata
   m_XferDisconnectDuringSetup = FALSE;
   featurename = m_IsAreaScan ? "ChunkModeActive" : "endOfLineMetadataMode";
   if (m_AcqDevice[deviceIndex]->GetFeatureInfo(featurename, m_Feature[deviceIndex]))
   {
      SapFeature::WriteMode writeMode;
      if (m_Feature[deviceIndex]->GetWriteMode(&writeMode))
         m_XferDisconnectDuringSetup = (writeMode == SapFeature::WriteNotConnected);
   }

   // Create buffer object
   if (m_Buffers[deviceIndex] && !*m_Buffers[deviceIndex])
   {
      if (!m_Buffers[deviceIndex]->Create())
      {
         DestroyObjects(deviceIndex);
         return FALSE;
      }
      // Clear all buffers
      m_Buffers[deviceIndex]->Clear();
      m_BufferIsValid = new BOOL[m_Buffers[deviceIndex]->GetCount()];

   }

   // Create temp buffer object
   if (m_tempBuffers[deviceIndex] && !*m_tempBuffers[deviceIndex])
   {
       if (!m_tempBuffers[deviceIndex]->Create())
       {
           DestroyObjects(deviceIndex);
           return FALSE;
       }
       // Clear temp all buffers
       m_tempBuffers[deviceIndex]->Clear();

       m_tempBufferIsValid = new BOOL[m_tempBuffers[deviceIndex]->GetCount()];
   }

   // Create view object
   if (deviceIndex == 0) {
       str.Format(_T("Build view 1"));
       if (debug) MessageBox(str);
       if (m_View[deviceIndex] && !*m_View[deviceIndex] && !m_View[deviceIndex]->Create())
       {
           DestroyObjects(deviceIndex);
           return FALSE;
       }
   }

   if (m_Xfer[deviceIndex] && !*m_Xfer[deviceIndex])
   {
      // Set number of frames per callback
      m_Xfer[deviceIndex]->GetPair(0)->SetFramesPerCallback(m_nFramesPerCallback);

      // If there is a large number of buffers, temporarily boost the command timeout value,
      // since the call to Create may take a long time to complete.
      // As a safe rule of thumb, use 100 milliseconds per buffer.
      int oldCommandTimeout = SapManager::GetCommandTimeout();
      int newCommandTimeout = 100 * m_Buffers[deviceIndex]->GetCount();

      if (newCommandTimeout < oldCommandTimeout)
         newCommandTimeout = oldCommandTimeout;

      SapManager::SetCommandTimeout(newCommandTimeout);

      // Create transfer object
      if (!m_Xfer[deviceIndex]->Create())
      {
         DestroyObjects(deviceIndex);
         return FALSE;
      }

      // Restore original command timeout value
      SapManager::SetCommandTimeout(oldCommandTimeout);

      m_Xfer[deviceIndex]->Init(TRUE); // initialize tranfer object and reset source/destination index

      // Retrieve number of frames per callback
      // It may be less than what we have asked for.
      nFramesPerCallback = m_Xfer[deviceIndex]->GetPair(0)->GetFramesPerCallback();
      if (m_nFramesPerCallback > nFramesPerCallback)
      {
         m_nFramesPerCallback = nFramesPerCallback;
         AfxMessageBox(_T("No memory"));
      }
   }

   // Show type of metadata in title bar
   if (deviceIndex == 0) {
       str.Format(_T("Build view 2"));
       if (debug) MessageBox(str);
       m_appTitle = m_originalAppTitle;
       m_metadataType = m_Metadata[deviceIndex]->GetMetadataType();
       switch (m_metadataType)
       {
       case SapMetadata::MetadataPerFrame:
           m_appTitle += " (Per-Frame Metadata)";
           break;
       case SapMetadata::MetadataPerLine:
           m_appTitle += " (Per-Line Metadata)";
           m_ImageWnd1.EnableMarker();
           SetDlgItemText(IDC_STATIC_METADATA_LIST_HEADER, _T("Metadata of the displayed buffer and marker"));
           break;
       default:
           m_appTitle += " -- Acquisition device doesn't support Metadata";
           break;
       }
       SetWindowText(m_appTitle);

       ReadCameraTimestamp();

       if (m_XferDisconnectDuringSetup)
           m_Xfer[deviceIndex]->Disconnect();

       // Feed the list of selectors
       m_checklistMetadata.ResetContent();
       UINT selectorCount = m_Metadata[deviceIndex]->GetSelectorCount();
       char selectorName[MAX_PATH] = { 0 };
       for (UINT selectorIndex = 0; selectorIndex < selectorCount; selectorIndex++)
       {
           if (m_Metadata[deviceIndex]->GetSelectorName(selectorIndex, selectorName, MAX_PATH))
           {
               int pos = m_checklistMetadata.AddString(CString(selectorName));
               m_checklistMetadata.SetItemData(pos, selectorIndex);
               m_checklistMetadata.SetCheck(pos, m_Metadata[deviceIndex]->IsSelected(selectorIndex) ? BST_CHECKED : BST_UNCHECKED);
           }
       }

       if (m_XferDisconnectDuringSetup)
           m_Xfer[deviceIndex]->Connect();

       // Clear extracted chunks in their display listboxes
       m_listMetadataView.ResetContent();
       m_listMetadataView2.ResetContent();

       UpdateMenu();
   }

   return TRUE;
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::DestryObjects
// Purpose   : Destroy sapera objects.
// Parameters: None
//==============================================================================
BOOL CGigEMetaDataDemoDlg::DestroyObjects(int deviceIndex, bool destroyAcqDevice)
{
   // Destroy transfer object
   if (m_Xfer[deviceIndex] && *m_Xfer[deviceIndex])
      m_Xfer[deviceIndex]->Destroy();

   // Destroy view object
   if (m_View[deviceIndex] && *m_View[deviceIndex])
      m_View[deviceIndex]->Destroy();

   // Destroy buffer object
   if (m_BufferIsValid != NULL)
   {
      delete [] m_BufferIsValid;
      m_BufferIsValid = NULL;
   }

   if (m_Buffers[deviceIndex] && *m_Buffers[deviceIndex])
      m_Buffers[deviceIndex]->Destroy();

   // Destroy temp buffer object
   if (m_tempBufferIsValid != NULL)
   {
       delete[] m_tempBufferIsValid;
       m_tempBufferIsValid = NULL;
   }

   if (m_tempBuffers[deviceIndex] && *m_tempBuffers[deviceIndex])
       m_tempBuffers[deviceIndex]->Destroy();

   // Destroy metadata object
   if (m_Metadata[deviceIndex] && *m_Metadata[deviceIndex])
      m_Metadata[deviceIndex]->Destroy();

   // Destroy feature object
   if (m_Feature[deviceIndex] && *m_Feature[deviceIndex])
      m_Feature[deviceIndex]->Destroy();

   // Destroy acquisition object
   if (destroyAcqDevice && m_AcqDevice[deviceIndex] && *m_AcqDevice[deviceIndex])
      m_AcqDevice[deviceIndex]->Destroy();

   return TRUE;
}

#pragma region metadata related code

void CGigEMetaDataDemoDlg::OnBnClickedSaveMetadata()
{
   CFileDialog fileDlg(FALSE, _T("csv"), _T("metadata.csv"), OFN_HIDEREADONLY | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT, _T("Metadata Files (*.csv)|*.csv||"), this);
   if (fileDlg.DoModal() == IDOK)
   {
      CString pathName;
      CStringA path;
      CString pathNameRoot = fileDlg.GetPathName();
      CWaitCursor cur;
      pathNameRoot = pathNameRoot.Left(pathNameRoot.GetLength() - 4);
      for (int i=0; i < serverCount; i++) {
          pathName.Format(_T("%s-%i.csv"), pathNameRoot, i+1);
          if (m_Metadata[i]->SaveToCSV((CStringA)pathName)) {
              if (i == serverCount - 1)
                  MessageBox(_T("The metadata has been succesfully saved to the file"), _T("We are done"), MB_ICONINFORMATION);
          }
          else
              MessageBox(_T("Failed to save the metadata to the file"), _T("Failed to save"), MB_ICONERROR);
      }
   }
}

void CGigEMetaDataDemoDlg::OnBnClickedMetadataActiveMode()
{
    for (int i=0; i < serverCount; i++) {
        if (m_XferDisconnectDuringSetup)
            m_Xfer[i]->Disconnect();

        m_Metadata[i]->Enable(m_checkEnable.GetCheck() == BST_CHECKED);

        // Check if metadata selectors can be enabled through application code
        m_IsSelectAvailable = FALSE;
        char* featurename = m_IsAreaScan ? "ChunkEnable" : "endOfLineMetadataContentActivationMode";
        if (m_AcqDevice[i]->GetFeatureInfo(featurename, m_Feature[i]))
        {
            SapFeature::AccessMode accessMode;
            if (m_Feature[i]->GetAccessMode(&accessMode))
                m_IsSelectAvailable = (accessMode == SapFeature::AccessRW);
        }

        if (m_XferDisconnectDuringSetup)
            m_Xfer[i]->Connect();

        UpdateMenu();
    }
}

void CGigEMetaDataDemoDlg::OnClBnChkChangeCheckListMetadata()
{
   int itemIndex = m_checklistMetadata.GetCaretIndex();
   BOOL itemIndexChecked = (m_checklistMetadata.GetCheck(itemIndex) != 0);

   // Update selected item's checkbox
   m_Metadata[selectedServer-1]->Select(itemIndex, itemIndexChecked);

   // Update all items checkboxes, in case another one changed because of the itemIndex one
   UINT selectorCount = m_Metadata[selectedServer - 1]->GetSelectorCount();
   for (UINT selectorIndex = 0; selectorIndex < selectorCount; selectorIndex++)
   {
      BOOL bIsSelected = m_Metadata[selectedServer - 1]->IsSelected(selectorIndex);
      m_checklistMetadata.SetCheck(selectorIndex, bIsSelected ? BST_CHECKED : BST_UNCHECKED);
   }
}

void CGigEMetaDataDemoDlg::UpdateMetadataList()
{
   m_listMetadataView.ResetContent();
   m_listMetadataView2.ResetContent();

   if (!m_Metadata[masterServer]->IsEnabled())
      return;

   if (!m_BufferIsValid[m_Buffers[masterServer]->GetIndex()])
      return;

   BOOL bExtractStatus = FALSE;

   if (m_metadataType == SapMetadata::MetadataPerFrame)
      bExtractStatus = m_Metadata[masterServer]->Extract(m_Buffers[masterServer]->GetIndex());
   else if (m_metadataType == SapMetadata::MetadataPerLine)
   {
      uint markerLine = m_ImageWnd1.GetMarkerLine();
      bExtractStatus = m_Metadata[masterServer]->Extract(m_Buffers[masterServer]->GetIndex(), markerLine);
   }
   else
      return;

   if (bExtractStatus)
   {
      UINT resultCount = m_Metadata[masterServer]->GetExtractedResultCount();

      for (UINT resultIndex = 0; resultIndex < resultCount; resultIndex++)
      {
         char sResultName[MAX_PATH] = { 0 };
         char sResultValue[MAX_PATH] = { 0 };
         if (m_Metadata[masterServer]->GetExtractedResult(resultIndex, sResultName, MAX_PATH, sResultValue, MAX_PATH))
         {
            m_listMetadataView.AddString(CString(sResultName));
            m_listMetadataView2.AddString(CString(sResultValue));
         }
      }
   }
}

#pragma endregion

#pragma region MFC message handlers
//==============================================================================
// Name      : CGigEMetaDataDemoDlg::DestryObjects
// Purpose   : Handle commands from system, only handles about box.
// Parameters:
//    nID
//    lParam
//==============================================================================
void CGigEMetaDataDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
   if ((nID & 0xFFF0) == IDM_ABOUTBOX)
   {
      CAboutDlg dlgAbout;
      dlgAbout.DoModal();
   }
   else
      CDialog::OnSysCommand(nID, lParam);
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnPaint
// Purpose   : If you add a minimize button to your dialog, you will need the code
//             below to draw the icon.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnPaint(void)
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

      SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnQueryDragIcon
// Purpose   : The system calls this to obtain the cursor to display while the
//             user drags the minimized window.
// Parameters: None
//==============================================================================
HCURSOR CGigEMetaDataDemoDlg::OnQueryDragIcon(void)
{
   return (HCURSOR)m_hIcon;
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnDestroy
// Purpose   : Handle the destroy message.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnDestroy(void)
{
   CDialog::OnDestroy();

   serverCount = SapManager::GetServerCount(SapManager::ResourceAcqDevice); //Only count cameras
   for (int serverIndex = 0; serverIndex < serverCount; serverIndex++) {
       // Destroy all objects
       DestroyObjects(serverIndex);

       // Delete all objects
       if (m_View[serverIndex])			delete m_View[serverIndex];
       if (m_Xfer[serverIndex])			delete m_Xfer[serverIndex];
       if (m_Buffers[serverIndex])		delete m_Buffers[serverIndex];
       if (m_Metadata[serverIndex])   delete m_Metadata[serverIndex];
       if (m_Feature[serverIndex])	   delete m_Feature[serverIndex];
       if (m_AcqDevice[serverIndex])	delete m_AcqDevice[serverIndex];
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnSize
// Purpose   : Handle the size event.
// Parameters:
//    cx                      New X size.
//    cy                      New Y size.
//==============================================================================
void CGigEMetaDataDemoDlg::OnSize(UINT nType, int cx, int cy)
{
   CDialog::OnSize(nType, cx, cy);

   CRect rClient;
   GetClientRect(rClient);

   // resize slider
   if (m_SliderCtrl.GetSafeHwnd())
   {
      CRect rWnd;
      m_SliderCtrl.GetWindowRect(rWnd);
      ScreenToClient(rWnd);
      rWnd.right = rClient.right - 5;
      m_SliderCtrl.MoveWindow(rWnd);
   }

   // resize image viewer
   if (m_ImageWnd1.GetSafeHwnd())
   {
      CRect rWnd;
      m_ImageWnd1.GetWindowRect(rWnd);
      ScreenToClient(rWnd);
      rWnd.right = rClient.right - 5;
      rWnd.bottom = rClient.bottom - 5;
      m_ImageWnd1.MoveWindow(rWnd);
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnHScroll
// Purpose   : Handle the horizontal scroll bar events.
// Parameters:
//    nSBCode                 Scroll bar code.
//    nPos                    New scroll bar position.
//    pScrollBar              Scroll bar object.
//==============================================================================
void CGigEMetaDataDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
   if (pScrollBar->GetDlgCtrlID() == IDC_SLIDER)
   {
      // Get slider position
      UpdateData(TRUE);

      // Update buffer index
      m_Buffers[selectedServer-1]->SetIndex(m_Slider);

      // Refresh controls
      OnUpdateControls(0, 0);

      UpdateMetadataList();

      // Resfresh display
      m_View[0]->Show();
   }

   CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGigEMetaDataDemoDlg::OnEndSession(BOOL bEnding)
{
   CDialog::OnEndSession(bEnding);

   if (bEnding)
      OnDestroy(); // If ending the session, free the resources.
}

BOOL CGigEMetaDataDemoDlg::OnQueryEndSession()
{
   if (!CDialog::OnQueryEndSession())
      return FALSE;

   return TRUE;
}

#pragma endregion

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnKillfocusBufferFrameRate
// Purpose   : Handle the
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnKillfocusBufferFrameRate(void)
{
   UpdateData(TRUE);
   m_Buffers[0]->SetFrameRate(m_BufferFrameRate);
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::UpdateMenu
// Purpose   : Updates the menu items enabling/disabling the proper items
//             depending on the state of the application.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::UpdateMenu(void)
{
   BOOL bAcqNoGrab = m_Xfer[0] && *m_Xfer[0] && !m_bRecordOn && !m_bPlayOn;
   BOOL bNoGrab = !m_bRecordOn && !m_bPlayOn;

   // Record Control
   m_recordButton.EnableWindow(bAcqNoGrab);
   m_playButton.EnableWindow(bNoGrab);
   m_pauseButton.EnableWindow(!bNoGrab);
   m_stopButton.EnableWindow(!bNoGrab);

   m_pauseButton.SetWindowText(m_bPauseOn ? _T("Continue") : _T("Pause"));

   // General Options
   GetDlgItem(IDC_BUFFER_OPTIONS)->EnableWindow(bNoGrab);
   GetDlgItem(IDC_LOAD_CAM_VIC)->EnableWindow(m_Xfer[0] && bNoGrab);
   GetDlgItem(IDC_HIGH_FRAME_RATE)->EnableWindow(m_Xfer[0] && bNoGrab);

   // File Options
   GetDlgItem(IDC_FILE_LOAD)->EnableWindow(bNoGrab);
   GetDlgItem(IDC_FILE_SAVE)->EnableWindow(bNoGrab);

   // Metadata controls
   BOOL bIsMetadataEnabled = m_Metadata[0]->IsEnabled();

   m_checkEnable.SetCheck(bIsMetadataEnabled ? BST_CHECKED : BST_UNCHECKED);
   m_checkEnable.EnableWindow(bAcqNoGrab);
   m_checklistMetadata.EnableWindow(bAcqNoGrab && bIsMetadataEnabled && m_IsSelectAvailable);
   m_listMetadataView.EnableWindow(bAcqNoGrab && bIsMetadataEnabled);
   m_listMetadataView2.EnableWindow(bAcqNoGrab && bIsMetadataEnabled);
   if (!bIsMetadataEnabled)
   {
      m_listMetadataView.ResetContent();
      m_listMetadataView2.ResetContent();
   }

   // Slider
   m_SliderCtrl.EnableWindow(bNoGrab || (m_bPlayOn && m_bPauseOn));
   m_SliderCtrl.SetRange(0, m_Buffers[0]->GetCount() - 1, TRUE);

   // If last control was disabled, set default focus
   if (!GetFocus())
      GetDlgItem(IDC_BUFFER_OPTIONS)->SetFocus();

   // Update control values
   PostMessage(WM_UPDATE_CONTROLS, 0, 0);
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::UpdateFrameRate
// Purpose   : Calculate statistics on frame rate.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::UpdateFrameRate(void)
{
   if (m_Xfer[0]->UpdateFrameRateStatistics())
   {
      SapXferFrameRateInfo* pStats = m_Xfer[0]->GetFrameRateStatistics();

      if (pStats->IsLiveFrameRateAvailable() && !pStats->IsLiveFrameRateStalled())
      {
         CString sLiveFrameRate;
         sLiveFrameRate.Format(_T("%.1f"), pStats->GetLiveFrameRate());
         m_LiveFrameRate = sLiveFrameRate;
      }
      else
      {
         m_LiveFrameRate = _T("N/A");
      }

      m_BufferFrameRate = pStats->GetBufferFrameRate();
      m_MinTime = pStats->GetMinTimePerFrame();
      m_MaxTime = pStats->GetMaxTimePerFrame();

      if (pStats->IsBufferFrameRateAvailable())
         m_Buffers[0]->SetFrameRate(m_BufferFrameRate);
      else
         m_Buffers[0]->SetFrameRate(pStats->GetLiveFrameRate());
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::CheckForLastFrame
// Purpose   : Check if the last frame needed has been acquired.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::CheckForLastFrame(void)
{
    static bool save = false;
    // Check for last frame
   if (m_Buffers[masterServer]->GetIndex() == m_Buffers[masterServer]->GetCount() - 1)
   {
      if (m_bRecordOn)
      {
         m_bRecordOn = FALSE;
         KillTimer(1);
      }
      if (m_bPlayOn)
      {
         m_bPlayOn = FALSE;
         KillTimer(1);
      }
      if (debug) {
          CString str = "Buffer test\n";
          CString tempStr = "Buffer test\n";
          uint8_t pixel = 0;
          for (int j = 0; j < serverCount; j++) {
            for (int i = 0; i < m_Buffers[j]->GetCount(); i++) {
                  m_Buffers[j]->ReadElement(i, 200, 200, &pixel);
                  tempStr.Format(_T("%d\t"), pixel);
                  str += tempStr;
              }
              str += "\n";
          }
          MessageBox(str);
      }

      UpdateMenu();

      UpdateMetadataList();
   }
   else if (m_Buffers[0]->GetIndex() == 50 && !save) {
       save = true;
       CString pathName;
       pathName.Format(_T("E:/Sapera_temp/test.raw"));
       m_tempBuffers[0]->CopyAll(m_Buffers[0]);
       //m_Buffers[0]->SetIndex(0);
       if (m_tempBuffers[0]->Save((CStringA)pathName, "-format raw", 1, 10));
       else MessageBox(_T("N"));
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnTimer
// Purpose   :
// Parameters:
//    nIDEvent
//==============================================================================
void CGigEMetaDataDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
   if (nIDEvent == 1)
   {
      // Increase buffer index
      m_Buffers[selectedServer-1]->Next();

      // Calculate the normal frame count in the interval
      clock_t curClock = clock();
      float elapsedSeconds = (curClock - m_PlayLastClock) / (float)CLOCKS_PER_SEC;
      m_PlayLastClock = curClock;
      int normalFrameCountInInterval = (int)(elapsedSeconds * m_BufferFrameRate);

      // Skip some frame if we lose some time since last been here (or since OnBnClickedPlay)
      int i;
      for (i = 1; (i < normalFrameCountInInterval) && (m_Buffers[selectedServer-1]->GetIndex() < m_Buffers[selectedServer-1]->GetCount() - 1); i++)
         m_Buffers[selectedServer-1]->Next();

      // Resfresh display
      m_View[0]->Show();

      // Check if last frame is reached
      CheckForLastFrame();

      // Refresh controls
      PostMessage(WM_UPDATE_CONTROLS, 0, 0);
   }

   CDialog::OnTimer(nIDEvent);
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnUpdateControls
// Purpose   : Update the control values.
// Parameters: None
//==============================================================================
LRESULT CGigEMetaDataDemoDlg::OnUpdateControls(WPARAM, LPARAM)
{
   if (m_Buffers[0])
   {
      // Update edit controls
      m_ActiveBuffer = m_Buffers[0]->GetIndex() + 1;
      m_BufferCount = m_Buffers[0]->GetCount();
      m_Slider = m_Buffers[0]->GetIndex();

      BOOL bNoGrab = !m_bRecordOn && !m_bPlayOn;
      if (bNoGrab)
         m_LiveFrameRate = _T("N/A");

      m_BufferFrameRate = m_Buffers[0]->GetFrameRate();

      m_Buffers[0]->GetDeviceTimeStamp(&m_TimestampBuffer);
      if (m_Buffers[0]->GetIndex())
      {
         UINT64 timestampBufferPrevious = 0;
         m_Buffers[0]->GetDeviceTimeStamp(m_Buffers[0]->GetIndex() - 1, &timestampBufferPrevious);
         m_TimestampBufferDelta = m_TimestampBuffer - timestampBufferPrevious;
      }
      else
         m_TimestampBufferDelta = 0;

      UpdateData(FALSE);
   }
   return 0;
}

//*****************************************************************************************
//
//             Record Control
//
//*****************************************************************************************

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedRecord
// Purpose   : Start recording frames.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedRecord(void)
{
    int masterServer= NULL;
    
    //Reset camera timestamps
    BOOL bIsAvailable = FALSE;
    for (int i = 0; i < serverCount; i++)
            m_AcqDevice[i]->SetFeatureValue("timestampControlReset", 1);
   
    // Acquire all frames
    char serverName[CORSERVER_MAX_STRLEN];
    CString str;
    for (int deviceIndex = 0; deviceIndex < serverCount; deviceIndex++) {
        SapManager::GetServerName(deviceIndex + 1, serverName, sizeof(serverName)); //Get Server name
        
        // Reset source and destination indices
        m_Xfer[deviceIndex]->Init();

        // Reset the frame rate statistics ahead of each transfer stream
        SapXferFrameRateInfo* pStats = m_Xfer[deviceIndex]->GetFrameRateStatistics();
        pStats->Reset();

        // Make all buffers valid for metadata (may be set as invalid in transfer callback)
        int bufCount = m_Buffers[deviceIndex]->GetCount();
        for (int bufIndex = 0; bufIndex < bufCount; bufIndex++)
            m_BufferIsValid[bufIndex] = TRUE;

        if (CString(serverName).Find(_T("NIR")) != std::string::npos) { //Start the NIR cameras first since the need to be waiting for a trigger input
            if (m_Xfer[deviceIndex]->Snap(m_Buffers[deviceIndex]->GetCount()))
            {
                m_bRecordOn = TRUE;
                UpdateMenu();
            }
        }
        else {
            masterServer = deviceIndex;
        }
    }
    Sleep(500);
    if (m_Xfer[masterServer]->Snap(m_Buffers[masterServer]->GetCount())) //Start the vis camera last - so it will start triggering the active NIR cameras
    {
        m_bRecordOn = TRUE;
        UpdateMenu();
    }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedPlay
// Purpose   : Play back the frames.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedPlay(void)
{
    m_Buffers[selectedServer-1]->SetIndex(0); // Initialize buffer index

   // Start playback timer
   m_PlayLastClock = clock();
   int frameTime = max(1, (int)(1000.0 / m_Buffers[selectedServer-1]->GetFrameRate()));
   SetTimer(1, frameTime, NULL);

   m_bPlayOn = TRUE;
   UpdateMenu();
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedPause
// Purpose   : Pause the recording or playing of frames.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedPause(void)
{
    for (int i = 0; i < serverCount; i++) {
        if (!m_bPauseOn)
        {
            // Check if recording or playing
            if (m_bRecordOn)
            {
                KillTimer(1);
                // Stop current acquisition
                if (!m_Xfer[i]->Freeze())
                    return;

                if (CAbortDlg(this, m_Xfer[i]).DoModal() != IDOK)
                    m_Xfer[i]->Abort();
            }
            else if (m_bPlayOn)
                KillTimer(1); // Stop playback timer
        }
        else
        {
            // Check if recording or playing
            if (m_bRecordOn)
            {
                int frameTime = max(1, (int)(1000.0 / m_Buffers[i]->GetFrameRate()));
                SetTimer(1, frameTime, NULL);
                // Acquire remaining frames
                if (!m_Xfer[i]->Snap(m_Buffers[i]->GetCount() - m_Buffers[i]->GetIndex() - 1))
                    return;
            }
            else if (m_bPlayOn)
            {
                // Restart playback timer
                int frameTime = (int)(1000.0 / m_Buffers[i]->GetFrameRate());
                SetTimer(1, frameTime, NULL);
            }
        }

        m_bPauseOn = !m_bPauseOn;
        UpdateMenu();
    }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedStop
// Purpose   : Stop the recording or playing of frames.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedStop(void)
{
   // Check if recording or playing
   if (m_bRecordOn)
   {
      // Stop current acquisition
      if (!m_Xfer[0]->Freeze())
         return;

      if (CAbortDlg(this, m_Xfer[0]).DoModal() != IDOK)
         m_Xfer[0]->Abort();

      m_bRecordOn = FALSE;
   }
   else if (m_bPlayOn)
   {
      // Stop playback timer
      KillTimer(1);
      m_bPlayOn = FALSE;
   }

   m_bPauseOn = FALSE;
   UpdateMenu();
}

//*****************************************************************************************
//
//             General Options
//
//*****************************************************************************************

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedBufferOptions
// Purpose   : Change the number of buffers used.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedBufferOptions(void)
{   
   CBufDlg dlg(this, m_Buffers[0], m_View[0]->GetDisplay());
   for (int i = 0; i < serverCount; i++) {
       if (dlg.DoModal() == IDOK)
       {
           CWaitCursor cur;

           // Destroy objects
           DestroyObjects(i, false);

           // Update buffer object
           SapBuffer buf = *m_Buffers[i];
           CString str;
           str.Format(_T("%d"), dlg.m_Count);
           MessageBox(str);
           *m_Buffers[i] = dlg.GetBuffer();

           // Recreate objects
           if (!CreateObjects(i, false))
           {
               *m_Buffers[i] = buf;
               CreateObjects(i, false);
           }

           // empty list of metadata
           m_listMetadataView.ResetContent();
           m_listMetadataView2.ResetContent();

           m_ImageWnd1.Reset();
           InvalidateRect(NULL);
           UpdateWindow();
           UpdateMenu();
       }
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedLoadAcqConfig
// Purpose   : Select a new configuration file for the acquisition.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedLoadAcqConfig(void)
{
    char serverName[CORSERVER_MAX_STRLEN];
    CString str;

    for (int serverIndex = 0; serverIndex < serverCount; serverIndex++) {
        SapManager::GetServerName(serverIndex + 1, serverName, sizeof(serverName)); //Get Server name
        str.Format(_T("Select config file for camera #%d: %s"), serverIndex, CString(serverName));
        if(debug) MessageBox(str);
        // Set acquisition parameters
        CAcqConfigDlg dlg(this, m_AcqDevice[serverIndex]->GetLocation(), m_AcqDevice[serverIndex]->GetConfigFile(), CAcqConfigDlg::ServerAcqDevice);
        if (dlg.DoModal() == IDOK)
        {
            // Destroy objects
            DestroyObjects(serverIndex);

            // Backup
            SapLocation loc = m_AcqDevice[serverIndex]->GetLocation();
            const char* configFile = m_AcqDevice[serverIndex]->GetConfigFile();

            // Update object
            m_AcqDevice[serverIndex]->SetLocation(dlg.GetLocation());
            m_AcqDevice[serverIndex]->SetConfigFile(dlg.GetConfigFile());
            m_Feature[serverIndex]->SetLocation(dlg.GetLocation());

            // Recreate objects
            if (!CreateObjects(serverIndex))
            {
                m_AcqDevice[serverIndex]->SetLocation(loc);
                m_AcqDevice[serverIndex]->SetConfigFile(configFile);
                m_Feature[serverIndex]->SetLocation(loc);
                CreateObjects(serverIndex);
            }

            ReadCameraTimestamp();

            m_ImageWnd1.Reset();
            InvalidateRect(NULL);
            UpdateWindow();
            UpdateMenu();
        }
    }
}

//*****************************************************************************************
//
//             File Options
//
//*****************************************************************************************

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedFileLoad
// Purpose   : Load a file to the buffers.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedFileLoad(void)
{
   if (m_Buffers[0]->GetFormat() == SapFormatMono16)
   {
      MessageBox(_T("Sequence images in AVI format are sampled at 8-bit pixel depth.\nYou cannot load a sequence in the current configuration."));
      return;
   }

   if (m_Buffers[0]->GetFormat() == SapFormatRGBR888)
   {
      MessageBox(_T("Sequence images acquired in RGBR888 format (red first) were saved as RGB888 (blue first).\nYou cannot load a sequence in the current configuration."));
      return;
   }

   CLoadSaveDlg dlg(this, m_Buffers[0], TRUE, TRUE);
   if (dlg.DoModal() == IDOK)
   {
      InvalidateRect(NULL);
      UpdateWindow();
      UpdateMenu();
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedFileSave
// Purpose   : Save buffers to a file.
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedFileSave(void)
{
    for (int i = 0; i < serverCount; i++) {
        if (m_Buffers[i]->GetFormat() == SapFormatMono16)
            MessageBox(_T("Saving images in AVI format requires downsampling them to 8-bit pixel depth.\nYou will not be able to reload this sequence in this application unless you change the buffer format."));

        if (m_Buffers[i]->GetFormat() == SapFormatRGBR888)
            MessageBox(_T("Saving images in AVI format requires conversion to RGB888 format (blue first).\nYou will not be able to reload this sequence in this application unless you change the buffer format."));

        CLoadSaveDlg dlg(this, m_Buffers[i], FALSE, TRUE);
        dlg.DoModal();
    }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedFileLoadCurrent
// Purpose   :
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedFileLoadCurrent(void)
{
    CLoadSaveDlg dlg(this, m_Buffers[0], TRUE, FALSE);
   if (dlg.DoModal() == IDOK)
   {
      InvalidateRect(NULL);
      UpdateWindow();
      UpdateMenu();
   }
}

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedFileSaveCurrent
// Purpose   :
// Parameters: None
//==============================================================================
void CGigEMetaDataDemoDlg::OnBnClickedFileSaveCurrent(void)
{
   CLoadSaveDlg dlg(this, m_Buffers[0], FALSE, FALSE);
   dlg.DoModal();
}

//==============================================================================

//==============================================================================
// Name      : CGigEMetaDataDemoDlg::OnBnClickedHighFrameRate
// Purpose   :
// Parameters: None
//==============================================================================
#include "HighFrameRateDlg.h"

void CGigEMetaDataDemoDlg::OnBnClickedHighFrameRate()
{
    serverCount = SapManager::GetServerCount(SapManager::ResourceAcqDevice); //Only count cameras
    for (int serverIndex = 0; serverIndex < serverCount; serverIndex++) {
        CHighFrameRateDlg dlg(this, m_nFramesPerCallback, m_Xfer[serverIndex]);

        if (dlg.DoModal() == IDOK)
        {
            CWaitCursor cursor;

            m_nFramesPerCallback = dlg.GetNFramesPerCallback();

            m_Xfer[serverIndex]->Destroy();

            CreateObjects(serverIndex);
        }
    }
}

//==============================================================================

void CGigEMetaDataDemoDlg::ReadCameraTimestamp(void)
{
   BOOL bIsAvailable = FALSE;
   CString strArray[3];
   for (int i = 0; i < serverCount; i++) {
       m_AcqDevice[i]->SetFeatureValue("timestampControlLatch", 1);
       //// Current feature name in SFNC
       //if (m_AcqDevice[i]->IsFeatureAvailable("TimestampLatch", &bIsAvailable) && bIsAvailable)
       //{
       //    m_AcqDevice[i]->SetFeatureValue("TimestampLatch", 1);
       //}
       //// Deprecated in SFNC
       //else if (m_AcqDevice[i]->IsFeatureAvailable("GevTimestampControlLatch", &bIsAvailable) && bIsAvailable)
       //{
       //    m_AcqDevice[i]->SetFeatureValue("GevTimestampControlLatch", 1);
       //}
       //// Specific to Teledyne DALSA
       //else if (m_AcqDevice[i]->IsFeatureAvailable("timestampControlLatch", &bIsAvailable) && bIsAvailable)
       //{
       //    m_AcqDevice[i]->SetFeatureValue("timestampControlLatch", 1);
       //}
   }
   UINT64 timestamp = 0;
   for (int i = 0; i < serverCount; i++) {
       // Current feature name in SFNC
       if (m_AcqDevice[i]->IsFeatureAvailable("TimestampLatchValue", &bIsAvailable) && bIsAvailable)
       {
           m_AcqDevice[i]->GetFeatureValue("TimestampLatchValue", &timestamp);
       }
       // Deprecated in SFNC
       else if (m_AcqDevice[i]->IsFeatureAvailable("GevTimestampValue", &bIsAvailable) && bIsAvailable)
       {
           m_AcqDevice[i]->GetFeatureValue("GevTimestampValue", &timestamp);
       }
       // Specific to Teledyne DALSA
       else if (m_AcqDevice[i]->IsFeatureAvailable("timestampValue", &bIsAvailable) && bIsAvailable)
       {
           m_AcqDevice[i]->GetFeatureValue("timestampValue", &timestamp);
       }
       char strBuf[64];
       _ui64toa_s(timestamp, strBuf, sizeof(strBuf), 10);
       m_TimestampCurrent = CString(strBuf);
       strArray[i] = m_TimestampCurrent;
   }
   m_TimestampCurrent.Format(_T("%s %s %s"), strArray[0], strArray[1], strArray[2]);
}

void CGigEMetaDataDemoDlg::OnBnClickedReadCurrentTimestamp()
{
   ReadCameraTimestamp();

   UpdateData(FALSE);
}

void CGigEMetaDataDemoDlg::OnBnClickedResetTimestamp()
{
   BOOL bIsAvailable = FALSE;
   for (int i = 0; i < serverCount; i++) {
       //// Current feature name in SFNC
       //if (m_AcqDevice[i]->IsFeatureAvailable("TimestampReset", &bIsAvailable) && bIsAvailable)
       //{
       //    m_AcqDevice[i]->SetFeatureValue("TimestampReset", 1);
       //}
       //// Deprecated in SFNC
       //else if (m_AcqDevice[i]->IsFeatureAvailable("GevTimestampControlReset", &bIsAvailable) && bIsAvailable)
       //{
       //    m_AcqDevice[i]->SetFeatureValue("GevTimestampControlReset", 1);
       //}
       //// Specific to Teledyne DALSA
       //else if (m_AcqDevice[i]->IsFeatureAvailable("timestampControlReset", &bIsAvailable) && bIsAvailable)
       //{
           m_AcqDevice[i]->SetFeatureValue("timestampControlReset", 1);
       //}
       //else
       //{
       //    MessageBox(_T("Timestamp reset is not supported for this camera"));
       //    return;
       //}
   }

   OnBnClickedReadCurrentTimestamp();
}


BOOL CGigEMetaDataDemoDlg::PreTranslateMessage(MSG* pMsg)
{
   if (pMsg->message == WM_KEYDOWN)
   {
      if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
      {
         return TRUE;                // Do not process further
      }
   }

   return CDialog::PreTranslateMessage(pMsg);
}

void CGigEMetaDataDemoDlg::StartCounter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
        MessageBox(_T("QueryPerformanceFrequency failed!\n"));

    PCFreq = double(li.QuadPart) / 1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}
double CGigEMetaDataDemoDlg::GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - CounterStart) / PCFreq;
}
