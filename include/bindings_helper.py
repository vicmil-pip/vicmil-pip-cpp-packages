import ctypes
import inspect
import pathlib
import os
from typing import Any

class LoadedLibrary:
    def __init__(self, lib_path: str):
        if os.name == "nt": self.loaded_library = None # : Union[ctypes.WinDLL]
        else: self.loaded_library = None # : Union[ctypes.CDLL]
        
        self._load_cpp_library(lib_path=lib_path)

        self._allocations = dict()
        self.allocation_counter = 0

    def _load_cpp_library(self, lib_path: str):
        err_win = None
        err_deb = None

        try: # Linux
            self._load_so(lib_path=lib_path)
            return
        except Exception as err:
            err_deb = err

        try: # Windows
            self._load_dll(lib_path=lib_path)
            return
        except Exception as err:
            err_win = err

        print(err_win)
        print(err_deb)
        
        # Could not load library
        raise Exception

    def _load_dll(self, lib_path: str):
        self.loaded_library = ctypes.windll.LoadLibrary(str(lib_path) + ".dll")

    def _load_so(self, lib_path: str):
        self.loaded_library = ctypes.CDLL(str(lib_path) + ".so")


    # ==================== Manage allocations =========================
    def register_allocation(self, allocation_tag: str) -> int:
        # Returns an integer, which corresponds to a reference to the allocation
        self.allocation_counter += 1
        self._allocations[self.allocation_counter] = allocation_tag

    def register_deallocation(self, allocation_id: int):
        del self._allocations[self.allocation_counter]

    def get_allocated_objects(self):
        return list(self._allocations)

    def cleanup(self):
        # Always call cleanup before closing the application
        if len(self._allocations) > 0:
            print("Some c++ objects have not been deallocated")
            iter = 0
            for allocation, tag in self._allocations.items():
                iter += 1
                if iter > 10:
                    break
                print(allocation, tag)

        else:
            print("Cleanup successfull! All c++ objects allocated by python have been deallocated")



class CppArgument:
    def __init__(self):
        self.argument_type: str = None
        self.argument_name: str = None
        self.allocation_id: int = None
        self._cpp_object: Any = None
        self._other_args: Any = None

    def get_value(self):
        raise NotImplementedError("Subclasses should implement get_value()")


class CppPtr(CppArgument):
    def __init__(self, type_: str = None):
        self.argument_type = type_
        self._cpp_object: ctypes.c_void_p = ctypes.c_void_p()

    def get_ptr_to_arg(arg: CppArgument):
        new_ptr = CppPtr()
        new_ptr.argument_type = arg.argument_type + "*"
        new_ptr._cpp_object = ctypes.byref(arg._cpp_object)
        return new_ptr


class CppCharPtr(CppArgument):
    def __init__(self, input_str: str = None):
        self.argument_type = "const char*"

        if input_str is not None:
            self._other_args = ctypes.create_string_buffer(input_str.encode('utf-8'))
            self._cpp_object = ctypes.cast(self._other_args, ctypes.c_char_p)
            self.need_destructor = False # Will be deallocated automatically when ctypes object is deallocated
        else:
            self._other_args = None
            self._cpp_object = None

    def get_value(self):
        return self._cpp_object.value.decode('utf-8') if self._cpp_object.value else None

class CppInt(CppArgument):
    def __init__(self, value: int = None):
        self.argument_type = "int"

        if value is not None:
            self._cpp_object = ctypes.c_int(value)
            self.need_destructor = False
        else:
            self._cpp_object = None
            self.need_destructor = False

    def get_value(self):
        return self._cpp_object.value

class CppFloat(CppArgument):
    def __init__(self, value: float = None):
        self.argument_type = "float"

        if value is not None:
            self._cpp_object = ctypes.c_float(value)
            self.need_destructor = False
        else:
            self._cpp_object = None
            self.need_destructor = False

    def get_value(self):
        return self._cpp_object.value if self._cpp_object else None

class CppDouble(CppArgument):
    def __init__(self, value: float = None):
        self.argument_type = "double"

        if value is not None:
            self._cpp_object = ctypes.c_double(value)
            self.need_destructor = False
        else:
            self._cpp_object = None
            self.need_destructor = False

    def get_value(self):
        return self._cpp_object.value if self._cpp_object else None

class CppBool(CppArgument):
    def __init__(self, value: bool = None):
        self.argument_type = "bool"

        if value is not None:
            self._cpp_object = ctypes.c_bool(value)
            self.need_destructor = False
        else:
            self._cpp_object = None
            self.need_destructor = False

    def get_value(self):
        return self._cpp_object.value if self._cpp_object else None
    

class CppIntArrayPtr(CppArgument):
    def __init__(self):
        self.length = 0
        self._cpp_object: ctypes.POINTER(ctypes.c_int) = ctypes.POINTER(ctypes.c_int)()
        self.argument_type = "int*" 


    def set(self, index: int, value: int):
        if index >= 0 and index < self.length:
            c_value: ctypes.c_int = ctypes.c_int(value)
            self._cpp_object[index] = c_value

    def get(self, index: int):
        if index >= 0 and index < self.length:
            value: int = int(self._cpp_object[index])
            return value

