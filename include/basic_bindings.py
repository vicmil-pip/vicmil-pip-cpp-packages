import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from typing import List

from vicmil_pip.packages.cppBindings.include.bindings_helper import *
from vicmil_pip.packages.cppBindings.src.bin.bindings_util_bindings import CppBindings

class CppIntVector(CppArgument):
    def __init__(self, bindings: CppBindings, input_vec: List[int] = None):
        self.bindings: CppBindings = bindings

        self.call_cpp_destructor = False

        if input_vec is not None:
             # Create the vector
            self._cpp_object: CppPtr = CppPtr("std::vector<int>*")
            ptr_ptr = CppPtr.get_ptr_to_arg(self._cpp_object)

            return_val: CppArgument = self.bindings.new_int_vector(ptr_ptr)
            self.allocation_id = return_val.allocation_id

            # Update the vector with the input vec
            self.from_list(input_vec)

            self.call_cpp_destructor = True

        else:
            self._cpp_object = None
            self.need_destructor = False

    def __len__(self) -> int:
        if self._cpp_object is None:
            raise Exception("Cannot get length of vector that has not been allocated!")
        
        length_arg = CppInt(value=0)
        length_ptr = CppPtr.get_ptr_to_arg(length_arg)
        self.bindings.get_int_vector_length(self._cpp_object, length_ptr, allocated_object=False)
        return length_arg.get_value()

    def set(self, key, value):
        if key < len(self):
            c_value: ctypes.c_int = ctypes.c_int(value)
            array_ptr: CppIntArrayPtr = self._get_array_ptr()
            array_ptr.set(key, c_value)
        return None

    def get(self, key):
        if key < len(self):
            array_ptr: CppIntArrayPtr = self._get_array_ptr()
            value: int = array_ptr.get(key)
            return value
        return None

    def back(self):
        if len(self) >= 1:
            return self.get(len(self)-1)
        return None

    def from_list(self, list_: list):
        self.resize(new_length=len(list_))
        array_ptr: CppIntArrayPtr = self._get_array_ptr()
        for i in range(0, len(list_)):
            array_ptr.set(i, list_[i])

    def to_list(self):
        new_list = list()
        array_ptr: CppIntArrayPtr = self._get_array_ptr()
        for i in range(0, len(self)):
            value: int = array_ptr.get(i)
            new_list.append(value)

    def resize(self, new_length):
        length = CppInt(new_length)
        self.bindings.resize_int_vector(self._cpp_object, length, allocated_object=False)

    def _get_array_ptr(self) -> CppIntArrayPtr:
        cpp_array_pointer = CppIntArrayPtr()
        array_ptr_ptr = CppPtr.get_ptr_to_arg(cpp_array_pointer)
        self.bindings.get_int_vector_array_ptr(self._cpp_object, array_ptr_ptr, allocated_object=False)

        return cpp_array_pointer
    
    def delete(self):
        if self.call_cpp_destructor and self._cpp_object is not None:
            self.bindings.delete_int_vector(self._cpp_object, allocated_object=False)
            self._cpp_object = None
            self.bindings.loaded_library.register_deallocation(self.allocation_id)