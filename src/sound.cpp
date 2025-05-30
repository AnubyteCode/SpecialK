﻿// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/**
 * This file is part of Special K.
 *
 * Special K is free software : you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by The Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * Special K is distributed in the hope that it will be useful,
 *
 * But WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Special K.
 *
 *   If not, see <http://www.gnu.org/licenses/>.
 *
**/

#include <SpecialK/stdafx.h>

#ifdef  __SK_SUBSYSTEM__
#undef  __SK_SUBSYSTEM__
#endif
#define __SK_SUBSYSTEM__ L"  WASAPI  "

const IID IID_IAudioClient3 = __uuidof(IAudioClient3);

SK_IAudioClient3
__stdcall
SK_WASAPI_GetAudioClient (SK_IMMDevice pDevice, bool uncached)
{
  if (config.compatibility.using_wine)
    return nullptr;

  static bool             unsupported   = false;
  static SK_IAudioClient3 pCachedClient = nullptr;
  static DWORD            dwLastUpdate  = 0;

  if (unsupported)
    return nullptr;

  // TODO: Stash this in the session manager SK already has, and keep it
  //         around persistently
  if (SK::ControlPanel::current_time > dwLastUpdate + 2500UL || uncached)
  {
    dwLastUpdate = SK::ControlPanel::current_time;

    HRESULT hr = S_OK; 

    try
    {
      if (pDevice == nullptr)
      {
        SK_IMMDeviceEnumerator pDevEnum = nullptr;

        ThrowIfFailed (
          pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

        if (pDevEnum == nullptr)
          return nullptr;

        ThrowIfFailed (
          pDevEnum->GetDefaultAudioEndpoint (eRender,
                                               eConsole,
                                                 &pDevice));
      }

      if (pDevice == nullptr)
        return nullptr;

      SK_ComPtr <IAudioClient3> pAudioClient;

      ThrowIfFailed (hr = pDevice->Activate (IID_IAudioClient3, CLSCTX_ALL, nullptr, IID_PPV_ARGS_Helper (&pAudioClient.p)));

      pCachedClient =
        pAudioClient;
    }

    catch (const std::exception& e)
    {
      SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
                ),L"  WASAPI  " );

      if (hr == E_NOINTERFACE)
        unsupported = true;

      return nullptr;
    }
  }

  return
    pCachedClient;
}

using CPROPVARIANT                                        = const PROPVARIANT;
using ISpatialAudioClient__ActivateSpatialAudioStream_pfn = HRESULT (STDMETHODCALLTYPE *)
    ( ISpatialAudioClient *This,
        _In_ CPROPVARIANT *activationParams,
        _In_       REFIID  riid,
_COM_Outptr_       void  **stream );

ISpatialAudioClient__\
ActivateSpatialAudioStream_pfn
ActivateSpatialAudioStream_Original = nullptr;

bool             ProcessUsingSpatialAudio=false;
bool SK_WASAPI_IsProcessUsingSpatialAudio (void)
{ return         ProcessUsingSpatialAudio;     }

HRESULT
STDMETHODCALLTYPE
ISpatialAudioClient__ActivateSpatialAudioStream_Detour (
ISpatialAudioClient *This,
CPROPVARIANT        *activationParams,
REFIID               riid,
void               **stream )
{
  SK_LOG_FIRST_CALL

  HRESULT hr =
    ActivateSpatialAudioStream_Original (
      This, activationParams,
        riid, stream
  );

  if (SUCCEEDED (hr))
  {
    ProcessUsingSpatialAudio = true;
  }

  return hr;
}

SK_ISpatialAudioClient
__stdcall
SK_WASAPI_GetSpatialAudioClient (SK_IMMDevice pDevice, bool uncached)
{
  if (config.compatibility.using_wine)
    return nullptr;

  static bool                   unsupported   = false;
  static SK_ISpatialAudioClient pCachedClient = nullptr;
  static DWORD                  dwLastUpdate  = 0;

  if (unsupported)
    return nullptr;

  // TODO: Stash this in the session manager SK already has, and keep it
  //         around persistently
  if (SK::ControlPanel::current_time > dwLastUpdate + 2500UL || uncached)
  {
    dwLastUpdate = SK::ControlPanel::current_time;

    HRESULT hr = S_OK; 

    try
    {
      if (pDevice == nullptr)
      {
        SK_IMMDeviceEnumerator pDevEnum = nullptr;

        ThrowIfFailed (
          pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

        if (pDevEnum == nullptr)
          return nullptr;

        ThrowIfFailed (
          pDevEnum->GetDefaultAudioEndpoint (eRender,
                                               eConsole,
                                                 &pDevice));
      }

      if (pDevice == nullptr)
        return nullptr;

      SK_ComPtr                     <ISpatialAudioClient>                        pSpatialAudioClient; ThrowIfFailed ( hr =
        pDevice->Activate (__uuidof (ISpatialAudioClient), CLSCTX_INPROC_SERVER,
                                                  nullptr, IID_PPV_ARGS_Helper (&pSpatialAudioClient.p)) );

      pCachedClient =
        pSpatialAudioClient;
    }

    catch (const std::exception& e)
    {
      SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
                ),L"  WASAPI  " );

      if (hr == E_NOINTERFACE)
        unsupported = true;

      return nullptr;
    }
  }

  AudioObjectType                                obj_type_mask = AudioObjectType_None;
  pCachedClient->GetNativeStaticObjectTypeMask (&obj_type_mask);

  if (obj_type_mask != AudioObjectType_None)
  {
    if ((obj_type_mask & AudioObjectType_Dynamic)          != 0) SK_LOGi1 (L"Spatial Channel: Dynamic");
    if ((obj_type_mask & AudioObjectType_FrontLeft)        != 0) SK_LOGi1 (L"Spatial Channel: Front Left");
    if ((obj_type_mask & AudioObjectType_FrontRight)       != 0) SK_LOGi1 (L"Spatial Channel: Front Right");
    if ((obj_type_mask & AudioObjectType_FrontCenter)      != 0) SK_LOGi1 (L"Spatial Channel: Front Center");
    if ((obj_type_mask & AudioObjectType_LowFrequency)     != 0) SK_LOGi1 (L"Spatial Channel: Low Frequency");
    if ((obj_type_mask & AudioObjectType_SideLeft)         != 0) SK_LOGi1 (L"Spatial Channel: Side Left");
    if ((obj_type_mask & AudioObjectType_SideRight)        != 0) SK_LOGi1 (L"Spatial Channel: Side Right");
    if ((obj_type_mask & AudioObjectType_BackLeft)         != 0) SK_LOGi1 (L"Spatial Channel: Back Left");
    if ((obj_type_mask & AudioObjectType_BackRight)        != 0) SK_LOGi1 (L"Spatial Channel: Back Right");
    if ((obj_type_mask & AudioObjectType_TopFrontLeft)     != 0) SK_LOGi1 (L"Spatial Channel: Top Front Left");
    if ((obj_type_mask & AudioObjectType_TopFrontRight)    != 0) SK_LOGi1 (L"Spatial Channel: Top Front Right");
    if ((obj_type_mask & AudioObjectType_TopBackLeft)      != 0) SK_LOGi1 (L"Spatial Channel: Top Back Left");
    if ((obj_type_mask & AudioObjectType_TopBackRight)     != 0) SK_LOGi1 (L"Spatial Channel: Top Back Right");
    if ((obj_type_mask & AudioObjectType_BottomFrontLeft)  != 0) SK_LOGi1 (L"Spatial Channel: Bottom Front Left");
    if ((obj_type_mask & AudioObjectType_BottomFrontRight) != 0) SK_LOGi1 (L"Spatial Channel: Bottom Front Right");
    if ((obj_type_mask & AudioObjectType_BottomBackLeft)   != 0) SK_LOGi1 (L"Spatial Channel: Bottom Back Left");
    if ((obj_type_mask & AudioObjectType_BottomBackRight)  != 0) SK_LOGi1 (L"Spatial Channel: Bottom Back Right");
    if ((obj_type_mask & AudioObjectType_BackCenter)       != 0) SK_LOGi1 (L"Spatial Channel: Back Center");
    if ((obj_type_mask & AudioObjectType_StereoLeft)       != 0) SK_LOGi1 (L"Spatial Channel: Stereo Left");
    if ((obj_type_mask & AudioObjectType_StereoRight)      != 0) SK_LOGi1 (L"Spatial Channel: Stereo Right");

    UINT32                                                   maxDynamicObjCount = 0;
    if (SUCCEEDED (pCachedClient->GetMaxDynamicObjectCount (&maxDynamicObjCount)))
    {
      SK_LOGi1 (L"Spatial Audio Client Supports up to %lu dynamic objects.");

      SK_RunOnce (
        // ISpatialAudioClient
        // -------------------
        //  0  QueryInterface
        //  1  AddRef
        //  2  Release
        //  3  GetStaticObjectPosition
        //  4  GetNativeStaticObjectTypeMask
        //  5  GetMaxDynamicObjectCount
        //  6  GetSupportedAudioObjectFormatEnumerator
        //  7  GetMaxFrameCount
        //  8  IsAudioObjectFormatSupported
        //  9  IsSpatialAudioStreamAvailable
        // 10  ActivateSpatialAudioStream

        void** vftable = *(void***)pCachedClient.p;

        SK_CreateFuncHook ( L"ISpatialAudioClient::ActivateSpatialAudioStream",
                              vftable [10],
                              ISpatialAudioClient__ActivateSpatialAudioStream_Detour,
          static_cast_p2p <void> (                &ActivateSpatialAudioStream_Original) );
        SK_EnableHook (       vftable [10]                                              );
      );
    }
  }

  return
    pCachedClient;
}

SK_IAudioMeterInformation
__stdcall
SK_WASAPI_GetAudioMeterInfo (SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IAudioMeterInformation pMeterInfo = nullptr;

  try
  {
    if (pDevice == nullptr)
    {
      SK_IMMDeviceEnumerator pDevEnum = nullptr;

      ThrowIfFailed (
        pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

      if (pDevEnum == nullptr)
        return nullptr;

      ThrowIfFailed (
        pDevEnum->GetDefaultAudioEndpoint (eRender,
                                             eConsole,
                                               &pDevice));
    }

    if (pDevice == nullptr)
      return nullptr;

    ThrowIfFailed (
      pDevice->Activate ( __uuidof (IAudioMeterInformation),
                                      CLSCTX_ALL,
                                        nullptr,
                                          IID_PPV_ARGS_Helper (&pMeterInfo.p) ));
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );

    pMeterInfo = nullptr;
  }

  return pMeterInfo;
}

// OLD NAME for DLL Export
//   [ Smart Ptr. Not Allowed;  Caller -MUST- Release ]
IAudioMeterInformation*
__stdcall
SK_GetAudioMeterInfo (void)
{
  return
    SK_WASAPI_GetAudioMeterInfo ();
}

SK_IMMDevice          SK_MMDevAPI_DefaultDevice;
SK_IMMDevice          SK_MMDevAPI_VolumeDevice;
SK_ISimpleAudioVolume SK_VolumeControl;

SK_WASAPI_AudioLatency
__stdcall
SK_WASAPI_GetCurrentLatency (SK_IMMDevice pDevice)
{
  auto pAudioClient =
    SK_WASAPI_GetAudioClient (pDevice);

  if (! pAudioClient.p)
    return { 0.0f, 0 };

  try
  {
    WAVEFORMATEX                                      *pFormat;
    UINT32                                                       currentPeriodInFrames;
    ThrowIfFailed (
      pAudioClient->GetCurrentSharedModeEnginePeriod (&pFormat, &currentPeriodInFrames));

    return
    {
      static_cast <float> ((1.0 / static_cast <double> (pFormat->nSamplesPerSec) * currentPeriodInFrames) * 1000.0),
                                                                                   currentPeriodInFrames,
                                                        pFormat->nSamplesPerSec
    };
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return
    { 0.0f, 0 };
}

SK_WASAPI_AudioLatency
__stdcall
SK_WASAPI_GetDefaultLatency (SK_IMMDevice pDevice)
{
  auto pAudioClient =
    SK_WASAPI_GetAudioClient (pDevice);

  if (! pAudioClient.p)
    return { 0.0f, 0 };

  try
  {
    WAVEFORMATEX *pFormat;

    ThrowIfFailed (
      pAudioClient->GetMixFormat (&pFormat));

    UINT32 defaultPeriodInFrames;
    UINT32 fundamentalPeriodInFrames;
    UINT32 minPeriodInFrames;
    UINT32 maxPeriodInFrames;

    ThrowIfFailed (
      pAudioClient->GetSharedModeEnginePeriod ( pFormat,
                                                  &defaultPeriodInFrames,
                                              &fundamentalPeriodInFrames,
                                                      &minPeriodInFrames,
                                                      &maxPeriodInFrames ));

    return
    {
      static_cast <float> ((1.0 / static_cast <double> (pFormat->nSamplesPerSec) * defaultPeriodInFrames) * 1000.0),
                                                                                   defaultPeriodInFrames,
                                                        pFormat->nSamplesPerSec
    };
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return
    { 0.0f, 0 };
}

SK_WASAPI_AudioLatency
__stdcall
SK_WASAPI_GetMinimumLatency (SK_IMMDevice pDevice)
{
  auto pAudioClient =
    SK_WASAPI_GetAudioClient (pDevice);

  if (! pAudioClient.p)
    return { 0.0f, 0 };

  try
  {
    WAVEFORMATEX *pFormat;

    ThrowIfFailed (
      pAudioClient->GetMixFormat (&pFormat));

    UINT32 defaultPeriodInFrames;
    UINT32 fundamentalPeriodInFrames;
    UINT32 minPeriodInFrames;
    UINT32 maxPeriodInFrames;

    ThrowIfFailed (
      pAudioClient->GetSharedModeEnginePeriod ( pFormat,
                                                  &defaultPeriodInFrames,
                                              &fundamentalPeriodInFrames,
                                                      &minPeriodInFrames,
                                                      &maxPeriodInFrames ));

    return
    {
      static_cast <float> ((1.0 / static_cast <double> (pFormat->nSamplesPerSec) * minPeriodInFrames) * 1000.0),
                                                                                   minPeriodInFrames,
                                                        pFormat->nSamplesPerSec
    };
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return
    { 0.0f, 0 };
}

SK_WASAPI_AudioLatency
__stdcall
SK_WASAPI_GetMaximumLatency (void)
{
  auto pAudioClient =
    SK_WASAPI_GetAudioClient ();

  if (! pAudioClient.p)
    return { 0.0f, 0 };

  try
  {
    WAVEFORMATEX *pFormat;

    ThrowIfFailed (
      pAudioClient->GetMixFormat (&pFormat));

    UINT32 defaultPeriodInFrames;
    UINT32 fundamentalPeriodInFrames;
    UINT32 minPeriodInFrames;
    UINT32 maxPeriodInFrames;

    ThrowIfFailed (
      pAudioClient->GetSharedModeEnginePeriod ( pFormat,
                                                  &defaultPeriodInFrames,
                                              &fundamentalPeriodInFrames,
                                                      &minPeriodInFrames,
                                                      &maxPeriodInFrames ));

    return
    {
      static_cast <float> ((1.0 / static_cast <double> (pFormat->nSamplesPerSec) * maxPeriodInFrames) * 1000.0),
                                                                                   maxPeriodInFrames,
                                                        pFormat->nSamplesPerSec
    };
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return
    { 0.0f, 0 };
}

SK_WASAPI_AudioLatency
__stdcall
SK_WASAPI_SetLatency (SK_WASAPI_AudioLatency latency, SK_IMMDevice pDevice)
{
  auto pAudioClient =
    SK_WASAPI_GetAudioClient (pDevice);

  if (! pAudioClient.p)
    return { 0.0f, 0 };

  try
  {
    WAVEFORMATEX *pFormat;

    ThrowIfFailed (
      pAudioClient->GetMixFormat (&pFormat));

    ThrowIfFailed (
      pAudioClient->InitializeSharedAudioStream (0, latency.frames, pFormat, nullptr));

    ThrowIfFailed (
      pAudioClient->Start ());

    // We need to keep this alive after setting it, we can destroy it if changing it.
    static SK_IAudioClient3
        pPersistentClient  = nullptr;
    if (pPersistentClient != nullptr)
        pPersistentClient.Release ();

    pPersistentClient =
      pAudioClient.Detach ();

    return
    {
      static_cast <float> ((1.0 / static_cast <double> (pFormat->nSamplesPerSec) * latency.frames) * 1000.0),
                                                                                   latency.frames,
                                                        pFormat->nSamplesPerSec
    };
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return
    { 0.0f, 0 };
}

void
__stdcall
SK_WASAPI_GetAudioSessionProcs (size_t* count, DWORD* procs)
{
  if (config.compatibility.using_wine)
    return;

  std::set <DWORD> unique_procs;

  size_t max_count = 0;

  if (count != nullptr)
  {
    max_count = *count;
    *count    = 0;
  }

  int                        num_sessions = 0;

  SK_IMMDevice               pDevice;
  SK_IMMDeviceEnumerator     pDevEnum;
  SK_IAudioSessionEnumerator pSessionEnum;
  SK_IAudioSessionManager2   pSessionMgr2;
  SK_IAudioSessionControl    pSessionCtl;
  SK_IAudioSessionControl2   pSessionCtl2;

  try
  {
    ThrowIfFailed (
      pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

    ThrowIfFailed (
      pDevEnum->GetDefaultAudioEndpoint (eRender,
                                           eConsole,
                                             &pDevice));

    ThrowIfFailed (
      pDevice->Activate (
                  __uuidof (IAudioSessionManager2),
                    CLSCTX_ALL,
                      nullptr,
                        reinterpret_cast <void **>(&pSessionMgr2.p)));

    ThrowIfFailed (
      pSessionMgr2->GetSessionEnumerator (&pSessionEnum.p));

    ThrowIfFailed (
      pSessionEnum->GetCount (&num_sessions));
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed "
                L"During Enumerator Setup : %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );

    return;
  }

  for (int pass = 0; pass < 2;            pass++) // First Pass:   Top-level windows
  for (int i    = 0;    i < num_sessions; i++   ) // Second Pass:  Everything else
  {
    try
    {
      ThrowIfFailed (
        pSessionEnum->GetSession (i, &pSessionCtl.p));

      ThrowIfFailed (
        pSessionCtl->QueryInterface (IID_PPV_ARGS (&pSessionCtl2.p)));

      DWORD dwProcess = 0;

      ThrowIfFailed (
        pSessionCtl2->GetProcessId (&dwProcess));

      AudioSessionState state;

      if (SUCCEEDED (pSessionCtl2->GetState (&state)) && state == AudioSessionStateActive)
      {
        if ( unique_procs.count (dwProcess) == 0  && ( max_count == 0 || (count != nullptr && *count < max_count ) ) )
        {
          if ((pass == 1 || SK_FindRootWindow (dwProcess).root != nullptr) || dwProcess == 0)
          {
            if (procs != nullptr)
              procs [unique_procs.size ()] = dwProcess;

            unique_procs.insert (dwProcess);

            if (count != nullptr)
              (*count)++;

            wchar_t* wszDisplayName = nullptr;
            if (SUCCEEDED (pSessionCtl2->GetSessionIdentifier (&wszDisplayName)))
            {
              dll_log->Log  (L"Name: %ws", wszDisplayName);
              CoTaskMemFree (wszDisplayName);
            }

            SK_LOG4 ( ( L" Audio Session (pid=%lu)", dwProcess ),
                        L"  WASAPI  " );
          }
        }
      }
    }

    catch (const std::exception& e)
    {
      SK_LOG0 ( ( L"%ws (...) Failed "
                  L"During Process Enumeration : %hs", __FUNCTIONW__, e.what ()
                ),L"  WASAPI  " );

      continue;
    }
  }
}

SK_IAudioSessionControl
__stdcall
SK_WASAPI_GetAudioSessionControl ( EDataFlow     data_flow     = eRender,
                                   ERole         endpoint_role = eConsole,
                                   SK_IMMDevice  pDevice       = nullptr,
                                   DWORD         proc_id       = GetCurrentProcessId (),
                                   IMMDevice**   ppDevice      = nullptr )
{
  if (config.compatibility.using_wine)
    return nullptr;

  static BOOL bCanWineCountCorrectly = SK_NoPreference;

  // Prevent incorrectly implemented Windows Audio APIs from leaking memory.
  if (! bCanWineCountCorrectly)
    return nullptr;

  SK_IMMDeviceEnumerator     pDevEnum;
  SK_IAudioSessionEnumerator pSessionEnum;
  SK_IAudioSessionManager2   pSessionMgr2;
  SK_IAudioSessionControl2   pSessionCtl2;

  int num_sessions = 0;

  try {
    if (pDevice == nullptr)
    {
      ThrowIfFailed (
        pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

      ThrowIfFailed (
        pDevEnum->GetDefaultAudioEndpoint ( data_flow,
                                              endpoint_role,
                                                &pDevice.p ));

      if (ppDevice != nullptr) {
         *ppDevice  = pDevice;
                      pDevice.p->AddRef ();
      }
    }

    if (! pDevice)
      return nullptr;

    ThrowIfFailed (
      pDevice->Activate (
        __uuidof (IAudioSessionManager2),
          CLSCTX_ALL,
            nullptr,
              reinterpret_cast <void **>(&pSessionMgr2.p)));

    ThrowIfFailed (
      pSessionMgr2->GetSessionEnumerator (&pSessionEnum.p));

    ThrowIfFailed (
      pSessionEnum->GetCount (&num_sessions));
  }

  catch (const std::exception& e)
  {
    auto _LogException = [&](void)
    {
      SK_LOG0 ( ( L"%ws (...) Failed "
                  L"During Enumerator Setup : %hs", __FUNCTIONW__, e.what ()
                ),L"  WASAPI  " );
    };

    if (config.system.log_level > 0)
    {
      _LogException ();
    }

    else
    {
      // This will happen a lot, just log it once...
      SK_RunOnce (_LogException ());
    }

    return nullptr;
  }

  //
  // Sanity check for Wine -- if they ever correctly implement
  //   the Windows Core Audio APIs, audio functions in Special K
  //     will automatically become available.
  //
  if (config.compatibility.using_wine)
  {
    // It's highly unlikely that any Linux system will have
    //   128 audio sessions... if they do, that's too many to
    //     sensibly manage with SK's mixing UI anyway.
    if (num_sessions > 128)
    {
      // Wine indicates success for pSessionEnum->GetCount (...),
      //   but the returned count is uninitialized garbage.
      bCanWineCountCorrectly = FALSE;
      return nullptr;
    }
  }

  for (int i = 0; i < num_sessions; i++)
  {
    try
    {
      SK_IAudioSessionControl pSessionCtl (nullptr);
          AudioSessionState   state (
            AudioSessionStateInactive
          );          DWORD   dwProcess = 0;

      ThrowIfFailed (
        pSessionEnum->GetSession (i, &pSessionCtl.p));

      ThrowIfFailed (
        pSessionCtl->GetState (&state));

      if (AudioSessionStateActive != state)
        continue;

      ThrowIfFailed (
          pSessionCtl->QueryInterface (
            IID_PPV_ARGS (&pSessionCtl2.p)));

      ThrowIfFailed (
        pSessionCtl2->GetProcessId (&dwProcess));

      pSessionCtl2 = nullptr;

      if (dwProcess == proc_id)
        return pSessionCtl;
    }

    catch (const std::exception& e)
    {
      SK_LOG0 ( ( L"%ws (...) Failed "
                  L"During Session Enumeration : %hs", __FUNCTIONW__, e.what ()
                ),L"  WASAPI  " );
    }
  }

  return nullptr;
}

SK_IChannelAudioVolume
__stdcall
SK_WASAPI_GetChannelVolumeControl (DWORD proc_id, SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IAudioSessionControl pSessionCtl =
    SK_WASAPI_GetAudioSessionControl (eRender, eConsole, pDevice, proc_id);

  if (pSessionCtl != nullptr)
  {
    SK_IChannelAudioVolume                                     pChannelAudioVolume;
    if (SUCCEEDED (pSessionCtl->QueryInterface (IID_PPV_ARGS (&pChannelAudioVolume.p))))
      return                                                   pChannelAudioVolume.p;
  }

  return nullptr;
}

SK_ISimpleAudioVolume
__stdcall
SK_WASAPI_GetVolumeControl (DWORD proc_id, SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IMMDevice pVolumeDevice;

  SK_IAudioSessionControl pSessionCtl =
    SK_WASAPI_GetAudioSessionControl (eRender, eConsole, pDevice, proc_id, &pVolumeDevice.p);

  if (pSessionCtl != nullptr)
  {
    //SK_VolumeDevice = pDevice;
    SK_ISimpleAudioVolume                                      pSimpleAudioVolume;
    if (SUCCEEDED (pSessionCtl->QueryInterface (IID_PPV_ARGS (&pSimpleAudioVolume.p))))
    {
      SK_MMDevAPI_DefaultDevice = pVolumeDevice;
      SK_MMDevAPI_VolumeDevice  = pVolumeDevice;
      return                                                   pSimpleAudioVolume;
    }
  }

  return nullptr;
}

SK_IAudioEndpointVolume
__stdcall
SK_MMDev_GetEndpointVolumeControl (SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IAudioEndpointVolume pEndVol = nullptr;

  try {
    if (pDevice == nullptr)
    {
      SK_IMMDeviceEnumerator pDevEnum = nullptr;

      ThrowIfFailed (
        pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

      ThrowIfFailed (
        pDevEnum->GetDefaultAudioEndpoint (eRender,
                                             eConsole,
                                               &pDevice.p));
    }

    if (pDevice == nullptr)
      return nullptr;

    ThrowIfFailed (
      pDevice->Activate (__uuidof (IAudioEndpointVolume),
                           CLSCTX_ALL,
                             nullptr,
                               IID_PPV_ARGS_Helper (&pEndVol.p)));
  }

  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );

    pEndVol = nullptr;
  }

  return pEndVol;
}

SK_IAudioLoudness
__stdcall
SK_MMDev_GetLoudness (SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IAudioLoudness pLoudness = nullptr;

  try
  {
    if (pDevice == nullptr)
    {
      SK_IMMDeviceEnumerator pDevEnum = nullptr;

      ThrowIfFailed (
        pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

      ThrowIfFailed (
        pDevEnum->GetDefaultAudioEndpoint (eRender,
                                             eConsole,
                                               &pDevice));
    }

    if (pDevice == nullptr)
      return nullptr;

    pDevice->Activate (__uuidof (IAudioLoudness),
                         CLSCTX_ALL,
                           nullptr,
                             IID_PPV_ARGS_Helper (&pLoudness.p));
  }

  catch (const std::exception& e)
  {
    // This is optional and almost nothing implements it,
    //   silence this at log level 0!
    SK_LOG1 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );

    pLoudness = nullptr;
  }

  return pLoudness;
}

SK_IAudioAutoGainControl
__stdcall
SK_MMDev_GetAutoGainControl (SK_IMMDevice pDevice)
{
  if (config.compatibility.using_wine)
    return nullptr;

  SK_IAudioAutoGainControl pAutoGain = nullptr;

  try
  {
    if (pDevice == nullptr)
    {
      SK_IMMDeviceEnumerator pDevEnum = nullptr;

      ThrowIfFailed (
        pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));

      ThrowIfFailed (
        pDevEnum->GetDefaultAudioEndpoint (eRender,
                                             eConsole,
                                               &pDevice.p));
    }

    if (pDevice == nullptr)
      return nullptr;

    pDevice->Activate (__uuidof (IAudioAutoGainControl),
                         CLSCTX_ALL,
                           nullptr,
                             IID_PPV_ARGS_Helper (&pAutoGain.p));
  }

  catch (const std::exception& e)
  {
    // This is optional and almost nothing implements it,
    //   silence this at log level 0!
    SK_LOG1 ( ( L"%ws (...) Failed: %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  }

  return pAutoGain;
}

#include <dsound.h>

// Misnomer, since this uses DirectSound instead (much simpler =P)
const char*
__stdcall
SK_WASAPI_GetChannelName (int channel_idx)
{
  if (config.compatibility.using_wine)
    return nullptr;

  static bool init = false;

  static std::unordered_map <int, std::string> channel_names;

  // Delay this because of the Steam Overlay
  if ((! init) && SK_GetFramesDrawn () > 1)
  {
    DWORD dwConfig = 0x00;

    using DirectSoundCreate_pfn = HRESULT (WINAPI *)(
      LPGUID         lpGuid,
      LPDIRECTSOUND* ppDS,
      LPUNKNOWN      pUnkOuter
    );

    HMODULE hModDSound =
      SK_LoadLibraryW (L"dsound.dll");

    if (hModDSound != nullptr)
    {
      auto DirectSoundCreate_Import =
        reinterpret_cast <DirectSoundCreate_pfn> (
       SK_GetProcAddress ( hModDSound,
                             "DirectSoundCreate" )
        );

      if (DirectSoundCreate_Import != nullptr)
      {
        SK_ComPtr <IDirectSound> pDSound = nullptr;

        HRESULT hr;

        if (SUCCEEDED ((hr = DirectSoundCreate_Import (nullptr, &pDSound, nullptr))))
        {
          if (SUCCEEDED ((hr = pDSound->Initialize (nullptr))) || hr == DSERR_ALREADYINITIALIZED)
          {
            if (FAILED ((hr = pDSound->GetSpeakerConfig (&dwConfig))))
            {
              dwConfig = 0x00;
              SK_LOG0 ( ( L" >> IDirectSound::GetSpeakerConfig (...) Failed: hr=%x <<", hr ),
                          L"  DSound  " );
            }
          }

          else
            SK_LOG0 ( ( L" >> IDirectSound::Initialize (...) Failed: hr=%x <<", hr ),
                        L"  DSound  " );
        }

        else
          SK_LOG0 ( ( L" >> DirectSoundCreate (...) Failed: hr=%x <<", hr ),
                      L"  DSound  " );
      }

      SK_FreeLibrary (hModDSound);
    }

    switch (DSSPEAKER_CONFIG (dwConfig))
    {
      case DSSPEAKER_HEADPHONE:
        channel_names.try_emplace (0, "Headphone Left");
        channel_names.try_emplace (1, "Headphone Right");
        break;

      case DSSPEAKER_MONO:
      //case KSAUDIO_SPEAKER_MONO:
        channel_names.try_emplace (0, "Center");
        break;

      case DSSPEAKER_STEREO:
      //case KSAUDIO_SPEAKER_STEREO:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        break;

       case DSSPEAKER_QUAD:
      //case KSAUDIO_SPEAKER_QUAD:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Back Left");
        channel_names.try_emplace (3, "Back Right");
        break;

      case DSSPEAKER_SURROUND:
      //case KSAUDIO_SPEAKER_SURROUND:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Front Center");
        channel_names.try_emplace (3, "Back Center");
        break;

      case DSSPEAKER_5POINT1:
      //case KSAUDIO_SPEAKER_5POINT1:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Center");
        channel_names.try_emplace (3, "Low Frequency Emitter");
        channel_names.try_emplace (4, "Back Left");
        channel_names.try_emplace (5, "Back Right");
        break;

      case DSSPEAKER_5POINT1_SURROUND:
      //case KSAUDIO_SPEAKER_5POINT1_SURROUND:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Center");
        channel_names.try_emplace (3, "Low Frequency Emitter");
        channel_names.try_emplace (4, "Side Left");
        channel_names.try_emplace (5, "Side Right");
        break;

      case DSSPEAKER_7POINT1:
      //case KSAUDIO_SPEAKER_7POINT1:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Center");
        channel_names.try_emplace (3, "Low Frequency Emitter");
        channel_names.try_emplace (4, "Back Left");
        channel_names.try_emplace (5, "Back Right");
        channel_names.try_emplace (6, "Front Left of Center");
        channel_names.try_emplace (7, "Front Right of Center");
        break;

      case DSSPEAKER_7POINT1_SURROUND:
      //case KSAUDIO_SPEAKER_7POINT1_SURROUND:
        channel_names.try_emplace (0, "Front Left");
        channel_names.try_emplace (1, "Front Right");
        channel_names.try_emplace (2, "Center");
        channel_names.try_emplace (3, "Low Frequency Emitter");
        channel_names.try_emplace (4, "Back Left");
        channel_names.try_emplace (5, "Back Right");
        channel_names.try_emplace (6, "Side Left");
        channel_names.try_emplace (7, "Side Right");
        break;

      default:
        SK_LOG0 ( ( L" >> UNKNOWN Speaker Config: %x <<", dwConfig ),
                    L"  WASAPI  " );
        break;
    }

    init = true;
  }

  if (channel_names.count (channel_idx))
     return channel_names [channel_idx].c_str ();

  // We couldn't use the Speaker Config to get a name, so just return
  //   the ordinal instead and add that fallback name to the hashmap
  else
  {
    static char szChannelOrdinal [32] = { };
    snprintf   (szChannelOrdinal, 31, "Unknown Channel (%02i)", channel_idx);

    if (! init)
      return szChannelOrdinal;

           channel_names.try_emplace (channel_idx, szChannelOrdinal);
    return channel_names             [channel_idx].c_str ();
  }
}

// Rate-limit failures
static DWORD dwLastMuteCheck = 0;

void
__stdcall
SK_SetGameMute (bool bMute)
{
  if (config.compatibility.using_wine)
    return;

  if (dwLastMuteCheck < SK::ControlPanel::current_time - 750UL && (SK_VolumeControl == nullptr || SK_MMDevAPI_VolumeDevice != SK_MMDevAPI_DefaultDevice))
  {   dwLastMuteCheck = SK::ControlPanel::current_time;
    SK_VolumeControl = SK_WASAPI_GetVolumeControl (GetCurrentProcessId ());
  }

  SK_ComPtr<ISimpleAudioVolume> pVolume = SK_VolumeControl;

  if (             pVolume != nullptr)
  { if (SUCCEEDED (pVolume->SetMute (bMute, nullptr)))
    {
      SK_ImGui_CreateNotification (
        "Sound.Mute", SK_ImGui_Toast::Info,
          bMute ? "Game has been muted"
                : "Game has been unmuted",
          nullptr, 3333UL, SK_ImGui_Toast::UseDuration |
                           SK_ImGui_Toast::ShowCaption |
                           SK_ImGui_Toast::ShowNewest
      );
    }
  }
}

BOOL
__stdcall
SK_IsGameMuted (void)
{
  if (config.compatibility.using_wine)
    return FALSE;

  if (dwLastMuteCheck < SK::ControlPanel::current_time - 750UL && (SK_VolumeControl == nullptr || SK_MMDevAPI_VolumeDevice != SK_MMDevAPI_DefaultDevice))
  {   dwLastMuteCheck = SK::ControlPanel::current_time;
      SK_VolumeControl = SK_WASAPI_GetVolumeControl (GetCurrentProcessId ());
  }

  BOOL bMuted = FALSE;

  SK_ComPtr<ISimpleAudioVolume> pVolume = SK_VolumeControl;

  if ( pVolume != nullptr && SUCCEEDED (
       pVolume->GetMute ( &bMuted )    )
     ) return              bMuted;

  // Might not be initialized yet, only process this assertion
  //   if pVolume is an actual COM interface pointer.
  if ( pVolume != nullptr )
    SK_ReleaseAssert (! L"pVolume->GetMute (...) Failed");

  return
    bMuted;
}

bool
__stdcall
SK_WASAPI_Init (void)
{
  auto pVolumeCtl =
    SK_WASAPI_GetVolumeControl ();

  if (pVolumeCtl == nullptr)
    return false;

  SK_VolumeControl = pVolumeCtl;

  static float volume = 100.0f;
  static bool  mute   = SK_IsGameMuted ();

  static bool        once = false;
  if (std::exchange (once, true))
    return true;

  pVolumeCtl->GetMasterVolume (&volume);
                                volume *= 100.0f;

  auto cmd =
    SK_GetCommandProcessor ();

  class SoundListener : public SK_IVariableListener
  {
  public:
    virtual bool OnVarChange (SK_IVariable* var, void* val = nullptr)
    {
      if (val != nullptr && var != nullptr )
      {
        if (SK_VolumeControl != nullptr && var->getValuePointer () == &volume)
        {
          volume = *(float *)val;

          volume =
            std::clamp (volume, 0.0f, 100.0f);

          SK_ImGui_CreateNotification (
            "Sound.Volume", SK_ImGui_Toast::Info,
              SK_FormatString ("Game Volume: %1.0f%%", volume).c_str (),
              nullptr, 3333UL, SK_ImGui_Toast::UseDuration |
                               SK_ImGui_Toast::ShowCaption |
                               SK_ImGui_Toast::ShowNewest
          );

          SK_VolumeControl->SetMasterVolume (volume / 100.0f, nullptr);

          if (volume != 0.0f)
          {
            if (SK_IsGameMuted ())
                SK_SetGameMute (false);
          }
        }

        else if (var->getValuePointer () == &mute)
        {
          mute = *(bool *)val;

          SK_SetGameMute (mute);
        }
      }

      return true;
    }
  } static sound_control;

  if (cmd != nullptr)
  {
    cmd->AddVariable ("Sound.Volume", SK_CreateVar (SK_IVariable::Float,   &volume, &sound_control));
    cmd->AddVariable ("Sound.Mute",   SK_CreateVar (SK_IVariable::Boolean, &mute,   &sound_control));
  }

  auto cur_lat = SK_WASAPI_GetCurrentLatency ();
  auto min_lat = SK_WASAPI_GetMinimumLatency ();

  SK_LOGi0 (
    L"Current Audio Mixing Latency: %.1f ms @ %d kHz", cur_lat.milliseconds,
                                                       cur_lat.samples_per_sec / 1000UL
  );
  SK_LOGi0 (
    L"Minimum Audio Mixing Latency: %.1f ms @ %d kHz", min_lat.milliseconds,
                                                       min_lat.samples_per_sec / 1000UL
  );

  if (config.sound.minimize_latency)
  {
    if (cur_lat.frames       != 0 && min_lat.frames != 0 &&
        cur_lat.milliseconds !=      min_lat.milliseconds)
    {
      auto latency =
        SK_WASAPI_SetLatency (min_lat);

      SK_LOG0 ( ( L"Shared Mixing Latency Changed from %.1f ms to %.1f ms",
                    cur_lat.milliseconds, latency.milliseconds
              ),L"  WASAPI  " );
    }
  }

  return true;
}

#include <SpecialK/sound.h>

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_AudioSession::OnDisplayNameChanged (PCWSTR NewDisplayName, LPCGUID EventContext)
{
  UNREFERENCED_PARAMETER (NewDisplayName);
  UNREFERENCED_PARAMETER (EventContext);

  if (! custom_name_)
  {
    app_name_ =
      SK_WideCharToUTF8 (NewDisplayName);
  }

  parent_->signalReset ();

  return S_OK;
};

HRESULT
SK_WASAPI_AudioSession::OnStateChanged (AudioSessionState NewState)
{
  parent_->SetSessionState (this, NewState);
  parent_->signalReset     ();

  return S_OK;
}

HRESULT
SK_WASAPI_AudioSession::OnSessionDisconnected (AudioSessionDisconnectReason DisconnectReason)
{
  UNREFERENCED_PARAMETER (DisconnectReason);

  parent_->RemoveSession (this);
  parent_->signalReset   ();

  return S_OK;
}


SK_IAudioMeterInformation
SK_WASAPI_AudioSession::getMeterInfo (void)
{
  return
    device_->control_.meter;
}

SK_IAudioEndpointVolume
SK_WASAPI_AudioSession::getEndpointVolume (void)
{
  return
    device_->control_.volume;
}

SK_IAudioLoudness
SK_WASAPI_AudioSession::getLoudness (void)
{
  return
    device_->control_.loudness;
}

SK_IAudioAutoGainControl
SK_WASAPI_AudioSession::getAutoGainControl (void)
{
  return
    device_->control_.auto_gain;
}

SK_WASAPI_SessionManager &
SK_WASAPI_GetSessionManager (void);

HRESULT
STDMETHODCALLTYPE
SK_MMDev_Endpoint::OnSessionCreated (IAudioSessionControl *pNewSession)
{
  if (pNewSession)
  {
    pNewSession->AddRef ();

    SK_IAudioSessionControl2                                             pSessionCtl2;
    if (SUCCEEDED (pNewSession->QueryInterface <IAudioSessionControl2> (&pSessionCtl2.p)))
    {
      AudioSessionState        state = AudioSessionStateExpired;
      pSessionCtl2->GetState (&state);

      DWORD dwProcess = 0;
      if (SUCCEEDED (pSessionCtl2->GetProcessId (&dwProcess)) && state != AudioSessionStateExpired)
      { 
        auto* pSession =
          new SK_WASAPI_AudioSession (pSessionCtl2, this, session_manager_);

        session_manager_->AddSession (pSession, state);
      }
    }
  }

  return S_OK;
}

size_t
SK_WASAPI_EndPointManager::getNumRenderEndpoints (DWORD dwState)
{
  if (dwState == DEVICE_STATEMASK_ALL)
    return render_devices_.size ();

  size_t count = 0;

  for ( UINT i = 0 ; i < render_devices_.size () ; ++i )
  {
    render_devices_ [i].device_->GetState (&render_devices_ [i].state_);

    if ((render_devices_ [i].state_ & dwState) != 0x0)
      ++count;
  }

  return count;
}

SK_MMDev_Endpoint&
SK_WASAPI_EndPointManager::getRenderEndpoint (UINT idx)
{
  static SK_MMDev_Endpoint invalid = {}; return idx < render_devices_.size  () ? render_devices_  [idx] : invalid;
}

size_t
SK_WASAPI_EndPointManager::getNumCaptureEndpoints (DWORD dwState)
{
  if (dwState == DEVICE_STATEMASK_ALL)
    return capture_devices_.size ();

  size_t count = 0;

  for ( UINT i = 0 ; i < capture_devices_.size () ; ++i )
  {
    capture_devices_ [i].device_->GetState (&render_devices_ [i].state_);

    if ((capture_devices_ [i].state_ & dwState) != 0x0)
      ++count;
  }

  return count;
}

SK_MMDev_Endpoint&
SK_WASAPI_EndPointManager::getCaptureEndpoint     (UINT idx)
{
  static SK_MMDev_Endpoint invalid = {}; return idx < capture_devices_.size () ? capture_devices_ [idx] : invalid;
}

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_EndPointManager::OnDeviceStateChanged (_In_ LPCWSTR pwstrDeviceId, _In_ DWORD dwNewState)
{
  std::ignore = pwstrDeviceId;
  std::ignore = dwNewState;

  resetSessionManager ();

  const SK_RenderBackend_V2 &rb =
    SK_GetCurrentRenderBackend ();

  if (StrStrW (pwstrDeviceId, rb.displays [rb.active_display].audio.paired_device) && dwNewState == DEVICE_STATE_ACTIVE)
    rb.routeAudioForDisplay (&rb.displays [rb.active_display], true);

  if (dwNewState != DEVICE_STATE_ACTIVE)
  {
    SK_MMDevAPI_DefaultDevice = nullptr;
  }

  return S_OK;
}

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_EndPointManager::OnDeviceAdded (_In_ LPCWSTR pwstrDeviceId)
{
  std::ignore = pwstrDeviceId;

  resetSessionManager ();

  const SK_RenderBackend_V2 &rb =
    SK_GetCurrentRenderBackend ();

  if (StrStrW (pwstrDeviceId, rb.displays [rb.active_display].audio.paired_device))
    rb.routeAudioForDisplay (&rb.displays [rb.active_display], true);

  return S_OK;
}

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_EndPointManager::OnDeviceRemoved (_In_ LPCWSTR pwstrDeviceId)
{
  std::ignore = pwstrDeviceId;

  resetSessionManager ();

  //SK_ImGui_Warning (pwstrDeviceId);

  return S_OK;
}

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_EndPointManager::OnDefaultDeviceChanged (_In_ EDataFlow flow, _In_ ERole role, _In_ LPCWSTR pwstrDefaultDeviceId)
{
  std::ignore = role;
  std::ignore = pwstrDefaultDeviceId;

  if (flow == eAll || flow == eRender)
  {
    //SK_ImGui_Warning (pwstrDefaultDeviceId);
    //resetSessionManager ();

    SK_IMMDevice pVolumeDevice;

    SK_IAudioSessionControl pSessionCtl =
      SK_WASAPI_GetAudioSessionControl (eRender, eConsole, nullptr, GetCurrentProcessId (), &pVolumeDevice.p);

    if (pSessionCtl != nullptr)
    {
      SK_MMDevAPI_DefaultDevice = pVolumeDevice;
    }
  }

  return S_OK;
}

HRESULT
STDMETHODCALLTYPE
SK_WASAPI_EndPointManager::OnPropertyValueChanged (_In_ LPCWSTR pwstrDeviceId, _In_ const PROPERTYKEY key)
{
  std::ignore = pwstrDeviceId;
  std::ignore = key;

  //SK_ImGui_Warning (pwstrDeviceId);
  //resetSessionManager ();

  return S_OK;
}

SK_IAudioMeterInformation
SK_WASAPI_SessionManager::getMeterInfo (void)
{
  static SK_ComPtr <IMMDeviceEnumerator>
      pDevEnum;
  if (pDevEnum == nullptr)
  {
    if (FAILED ((pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)))))
      return nullptr;
  }

  // Most game audio a user will not want to hear while a game is in the
  //   background will pass through eConsole.
  //
  //   eCommunication will be headset stuff and that's something a user is not
  //     going to appreciate having muted :) Consider overloading this function
  //       to allow independent control.
  //
  SK_ComPtr <IMMDevice> pDefaultDevice;
  if ( FAILED (
         pDevEnum->GetDefaultAudioEndpoint ( eRender,
                                               eMultimedia,
                                                 &pDefaultDevice )
              )
     ) return nullptr;

  return
    SK_WASAPI_GetAudioMeterInfo (pDefaultDevice);
}

void
SK_WASAPI_EndPointManager::Activate (void)
{
  std::scoped_lock <SK_Thread_HybridSpinlock> lock0 (activation_lock_);
  
  try
  {
    SK_ComPtr <IMMDeviceEnumerator> pDevEnum;
  
    ThrowIfFailed (
      pDevEnum.CoCreateInstance (__uuidof (MMDeviceEnumerator)));
  
    SK_RunOnce (
      pDevEnum->RegisterEndpointNotificationCallback (this)
    );
  
    SK_ComPtr <IMMDeviceCollection>                                dev_collection;
    ThrowIfFailed (
      pDevEnum->EnumAudioEndpoints (eRender, DEVICE_STATE_ACTIVE, &dev_collection.p));
  
    if (dev_collection.p != nullptr)
    {
      UINT                         uiDevices = 0;
      ThrowIfFailed (
        dev_collection->GetCount (&uiDevices));
  
      for ( UINT i = 0 ; i < uiDevices ; ++i )
      {
        SK_MMDev_Endpoint endpoint;
  
        SK_ComPtr <IMMDevice>       pDevice;
        ThrowIfFailed (
          dev_collection->Item (i, &pDevice.p));
  
        if (pDevice.p != nullptr)
        {
          wchar_t*           wszId = nullptr;
          ThrowIfFailed (
            pDevice->GetId (&wszId));
  
          auto _FindRenderDevice = [&](const wchar_t *device_id) -> SK_MMDev_Endpoint *
          {
            for ( auto& device : render_devices_ )
            {
              if (device.id_._Equal (device_id))
                return &device;
            }
  
            return nullptr;
          };
  
          if (wszId != nullptr && !_FindRenderDevice (wszId))
          {
            SK_ComPtr <IPropertyStore>                props;
            ThrowIfFailed (
              pDevice->OpenPropertyStore (STGM_READ, &props.p));
  
            if (props.p != nullptr)
            {
              PROPVARIANT       propvarName;
              PropVariantInit (&propvarName);
  
              ThrowIfFailed (
                props->GetValue (PKEY_Device_FriendlyName, &propvarName));
  
              endpoint.flow_   = eRender;
              endpoint.id_     = wszId;
              endpoint.name_   = SK_WideCharToUTF8 (propvarName.pwszVal);
              endpoint.device_ = pDevice;
  
              ThrowIfFailed (
                pDevice->GetState (&endpoint.state_));
  
              PropVariantClear (&propvarName);
  
              if (FAILED (pDevice->Activate (
                  __uuidof (IAudioSessionManager2),
                    CLSCTX_ALL,
                      nullptr,
                        reinterpret_cast <void **>(&endpoint.control_.sessions)
                 )       )                  ) return;
  
              endpoint.control_.meter.Attach (
                SK_WASAPI_GetAudioMeterInfo (pDevice).Detach ()
              );
  
              endpoint.control_.volume.Attach         (SK_MMDev_GetEndpointVolumeControl (pDevice).Detach ());
              endpoint.control_.auto_gain.Attach      (SK_MMDev_GetAutoGainControl       (pDevice).Detach ());
              endpoint.control_.loudness.    Attach   (SK_MMDev_GetLoudness              (pDevice).Detach ());
              endpoint.control_.audio_client.Attach   (SK_WASAPI_GetAudioClient          (pDevice).Detach ());
              endpoint.control_.spatial_client.Attach (SK_WASAPI_GetSpatialAudioClient   (pDevice).Detach ());
  
              endpoint.control_.sessions->RegisterSessionNotification (&endpoint);
  
              render_devices_.emplace_back (endpoint);
            }
  
            CoTaskMemFree (wszId);
          }
        }
      }
    }
    //pDevEnum->EnumAudioEndpoints (eCapture, DEVICE_STATEMASK_ALL, &dev_collection.p);
  }
  
  catch (const std::exception& e)
  {
    SK_LOG0 ( ( L"%ws (...) Failed "
                L"During Endpoint Enumeration : %hs", __FUNCTIONW__, e.what ()
              ),L"  WASAPI  " );
  
    return;
  }
}

void
SK_WASAPI_SessionManager::AddSession (SK_WASAPI_AudioSession *pSession, AudioSessionState state)
{
  const bool new_session =
    sessions_.emplace (pSession).second;

  SK_ReleaseAssert (new_session);

  SetSessionState (pSession, state);
}

void
SK_WASAPI_SessionManager::RemoveSession (SK_WASAPI_AudioSession *pSession)
{
  if (! pSession)
    return;

  SetSessionState (pSession, AudioSessionStateExpired);

  if (sessions_.count (pSession))
  {
    sessions_.erase   (pSession);
    pSession->Release ();
  }
}