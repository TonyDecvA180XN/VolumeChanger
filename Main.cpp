#include <stdio.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	//MessageBox(NULL, argv[0], argv[0], MB_ICONINFORMATION);
	if (argc != 1) return -1;

	HRESULT hr;
	GUID guidMyContext;
	hr = CoCreateGuid(&guidMyContext);


	CoInitialize(NULL);
	IMMDeviceEnumerator* deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&deviceEnumerator);

	IMMDeviceCollection* audioDevices;
	hr = deviceEnumerator->EnumAudioEndpoints(EDataFlow::eRender, DEVICE_STATE_ACTIVE, &audioDevices);

	UINT audioDeviceCount;
	hr = audioDevices->GetCount(&audioDeviceCount);
	//printf("Discovered %i devices.\n", audioDeviceCount);

	for (UINT aDeviceIdx = 0; aDeviceIdx < audioDeviceCount; aDeviceIdx++)
	{
		IMMDevice* device;
		hr = audioDevices->Item(aDeviceIdx, &device);

		LPWSTR deviceId;
		hr = device->GetId(&deviceId);

		IPropertyStore* properties;
		hr = device->OpenPropertyStore(STGM_READ, &properties);

		PROPVARIANT devNameContainer;
		PropVariantInit(&devNameContainer);
		hr = properties->GetValue(PKEY_Device_FriendlyName, &devNameContainer);

		IAudioEndpointVolume* volume;
		hr = device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID*)&volume);

		float db, linear;
		volume->GetMasterVolumeLevel(&db);
		volume->GetMasterVolumeLevelScalar(&linear);
		UINT step, max;
		volume->GetVolumeStepInfo(&step, &max);

		//printf("Device %2i. Name: %S.\n", aDeviceIdx + 1, devNameContainer.pwszVal);
		//printf("Volume: %f db or %f%% or %i/%i.\n", db, linear * 100, step, max);

		if (wcscmp(devNameContainer.pwszVal, L"Speakers (Realtek(R) Audio)") == 0)
		{
			float newVolume = float(step) / (max - 1);
			if (wcscmp(argv[0], L"UP") == 0)
			{
				//printf("Raising volume by 2%%.\n");
				newVolume = float(step + 1) / (max - 1);
			}
			if (wcscmp(argv[0], L"DOWN") == 0)
			{
				//printf("Reducing volume by 2%%.\n");
				newVolume = float(step - 1) / (max - 1);
			}
			volume->SetMasterVolumeLevelScalar(newVolume, &guidMyContext);
			printf("Volume: %i%%.\n", int(linear * 100));
		}

		volume->Release();
		PropVariantClear(&devNameContainer);
		properties->Release();
		device->Release();
	}

	deviceEnumerator->Release();
	return 0;
}