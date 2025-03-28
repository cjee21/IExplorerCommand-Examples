// dllmain.cpp : Defines the entry point for the DLL application.

// IExplorerCommand File Explorer shell extension for context menu entry

#include "pch.h"

#pragma warning(disable : 4324)

using Microsoft::WRL::ClassicCom;
using Microsoft::WRL::ComPtr;
using Microsoft::WRL::InhibitRoOriginateError;
using Microsoft::WRL::Module;
using Microsoft::WRL::ModuleType;
using Microsoft::WRL::RuntimeClass;
using Microsoft::WRL::RuntimeClassFlags;

constexpr const wchar_t* menu_entry_title = L"MyApplication";
constexpr const wchar_t* exe_filename = L"MyApplication.exe";

BOOL APIENTRY DllMain(_In_ HMODULE hModule, _In_ DWORD  ul_reason_for_call, _In_opt_ LPVOID lpReserved) {
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

namespace {

	// Extracted from
	// https://source.chromium.org/chromium/chromium/src/+/main:base/command_line.cc;l=109-159
	std::wstring QuoteForCommandLineArg(_In_ const std::wstring& arg) {
		// We follow the quoting rules of CommandLineToArgvW.
		// http://msdn.microsoft.com/en-us/library/17w5ykft.aspx
		const std::wstring quotable_chars(L" \\\"");
		if (arg.find_first_of(quotable_chars) == std::wstring::npos) {
			// No quoting necessary.
			return arg;
		}

		std::wstring out;
		out.push_back('"');
		for (size_t i = 0; i < arg.size(); ++i) {
			if (arg[i] == '\\') {
				// Find the extent of this run of backslashes.
				const size_t start = i;
				size_t end = start + 1;
				for (; end < arg.size() && arg[end] == '\\'; ++end) {}
				size_t backslash_count = end - start;

				// Backslashes are escapes only if the run is followed by a double quote.
				// Since we also will end the string with a double quote, we escape for
				// either a double quote or the end of the string.
				if (end == arg.size() || arg[end] == '"') {
					// To quote, we need to output 2x as many backslashes.
					backslash_count *= 2;
				}
				for (size_t j = 0; j < backslash_count; ++j)
					out.push_back('\\');

				// Advance i to one before the end to balance i++ in loop.
				i = end - 1;
			}
			else if (arg[i] == '"') {
				out.push_back('\\');
				out.push_back('"');
			}
			else {
				out.push_back(arg[i]);
			}
		}
		out.push_back('"');

		return out;
	}

}

class __declspec(uuid("20669675-b281-4c4f-94fb-cb6fd3995545")) ExplorerCommandHandler final : public RuntimeClass<RuntimeClassFlags<ClassicCom | InhibitRoOriginateError>, IExplorerCommand> {
public:
	// IExplorerCommand implementation:

	IFACEMETHODIMP GetTitle(_In_opt_ IShellItemArray* items, _Outptr_ PWSTR* name) {
		// Provide name for display in File Explorer context menu entry
		UNREFERENCED_PARAMETER(items);
		return SHStrDupW(menu_entry_title, name);
	}

	IFACEMETHODIMP GetIcon(_In_opt_ IShellItemArray* items, _Outptr_ PWSTR* icon) {
		// Provide icon for display in File Explorer context menu entry
		// Get path to application exe and use it as source for icon
		UNREFERENCED_PARAMETER(items);
		std::filesystem::path module_path{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
		module_path = module_path.remove_filename();
		module_path /= exe_filename;
		return SHStrDupW(module_path.c_str(), icon);
	}

	IFACEMETHODIMP GetToolTip(_In_opt_ IShellItemArray* items, _Outptr_ PWSTR* infoTip) {
		UNREFERENCED_PARAMETER(items);
		*infoTip = nullptr;
		return E_NOTIMPL;
	}

	IFACEMETHODIMP GetCanonicalName(_Out_ GUID* guidCommandName) {
		*guidCommandName = GUID_NULL;
		return S_OK;
	}

	IFACEMETHODIMP GetState(_In_opt_ IShellItemArray* items, _In_ BOOL okToBeSlow, _Out_ EXPCMDSTATE* cmdState) {
		// Provide state of File Explorer context menu entry
		// Set cmdState to ECS_ENABLED to show or to ECS_HIDDEN to hide the context menu entry
		UNREFERENCED_PARAMETER(items);
		UNREFERENCED_PARAMETER(okToBeSlow);
		*cmdState = ECS_ENABLED;
		return S_OK;
	}

	IFACEMETHODIMP GetFlags(_Out_ EXPCMDFLAGS* flags) {
		*flags = ECF_DEFAULT;
		return S_OK;
	}

	IFACEMETHODIMP EnumSubCommands(_Outptr_ IEnumExplorerCommand** enumCommands) {
		*enumCommands = nullptr;
		return E_NOTIMPL;
	}

	IFACEMETHODIMP Invoke(_In_opt_ IShellItemArray* items, _In_opt_ IBindCtx* bindCtx) {
		// Process items passed by File Explorer when context menu entry is invoked
		UNREFERENCED_PARAMETER(bindCtx);
		if (items) {
			// Return if no items
			DWORD count;
			RETURN_IF_FAILED(items->GetCount(&count));

			// Get path to application exe
			std::filesystem::path module_path{ wil::GetModuleFileNameW<std::wstring>(wil::GetModuleInstanceHandle()) };
			module_path = module_path.remove_filename();
			module_path /= exe_filename;

			// Prepare cmd line string to invoke application ("path\to\application.exe" "path\to\firstitem" "path\to\nextitem" ...)
			auto command = wil::str_printf<std::wstring>(LR"-("%s")-", module_path.c_str()); // Path to application.exe
			// Add multiple selected files/folders to cmd line as parameters
			for (DWORD i = 0; i < count; ++i) {
				ComPtr<IShellItem> item;
				auto result = items->GetItemAt(i, &item);
				if (SUCCEEDED(result)) {
					wil::unique_cotaskmem_string path;
					result = item->GetDisplayName(SIGDN_FILESYSPATH, &path);
					if (SUCCEEDED(result)) {
						// Append the item path to the existing command, adding quotes and escapes as needed
						command = wil::str_printf<std::wstring>(LR"-(%s %s)-", command.c_str(), QuoteForCommandLineArg(path.get()).c_str());
					}
				}
			}

			// Invoke application using CreateProcess with the command string prepared above
			wil::unique_process_information process_info;
			STARTUPINFOW startup_info = { sizeof(startup_info) };
			RETURN_IF_WIN32_BOOL_FALSE(CreateProcessW(
				nullptr,
				command.data(),
				nullptr /* lpProcessAttributes */,
				nullptr /* lpThreadAttributes */,
				false /* bInheritHandles */,
				CREATE_NO_WINDOW,
				nullptr,
				nullptr,
				&startup_info,
				&process_info));
		}
		return S_OK;
	}
};

CoCreatableClass(ExplorerCommandHandler)
CoCreatableClassWrlCreatorMapInclude(ExplorerCommandHandler)

_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID* ppv) {
	if (ppv == nullptr)
		return E_POINTER;
	*ppv = nullptr;
	return Module<ModuleType::InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}

__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow(void) {
	return Module<ModuleType::InProc>::GetModule().GetObjectCount() == 0 ? S_OK : S_FALSE;
}