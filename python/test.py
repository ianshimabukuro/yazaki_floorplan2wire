import os
import sys
import ctypes
import win32process
import win32api
import win32con

def list_loaded_modules():
    pid = os.getpid()
    print(f"Inspecting DLLs loaded by current Python process (PID: {pid})...\n")

    process_handle = win32api.OpenProcess(win32con.PROCESS_QUERY_INFORMATION | win32con.PROCESS_VM_READ, False, pid)
    h_modules = win32process.EnumProcessModules(process_handle)

    for h_module in h_modules:
        module_name = win32process.GetModuleFileNameEx(process_handle, h_module)
        if "vtk" in module_name.lower():
            print(module_name)

    win32api.CloseHandle(process_handle)

if __name__ == "__main__":
    import OCP.OCP  # or whatever is causing issues
    list_loaded_modules()


