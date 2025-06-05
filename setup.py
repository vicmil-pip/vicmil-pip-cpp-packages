"""
Installs everything necessary for this package

On linux: install minimal clang compiler from google drive

On windows: install minimal mingw64 compiler from google drive
"""

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.pyUtil import *

import zipfile

def get_compiler_path():
    if platform.system() == "Windows":
        return get_directory_path(__file__) + "/compiler/mingw64/bin/g++.exe"
    if platform.system() == "Linux":
        return get_directory_path(__file__) + "/compiler/gcc-13.2.0/bin/g++"
    

def get_output_file_extension():
    if platform.system() == "Windows":
        return ".exe"
    if platform.system() == "Linux":
        return ".out"
    

def add_env_paths_to_compiler():
    if platform.system() == "Linux":
        compiler_path = get_directory_path(__file__, 0) + "/compiler"
        os.environ["PATH"] = f"{compiler_path}/gcc-13.2.0/bin:{compiler_path}/gcc-13.2.0/libexec/gcc/x86_64-pc-linux-gnu/13.2.0:" + os.environ["PATH"]
        os.environ["GCC_EXEC_PREFIX"] = f"{compiler_path}/gcc-13.2.0/lib/gcc/"
    

def install():
    # Download compiler from google drive
    # mingw64 for windows
    # gcc for linux

    drive_url = None
    if platform.system() == "Windows":
        tmp_file_path = get_directory_path(__file__) + "/drive_download_temp.zip"
        zip_url = "https://github.com/vicmil-pip/mingw64/archive/refs/heads/main.zip"
        download_github_repo_as_zip(
            zip_url=zip_url,
            output_zip_file=tmp_file_path
        )

    if platform.system() == "Linux":
        tmp_file_path = get_directory_path(__file__) + "/drive_download_temp.tar.gz"
        drive_url = "https://drive.google.com/file/d/10JEJIwXqK7K311d2p4st9YKujWFHgHT3/view?usp=drive_link"

        download_file_from_google_drive(
            drive_url=drive_url,
            output_file=tmp_file_path
        )

    if not os.path.exists(tmp_file_path):
        raise Exception("Download failed!")

    # Extract file contents by unzipping the downloaded file
    if platform.system() == "Windows":
        unzip_without_top_dir(
            zip_file_path=tmp_file_path,
            destination_folder=get_directory_path(__file__) + "/compiler",
            delete_zip=True
        )
    if platform.system() == "Linux":
        untar_file(
            tar_path=tmp_file_path,
            extract_to=get_directory_path(__file__) + "/compiler",
            delete_tar=False
        )


def delete_file(file: str):
    if os.path.exists(file):
        os.remove(file)


import tarfile

def untar_file(tar_path, extract_to, delete_tar=False):
    with tarfile.open(tar_path, 'r:gz') as tar:
        tar.extractall(path=extract_to)

    if delete_tar:
        delete_file(tar_path)


def unzip_file(zip_file_path: str, destination_folder: str, delete_zip=False):
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        zip_ref.extractall(destination_folder)

    if delete_zip:
        delete_file(zip_file_path)


if __name__ == "__main__":
    install()