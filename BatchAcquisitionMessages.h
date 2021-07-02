/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2017-2021 UniZG, Zagreb. All rights reserved.
 * (c) 2017-2021 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionMessages.h
  \brief  Text strings for all messages, menus, and debugging information.

  This file contains strings for all messages that are output to the user.

  \author Tomislav Petkovic
  \author Ivan Vrkic
  \date   2021-06-25
*/


#ifndef __BATCHACQUISITIONMESSAGES_H
#define __BATCHACQUISITIONMESSAGES_H



/****** CONSOLE WINDOW ******/

#pragma region // Menus and messages for main command window

#ifdef __BATCHACQUISITIONMAIN_CPP

static const TCHAR gNameCommandWindow[] =
  L"BatchAcquisition Command Window";

// Normal width of the console window is 80 cols.
// 0        1        2        3        4        5        6        7        8
// 0123456790123456790123456790123456790123456790123456790123456790123456790

static const TCHAR gMsgWelcomeMessage[] =
  L"Welcome to the BatchAcquisition program.\n"
  L"\n"
  L"This is the main user interation and information window.\n"
  L"Additional windows are the DirectX Display Window, the Camera Preview\n"
  L"Window, and VTK Display Window.\n"
  L"\n"
  L"IMPORTANT: All DirectX Display Windows must be manually dragged to\n"
  L"displays associated with the projectors before video mode is selected.\n"
  L"Before starting the acquisition always use command letter F to make\n"
  L"all DirectX Display Windows fullscreen.\n";

static const TCHAR gMsgCameraSDK[] =
  L"Please select camera SDK:\n"
  L" 1) PointGrey FlyCapture 2 (default)\n"
  L" 2) Teledyne Dalsa SaperaLT SDK\n"
  L" 3) Basler Pylon SDK\n"
  L" 4) FLIR Spinnaker SDK\n"
  L" 5) Acquisition from a directory\n";

static const TCHAR gMsgCameraSDKExceptFromFile[] =
  L"Please select camera SDK:\n"
  L" 1) PointGrey FlyCapture 2 (default)\n"
  L" 2) Teledyne Dalsa SaperaLT SDK\n"
  L" 3) Basler Pylon SDK\n"
  L" 4) FLIR Spinnaker SDK\n";

static const TCHAR gMsgCameraSDKUseDefault[] =
  L"Using default camera SDK.\n";

static const TCHAR gMsgCameraSDKRevertToDefault[] =
  L"Reverting to default camera SDK.\n";

static const TCHAR gMsgCameraSDKUseFlyCapture2[] =
  L"Using FlyCapture2 SDK.\n";

static const TCHAR gMsgCameraSDKUnsupportedFlyCapture2[] =
  L"Program compiled without support for FlyCapture2 SDK.\n";

static const TCHAR gMsgCameraSDKUseSaperaLT[] =
  L"Using SaperaLT SDK.\n";

static const TCHAR gMsgCameraSDKUnsupportedSaperaLT[] =
  L"Program compiled without support for Sapera SDK.\n";

static const TCHAR gMsgCameraSDKUsePylon[] =
  L"Using Pylon SDK.\n";

static const TCHAR gMsgCameraSDKUnsupportedPylon[] =
  L"Program compiled without support for Sapera SDK.\n";

static const TCHAR gMsgCameraSDKUseSpinnaker[] =
  L"Using Spinnaker SDK.\n";

static const TCHAR gMsgCameraSDKUnsupportedSpinnaker[] =
  L"Program compiled without support for Spinnaker SDK.\n";

static const TCHAR gMsgCameraSDKUseFromFile[] =
  L"Using dummy from file acquisition.\n";

static const TCHAR gMsgMainMenu[] =
  L"MAIN MENU:\n"
  L" SPC  Start sequential batch acquisition\n"
  L" ENT  Start simultaneous batch acquisition\n"
  L"0) Print active configuration\n"
  L"1) Configure acquisition parameters\n"
  L"2) Change input directory for projector (CTRL for camera)\n"
  //I") Rescan all input directories\n"
  L"3) Input session name (CTRL to select output directory)\n"
  L"4) Change fullscreen resolution and refresh rate\n"
  L"5) Change exposure time multiplier\n"
  L"6) Change delay times and intervals\n"
  L"7) Disable/enable saving of acquired images in PNG format\n"
  L"8) Disable/enable saving of acquired images in RAW format\n"
  //L"V) Disable/enable camera live view\n"
  L"F) Set all projector windows to fullscreen mode\n"
  L"W) Set all projector windows to windowed mode\n"
  L"S) Stop/start continuous acquisition\n"
  L"C) Open camera configuration dialog for active camera\n"
  L"D) Add camera\n"
  L"P) Add projector\n"
  L"X) Remove camera\n"
  L"L) Remove projector\n"
  L"R) Start 3D reconstruction on the last acquired dataset\n"
  L"N) Set acquisition name tag\n"
  L"H/M) Print this menu\n"
  L"Q/ESC) Quit the application\n";

static const TCHAR gMsgBatchConfigurationSubmenu[] =
  L"Change batch acquisition parameters:\n"
  L" 0) Return to main menu (default action)\n"
  L" 1) Toggle blocking acquisition mode; currently %s\n"
  L" 2) Toggle concurrent delay in blocking acquisition mode; currently %s\n"
  L" 3) Toggle fixed SL pattern; currently %s\n"
  L" 4) Set number of images to acquire for a fixed SL pattern; currently %d\n";

static const TCHAR gMsgStringEnabled[] =
  L"enabled";

static const TCHAR gMsgStringDisabled[] =
  L"disabled";

static const TCHAR gMsgBatchConfigurationNoChange[] =
  L"Batch acquisition parameters were not changed. Returning to main menu.\n";

static const TCHAR gMsgBatchConfigurationBlockingModePrint[] =
  L"Blocking acquisition mode is %s.\n";

static const TCHAR gMsgBatchConfigurationBlockingModeEnabled[] =
  L"Enabled blocking acquisition mode.\n";

static const TCHAR gMsgBatchConfigurationBlockingModeDisabled[] =
  L"Disabled blocking acquisition mode.\n"
  L"Non-blocking acquisition mode will be used.\n";

static const TCHAR gWarningBatchFrameDropPossible[] =
  L"[WARNING] Frames may be dropped during batch acquisition!\n";

static const TCHAR gMsgBatchConfigurationConcurrentDelayPrint[] =
  L"Concurrent delay for blocking acquisition mode is %s.\n";

static const TCHAR gMsgBatchConfigurationConcurrentDelayEnabled[] =
  L"Enabled concurrent delay.\n"
  L"Concurrent delay is auto-activated if delay is larger than exposure.\n";

static const TCHAR gMsgBatchConfigurationConcurrentDelayDisabled[] =
  L"Disabled concurrent delay.\n";

static const TCHAR gMsgBatchConfigurationFixedSLPatternPrint[] =
  L"Fixed SL pattern is %s.\n";

static const TCHAR gMsgBatchConfigurationFixedSLPatternEnabled[] =
  L"Fixed SL pattern enabled.\n"
  L"The first image in input directory will be used as a fixed SL pattern.\n";

static const TCHAR gMsgBatchConfigurationFixedSLPatternDisabled[] =
  L"Fixed SL pattern disabled.\n";

static const TCHAR gMsgBatchConfigurationNumAcquirePrint[] =
  L"Set to acquire %d images when fixed SL pattern is enabled.\n";

static const TCHAR gMsgBatchConfigurationNumAcquireQuery[] =
  L"Enter number of images to acquire:\n"
  L">";

static const TCHAR gMsgBatchConfigurationNumAcquireChanged[] =
  L"Number of images to acquire changed from %d to %d.\n";

static const TCHAR gMsgBatchConfigurationNumAcquireNotChanged[] =
  L"Number of images to acquire remains %d.\n";

static const TCHAR gMsgSelectProjectorQuery[] =
  L"Select projector ID (press ENTER for default selection):\n";

static const TCHAR gMsgSelectProjectorItem[] =
  L" %d) projector [PRJ %d]\n";

static const TCHAR gMsgSelectProjectorItemDefault[] =
  L" %d) projector [PRJ %d] (default)\n";

static const TCHAR gMsgSelectProjectorUserChoice[] =
  L"Selected projector [PRJ %d].\n";

static const TCHAR gMsgSelectProjectorDefaultChoice[] =
  L"Using projector [PRJ %d].\n";

static const TCHAR gMsgSelectCameraQuery[] =
  L"Select camera ID (press ENTER for default selection):\n";

static const TCHAR gMsgSelectCameraItem[] =
  L" %d) camera [CAM %d]\n";

static const TCHAR gMsgSelectCameraItemDefault[] =
  L" %d) camera [CAM %d] (default)\n";

static const TCHAR gMsgSelectCameraUserChoice[] =
  L"Selected camera [CAM %d].\n";

static const TCHAR gMsgSelectCameraDefaultChoice[] =
  L"Using camera [CAM %d].\n";

static const TCHAR gMsgSetInputDirectoryForProjector[] =
  L"[PRJ %d] Changed input directory to %s.\n"
  L"[PRJ %d] Found %d images to display.\n";

static const TCHAR gMsgSetInputDirectoryForCamera[] =
  L"[CAM %d] Changed input directory to %s.\n"
  L"[CAM %d] Found %d images to capture.\n";

static const TCHAR gMsgSetInputDirectoryForCameraUnchanged[] =
  L"[CAM %d] Input directory remains %s.\n";

static const TCHAR gMsgSetInputDirectoryForCameraNotFromFile[] =
  L"[CAM %d] Acquisition is not from file.\n";

static const TCHAR gMsgRescanInputDirectoryProjector[] =
  L"[PRJ %d] Found %d images in directory %s.\n";

static const TCHAR gMsgRescanInputDirectoryCamera[] =
  L"[CAM %d] Found %d images in directory %s.\n";

static const TCHAR gMsgOutputDirectoryChanged[] =
  L"Changed output directory to %s.\n";

static const TCHAR gMsgOutputDirectoryPrint[] =
  L"[ENC %d] Output directory is %s.\n";

static const TCHAR gMsgOutputDirectoryInvalid[] =
  L"[ENC %d] Output directory not set.\n";

static const TCHAR gMsgSessionSubdirectoryPrint[] =
  L"[ENC %d] Session name is %s.\n";

static const TCHAR gMsgSessionSubdirectoryInvalid[] =
  L"[ENC %d] Session name not set.\n";

static const TCHAR gMsgAcquisitionTagPrint[] =
  L"[ENC %d] Acquisition tag is %s.\n";

static const TCHAR gMsgAcquisitionTagInvalid[] =
  L"[ENC %d] Acquisition tag is not set.\n";

static const TCHAR gMsgOutputDirectoryNoAttachedCameras[] =
  L"Cannot set output directory as there are no attached cameras!\n";

static const TCHAR gMsgSetSessionSubdirectoryPrintUndefined[] =
  L"Session name is undefined.\n";

static const TCHAR gMsgSetSessionSubdirectoryPrintDefined[] =
  L"Session name set to '%s'.\n";

static const TCHAR gMsgSetSessionSubdirectoryQuery[] =
  L"Enter new session name:\n"
  L">";

static const TCHAR gMsgSetSessionSubdirectoryChanged[] =
  L"Session name changed from '%s' to '%s'.\n";

static const TCHAR gMsgSetSessionSubdirectoryChangedNoDestination[] =
  L"Session name changed from '%s' to undefined.\n";

static const TCHAR gMsgSetSessionSubdirectoryChangedNoSource[] =
  L"Session name changed from undefined to '%s'.\n";

static const TCHAR gMsgSetSessionSubdirectoryUnchangedDefined[] =
  L"Session name remains '%s'.\n";

static const TCHAR gMsgSetSessionSubdirectoryUnchangedUndefined[] =
  L"Session name remains undefined.\n";

static const TCHAR gMsgSetAcquisitionTagPrintUndefined[] =
  L"Acquisition tag is undefined.\n";

static const TCHAR gMsgSetAcquisitionTagPrintDefined[] =
  L"Acquisition tag set to %s.\n";

static const TCHAR gMsgSetAcquisitionTagQuery[] =
  L"Enter new acquisition tag:\n"
  L">";

static const TCHAR gMsgSetAcquisitionTagChanged[] =
  L"Acquisition tag changed from '%s' to '%s'.\n";

static const TCHAR gMsgSetAcquisitionTagChangedNoDestination[] =
  L"Acquisition tag changed from '%s' to undefined.\n";

static const TCHAR gMsgSetAcquisitionTagChangedNoSource[] =
  L"Acquisition tag changed from undefined to '%s'.\n";

static const TCHAR gMsgSetAcquisitionTagUnchangedDefined[] =
  L"Acquisition tag remains '%s'.\n";

static const TCHAR gMsgSetAcquisitionTagUnchangedUndefined[] =
  L"Acquisition tag remains undefined.\n";

static const TCHAR gMsgSystemConfiguration[] =
  L"ACTIVE CONFIGURATION\n";

static const TCHAR gMsgBatchCommandDisabled[] =
  L"Command disabled as batch acquisition is in progress!\n";

static const TCHAR gMsgBatchItemsRemaining[] =
  L"Number of acquired images waiting for processing: %d\n";

static const TCHAR gMsgBatchItemsAllProcessed[] =
  L"All acquired images processed.\n";

static const TCHAR gMsgBatchSequentialNoAttachedCameras[] =
  L"Cannot start sequential acquisition as there are no attached cameras!\n";

static const TCHAR gMsgBatchSequentialBegin[] =
  L"SEQUENTIAL ACQUISITION STARTED!\n";

static const TCHAR gMsgBatchSequentialEnd[] =
  L"SEQUENTIAL ACQUISITION ENDED!\n";

static const TCHAR gMsgBatchSimultaneousNoAttachedCameras[] =
  L"Cannot start simultaneous acquisition as there are no attached cameras!\n";

static const TCHAR gMsgBatchSimultaneousBegin[] =
  L"SIMULTANEOUS ACQUISITION STARTED!\n";

static const TCHAR gMsgBatchSimultaneousEnd[] =
  L"SIMULTANEOUS ACQUISITION ENDED!\n";

static const TCHAR gWarningUnequalNumberOfProjectorImages[] =
  L"[WARNING] Adding black frames to projectors with less than %d frames.\n";

static const TCHAR gMsgBatchOutputDirectory[] =
  L"Output directory is: %s\n";

static const TCHAR gMsgBatchSequentialProjectorBegin[] =
  L"[PRJ %d] Batch acquisition started.\n";

static const TCHAR gMsgBatchSequentialProjectorNumberOfImages[] =
  L"[PRJ %d] Have %d image(s) to project and %d to acquire.\n";

static const TCHAR gMsgBatchSimultaneousProjectorNumberOfImages[] =
  L"[PRJ %d] Have %d image(s) to project and %d to acquire.\n";

static const TCHAR gMsgBatchSequentialProjectorEnd[] =
  L"[PRJ %d] Batch acquisition completed.\n";

static const TCHAR gMsgBatchSequentialProjectorSkip[] =
  L"[PRJ %d] Skipping acquisition due to no attached cameras.\n";

static const TCHAR gMsgBatchUsingConcurrentDelay[] =
  L"[PRJ %d] Concurrent delay enabled.\n";

static const TCHAR gMsgTimingStatisticsRenderTime[] =
  L"[PRJ %d] Render time: min %.2lf ms, avg %.2lf +- %.2lf ms, max %.2lf ms\n";

static const TCHAR gMsgTimingStatisticsPresentTime[] =
  L"[PRJ %d] Present time: min %.2lf ms, avg %.2lf +- %.2lf ms, max %.2lf ms\n";

static const TCHAR gMsgTimingStatisticsVBLANKTime[] =
  L"[PRJ %d] VBLANK waiting time: min %.2lf ms, avg %.2lf +- %.2lf ms, max %.2lf ms \n";

static const TCHAR gMsgTimingStatisticsTotalTimeProjector[] =
  L"[PRJ %d] Total time: %.2lf s\n";

static const TCHAR gMsgTimingStatisticsFPSProjector[] =
  L"[PRJ %d] Projecting speed: %.2lf FPS\n";

static const TCHAR gMsgTimingStatisticsTriggerTime[] =
  L"[CAM %d] Trigger time: min %.2lf ms, avg %.2lf +- %.2lf ms, max %.2lf ms\n";

static const TCHAR gMsgTimingStatisticsAcquisitionTime[] =
  L"[CAM %d] Acquisition time: min %.2lf ms, avg %.2lf +- %.2lf ms, max %.2lf ms\n";

static const TCHAR gMsgTimingStatisticsTotalTimeCamera[] =
  L"[CAM %d] Total time: %.2lf s\n";

static const TCHAR gMsgTimingStatisticsFPSCamera[] =
  L"[CAM %d] Acquisition speed: %.2lf FPS\n";

static const TCHAR gMsgAttachedCamerasListStart[] =
  L"[PRJ %d] Projector controls %d camera(s): ";

static const TCHAR gMsgAttachedCamerasListItemValid[] =
  L"[CAM %d]";

static const TCHAR gMsgAttachedCamerasListItemInvalid[] =
  L"NULL";

static const TCHAR gMsgAttachedCamerasListSeparator[] =
  L", ";

static const TCHAR gMsgAttachedCamerasListEnd[] =
  L"\n";

static const TCHAR gMsgAttachedCamerasListNone[] =
  L"[PRJ %d] No cameras are attached to this projector.\n";

static const TCHAR gMsgAttachedCamerasListCameraUID[] =
  L"        [CAM %d] Camera UID is %s.\n";

static const TCHAR gMsgSubMenuSaveToPNGForAll[] =
  L"Change save to PNG for all projectors:\n";

static const TCHAR gMsgSubMenuSaveToPNGForProjector[] =
  L"Change save to PNG for projector [PRJ %d]:\n";

static const TCHAR gMsgSubMenuSaveToPNGReturnToMainMenu[] =
  L"0) Return to main menu\n";

static const TCHAR gMsgSubMenuSaveToPNGSelectProjector[] =
  L"1) Select projector\n";

static const TCHAR gMsgSubMenuSaveToPNGActivate[] =
  L"7) Activate save to PNG (default choice)\n";

static const TCHAR gMsgSubMenuSaveToPNGDeactivate[] =
  L"7) Deactivate save to PNG (default choice)\n";

static const TCHAR gMsgSubMenuSaveToPNGNoChange[] =
  L"Save to PNG flag was not changed. Returing to main menu.\n";

static const TCHAR gMsgImageSavePNGEnabled[] =
  L"[PRJ %d] Image saving in PNG format enabled.\n";

static const TCHAR gMsgImageSavePNGDisabled[] =
  L"[PRJ %d] Image saving in PNG format disabled.\n";

static const TCHAR gMsgSubMenuSaveToRAWForAll[] =
  L"Change save to RAW for all projectors:\n";

static const TCHAR gMsgSubMenuSaveToRAWForProjector[] =
  L"Change save to RAW for projector [PRJ %d]:\n";

static const TCHAR gMsgSubMenuSaveToRAWReturnToMainMenu[] =
  L"0) Return to main menu\n";

static const TCHAR gMsgSubMenuSaveToRAWSelectProjector[] =
  L"1) Select projector\n";

static const TCHAR gMsgSubMenuSaveToRAWActivate[] =
  L"8) Activate save to RAW (default choice)\n";

static const TCHAR gMsgSubMenuSaveToRAWDeactivate[] =
  L"8) Deactivate save to RAW (default choice)\n";

static const TCHAR gMsgSubMenuSaveToRAWNoChange[] =
  L"Save to RAW flag was not changed. Returing to main menu.\n";

static const TCHAR gMsgImageSaveRAWEnabled[] =
  L"[PRJ %d] Image saving in RAW format enabled.\n";

static const TCHAR gMsgImageSaveRAWDisabled[] =
  L"[PRJ %d] Image saving in RAW format disabled.\n";

static const TCHAR gMsgImageSaveToPNGAndRaw[] =
  L"[PRJ %d] Acquired images are saved as PNG and RAW.\n";

static const TCHAR gMsgImageSaveToPNG[] =
  L"[PRJ %d] Acquired images are saved as PNG.\n";

static const TCHAR gMsgImageSaveToRAW[] =
  L"[PRJ %d] Acquired images are saved as RAW.\n";

static const TCHAR gMsgImageSaveToNone[] =
  L"[PRJ %d] Acquired images are not saved to disk.\n";

static const TCHAR gMsgLiveViewEnabled[] =
  L"[PRJ %d] Camera live preview enabled.\n";

static const TCHAR gMsgLiveViewDisabled[] =
  L"[PRJ %d] Camera live preview disabled.\n";

static const TCHAR gMsgLiveViewInvalid[] =
  L"[PRJ %d] Unexpected error while toggling live preview.\n";

static const TCHAR gMsgCyclingStop[] =
  L"[PRJ %d] Stopping continuous acquisition and flushing output queue...\n";

static const TCHAR gMsgCyclingStopped[] =
  L"[PRJ %d] Continuous acquisition stopped.\n";

static const TCHAR gMsgCyclingStart[] =
  L"[PRJ %d] Starting continuous acquisition and filling input queue...\n";

static const TCHAR gMsgCyclingStarted[] =
  L"[PRJ %d] Continuous acquisition started.\n";

static const TCHAR gMsgInvalidProjector[] =
  L"[PRJ %d] Projector not found!\n";

static const TCHAR gMsgInvalidProjectorWindow[] =
  L"[PRJ %d] DirectX display window not found!\n";

static const TCHAR gMsgInvalidProjectorImageDecoder[] =
  L"[PRJ %d] Image decoder not found!\n";

static const TCHAR gMsgInvalidCamera[] =
  L"[CAM %d] Camera not found!\n";

static const TCHAR gMsgExposureNoAttachedCameras[] =
  L"There are no cameras attached to set exposure. Add camera first!\n";

static const TCHAR gMsgExposureMultiplierPrint[] =
  L"[CAM %d] Exposure multiplier is %.2lf (expected shutter speed %.0lf us).\n";

static const TCHAR gMsgExposureMultiplierQuery[] =
  L"Enter new exposure multiplier for [CAM %d]:\n"
  L">";

static const TCHAR gMsgExposureMultiplierChanged[] =
  L"[CAM %d]: Exposure multiplier changed from %.2lf to %.2lf.\n";

static const TCHAR gMsgCameraConfigurationDialogNoAttachedCameras[] =
  L"There are no cameras to configure. Add camera first!\n";

static const TCHAR gMsgDelayTimesSubmenu[] =
  L"Change delay and present times for [PRJ %d]:\n"
  L" 0) Return to main menu (default action)\n"
  L" 1) Set delay for blocking acquisition; currently %.3lf ms\n"
  L" 2) Set delay for non-blocking acquisition; currently %.3lf ms\n"
  L" 3) Set present time for non-blocking acquisition; currently %ld VBLANKs\n";

static const TCHAR gMsgDelayTimesNoChange[] =
  L"[PRJ %d] Delay and present times were not changed. Returning to main menu.\n";

static const TCHAR gMsgDelayTimeBlockingPrint[] =
  L"[PRJ %d] Blocking delay is %.2lf ms.\n";

static const TCHAR gMsgDelayTimeBlockingQuery[] =
  L"[PRJ %d] Enter blocking delay in ms:\n"
  L">";

static const TCHAR gMsgDelayTimeBlockingChanged[] =
  L"[PRJ %d] Blocking delay changed from %.2lf ms to %.2lf ms.\n";

static const TCHAR gMsgDelayTimeBlockingNotChanged[] =
  L"[PRJ %d] Blocking delay remains %.2lf ms.\n";

static const TCHAR gMsgDelayTimeNonBlockingPrint[] =
  L"[PRJ %d] Non-blocking delay is %.3lf ms = %ld VBLANKs + %.0lf us.\n";

static const TCHAR gMsgDelayTimeNonBlockingQuery[] =
  L"[PRJ %d] Enter non-blocking delay in ms:\n"
  L">";

static const TCHAR gMsgDelayTimeNonBlockingChanged[] =
  L"[PRJ %d] Non-blocking delay changed from %.3lf ms to %.3lf ms = %ld VBLANKs + %.0lf us.\n";

static const TCHAR gMsgDelayTimeNonBlockingNotChanged[] =
  L"[PRJ %d] Non-blocking delay remains %.3lf ms = %ld VBLANKs + %.0lf us.\n";

static const TCHAR gMsgPresentTimeNonBlockingPrint[] =
  L"[PRJ %d] Non-blocking present time is %ld VBLANKs.\n";

static const TCHAR gMsgPresentTimeNonBlockingQuery[] =
  L"[PRJ %d] Enter non-blocking present time in VBLANKs:\n"
  L">";

static const TCHAR gMsgPresentTimeNonBlockingChanged[] =
  L"[PRJ %d] Non-blocking present time changed from %ld VBLANKs to %ld VBLANKs.\n";

static const TCHAR gMsgPresentTimeNonBlockingNotChanged[] =
  L"[PRJ %d] Non-blocking present time remains %ld VBLANKs.\n";

static const TCHAR gMsgProjectorFullscreenMode[] =
  L"[PRJ %d] Fullscreen mode set to %u x %u @ %.2lf.Hz.\n";

static const TCHAR gMsgProjectorMonitorName[] =
  L"[PRJ %d] Output window is on %s.\n";

static const TCHAR gMsgProjectorMonitorUnknown[] =
  L"[PRJ %d] Output window is not visible on any monitor!\n";

static const TCHAR gMsgProjectorInputDirectory[] =
  L"[PRJ %d] Set to project %d frames from %s.\n";

static const TCHAR gMsgProjectorInputDirectoryInvalid[] =
  L"[PRJ %d] Input directory not set!";

static const TCHAR gMsgProjectorInputDirectoryEmpty[] =
  L"[PRJ %d] Input directory %s is empty.";

static const TCHAR gNameWindowPreview[] =
  L"Camera Preview Window";

static const TCHAR gMsgDeleteCameraNoAttachedCameras[] =
  L"There are no cameras to delete. Add camera first!\n";

static const TCHAR gMsgDeleteCameraError[] =
  L"Error deleting [CAM %d].\n";

static const TCHAR gMsgDeleteCameraSucceeded[] =
  L"Deleted camera [CAM %d].\n";

static const TCHAR gMsgDeleteProjectorOneProjectorRemaining[] =
  L"The last projector cannot be deleted.\n";

static const TCHAR gMsgDeleteProjectorErrorHasCamerasAttached[] =
  L"Cannot delete [PRJ %d] due to attached cameras. Delete cameras first!\n";

static const TCHAR gMsgDeleteProjectorError[] =
  L"Error deleting [PRJ %d].\n";

static const TCHAR gMsgDeleteProjectorSucceeded[] =
  L"Deleted projector [PRJ %d].\n";

static const TCHAR gMsgReconstructionNoCamerasAttached[] =
  L"[ERROR] There are no cameras attached to record data. Add camera first!\n";

static const TCHAR gMsgReconstructionNoImagesAcquired[] =
  L"[ERROR] There are no acquired images. Run image acquisition first!\n";

static const TCHAR gMsgReconstructionMenu[] =
  L"3D RECONSTRUCTION:\n"
  L"0) Change reconstruction parameters\n"
  L"1) PS+GC using 8PS+(4+4)GC+B+W column code\n"
  L"2) PS+GC using 8PS+(4+4)GC+B+W row code\n"
  L"3) PS+GC using 8PS+(4+4)GC+B+W+8PS+(4+4)GC column and row code\n"
  L"4) MPS using 8PS(n15)+8PS(n19) column code\n"
  L"5) MPS using 8PS(n15)+8PS(n19) row code\n"
  L"6) MPS using 8PS(n15)+8PS(n19) column and row code\n"
  L"7) MPS using 3PS(n20)+3PS(n21)+3PS(n25) column code\n"
  L"8) MPS using 3PS(n20)+3PS(n21)+3PS(n25) row code\n"
  L"9) MPS using 3PS(n20)+3PS(n21)+3PS(n25) column and row code (default)\n";

static const TCHAR gMsgReconstructionMenuConfigurationParameters[] =
  L"SET 3D RECONSTRUCTION PARAMETERS:\n"
  L"0) Return to 3D reconstruction menu (default)\n"
  L"1) Set relative dynamic range threshold (rel_thr = %.2lf)\n"
  L"2) Set distance threshold in mm (dst_thr = %.2lf)\n";

static const TCHAR gMsgReconstructionConfigurationRelativeThresholdPrint[] =
  L"Relative dynamic range threshold set to %lf.\n";

static const TCHAR gMsgReconstructionConfigurationRelativeThresholdQuery[] =
  L"Enter new dynamic range threshold:\n"
  L">";

static const TCHAR gMsgReconstructionConfigurationRelativeThresholdChanged[] =
  L"Relative dynamic range threshold changed from %lf to %lf.\n";

static const TCHAR gMsgReconstructionConfigurationRelativeThresholdNotChanged[] =
  L"Relative dynamic range threshold remains %lf.\n";

static const TCHAR gMsgReconstructionConfigurationDistanceThresholdPrint[] =
  L"Distance threshold set to %lf mm.\n";

static const TCHAR gMsgReconstructionConfigurationDistanceThresholdQuery[] =
  L"Enter new distance threshold in mm:\n"
  L">";

static const TCHAR gMsgReconstructionConfigurationDistanceThresholdChanged[] =
  L"Distance threshold changed from %lf to %lf mm.\n";

static const TCHAR gMsgReconstructionConfigurationDistanceThresholdNotChanged[] =
  L"Distance threshold remains %lf mm.\n";

static const TCHAR gMsgReconstructionConfigurationNoChange[] =
  L"Reconstruction parameters were not changed. Returing to 3D reconstruction menu.\n";

static const TCHAR gMsgReconstructionForCameraStart[] =
  L"[CAM %d]+[PRJ %d] Starting 3D reconstruction.\n";

static const TCHAR gMsgReconstructionForCameraMissingImages[] =
  L"[ERROR] All images were not acquired for [CAM %d]. Aborting reconstruction!\n";

static const TCHAR gMsgReconstructionForCameraNotFullscreen[] =
  L"[WARNING] Projection for [PRJ %d] was not fullscreen.\n";

static const TCHAR gMsgReconstructionForCameraCompleted[] =
  L"[CAM %d]+[PRJ %d] 3D reconstruction completed.\n";

static const TCHAR gMsgReconstructionForCameraFailed[] =
  L"[CAM %d]+[PRJ %d] 3D reconstruction FAILED!\n";

static const TCHAR gMsgReconstructionReturnToMainMenu[] =
  L"No more reconstructions to perform. Returning to main menu.\n";

#endif /* __BATCHACQUISITIONMAIN_CPP */

#if defined(__BATCHACQUISITIONMAIN_CPP) || defined(__BATCHACQUISITIONRENDERING_CPP)

static const TCHAR gMsgQueryInputDirectoryForProjector[] =
  L"[PRJ %d] Select folder with input images:";

#endif

#pragma endregion // Menus and messages for main command window



/****** RENDERING THREAD ******/

#pragma region // Rendering thread

#ifdef __BATCHACQUISITIONRENDERING_CPP

static const char gDbgTriggerDelayKnownMetadata[] =
  "[PRJ %d] Trigger(s) for frame %ld delayed %.3lf ms at VBLANK %ld in %s:%d.\n";

static const char gDbgTriggerDrop[] =
  "[PRJ %d] Dropped trigger(s) in %s:%d.\n";

static const char gDbgTriggerDropForMetadata[] =
  "[PRJ %d] Dropping trigger(s) for frame %ld at VBLANK %ld in %s:%d.\n";

static const char gDbgTriggerDropForCamera[] =
  "[PRJ %d]->[CAM %d] Dropped trigger in %s:%d\n";

static const char gDbgTriggerDropDueToMainPrepareDraw[] =
  "[PRJ %d] Dropped trigger(s) due to MAIN_PREPARE_DRAW in %s:%d.\n";

static const char gDbgTriggerDropDueToDrawTerminate[] =
  "[PRJ %d] Dropped trigger(s) due to DRAW_TERMINATE in %s:%d.\n";

static const char gDbgDidNotReceiveMainEndCamera[] =
  "[PRJ %d] Did not receive all MAIN_END_CAMERA events in %s:%d.\n";

static const char gDbgAbortPreview[] =
  "[PRJ %d] Aborting live preview in %s:%d.\n";

static const char gDbgAbortPreviewDueToDrawTerminate[] =
  "[PRJ %d] Aborting live preview due to DRAW_TERMINATE in %s:%d.\n";

static const char gDbgAbortPreviewDueToMainPrepareDraw[] =
  "[PRJ %d] Aborting live preview due to MAIN_PREPARE_DRAW in %s:%d.\n";

static const char gDbgAbortSynchronizePresent[] =
  "[PRJ %d] Aborting present sync in %s:%d.";

static const char gDbgAbortSynchronizePresentDueToDrawTerminate[] =
  "[PRJ %d] Aborting present sync due to DRAW_TERMINATE in %s:%d.";

static const char gDbgAbortSynchronizePresentDueToMainPrepareDraw[] =
  "[PRJ %d] Aborting present sync due to MAIN_PREPARE_DRAW in %s:%d.";

static const char gDbgAbortSynchronizeVBLANK[] =
  "[PRJ %d] Aborting VBLANK sync in %s:%d.";

static const char gDbgAbortSynchronizeVBLANKDueToDrawTerminate[] =
  "[PRJ %d] Aborting VBLANK sync due to DRAW_TERMINATE in %s:%d.";

static const char gDbgAbortSynchronizeVBLANKDueToMainPrepareDraw[] =
  "[PRJ %d] Aborting VBLANK sync due to MAIN_PREPARE_DRAW in %s:%d.";

static const char gDbgAbortSynchronizeTriggers[] =
  "[PRJ %d] Aborting trigger sync in %s:%d.";

static const char gDbgAbortSynchronizeTriggersDueToDrawTerminate[] =
  "[PRJ %d] Aborting trigger sync due to DRAW_TERMINATE in %s:%d.";

static const char gDbgAbortSynchronizeTriggersDueToMainPrepareDraw[] =
  "[PRJ %d] Aborting trigger sync due to MAIN_PREPARE_DRAW in %s:%d.";

static const char gDbgUnexpectedStallDuringSynchronizePresent[] =
  "[PRJ %d] Unexpected stall for present sync in %s:%d.\n";

static const char gDbgUnexpectedStallDuringSynchronizeVBLANK[] =
  "[PRJ %d] Unexpected stall for VBLANK sync in %s:%d.\n";

static const char gDbgUnexpectedStallDuringSynchronizeTriggers[] =
  "[PRJ %d] Unexpected stall for trigger sync in %s:%d.\n";

static const char gDbgProjectorSynchronizationPresentCounterMismatch[] =
  "[PRJ %d]->[PRJ %d] Read present counter value of %ld instead of expected %ld.\n";

static const char gDbgProjectorSynchronizationVBLANKCounterMismatch[] =
  "[PRJ %d]->[PRJ %d] Read VBLANK counter value of %ld instead of expected %ld.\n";

static const char gDbgFrameRenderFailed[] =
  "[PRJ %d] Render operation for frame %d failed.\n";

static const char gDbgFramePresentFailed[] =
  "[PRJ %d] Present operation for frame %d failed.\n";

static const char gDbgWaitForVBLANKFailed[] =
  "[PRJ %d] Wait for VBLANK for frame %d failed.\n";

static const TCHAR gDbgEventProcessingTooLong[] =
  L"[PRJ %d] Event %s took %.2lf%% of screen refresh interval.\n";

static const TCHAR gDbgProjectorIDChanged[] =
  L"[PRJ %d] Changed projector ID from %d to %d.\n";

static const TCHAR gDbgProjectorIDNotChanged[] =
  L"[PRJ %d] Projector ID remains the same.\n";

#endif /* __BATCHACQUISITIONRENDERING_CPP */

#pragma endregion // Rendering thread


/****** ACQUISITION THREAD AND IMAGE ENCODER ******/

#pragma region // Acquisition thread and image encoder

#ifdef __BATCHACQUISITIONACQUISITION_CPP

static const char gMsgAcquisitionFlyCap2RevertToFromFile[] =
  "Initialization of FlyCapture2 SDK failed. Reverting to acquisition from file(s).";

static const char gMsgAcquisitionSaperaLTRevertToFromFile[] =
  "Initialization of SaperaLT SDK failed. Reverting to acquisition from file(s).";

static const char gMsgAcquisitionPylonRevertToFromFile[] =
  "Initialization of Pylon SDK failed. Reverting to acquisition from file(s).";

static const char gMsgAcquisitionSpinnakerRevertToFromFile[] =
  "Initialization of Spinnaker SDK failed. Reverting to acquisition from file(s).";

static const char dDbgTriggerDropKnownMetadata[] =
  "[CAM %d] Dropping trigger for frame %d in %s:%d.\n";

static const char dDbgTriggerStallKnownMetadata[] =
  "[CAM %d] Trigger for frame %d stalled in %s:%d.\n";

static const char dDbgInvalidTriggerForCamera[] =
  "[CAM %d] Received CAMERA_INVALID_TRIGGER for frame %d.\n";

static const char dDbgTriggerConfirmationTimeoutExpiredForCamera[] =
  "[CAM %d] Trigger confirmation timeout expired for frame %d.\n";

static const char dDbgDropPresentForProjector[] =
  "[CAM %d]->[PRJ %d] Dropped present in %s:%d.\n";

static const char dDbgDropPresentForProjectorDueToCameraTerminate[] =
  "[CAM %d]->[PRJ %d] Dropped present due to CAMERA_TERMINATE in %s:%d.\n";

static const char dDbgDropPresentForProjectorDueToMainPrepareCamera[] =
  "[CAM %d]->[PRJ %d] Dropped present due to MAIN_PREPARE_CAMERA in %s:%d.\n";

static const char dDbgDropRenderForProjector[] =
  "[CAM %d]->[PRJ %d] Dropped render in %s:%d.\n";

static const char dDbgDropRenderForProjectorDueToCameraTerminate[] =
  "[CAM %d]->[PRJ %d] Dropped render due to CAMERA_TERMINATE in %s:%d.\n";

static const char dDbgDropRenderForProjectorDueToMainPrepareCamera[] =
  "[CAM %d]->[PRJ %d] Dropped render due to MAIN_PREPARE_CAMERA in %s:%d.\n";

static const char gDbgTriggerFailedForFrame[] =
  "[CAM %d] Trigger failed for frame %ld.\n";

static const char gDbgRepeatTriggerForFrame[] =
  "[CAM %d] Retriggering the camera for frame %ld.\n";

static const char gDbgRepeatTriggerFailedForFrame[] =
  "[CAM %d] Retriggering failed for frame %ld.\n";

static const char gDbgTriggerTimeoutForFrame[] =
  "[CAM %d] Trigger timeout expired for frame %ld.\n";

static const TCHAR gDbgEventProcessingTooLong[] =
  L"[CAM %d] Event %s took %.2lf%% of camera exposure time.\n";

static const TCHAR gDbgCameraIDChanged[] =
  L"[CAM %d] Changed camera ID from %d to %d.\n";

static const TCHAR gDbgCameraIDNotChanged[] =
  L"[CAM %d] Camera ID remains the same.\n";

#endif /* __BATCHACQUISITIONACQUISITION_CPP */


#ifdef __BATCHACQUISITIONIMAGEENCODER_CPP

static const TCHAR gMsgImageEncoderSetDataDirectory[] =
  L"Select folder for storing acquisitions:";

static const TCHAR gWarningImageEncoderDelayMeasurement[] =
  L" [WARNING] Invalid delay measurement! Change the exposure time.\n";

static const char gDbgImageEncoderCannotCreateDirectory[] =
  "[ENC %d] Cannot create output directory for acquired images!";

static const char gDbgImageEncoderCannotCreateCameraDirectory[] =
  "[ENC %d] Cannot create output subdirectory for [CAM %d].";

static const char gDbgImageEncoderIDChanged[] =
  "[ENC %d] Changed encoder ID from %d to %d.\n";

static const char gDbgImageEncoderIDNotChanged[] =
  "[ENC %d] Image encoder ID remains the same.\n";

#endif /* __BATCHACQUISITIONIMAGEENCODER_CPP */



#ifdef __BATCHACQUISITIONIMAGEDECODER_CPP

static const char gDbgImageDecoderIDChanged[] =
  "[DEC %d] Changed decoder ID from %d to %d.\n";

static const char gDbgImageDecoderIDNotChanged[] =
  "[DEC %d] Image decoder ID remains the same.\n";

#endif /* __BATCHACQUISITIONIMAGEDECODER_CPP */

#pragma endregion // Acquisition thread and image encoder



/****** CAMERA SDK ******/

#pragma region // Camera selection and general SDK messages

#ifdef __BATCHACQUISITIONFLYCAPTURE2_CPP

static const TCHAR gMsgFlyCapture2Version[] =
  L"[FlyCapture2] Version v%u.%u.%u.%u\n";

static const TCHAR gMsgCameraDetectionSucceeded[] =
  L"[FlyCapture2] Detected %u camera(s).\n";

static const TCHAR gMsgCameraDetectionFailed[] =
  L"[FlyCapture2] No cameras detected. Aborting!\n";

static const TCHAR gMsgCameraDetectionNoFreeCameras[] =
  L"[FlyCapture2] All detected cameras are already in use. Aborting!\n";

static const TCHAR gMsgCameraDetectionAvailable[] =
  L"[FlyCapture2] Of %u detected camera(s) %d is/are available.\n";

static const TCHAR gMsgCameraDetectionOneAvailable[] =
  L"[FlyCapture2] Connecting to only available camera.\n";

static const TCHAR gMsgCameraSelectionMenu[] =
  L"Select camera to use:\n";

static const TCHAR gMsgCameraSelectionListDetails[] =
  L" 0) Print detailed information for all found cameras\n";

static const TCHAR gMsgCameraSelectionListNoDetails[] =
  L" 0) Print short information for all found cameras\n";

static const char gMsgCameraSelectionMenuItem[] =
  " %d) %s, S/N %u\n";

static const char gMsgCameraSelectionMenuItemDefault[] =
  " %d) %s, S/N %u (default choice)\n";

static const char gMsgCameraSelectionMenuItemDetails[] =
  " %d) %s, S/N %u\n"
  "     Serial number: %u\n"
  "     Camera model: %s\n"
  "     Camera vendor: %s\n"
  "     Sensor info: %s\n"
  "     Sensor resolution: %s\n"
  "     Driver name: %s\n"
  "     Firmware version: %s\n"
  "     Firmware build time: %s\n";

static const char gMsgCameraSelectionMenuItemDetailsConnectionIEEE1394[] =
  "     Connection type is IEEE1394.\n";

static const char gMsgCameraSelectionMenuItemDetailsConnectionUSB2[] =
  "     Connection type is USB 2.\n";

static const char gMsgCameraSelectionMenuItemDetailsConnectionUSB3[] =
  "     Connection type is USB 3.\n";

static const char gMsgCameraSelectionMenuItemDetailsConnectionETH[] =
  "     Connection type is Ethernet.\n";

static const char gMsgCameraSelectionMenuItemDetailsDefault[] =
  " %d) %s, S/N %u (default choice)\n"
  "     Serial number: %u\n"
  "     Camera model: %s\n"
  "     Camera vendor: %s\n"
  "     Sensor info: %s\n"
  "     Sensor resolution: %s\n"
  "     Driver name: %s\n"
  "     Firmware version: %s\n"
  "     Firmware build time: %s\n";

static const TCHAR gMsgCameraSelectionMenuRevertToDefault[] =
  L"Invalid selection. Reverting to default choice.\n";

static const char gMsgConnectedToCamera[] =
  "[FlyCapture2] Connected to %s, S/N %u.\n";

static const char gMsgCameraDoesNotSupportSoftwareTrigger[] =
  "[FlyCapture2] Camera does not support software trigger. Aborting!\n";

static const TCHAR gDbgCameraControlDialogOpen[] =
  L"[CAM %d] Opening camera control dialog.\n";

static const TCHAR gDbgCameraControlDialogClose[] =
  L"[CAM %d] Closing camera control dialog.\n";

#endif /* __BATCHACQUISITIONFLYCAPTURE2_CPP */


#ifdef __BATCHACQUISITIONSAPERA_CPP

static const TCHAR gMsgSaperaLTLoadDLLFailed[] =
  L"[SaperaLT] Loading of SaperaLT DLLs failed. Aborting!\n";

static const TCHAR gMsgSaperaLTVersion[] =
  L"[SaperaLT] Version v%d.%d.%d.%d\n";

static const TCHAR gMsgServerDetectionSucceeded[] =
  L"[SaperaLT] Detected %d acquisition boards and/or cameras.\n";

static const TCHAR gMsgServerDetectionFailed[] =
  L"[SaperaLT] No acquisition boards or cameras detected. Aborting!\n";

static const TCHAR gMsgServerNoAttachedDevices[] =
  L"[SaperaLT] There are no available cameras to use. Aborting!\n";

static const TCHAR gMsgServerOneAvailable[] =
  L"[SaperaLT] Connecting to only available camera.\n";

static const TCHAR gMsgServerSelectionMenu[] =
  L"Select acquisition board or camera to use:\n";

static const TCHAR gMsgServerSelectionListDetails[] =
  L" 0) Print detailed information for all found devices\n";

static const TCHAR gMsgServerSelectionListNoDetails[] =
  L" 0) Print short information for all found devices\n";

static const char gMsgServerSelectionMenuItem[] =
  " %d) %s\n";

static const char gMsgServerSelectionMenuItemDefault[] =
  " %d) %s(default choice)\n";

static const char gMsgServerSelectionMenuCameraItem[] =
  " %d) %s, S/N %s \n";

static const char gMsgServerSelectionMenuCameraItemDefault[] =
  " %d) %s, S/N %s (default choice)\n";

static const TCHAR gMsgServerSelectionMenuRevertToDefault[] =
  L"Invalid selection. Reverting to default choice.\n";

static const char gMsgServerSelectionMenuCameraSN[] =
  "     Serial number: %s\n";

static const char gMsgServerSelectionMenuCameraModel[] =
  "     Camera model: %s\n";

static const char gMsgServerSelectionMenuCameraVersion[] =
  "     Camera version: %s\n";

static const char gMsgServerSelectionMenuCameraVendor[] =
  "     Camera vendor: %s\n";

static const char gMsgServerSelectionMenuCameraFirmwareVersion[] =
  "     Firmware version: %s\n";

static const char gMsgCameraSetTriggerMode[] =
  "[SaperaLT] Trigger mode set to \"%s\".\n";

static const char gMsgCameraSetTriggerSource[] =
  "[SaperaLT] Trigger source set to \"%s\".\n";

static const char gMsgCameraSetExposureAlignment[] =
  "[SaperaLT] Exposure alignment set to \"%s\".\n";

static const TCHAR gDbgImageTransferFailed[] =
  L"[CAM %d] Image data transfer failed for frame %ld.\n";

static const TCHAR gDbgRequeueSLPattern[] =
  L"[CAM %d] Requeuing pattern %s; retry %d of %d.\n";

#endif /* __BATCHACQUISITIONSAPERA_CPP */


#ifdef __BATCHACQUISITIONPYLONCALLBACKS_CPP

static const TCHAR gDbgEventOnAttach[] =
  L"[Pylon] OnAttach event for device %s.";
static const TCHAR gDbgEventOnAttached[] =
L"[Pylon] OnAttached event for device %s.";
static const TCHAR gDbgEventOnOpen[] =
L"[Pylon] OnOpen event for device% s.";
static const TCHAR gDbgEventOnOpened[] =
L"[Pylon] OnOpened event for device% s.";

static const TCHAR gDbgEventOnGrabStart[] =
L"[Pylon] OnGrabStart event for device %s.";
static const TCHAR gDbgEventOnGrabStarted[] =
L"[Pylon] OnGrabStarted event for device %s.";
static const TCHAR gDbgEventOnGrabStop[] =
L"[Pylon] OnGrabStop event for device %s.";
static const TCHAR gDbgEventOnGrabStopped[] =
L"[Pylon] OnGrabStopped event for device %s.";

static const TCHAR gDbgEventOnClose[] =
L"[Pylon] OnClose event for device %s.";
static const TCHAR gDbgEventOnClosed[] =
L"[Pylon] OnClosed event for device %s.";
static const TCHAR gDbgEventOnDestroy[] =
L"[Pylon] OnDestroy event for device %s.";
static const TCHAR gDbgEventOnDestroyed[] =
L"[Pylon] OnDestroyed event";
static const TCHAR gDbgEventOnDetach[] =
L"[Pylon] OnDetach event for device %s.";
static const TCHAR gDbgEventOnDetached[] =
L"[Pylon] OnDetached event for device %s.";
static const TCHAR gDbgEventOnGrabError[] =
L"[Pylon] OnGrabError event for device %s.";
static const TCHAR gDbgEventOnCameraDeviceRemoved[] =
L"[Pylon] OnCameraDeviceRemoved event for device %s.";
static const TCHAR gDbgEventOnGrabErrorMessage[] =
L"[Pylon] Error Message: %s.";

static const TCHAR gDbgImageTransferFailed[] =
L"[CAM %d] Image data transfer failed for frame %ld.\n";

static const TCHAR gDbgRequeueSLPattern[] =
L"[CAM %d] Requeuing pattern %s; retry %d of %d.\n";
#endif /* __BATCHACQUISITIONPYLONCALLBACKS_CPP */


#if defined(__BATCHACQUISITIONSAPERACALLBACKS_CPP) || defined(__BATCHACQUISITIONPYLONCALLBACKS_CPP)

static const char gDbgCallbackInformation[] =
  "[SaperaLT] Event %s (%d) received %d times at QPC %llu.\n";

static const char gDbgCallbackInformationKnownCamera[] =
  "[CAM %d] Event %s (%d) received %d times at QPC %llu.\n";

static const char gDbgCameraFrameSkipped[] =
  "[CAM %d] Frame skipped in %s:%d.\n";

static const char gDbgCameraIncreaseTimeout[] =
  "[CAM %d] Increasing transfer timeout to %d ms in %s:%d.\n";

#endif /* __BATCHACQUISITIONSAPERACALLBACKS_CPP */


#if defined(__BATCHACQUISITIONFLYCAPTURE2_CPP) || defined(__BATCHACQUISITIONSAPERA_CPP)

static const TCHAR gMsgExposureTimeSet[] =
  L"[CAM %d] Set shutter speed to %.2lf us.\n";

static const TCHAR gMsgExposureTimeSetLargeDifference[] =
  L"[CAM %d] [WARNING] Set shutter time differs from requested more than 0.5%%.\n";

static const TCHAR gMsgExposureTimeReadError[] =
  L"[CAM %d] Cannot read shutter speed from the camera!\n";

#endif


#if defined(__BATCHACQUISITIONFROMFILE_CPP) || defined(__BATCHACQUISITIONACQUISITION_CPP)

static const TCHAR gMsgQueryInputDirectoryForCamera[] =
  L"[CAM %d] Select folder with input images:";

#endif

#pragma endregion // Camera selection and general SDK messages



/****** DIRECTX  ******/

#pragma region // Video mode selection and DirectX messages

#ifdef __BATCHACQUISITIONSWAPCHAIN_CPP

static const TCHAR gMsgQuickResolutionMenu[] =
  L"Select resolution for projector [PRJ %d]\n"
  L" 0) Return to main menu\n"
  L" 1) Set resolution to %u x %u @ %.2lfHz (default choice)\n"
  L" 2) List all supported display modes\n";

static const TCHAR gMsgFullscreenModeMenu[] =
  L"Select fullscreen mode for projector [PRJ %d]:\n";

static const TCHAR gMsgFullscreenModeMenuItem[] =
  L" %3u) %4u x %4u @ %6.2lf Hz [%.0lf/%.0lf]\n";

static const TCHAR gMsgFullscreenModeMenuItemDefault[] =
  L" %3u) %4u x %4u @ %6.2lf Hz [%.0lf/%.0lf] (default choice)\n";

static const TCHAR gMsgFullscreenModeMenuQuery[] =
  L"[PRJ %d] Enter fullscreen mode number:\n"
  L">";

static const TCHAR gMsgFullscreenModeInvalidResponse[] =
  L"[PRJ %d] Invalid mode number. Reverting to default selection.\n";

static const TCHAR gMsgFullscreenModeChanged[] =
  L"[PRJ %d] Changed mode from %u x %u @ %.2lfHz to %u x %u @ %.2lfHz.\n";

static const TCHAR gMsgFullscreenModeNotChanged[] =
  L"[PRJ %d] Fullscreen mode remains %u x %u @ %.2lfHz.\n";

static const TCHAR gMsgCannotGetContainingOutput[] =
  L"[PRJ %d] Cannot get DXGI output which contains DXGI swap chain.\n";

static const TCHAR gMsgCannotGetDisplayModeList[] =
  L"[PRJ %d] Cannot list available display modes.\n";

static const TCHAR gMsgCannotGetMonitorHandle[] =
  L"[PRJ %d] Cannot get monitor handle.\n";

static const TCHAR gMsgCannotGetMonitorData[] =
  L"[PRJ %d] Cannot get monitor information.\n";

static const TCHAR gMsgCycleThroughFullScreenAndWindowedMode[] =
  L"[PRJ %d] Cycle window through full-screen and windowed mode and try again.\n";

static const TCHAR gDbgCannotGetContainingOutput[] =
  L"Cannot get containing output for DXGI swap chain.\n";

static const TCHAR gMsgRestartApplication[] =
  L"\n"
  L"\tPLEASE RESTART THE APPLICATION!\n"
  L"\tUnspecified DXGI error may prevent proper operation.\n"
  L"\n";

#endif /* __BATCHACQUISITIONSWAPCHAIN_CPP */


#ifdef __BATCHACQUISITIONWINDOWDISPLAY_CPP

static const TCHAR gNameWindowDisplay[] =
  L"DirectX Display Window";

static const TCHAR gMsgFullscreenModeRequested[] =
  L"[PRJ %d] Requested fullscreen mode %u x %u @ %.2lf.Hz.\n";

static const TCHAR gMsgFullscreenModeAchieved[] =
  L"[PRJ %d] Achieved fullscreen mode %u x %u @ %.2lf.Hz.\n";

static const TCHAR gMsgWindowedModeAchieved[] =
  L"[PRJ %d] Returned to windowed mode %u x %u @ %.2lf.Hz.\n";

static const TCHAR gMsgPresentSuspended[] =
  L"[PRJ %d] Suspending present. Press F to resume presenting.\n";

static const TCHAR gMsgPresentResumed[] =
  L"[PRJ %d] Resuming present. Press F to stop presenting.\n";

static const TCHAR gDbgSwapChainIsFullscreen[] =
  L"[PRJ %d] Swap chain is already fullscreen.\n";

static const TCHAR gDbgSwapChainIsWindowed[] =
  L"[PRJ %d] Swap chain is already windowed.\n";

static const TCHAR gDbgSwapChainRecreationSkipped[] =
  L"[PRJ %d] Skipping swap chain re-creation.\n";

static const TCHAR gDbgSwapChainRecreated[] =
  L"[PRJ %d] Swap chain re-created.\n";

static const TCHAR gDbgSwapChainResizeDeferred[] =
  L"[PRJ %d] Swap chain resize to %u x %u deferred.\n";

static const TCHAR gDbgSwapChainResized[] =
  L"[PRJ %d] Swap chain resized to %u x %u.\n";

static const TCHAR gDbgRecreatingRenderTarget[] =
  L"[PRJ %d] Recreating render target due to D2DERR_RECREATE_TARGET error.\n";

#endif /* __BATCHACQUISITIONWINDOWDISPLAY_CPP */

#pragma endregion // Video mode selection and DirectX messages



/****** MISCELLANEOUS ******/

#ifdef __BATCHACQUISITIONKEYBOARD_CPP

static const TCHAR gMsgMenuSelectionTimeout[] =
  L"\rTimeout in %d seconds.      ";

static const TCHAR gMsgMenuSelectionTimeoutClear[] =
  L"\r                              \r";

#endif /* __BATCHACQUISITIONKEYBOARD_CPP */


#ifdef __BATCHACQUISITIONWINDOWPREVIEW_CPP

static const TCHAR gNameWindowPreviewNoCamera[] =
  L"Camera Preview Window - Press number key to activate";

static const TCHAR gNameWindowPreviewKnownCameraID[] =
  L"[CAM %d] slaved to [PRJ %d]";

static const TCHAR gNameWindowPreviewKnownCameraIDAndUID[] =
  L"[CAM %d] slaved to [PRJ %d]; UID %s";

static const TCHAR gDbgRecreatingRenderTarget[] =
  L"Recreating render target for camera preview window due to D2DERR_RECREATE_TARGET error.\n";

static const TCHAR gMsgInformationPopUpTitle[] =
  L"Information";

static const TCHAR gMsgCameraControlNotImplementedForSapera[] =
  L"Camera control dialog is not implemented for Sapera SDK!";

#endif /* __BATCHACQUISITIONWINDOWPREVIEW_CPP */


#ifdef __BATCHACQUISITIONFILELIST_CPP

static const TCHAR gMsgFileListSetDirectory[] =
  L"Select folder with input images:";

#endif /* __BATCHACQUISITIONFILELIST_CPP */


#ifdef __BATCHACQUISITIONDEBUG_CPP

static const TCHAR gDbgReceivedWindowMessage[] =
  L"[HWDN %Ix] %s\n";

#endif /* __BATCHACQUISITIONDEBUG_CPP */


#ifdef __BATCHACQUISITIONPROCESSING_CPP

static const TCHAR gMsgProcessingLoadGeometry[] =
  L"[CAM %d]+[PRJ %d] Loading geometric calibration data.\n";

static const TCHAR gMsgProcessingLoadGeometryDuration[] =
  L"[CAM %d]+[PRJ %d] Loading geometric calibration took %.2lf ms.\n";

static const TCHAR gMsgProcessingCannotLoadCameraGeometry[] =
  L"[ERROR] Cannot load geometry information for camera UID %s.\n";

static const TCHAR gMsgProcessingCannotLoadCameraGeometryNoName[] =
  L"[ERROR] Cannot load geometry information for unnamed camera.\n";

static const TCHAR gMsgProcessingCameraResolutionMismatch[] =
  L"[WARNING] Camera resolution mismatch!\n[CAM %d] Camera is %d x %d; XML states %.2lf x %.2lf.\n";

static const TCHAR gMsgProcessingCannotLoadProjectorGeometry[] =
  L"[ERROR] Cannot load geometry information for projector UID %s.\n";

static const TCHAR gMsgProcessingCannotLoadProjectorGeometryNoName[] =
  L"[ERROR] Cannot load geometry information for unnamed projector.\n";

static const TCHAR gMsgProcessingProjectorResolutionMismatch[] =
  L"[WARNING] Projector resolution mismatch.\n[PRJ %d] Projector window is %d x %d; XML states %.2lf x %.2lf.\n";

static const TCHAR gMsgProcessingDecodeSLCode[] =
  L"[CAM %d]+[PRJ %d] Decoding %s code.\n";

static const TCHAR gMsgProcessingDecodeSLCodeDuration[] =
  L"[CAM %d]+[PRJ %d] SL decoding took %.2lf ms.\n";

static const TCHAR gMsgProcessingPSGCPhaseEstimationDuration[] =
  L"[CAM %d]+[PRJ %d] Phase estimation took %.2lf ms.\n";

static const TCHAR gMsgProcessingPSGCDynamicRangeComputationDuration[] =
  L"[CAM %d]+[PRJ %d] Dynamic range computation took %.2lf ms.\n";

static const TCHAR gMsgProcessingPSGCPhaseUnwrappingDuration[] =
  L"[CAM %d]+[PRJ %d] Phase unwrapping took %.2lf ms.\n";

static const TCHAR gMsgProcessingMPSPreparationDuration[] =
  L"[CAM %d]+[PRJ %d] KD tree construction took %.2lf ms.\n";

static const TCHAR gMsgProcessingMPSPhaseEstimationDuration[] =
  L"[CAM %d]+[PRJ %d] Phase estimation took %.2lf ms.\n";

static const TCHAR gMsgProcessingMPSDynamicRangeAndTextureComputationDuration[] =
  L"[CAM %d]+[PRJ %d] Dynamic range and texture computation took %.2lf ms.\n";

static const TCHAR gMsgProcessingMPSPhaseUnwrappingDuration[] =
  L"[CAM %d]+[PRJ %d] Phase unwrapping took %.2lf ms.\n";

static const TCHAR gMsgProcessingPrepareTexture[] =
  L"[CAM %d]+[PRJ %d] Preparing texture.\n";

static const TCHAR gMsgProcessingPrepareTextureDuration[] =
  L"[CAM %d]+[PRJ %d] Texture preparation took %.2lf ms.\n";

static const TCHAR gMsgProcessingPrepareDataForFiltering[] =
  L"[CAM %d]+[PRJ %d] Precomputing data required for point cloud filtering.\n";

static const TCHAR gMsgProcessingPrepareDataForFilteringDuration[] =
  L"[CAM %d]+[PRJ %d] Precomputation took %.2lf ms.\n";

static const TCHAR gMsgProcessing3DReconstruction[] =
  L"[CAM %d]+[PRJ %d] Triangulating two views.\n";

static const TCHAR gMsgProcessing3DReconstructionDuration[] =
  L"[CAM %d]+[PRJ %d] Triangulation took %.2lf ms.\n";

static const TCHAR gMsgProcessingPrepareDataForVTK[] =
  L"[CAM %d]+[PRJ %d] Preparing data for visualization.\n";

static const TCHAR gMsgProcessingPrepareDataForVTKDuration[] =
  L"[CAM %d]+[PRJ %d] Preparation took %.2lf ms.\n";

static const TCHAR gMsgProcessingDone[] =
  L"[CAM %d]+[PRJ %d] Point cloud pushed to VTK visualization window.\n";

#endif /* __BATCHACQUISITIONPROCESSING_CPP */


#ifdef __BATCHACQUISITIONPROCESSINGPHASESHIFT_CPP

static const TCHAR gDbgGCDInputsAreNotWholeNumbers[] =
  L"Inputs to mps_gcd are not whole numbers.\n";

static const TCHAR gDbgGCDInputsHaveOverflow[] =
  L"Inputs to mps_gcd have overflow; least significant binary digits are missing.\n";

static const TCHAR gDbgLCMInputsAreNotWholeNumbers[] =
  L"Inputs to mps_lcm are not whole numbers.\n";

static const TCHAR gDbgLCMInputsHaveOverflow[] =
  L"Inputs to lcm_gcd itself have overflow or their product produces overflow.\n";

static const TCHAR gDbgFringeCountsAreNotWholeNumbers[] =
  L"Fringe counts are not whole numbers.\n";

static const TCHAR gDbgPeriodsAreNotRelativelyPrime[] =
  L"Periods are not relatively prime integers.\n";



#endif /* __BATCHACQUISITIONPROCESSINGPHASESHIFT_CPP */


#ifdef __BATCHACQUISITIONVTK_CPP

static const char gMsgWindowTitleNoData[] =
  "[VTK Visualization] No data available! Run 3D reconstruction.";

static const char gMsgWindowTitleHaveCloud[] =
  "[VTK Visualization] Active cloud ID %d for [CAM %d]+[PRJ %d]";

static const char gMsgWindowTitleNoCloud[] =
  "[VTK Visualization] Active cloud ID %d has no points";

static const char gMsgThresholdDynamicRange[] =
  "Dynamic range threshold [CAM %d]+[PRJ %d]";

static const char gMsgThresholdRayDistance[] =
  "Ray distance threshold [CAM %d]+[PRJ %d]";

static const char gMsgThresholdPhaseDistance[] =
  "Phase distance threshold [CAM %d]+[PRJ %d]";

static const char gMsgThresholdPhaseDeviation[] =
  "Phase standard deviation threshold [CAM %d]+[PRJ %d]";

static const char gMsgThresholdNoData[] =
  "No data to threshold!";

static const char gMsgSurfaceStart[] =
  "[CAM %d]+[PRJ %d] Creating surface from point cloud data.\n";

static const char gMsgSurfaceComplete[] =
  "[CAM %d]+[PRJ %d] Object surface extracted after %.2lf ms.\n";

static const char gMsgOutlineStart[] =
  "[CAM %d]+[PRJ %d] Creating outline from point cloud data.\n";

static const char gMsgOutlineComplete[] =
  "[CAM %d]+[PRJ %d] Surface outline extracted after %.2lf ms.\n";

static const char gMsgClipStatisticsUpdateMessage[] =
  "Press C to update statistics!";

static const char gMsgNoDataAvailable[] =
  "No data available!\nRun 3D reconstruction.";

static const wchar_t gMsgSaveToPLYTitle[] =
  L"Save point cloud";

static const wchar_t gMsgSaveToPLYTitleOneCloud[] =
  L"Save point cloud [CAM %d]+[PRJ %d]";

static const wchar_t gMsgSaveToPLYTitleAllClouds[] =
  L"Save all point clouds to one PLY file";

static const wchar_t gMsgSaveToPLYExtensionDescription[] =
  L"Polygon File Format";

static const wchar_t gMsgSaveToX3DExtensionDescription[] =
  L"X3D";

static const wchar_t gMsgSaveToVRMLExtensionDescription[] =
  L"Virtual Reality Modeling Language";

static const wchar_t gMsgSaveVTKScene[] =
  L"Save VTK Scene";

#endif /* __BATCHACQUISITIONVTK_CPP */



#endif /* __BATCHACQUISITIONMESSAGES_H */
